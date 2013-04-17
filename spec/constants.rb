#!/usr/bin/env ruby

require 'pathname'

require 'openldap'


module OpenLDAP::TestConstants

	unless defined?( TEST_LDAP_URI )

		TEST_LDAP_STRING     = 'ldap://localhost:6363'
		TEST_LDAP_URI        = URI( TEST_LDAP_STRING )

		TEST_LDAPS_STRING    = 'ldaps://localhost:6364'
		TEST_LDAPS_URI       = URI( TEST_LDAPS_STRING )

		TEST_BASE            = 'dc=example,dc=com'

		TEST_LDAPBASE_STRING = "#{TEST_LDAP_STRING}/#{TEST_BASE}"
		TEST_LDAPBASE_URI    = URI( TEST_LDAPBASE_STRING )

		TEST_ADMIN_ROOT_DN   = "cn=admin,#{TEST_BASE}"
		TEST_ADMIN_PASSWORD  = 'secret'

		constants.each do |cname|
			const_get(cname).freeze
		end
	end

end

