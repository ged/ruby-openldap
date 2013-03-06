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

end # module OpenLDAP


