# -*- ruby -*-
#encoding: utf-8

require 'uri'
require 'openldap' unless defined?( OpenLDAP )

# OpenLDAP Result class
class OpenLDAP::Result
	extend Loggability

	# Loggability API -- log to the openldap logger.
	log_to :openldap

end # class OpenLDAP::Result


