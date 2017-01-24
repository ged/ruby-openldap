#!/usr/bin/env rspec -cfd -b

require_relative '../helpers'

require 'rspec'
require 'openldap/exceptions'

describe OpenLDAP, "exceptions" do

	it "autogenerates exceptions for OpenLDAP error status codes" do
		eclass = OpenLDAP::RESULT_EXCEPTION_CLASS[ OpenLDAP::LDAP_OPERATIONS_ERROR ]

		expect( eclass ).to be_a( Class )
		expect( eclass.name ).to eq( "OpenLDAP::OperationsError" )
		expect( eclass.result_code ).to eq( OpenLDAP::LDAP_OPERATIONS_ERROR )
	end


	describe OpenLDAP::Error do

		it "knows how to look up the exception class for a result code" do
			expect(
				OpenLDAP::Error.subclass_for(OpenLDAP::LDAP_INVALID_CREDENTIALS)
			).to eq( OpenLDAP::InvalidCredentials )
			expect(
				OpenLDAP::InvalidCredentials.result_code
			).to eq( OpenLDAP::LDAP_INVALID_CREDENTIALS )
		end

	end
end

