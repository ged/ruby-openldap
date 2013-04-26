#!/usr/bin/env rspec -cfd -b

require_relative '../helpers'

require 'rspec'
require 'openldap/message'

describe OpenLDAP::Message do

	before( :all ) do
		setup_logging( :fatal )
	end

	before( :each ) do
		@ldap = OpenLDAP.connect( 'ldap://localhost' )
		# @result = @ldap.search_ext( )
	end

	after( :all ) do
		reset_logging()
	end


	it "can be created with a connection and a message ID" do
		result = OpenLDAP::Result.new
	end

end

