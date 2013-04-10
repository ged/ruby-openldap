#!/usr/bin/env rspec -cfd -b

require_relative '../helpers'

require 'rspec'
require 'openldap/exceptions'

describe OpenLDAP, "exceptions" do

	before( :all ) do
		setup_logging( :fatal )
	end

	after( :all ) do
		reset_logging()
	end


	it "autogenerates exceptions for OpenLDAP error status codes" do
		eclass = OpenLDAP::RESULT_EXCEPTION_CLASS[ OpenLDAP::LDAP_OPERATIONS_ERROR ]

		eclass.should be_a( Class )
		eclass.name.should == "OpenLDAP::OperationsError"
		eclass.result_code.should == OpenLDAP::LDAP_OPERATIONS_ERROR
	end


	describe OpenLDAP::Error do

		it "knows how to look up the exception class for a result code" do
			OpenLDAP::Error.subclass_for( OpenLDAP::LDAP_INVALID_CREDENTIALS ).
				should == OpenLDAP::InvalidCredentials
			OpenLDAP::InvalidCredentials.result_code.should == OpenLDAP::LDAP_INVALID_CREDENTIALS
		end

	end
end

