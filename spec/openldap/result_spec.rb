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
require 'openldap/result'

describe OpenLDAP::Result do

	before( :all ) do
		setup_logging( :fatal )
	end

	before( :each ) do
		@ldap = OpenLDAP.connect( 'ldap://localhost' )
	end

	after( :all ) do
		reset_logging()
	end


	it "can be created with a connection and a message ID" do
		result = OpenLDAP::Result.new( )
	end

end

