# -*- ruby -*-
#encoding: utf-8

require 'loggability'

# A Ruby binding to OpenLDAP's libldap
module OpenLDAP
	extend Loggability


	# Loggability API -- set up a logger for all openldap logs
	log_as :openldap


	# Library version constant
	VERSION = '0.0.1'

	# Version-control revision constant
	REVISION = %q$Revision$


	### Get the library version.
	def self::version_string( include_buildnum=false )
		vstring = "%s %s" % [ self.name, VERSION ]
		vstring << " (build %s)" % [ REVISION[/: ([[:xdigit:]]+)/, 1] || '0' ] if include_buildnum
		return vstring
	end



	### Create a new Connection object.
	###
	### :call-seq:
	###   OpenLDAP.connection                      -> connection
	###   OpenLDAP.connection( ldapurl )           -> connection
	###   OpenLDAP.connection( host, port=389 )    -> connection
	###
	### If no argument is given, the defaults are used (OpenLDAP.uris).
	def self::connection( *args )
		case args.first
		when NilClass
			self.log.info "Connecting using defaults: %p" % [ self.uris ]
			return OpenLDAP::Connection.new( self.uris.join(' ') )
		when URI, /^ldap(i|s)?:/i
			self.log.info "Connecting using an LDAP URL: %p" % [ args.first ]
			return OpenLDAP::Connection.new( args.join(' ') )
		else
			self.log.info "Connecting using host/port" % [ args ]
			uri = "ldap://%s:%d" % [ args.first, args[1] || 389 ]
			return OpenLDAP::Connection.new( uri )
		end
	end


	### Load the extension
	begin
		require 'openldap_ext'
	rescue LoadError => err
		# If it's a Windows binary gem, try the <major>.<minor> subdirectory
		if RUBY_PLATFORM =~/(mswin|mingw)/i
			major_minor = RUBY_VERSION[ /^(\d+\.\d+)/ ] or
				raise "Oops, can't extract the major/minor version from #{RUBY_VERSION.dump}"
			require "#{major_minor}/openldap_ext"
		else
			raise
		end

	end


	# Load the remaining Ruby parts of the library
	require 'openldap/exceptions'


	### Shortcut connection method: return a OpenLDAP::Connection object that will use
	### the specified +urls+ (or )
	def self::connect( *urls )
		return OpenLDAP::Connection.new( *urls )
	end

end # module OpenLDAP


