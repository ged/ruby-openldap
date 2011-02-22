#!/usr/bin/env rspec -cfd -b

BEGIN {
	require 'pathname'
	basedir = Pathname( __FILE__ ).dirname.parent.parent
	libdir = basedir + 'lib'

	$LOAD_PATH.unshift( basedir.to_s ) unless $LOAD_PATH.include?( basedir.to_s )
	$LOAD_PATH.unshift( libdir.to_s ) unless $LOAD_PATH.include?( libdir.to_s )
}

require 'rspec'
require 'spec/lib/helpers'
require 'openldap/connection'

describe OpenLDAP::Connection do

	before( :all ) do
		setup_logging( :debug )
	end

	after( :all ) do
		reset_logging()
	end


	TEST_LDAP_URI = 'ldap://localhost/dc=acme,dc=com'

	it "can be created with an LDAP uri" do
		OpenLDAP::Connection.new( TEST_LDAP_URI )
	end


end

