#!/usr/bin/env ruby

# The namespace for OpenLDAP classes.
# 
# @author Michael Granger <ged@FaerieMUD.org>
# 
module OpenLDAP

	# Library version constant
	VERSION = '0.0.1'

	# Version-control revision constant
	REVISION = %q$Revision$


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

end # module OpenLDAP

