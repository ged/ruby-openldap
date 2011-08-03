#!/usr/bin/env ruby

require 'pathname'

require 'openldap'


module OpenLDAP::TestConstants

	unless defined?( TEST_LDAP_URI )

		TEST_LOCAL_LDAP_STRING = 'ldap://localhost'
		TEST_LOCAL_LDAP_URI = URI( TEST_LOCAL_LDAP_STRING )

		TEST_LDAP_STRING = 'ldap://ldap.example.com'
		TEST_LDAP_URI = URI( TEST_LDAP_STRING )

		constants.each do |cname|
			const_get(cname).freeze
		end
	end

end

