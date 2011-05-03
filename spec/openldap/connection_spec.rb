#!/usr/bin/env rspec -cfd -b

BEGIN {
	require 'pathname'
	basedir = Pathname( __FILE__ ).dirname.parent.parent
	libdir = basedir + 'lib'

	$LOAD_PATH.unshift( basedir.to_s ) unless $LOAD_PATH.include?( basedir.to_s )
	$LOAD_PATH.unshift( libdir.to_s ) unless $LOAD_PATH.include?( libdir.to_s )
}

require 'uri'
require 'rspec'
require 'spec/lib/helpers'
require 'openldap/connection'

describe OpenLDAP::Connection do

	before( :all ) do
		setup_logging( :fatal )
	end

	after( :all ) do
		reset_logging()
	end


	TEST_LDAP_URI = URI( 'ldap://localhost' )

	TEST_LDAP_URI2 = URI( 'ldap://ldap.example.com' )

	it "can be created with an LDAP URI" do
		conn = OpenLDAP::Connection.new( TEST_LDAP_URI )
		conn.uris.should == [ TEST_LDAP_URI ]
	end

	it "can be created with an LDAP URI String" do
		conn = OpenLDAP::Connection.new( TEST_LDAP_URI.to_s )
		conn.uris.should == [ TEST_LDAP_URI ]
	end

	it "can be created with several LDAP URIs" do
		conn = OpenLDAP::Connection.new( TEST_LDAP_URI, TEST_LDAP_URI2 )
		conn.uris.should == [ TEST_LDAP_URI, TEST_LDAP_URI2 ]
	end

	describe "a connection instance" do

		before( :each ) do
			@conn = OpenLDAP::Connection.new( TEST_LDAP_URI )
		end

	end

end

