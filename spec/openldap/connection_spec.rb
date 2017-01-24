#!/usr/bin/env rspec -cfd -b

require_relative '../helpers'

require 'pry'
require 'uri'
require 'rspec'
require 'openldap/connection'

describe OpenLDAP::Connection, slapd: true, log: :debug do

	it "can be created with an LDAP URI" do
		conn = OpenLDAP::Connection.new( TEST_LDAP_URI )
		expect( conn.uris ).to eq( [ TEST_LDAP_URI ] )
	end


	it "can be created with an LDAP URI String" do
		conn = OpenLDAP::Connection.new( TEST_LDAP_URI.to_s )
		expect( conn.uris ).to eq( [ TEST_LDAP_URI ] )
	end


	it "can be created with several LDAP URIs" do
		conn = OpenLDAP::Connection.new( TEST_LDAP_URI, TEST_LDAPS_URI )
		expect( conn.uris ).to eq( [ TEST_LDAP_URI, TEST_LDAPS_URI ] )
	end


	context "instance connected to #{TEST_LDAP_URI}" do

		before( :each ) do
			@conn = OpenLDAP::Connection.new( TEST_LDAP_URI )
		end


		it "includes its URL in its inspect output" do
			expect( @conn.inspect ).to include( TEST_LDAP_STRING )
		end


		it "can set the cacert file used for TLS" do
			@conn.tls_cacertfile = Pathname( '/etc/openssl/cacerts/ldap.pem' )
			expect( @conn.tls_cacertfile ).to eq( '/etc/openssl/cacerts/ldap.pem' )
		end


		it "can set the cacert directory used for TLS" do
			@conn.tls_cacertdir = Pathname( '/etc/openssl/cacerts' )
			expect( @conn.tls_cacertdir ).to eq( '/etc/openssl/cacerts' )
		end


		it "can set the certificate file used for TLS" do
			@conn.tls_certfile = Pathname( '/etc/openssl/host.pem' )
			expect( @conn.tls_certfile ).to eq( '/etc/openssl/host.pem' )
		end


		it "can set the cipher suite used for TLS" do
			@conn.tls_cipher_suite = 'HIGH:MEDIUM:+SSLv2'
			expect( @conn.tls_cipher_suite ).to eq( 'HIGH:MEDIUM:+SSLv2' )
		end


		describe "when using OpenSSL" do

			before( :each ) do
				skip if @conn.tls_package != 'OpenSSL'
			end


			it "can set the CRL checking strategy used for TLS" do
				@conn.tls_crlcheck = :all
				expect( @conn.tls_crlcheck ).to eq( :all )
			end

		end


		describe "when using GnuTLS" do

			before( :each ) do
				skip if @conn.tls_package != 'GnuTLS'
			end


			it "ignores settings for the CRL checking strategy" do
				@conn.tls_crlcheck = :all
				expect( @conn.tls_crlcheck ).to eq( :none )
			end

		end


		it "can set the CRL file used for TLS" do
			@conn.tls_crlfile = '/var/db/036D5FF52BA395A08BA2F780F9ADEAD905431757.crl'
			expect( @conn.tls_crlfile ).to eq( '/var/db/036D5FF52BA395A08BA2F780F9ADEAD905431757.crl' )
		end


		it "can set the dhfile used for TLS" do
			@conn.tls_dhfile = '/etc/openssl/dh1024.pem'
			expect( @conn.tls_dhfile ).to eq( '/etc/openssl/dh1024.pem' )
		end


		it "can set the keyfile used for TLS" do
			@conn.tls_keyfile = '/etc/openssl/host.key'
			expect( @conn.tls_keyfile ).to eq( '/etc/openssl/host.key' )
		end


		it "can set the package used for TLS in recent versions of libldap" do
			if OpenLDAP.api_info[:vendor_name] == "OpenLDAP" &&
				OpenLDAP.api_info[:vendor_version] >= 20426
				expect( @conn.tls_package ).to match( /openssl|gnutls|moznss/i )
			else
				expect {
					@conn.tls_package
				}.to raise_error( NotImplementedError, /version of libldap/i )
			end
		end


		it "can set the minimum protocol version used for TLS" do
			@conn.tls_protocol_min = 11
			expect( @conn.tls_protocol_min ).to eq( 11 )
		end


		it "can set the random_file used for TLS" do
			# :TODO: Don't know how to test this appropriately.
			# @conn.tls_random_file = '/dev/urandom'
			expect( @conn.tls_random_file ).to be_nil()
		end


		it "can set the certificate verification strategy used for TLS" do
			@conn.tls_require_cert = :demand
			expect( @conn.tls_require_cert ).to eq( :demand )
		end


		it "can be set to time out if the connect(2) takes longer than 2 seconds" do
			@conn.network_timeout = 2.0
			expect( @conn.network_timeout ).to eq( 2.0 )
		end


		it "can have its network timeout reset" do
			@conn.network_timeout = nil
			expect( @conn.network_timeout ).to be_nil()
		end


		it "can connect asynchronously" do
			expect( @conn.async_connect? ).to be_falsey()
			@conn.async_connect = true
			expect( @conn.async_connect? ).to be_truthy()
		end


		it "can disable asynchronous connections after they've been enabled" do
			@conn.async_connect = true
			@conn.async_connect = false
			expect( @conn.async_connect? ).to be_falsey()
		end


		it "know that their file-descriptors are nil" do
			expect( @conn.fdno ).to be_nil()
		end


		it "fail to start TLS with strict cert-checking enabled" do
			expect {
				@conn.tls_require_cert = :demand
				@conn.start_tls
			}.to raise_error( RuntimeError )
		end


		it "can set the TLS CA certificate file" do
			@conn.tls_cacertfile = 'test_workdir/example.crt'
			expect( @conn.tls_cacertfile ).to eq( 'test_workdir/example.crt' )
		end


		context "that are connected" do

			before( :each ) do
				@conn.tls_require_cert = :never
				@conn.start_tls
			end


			it "know what their file-descriptor number is" do
				expect( @conn.fdno ).to be_a( Fixnum )
				expect( @conn.fdno ).to be > 2 # Something past STDERR
			end


			it "can return an IO for selecting against their file-descriptor" do
				expect( @conn.socket ).to be_an( IO )
				expect( @conn.socket.fileno ).to eq( @conn.fdno )
				expect( @conn.socket ).to_not be_autoclose()
				expect( @conn.socket ).to_not be_close_on_exec()
				expect( @conn.socket ).to be_binmode()
			end


			it "can bind anonymously" do
				@conn.bind
			end


			it "can bind as a user" do
				expect( @conn.bind( TEST_ADMIN_ROOT_DN, TEST_ADMIN_PASSWORD ) ).to eq( true )
			end


			it "raises an appropriate exception if unable to bind" do
				expect {
					@conn.bind( 'cn=nonexistant', 'nopenopenope' )
				}.to raise_error( OpenLDAP::InvalidCredentials, /bind/i )
			end


			it "requires a base to search" do
				expect {
					@conn.search
				}.to raise_error( ArgumentError, /given 0, expected 1\.\.9/i )
			end


			it "returns all entries under the base without a filter" do
				result = @conn.search( TEST_BASE )
				expect( result.fetch.count ).to eq( 2 )
			end


			it "raises an appropriate exception on an invalid filter" do
				expect {
					@conn.search( TEST_BASE, :subtree, "(objectClass=*" );
				}.to raise_error( OpenLDAP::FilterError, /bad search filter/i )
			end


			it "can constrain return values to a subset of attributes when searching" do
				result = @conn.search( TEST_BASE, :subtree, '(objectClass=*)', [:cn, :dc] )
				expect( result.fetch.count ).to eq( 2 )
			end


			# it "handles a single attr argument when searching" do
			# 	result = @conn.search( TEST_BASE, :subtree, '(objectClass=*)', 'cn' )
			#   expect( result.count ).to eq( 3 )
			# end
			# 
			#
			# it "stringifies the single attr argument when searching" do
			# 	result = @conn.search( TEST_BASE, :subtree, '(objectClass=*)', :cn )
			# 	expect( result.count ).to eq( 3 )
			# end

		end

	end

end

