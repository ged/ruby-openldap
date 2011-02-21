#!/usr/bin/env rspec -cfd -b

BEGIN {
	require 'pathname'
	basedir = Pathname( __FILE__ ).dirname.parent.parent
	libdir = basedir + 'lib'

	$LOAD_PATH.unshift( basedir.to_s ) unless $LOAD_PATH.include?( basedir.to_s )
	$LOAD_PATH.unshift( libdir.to_s ) unless $LOAD_PATH.include?( libdir.to_s )
}

require 'rspec'
require 'spec/lib/helpers'
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

