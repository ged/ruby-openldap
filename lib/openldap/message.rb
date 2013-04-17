# -*- ruby -*-
#encoding: utf-8

require 'uri'
require 'openldap' unless defined?( OpenLDAP )

# OpenLDAP Message class
class OpenLDAP::Message
	extend Loggability

	# Loggability API -- log to the openldap logger.
	log_to :openldap

end # class OpenLDAP::Message


