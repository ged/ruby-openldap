#!/usr/bin/ruby
# coding: utf-8

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


	BASEDIR        = Pathname( __FILE__ ).dirname.parent.parent

	TESTING_SLAPD_URI = 'ldap://localhost:6363/dc=example,dc=com'
	TESTING_SLAPD_SSL_URI = 'ldaps://localhost:6364/dc=example,dc=com'

	TEST_WORKDIR   = BASEDIR + 'test_workdir'
	TEST_DATADIR   = TEST_WORKDIR + 'data'

	SPEC_DIR       = BASEDIR + 'spec'
	SPEC_DATADIR   = SPEC_DIR + 'data'
	SPEC_SLAPDCONF = SPEC_DATADIR + 'slapd.conf'
	SPEC_DBCONFIG  = SPEC_DATADIR + 'DB_CONFIG'
	SPEC_LDIF      = SPEC_DATADIR + 'testdata.ldif'


	### Start a localized slapd daemon for testing.
	def start_testing_slapd

		unless TEST_DATADIR.exist?
			self.create_test_directories
			self.copy_test_files
			self.generate_ssl_cert
			self.install_initial_data
		end

		return self.start_slapd
	end


	### Stop the slapd started by #start_testing_slapd, if it's still alive.
	def stop_testing_slapd( pid )
		if pid
			$stderr.puts "Shutting down slapd at PID %p" % [ pid ]
			begin
				Process.kill( :TERM, pid )
				Process.waitpid2( pid )
			rescue Errno::ESRCH
				$stderr.puts "  not running."
				# Not running
			rescue Errno::EPERM
				$stderr.puts "  not allowed (not slapd?)."
			else
				$stderr.puts "  killed."
			end
		end
	end


	### Create the directory used for the testing instance of slapd.
	def create_test_directories
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
		request_key  = TEST_WORKDIR + 'example.key.org'
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
				'-key', request_key.to_s,
				'-out', cert_request.to_s

			system 'openssl', 'rsa',
				'-in', request_key.to_s,
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

		$stderr.puts( ">>> ", Shellwords.join(cmd) )
		system( *cmd, chdir: TEST_WORKDIR.to_s ) or
			raise "Couldn't load initial data: #{Shellwords.join(cmd)}"
		$stderr.puts "installed."
	end


	### Start the testing slapd and keep track of its PID.
	def start_slapd
		$stderr.print "Starting up testing slapd..."
		slapd = self.find_binary( 'slapd' )
		logio = File.open( TEST_WORKDIR + 'slapd.log', 'w' )

		cmd = [
			slapd,
			'-f', 'slapd.conf',
			'-d', '64',
			'-h', "ldap://localhost:6363 ldaps://localhost:6364"
		]

		$stderr.puts( ">>> ", Shellwords.join(cmd) )
		pid = spawn( *cmd, chdir: TEST_WORKDIR.to_s, [:out,:err] => logio )

		$stderr.puts "started at PID %d" % [ pid ]
		return pid
	end


	### Attempt to find the path to the binary with the specified +name+, returning it if found,
	### or +nil+ if not.
	def find_binary( name )
		return ENV[name.upcase] if ENV.key?( name.upcase )

		dirs = ENV['PATH'].split( File::PATH_SEPARATOR )
		dirs += dirs \
			.find_all {|dir| dir.end_with?('bin') } \
			.map {|dir| dir[0..-4] + 'libexec' }

		paths = dirs.collect {|dir| File.join(dir, name) }
		found = paths.find {|path| File.executable?(path) } or
			raise "Unable to find %p in your PATH, or in corresponding 'libexec' directories." %
				[name]

		return found
	end


end


### Mock with Rspec
RSpec.configure do |config|
	include OpenLDAP::SpecHelpers
	include OpenLDAP::TestConstants
	include Loggability::SpecHelpers

	config.mock_with :rspec
	config.include( OpenLDAP::SpecHelpers )

	config.filter_run_excluding( :ruby_1_9_only => true ) if RUBY_VERSION >= '1.9.0'
	config.filter_run_excluding( :ruby_2_0_only => true ) if RUBY_VERSION >= '2.0.0'
  # config.filter_run_excluding( :with_ldap_server => true ) unless TEST_CONFIG_FILE.exist?
end

# vim: set nosta noet ts=4 sw=4:

