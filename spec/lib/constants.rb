#!/usr/bin/env ruby

require 'pathname'

require 'openldap'


module OpenLDAP::TestConstants

	unless defined?( TEST_LDAP_URI )

		TEST_LDAP_STRING = 'ldap://localhost:6363'
		TEST_LDAP_URI = URI( TEST_LDAP_STRING )

		TEST_LDAPS_STRING = 'ldaps://localhost:6363'
		TEST_LDAPS_URI = URI( TEST_LDAPS_STRING )

		TEST_LDAPBASE_STRING = "#{TEST_LDAP_STRING}/dc=example,dc=com"
		TEST_LDAPBASE_URI = URI( TEST_LDAPBASE_STRING )

		constants.each do |cname|
			const_get(cname).freeze
		end
	end

end

