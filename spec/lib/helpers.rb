#!/usr/bin/ruby
# coding: utf-8

require 'yaml'
require 'fileutils'
require 'loggability/spechelpers'

require 'openldap'
require 'openldap/mixins'

require 'spec/lib/constants'


### RSpec helper functions.
module OpenLDAP::SpecHelpers
	include OpenLDAP::TestConstants

	###############
	module_function
	###############

	### Load the testing LDAP config options from a YAML file.
	def load_ldap_config
		unless defined?( @ldap_config ) && @ldap_config
			if TEST_CONFIG_FILE.exist?
				$stderr.puts "Loading LDAP config from #{TEST_CONFIG_FILE}" if $VERBOSE
				@ldap_config = YAML.load( TEST_CONFIG_FILE.read )
			else
				$stderr.puts "Skipping tests that require access to a live directory. Copy the ",
					"#{TEST_CONFIG_FILE}-example file and provide valid values for testing",
					"with an actual LDAP."
				@ldap_config = {}
			end
		end

		return @ldap_config
	end


end


### Mock with Rspec
RSpec.configure do |config|
	include OpenLDAP::TestConstants
	include Loggability::SpecHelpers

	config.mock_with :rspec
	config.include( OpenLDAP::SpecHelpers )

	config.filter_run_excluding( :ruby_1_9_only => true ) if RUBY_VERSION >= '1.9.0'
	config.filter_run_excluding( :ruby_2_0_only => true ) if RUBY_VERSION >= '2.0.0'
	config.filter_run_excluding( :with_ldap_server => true ) unless TEST_CONFIG_FILE.exist?
end

# vim: set nosta noet ts=4 sw=4:

