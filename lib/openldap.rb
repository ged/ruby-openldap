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

	require 'openldap/utils'

	### Logging
	# Log levels
	LOG_LEVELS = {
		'debug' => Logger::DEBUG,
		'info'  => Logger::INFO,
		'warn'  => Logger::WARN,
		'error' => Logger::ERROR,
		'fatal' => Logger::FATAL,
	}.freeze
	LOG_LEVEL_NAMES = LOG_LEVELS.invert.freeze

	@default_logger = Logger.new( $stderr )
	@default_logger.level = $DEBUG ? Logger::DEBUG : Logger::WARN

	@default_log_formatter = OpenLDAP::LogFormatter.new( @default_logger )
	@default_logger.formatter = @default_log_formatter

	@logger = @default_logger


	class << self
		# @return [Logger::Formatter] the log formatter that will be used when the logging 
		#    subsystem is reset
		attr_accessor :default_log_formatter

		# @return [Logger] the logger that will be used when the logging subsystem is reset
		attr_accessor :default_logger

		# @return [Logger] the logger that's currently in effect
		attr_accessor :logger
		alias_method :log, :logger
		alias_method :log=, :logger=
	end


	### Reset the global logger object to the default
	### @return [void]
	def self::reset_logger
		self.logger = self.default_logger
		self.logger.level = Logger::WARN
		self.logger.formatter = self.default_log_formatter
	end


	### Returns +true+ if the global logger has not been set to something other than
	### the default one.
	def self::using_default_logger?
		return self.logger == self.default_logger
	end


	### Get the library version.
	### @return [String] the library's version
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

	# Load the Ruby parts of the library
	require 'openldap/exceptions'

end # module OpenLDAP

