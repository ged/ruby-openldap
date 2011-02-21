#!/usr/bin/env ruby

require 'pathname'

require 'openldap'


module OpenLDAP::TestConstants

	unless defined?( TEST_LDAP_URI )

		TEST_LDAP_URI = 'ldap://ldap.acme.com/dc=acme,dc=com'

		constants.each do |cname|
			const_get(cname).freeze
		end
	end

end

