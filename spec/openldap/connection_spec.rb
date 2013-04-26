#!/usr/bin/env rspec -cfd -b

require_relative '../helpers'

require 'pry'
require 'uri'
require 'rspec'
require 'openldap/connection'

describe OpenLDAP::Connection do

	before( :all ) do
		setup_logging( :debug )
		@slapd_pid = start_testing_slapd()
	end

	after( :all ) do
		stop_testing_slapd( @slapd_pid )
		reset_logging()
	end


	it "can be created with an LDAP URI" do
		conn = OpenLDAP::Connection.new( TEST_LDAP_URI )
		conn.uris.should == [ TEST_LDAP_URI ]
	end

	it "can be created with an LDAP URI String" do
		conn = OpenLDAP::Connection.new( TEST_LDAP_URI.to_s )
		conn.uris.should == [ TEST_LDAP_URI ]
	end

	it "can be created with several LDAP URIs" do
		conn = OpenLDAP::Connection.new( TEST_LDAP_URI, TEST_LDAPS_URI )
		conn.uris.should == [ TEST_LDAP_URI, TEST_LDAPS_URI ]
	end

	context "instance connected to #{TEST_LDAP_URI}" do

		before( :each ) do
			@conn = OpenLDAP::Connection.new( TEST_LDAP_URI )
		end

		it "includes its URL in its inspect output" do
			@conn.inspect.should include( TEST_LDAP_STRING )
		end

		it "can set the cacert file used for TLS" do
			@conn.tls_cacertfile = Pathname( '/etc/openssl/cacerts/ldap.pem' )
			@conn.tls_cacertfile.should == '/etc/openssl/cacerts/ldap.pem'
		end

		it "can set the cacert directory used for TLS" do
			@conn.tls_cacertdir = Pathname( '/etc/openssl/cacerts' )
			@conn.tls_cacertdir.should == '/etc/openssl/cacerts'
		end

		it "can set the certificate file used for TLS" do
			@conn.tls_certfile = Pathname( '/etc/openssl/host.pem' )
			@conn.tls_certfile.should == '/etc/openssl/host.pem'
		end


		it "can set the cipher suite used for TLS" do
			@conn.tls_cipher_suite = 'HIGH:MEDIUM:+SSLv2'
			@conn.tls_cipher_suite.should == 'HIGH:MEDIUM:+SSLv2'
		end

		it "can set the CRL checking strategy used for TLS" do
			@conn.tls_crlcheck = :all
			@conn.tls_crlcheck.should == :all
		end

		it "can set the CRL file used for TLS" do
			@conn.tls_crlfile = '/var/db/036D5FF52BA395A08BA2F780F9ADEAD905431757.crl'
			@conn.tls_crlfile.should == '/var/db/036D5FF52BA395A08BA2F780F9ADEAD905431757.crl'
		end

		it "can set the dhfile used for TLS" do
			@conn.tls_dhfile = '/etc/openssl/dh1024.pem'
			@conn.tls_dhfile.should == '/etc/openssl/dh1024.pem'
		end

		it "can set the keyfile used for TLS" do
			@conn.tls_keyfile = '/etc/openssl/host.key'
			@conn.tls_keyfile.should == '/etc/openssl/host.key'
		end

		it "can set the package used for TLS in recent versions of libldap" do
			if OpenLDAP.api_info[:vendor_name] == "OpenLDAP" &&
				OpenLDAP.api_info[:vendor_version] >= 20426
				@conn.tls_package.should == 'OpenSSL'
			else
				expect {
					@conn.tls_package
				}.to raise_error( NotImplementedError, /version of libldap/i )
			end
		end

		it "can set the minimum protocol version used for TLS" do
			@conn.tls_protocol_min = 11
			@conn.tls_protocol_min.should == 11
		end

		it "can set the random_file used for TLS" do
			# :TODO: Don't know how to test this appropriately.
			# @conn.tls_random_file = '/dev/urandom'
			@conn.tls_random_file.should be_nil()
		end

		it "can set the certificate verification strategy used for TLS" do
			@conn.tls_require_cert = :demand
			@conn.tls_require_cert.should == :demand
		end

		it "can be set to time out if the connect(2) takes longer than 2 seconds" do
			@conn.network_timeout = 2.0
			@conn.network_timeout.should == 2.0
		end

		it "can have its network timeout reset" do
			@conn.network_timeout = nil
			@conn.network_timeout.should be_nil()
		end

		it "can connect asynchronously" do
			@conn.async_connect?.should be_false()
			@conn.async_connect = true
			@conn.async_connect?.should be_true()
		end

		it "can disable asynchronous connections after they've been enabled" do
			@conn.async_connect = true
			@conn.async_connect = false
			@conn.async_connect?.should be_false()
		end


		it "know that their file-descriptors are nil" do
			@conn.fdno.should be_nil()
		end


		it "fail to start TLS with strict cert-checking enabled" do
			pending "figuring out how to set TLS options that take effect" do
				expect {
					@conn.tls_require_cert = :demand
					@conn.start_tls
				}.to raise_error()
			end
		end


		it "can set the TLS CA certificate file" do
			@conn.tls_cacertfile = 'test_workdir/example.crt'
			@conn.tls_cacertfile.should == 'test_workdir/example.crt'
		end


		context "that are connected" do

			before( :each ) do
				@conn.tls_require_cert = :never
				@conn.start_tls
			end

			it "know what their file-descriptor number is" do
				@conn.fdno.should be_a( Fixnum )
				@conn.fdno.should > 2 # Something past STDERR
			end

			it "can return an IO for selecting against their file-descriptor" do
				@conn.socket.should be_an( IO )
				@conn.socket.fileno.should == @conn.fdno
				@conn.socket.should_not be_autoclose()
				@conn.socket.should_not be_close_on_exec()
				@conn.socket.should be_binmode()
			end

			it "can bind anonymously" do
				@conn.bind
			end

			it "can bind as a user" do
				@conn.bind( TEST_ADMIN_ROOT_DN, TEST_ADMIN_PASSWORD ).should == true
			end

			it "raises an appropriate exception if unable to bind" do
				expect {
					@conn.bind( 'cn=nonexistant', 'nopenopenope' )
				}.to raise_error( OpenLDAP::InvalidCredentials, /bind/i )
			end


			it "requires a base to search" do
				expect {
					@conn.search
				}.to raise_error( ArgumentError, /0 for 1/i )
			end

			it "returns all entries under the base without a filter" do
				result = @conn.search( TEST_BASE )
				result.next.count.should == 3
			end

			it "raises an appropriate exception on an invalid filter" do
				expect {
					@conn.search( TEST_BASE, :subtree, "(objectClass=*" );
				}.to raise_error( OpenLDAP::FilterError, /bad search filter/i )
			end

			it "can constrain return values to a subset of attributes when searching" do
				result = @conn.search( TEST_BASE, :subtree, '(objectClass=*)', [:cn, :dc] )
				result.next.count.should == 3
			end

			# it "handles a single attr argument when searching" do
			# 	result = @conn.search( TEST_BASE, :subtree, '(objectClass=*)', 'cn' )
			# 	result.count.should == 3
			# end
			# 
			# it "stringifies the single attr argument when searching" do
			# 	result = @conn.search( TEST_BASE, :subtree, '(objectClass=*)', :cn )
			# 	result.count.should == 3
			# end


		end


	end

end

