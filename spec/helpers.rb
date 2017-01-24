#!/usr/bin/ruby
# coding: utf-8

require 'rspec'

require 'erb'
require 'pathname'
require 'shellwords'
require 'yaml'
require 'fileutils'
require 'loggability/spechelpers'
require 'openssl'

require 'openldap'
require 'openldap/mixins'

require_relative 'constants'


### RSpec helper functions.
module OpenLDAP::SpecHelpers
	include OpenLDAP::TestConstants,
	        FileUtils

	include FileUtils::Verbose if $DEBUG || $VERBOSE


	BASEDIR        = Pathname( __FILE__ ).dirname.parent

	TESTING_SLAPD_URI = 'ldap://localhost:6363/dc=example,dc=com'
	TESTING_SLAPD_SSL_URI = 'ldaps://localhost:6364/dc=example,dc=com'

	TEST_WORKDIR   = BASEDIR + 'test_workdir'
	TEST_DATADIR   = TEST_WORKDIR + 'data'

	SPEC_DIR       = BASEDIR + 'spec'
	SPEC_DATADIR   = SPEC_DIR + 'data'
	SPEC_SLAPDCONF = SPEC_DATADIR + 'slapd.conf'
	SPEC_DBCONFIG  = SPEC_DATADIR + 'DB_CONFIG'
	SPEC_LDIF      = SPEC_DATADIR + 'testdata.ldif'


	### Inclusion callback -- install hooks in the specified +context+.
	def self::included( context )
		context.before( :all ) do
			OpenLDAP::SpecHelpers.trace( "Setting up testing slapd." )
			@slapd_pid ||= OpenLDAP::SpecHelpers.start_testing_slapd()
		end
		context.after( :all ) do
			if @slapd_pid
				OpenLDAP::SpecHelpers.trace( "Halting testing slapd at PID %d." % [@slapd_pid] )
				OpenLDAP::SpecHelpers.stop_testing_slapd( @slapd_pid )
			end
		end
	end


	###############
	module_function
	###############

	### Output the specified +msg+ if VERBOSE.
	def trace( *msg )
		return unless $VERBOSE
		$stderr.puts( *msg )
	end


	### Start a localized slapd daemon for testing.
	def start_testing_slapd
		unless TEST_DATADIR.exist?
			self.create_test_directories
			self.copy_test_files
			self.generate_ssl_cert
			self.install_initial_data
		else
			trace "Re-using existing test datadir #{TEST_DATADIR}"
		end

		return self.start_slapd
	end


	### Stop the slapd started by #start_testing_slapd, if it's still alive.
	def stop_testing_slapd( pid )
		if pid
			trace "Shutting down slapd at PID %p" % [ pid ]
			begin
				Process.kill( :TERM, pid )
				Process.waitpid2( pid )
			rescue Errno::ESRCH
				trace "  not running."
				# Not running
			rescue Errno::EPERM
				trace "  not allowed (not slapd?)."
			else
				trace "  killed."
			end
		end
	end


	### Create the directory used for the testing instance of slapd.
	def create_test_directories
		trace "Creating test directory #{TEST_DATADIR}"
		TEST_DATADIR.mkpath
		return TEST_WORKDIR
	end


	### Copy over any files necessary for testing to the testing directory.
	def copy_test_files
		install_with_filter( SPEC_SLAPDCONF, TEST_WORKDIR, binding() )
		install_with_filter( SPEC_DBCONFIG, TEST_DATADIR, binding )
	end


	### Copy the file from +source+ to +dest+ with ERB filtering using the given
	### +filter_binding+.
	def install_with_filter( source, dest, filter_binding )
		dest = dest + source.basename if dest.directory?
		contents = ERB.new( source.read(:encoding => 'UTF-8') )
		dest.open( 'w' ) do |io|
			io.print( contents.result(filter_binding) )
		end
	end


	### Generate a self-signed cert for testing SSL/TlS connections
	### Mostly stolen from https://gist.github.com/nickyp/886884
	def generate_ssl_cert
		cert_request = TEST_WORKDIR + 'example.csr'
		signing_key  = TEST_WORKDIR + 'example.key'
		cert         = TEST_WORKDIR + 'example.crt'

		unless File.exist?( cert )
			system 'openssl', 'req',
				'-new', '-newkey', 'rsa:4096',
				'-days', '365', '-nodes', '-x509',
				'-subj', '/C=US/ST=Oregon/L=Portland/O=IT/CN=localhost',
				'-keyout', signing_key.to_s,
				'-out', cert.to_s

			system 'openssl', 'req',
				'-new',
				'-subj', '/C=US/ST=Oregon/L=Portland/O=IT/CN=localhost',
				'-key', signing_key.to_s,
				'-out', cert_request.to_s

			system 'openssl', 'rsa',
				'-in', signing_key.to_s,
				'-out', signing_key.to_s

			system 'openssl', 'x509',
				'-req', '-days', '365',
				'-in', cert_request.to_s,
				'-signkey', signing_key.to_s,
				'-out', cert.to_s
		end
	end


	### Install the initial testing data into the data dir in +TEST_WORKDIR+.
	def install_initial_data
		$stderr.print "Installing testing directory data..."
		slapadd = self.find_binary( 'slapadd' )
		ldiffile = SPEC_LDIF.to_s
		configfile = TEST_WORKDIR + 'slapd.conf'

		cmd = [
			slapadd,
			'-f', configfile.to_s,
			'-l', ldiffile
		]

		trace ">>> ", Shellwords.join(cmd)
		system( *cmd, chdir: TEST_WORKDIR.to_s ) or
			raise "Couldn't load initial data: #{Shellwords.join(cmd)}"
		trace "installed."
	end


	### Start the testing slapd and keep track of its PID.
	def start_slapd
		trace "Starting up testing slapd..."
		slapd = self.find_binary( 'slapd' )
		logio = File.open( TEST_WORKDIR + 'slapd.log', 'w' )

		cmd = [
			slapd,
			'-f', 'slapd.conf',
			'-d', 'stats,args',
			'-h', "ldap://localhost:6363 ldaps://localhost:6364"
		]

		trace ">>> ", Shellwords.join( cmd )
		pid = spawn( *cmd, chdir: TEST_WORKDIR.to_s, [:out,:err] => logio )

		trace "started at PID %d" % [ pid ]
		return pid
	end


	### Attempt to find the path to the binary with the specified +name+, returning it if found,
	### or +nil+ if not.
	def find_binary( name )
		return ENV[ name.to_s.upcase ] if ENV.key?( name.to_s.upcase )

		dirs = ENV['PATH'].split( File::PATH_SEPARATOR )
		dirs += dirs.
			find_all {|dir| dir.end_with?('bin') }.
			map {|dir| dir[0..-4] + 'libexec' }

		paths = dirs.collect {|dir| File.join(dir, name) }
		found = paths.find {|path| File.executable?(path) } or
			raise "Unable to find %p in your PATH, or in corresponding 'libexec' directories." %
				[name]

		return found
	end

end


### Mock with Rspec
RSpec.configure do |config|
	include OpenLDAP::TestConstants

	config.run_all_when_everything_filtered = true
	config.filter_run :focus
	config.order = 'random'

	config.mock_with( :rspec ) do |mock|
		mock.syntax = :expect
	end

	config.include( OpenLDAP::SpecHelpers )
	config.include( Loggability::SpecHelpers )
end

# vim: set nosta noet ts=4 sw=4:

