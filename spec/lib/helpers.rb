#!/usr/bin/ruby
# coding: utf-8

require 'pathname'
require 'shellwords'
require 'yaml'
require 'fileutils'
require 'loggability/spechelpers'
require 'openssl'

require 'openldap'
require 'openldap/mixins'

require 'spec/lib/constants'


### RSpec helper functions.
module OpenLDAP::SpecHelpers
	include OpenLDAP::TestConstants,
	        FileUtils

	include FileUtils::Verbose if $DEBUG || $VERBOSE


	BASEDIR        = Pathname( __FILE__ ).dirname.parent.parent

	TESTING_SLAPD_URI = 'ldap://localhost:6363'
	TESTING_SLAPD_SSL_URI = 'ldaps://localhost:6364'

	TEST_WORKDIR   = BASEDIR + 'test_workdir'
	TEST_DATADIR   = TEST_WORKDIR + 'data'

	SPEC_DIR       = BASEDIR + 'spec'
	SPEC_DATADIR   = SPEC_DIR + 'data'
	SPEC_SLAPDCONF = SPEC_DATADIR + 'slapd.conf'
	SPEC_DBCONFIG  = SPEC_DATADIR + 'DB_CONFIG'
	SPEC_LDIF      = SPEC_DATADIR + 'testdata.ldif'


	### Start a localized slapd daemon for testing.
	def start_testing_slapd

		self.create_test_directories
		self.copy_test_files
		self.generate_ssl_cert
		self.install_initial_data
		slapd_pid = self.start_slapd

		return slapd_pid
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

		unless $DEBUG || ENV['MAINTAINER_MODE']
			$stderr.puts "Cleaning up #{TEST_WORKDIR}..."
			TEST_WORKDIR.rmtree
		end
	end


	### Create the directory used for the testing instance of slapd.
	def create_test_directories
		TEST_DATADIR.mkpath
		return TEST_WORKDIR
	end


	### Copy over any files necessary for testing to the testing directory.
	def copy_test_files
		install SPEC_SLAPDCONF, TEST_WORKDIR
		install SPEC_DBCONFIG, TEST_DATADIR
	end


	### Generate a self-signed cert for testing SSL/TlS connections
	### Mostly stolen from https://gist.github.com/nickyp/886884
	def generate_ssl_cert
		key = OpenSSL::PKey::RSA.new( 1024 )
		public_key = key.public_key

		subject = "/CN=*.example.com"

		cert = OpenSSL::X509::Certificate.new
		cert.subject = cert.issuer = OpenSSL::X509::Name.parse( subject )
		cert.not_before = Time.now
		cert.not_after = Time.now + 365 * 24 * 60 * 60
		cert.public_key = public_key
		cert.serial = 0x0
		cert.version = 2

		ef = OpenSSL::X509::ExtensionFactory.new
		ef.subject_certificate = cert
		ef.issuer_certificate = cert
		cert.extensions = [
			ef.create_extension( "basicConstraints", "CA:TRUE" , true ),
			ef.create_extension( "subjectKeyIdentifier", "hash" ),
			# ef.create_extension("keyUsage", "cRLSign,keyCertSign", true),
		]
		ext = ef.create_extension( "authorityKeyIdentifier", "keyid:always,issuer:always" )
		cert.add_extension( ext )

		cert.sign( key, OpenSSL::Digest::SHA1.new )

		pemfile = TEST_WORKDIR + 'example.pem'
		pemfile.open( 'w' ) {|io|
			io.puts(cert.to_pem)
			io.puts(key.to_pem)
		}
	end


	### Install the initial testing data into the data dir in +TEST_WORKDIR+.
	def install_initial_data
		slapadd = self.find_binary( 'slapadd' )
		ldiffile = SPEC_LDIF.to_s
		configfile = TEST_WORKDIR + 'slapd.conf'

		cmd = [
			slapadd,
			'-f', configfile.to_s,
			'-l', ldiffile
		]

		system( *cmd, chdir: TEST_WORKDIR.to_s ) or
		raise "Couldn't load initial data: #{Shellwords.join(cmd)}"
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

		puts( Shellwords.join(cmd) )
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

