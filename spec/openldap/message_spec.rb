#!/usr/bin/env rspec -cfd -b

require_relative '../helpers'

require 'rspec'
require 'openldap/message'

describe OpenLDAP::Message, :slapd do

	before( :each ) do
		@ldap = OpenLDAP.connect( 'ldap://localhost' )
		# @result = @ldap.search_ext( )
	end


	xit "can be created with a connection and a message ID" do
		result = OpenLDAP::Result.new( @ldap, @msgid )
	end

end

