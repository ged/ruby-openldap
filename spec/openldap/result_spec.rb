#!/usr/bin/env rspec -cfd -b

require_relative '../helpers'

require 'rspec'
require 'openldap/result'

describe OpenLDAP::Result do

	before( :each ) do
		@ldap = OpenLDAP.connect( 'ldap://localhost' )
	end


	it "can be created with a connection and a message ID" do
		result = OpenLDAP::Result.new( @ldap, 1 )
		expect( result ).to be_a( described_class )
	end

end

