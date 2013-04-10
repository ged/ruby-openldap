#!/usr/bin/env rspec -cfd -b

require_relative '../helpers'

require 'rspec'
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

