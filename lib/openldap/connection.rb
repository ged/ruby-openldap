#!/usr/bin/env ruby

require 'uri'
require 'loggability'
require 'openldap' unless defined?( OpenLDAP )

# OpenLDAP Connection class
class OpenLDAP::Connection
	extend Loggability


	# Loggability API -- log to the :openldap logger
	log_to :openldap


	# Default options for new OpenLDAP::Connections.
	DEFAULT_OPTIONS = {
		:protocol_version => 3,
	}

	# Default TLS options to set before STARTTLS
	DEFAULT_TLS_OPTIONS = {}

	# Mapping of names of TLS peer certificate-checking strategies into Fixnum values used by
	# the underlying library.
	TLS_REQUIRE_CERT_STRATEGIES = {
		:never  => OpenLDAP::LDAP_OPT_X_TLS_NEVER,
		:hard   => OpenLDAP::LDAP_OPT_X_TLS_HARD,
		:demand => OpenLDAP::LDAP_OPT_X_TLS_DEMAND,
		:allow  => OpenLDAP::LDAP_OPT_X_TLS_ALLOW,
		:try    => OpenLDAP::LDAP_OPT_X_TLS_TRY
	}

	# Inverse of TLS_REQUIRE_CERT_STRATEGIES
	TLS_REQUIRE_CERT_STRATEGY_NAMES = TLS_REQUIRE_CERT_STRATEGIES.invert

	# Mapping of names of TLS CRL evaluation strategies into Fixnum values used by
	# the underlying library.
	TLS_CRL_CHECK_STRATEGIES = {
		:none => OpenLDAP::LDAP_OPT_X_TLS_CRL_NONE,
		:peer => OpenLDAP::LDAP_OPT_X_TLS_CRL_PEER,
		:all  => OpenLDAP::LDAP_OPT_X_TLS_CRL_ALL
	}

	# Inverse of TLS_CRL_CHECK_STRATEGIES
	TLS_CRL_CHECK_STRATEGY_NAMES = TLS_CRL_CHECK_STRATEGIES.invert


	### Create a new OpenLDAP::Connection object that will attempt to connect to one of the
	### specified +urls+ in order.
	def initialize( *urls )
		options = if urls.last.is_a?( Hash ) then urls.pop else {} end
		options = DEFAULT_OPTIONS.merge( options )

		url_strings = urls.map( &self.method(:simplify_url) )
		self._initialize( url_strings.join(' ') )

		# Set options
		options.each do |opt, val|
			case opt
			when :timeout
				self.network_timeout = Float( val )
			else
				if self.respond_to?( "#{opt}=" )
					self.send( "#{opt}=", val )
				else
					self.log.info "Unknown option %p: ignoring" % [ opt ]
				end
			end
		end
	end


	######
	public
	######

	### Initiate TLS processing on the LDAP session. If called without a block, the call returns
	### when TLS handlers have been installed. If called with the block, the call runs asyncronously
	### and calls the block when TLS is installed. If there is an error, or TLS is already set up on
	### the connection, an appropriate OpenLDAP::Error is raised.
	###
	###    conn.start_tls( :tls_require_cert => :try )
	###
	def start_tls( options=DEFAULT_TLS_OPTIONS )
		options.each do |opt, val|
			if opt.to_s.index( 'tls_' ) != 0
				self.log.info "Skipping non-TLS option: %p" % [ opt ]
				next
			end

			self.send( "#{opt}=", val )
		end

		self._start_tls
	end


	### Get the current peer certificate-checking strategy (a Symbol). See #tls_require_cert=
	### for a list of the valid return values and what they mean.
	def tls_require_cert
		sym = TLS_REQUIRE_CERT_STRATEGY_NAMES[ self._tls_require_cert ] or
			raise IndexError, "unknown TLS certificate-checking strategy %p" %
				[self._tls_require_cert]
		return sym
	end


	### Set the current peer certificate-checking +strategy+ (a Symbol). One of:
	###
	### [:never]  This is the default. The library will not ask the peer for a certificate.
	### [:allow]  The peer certificate is requested. If no certificate is provided, the session
	###           proceeds normally. If a bad certificate is provided, it will be ignored and the
	###           session proceeds normally.
	### [:try]    The peer certificate is requested. If no certificate is provided, the session
	###           proceeds normally. If a bad certificate is provided, the session is immediately
	###           terminated.
	### [:demand] The peer certificate is requested. If no certificate is provided, or a bad
	###           certificate is provided, the session is immediately terminated.
    ###
	### Note that a valid client certificate is required in order to use the SASL EXTERNAL
	### authentication mechanism with a TLS session. As such, a non-default
	### setting must be chosen to enable SASL EXTERNAL authentication.
	def tls_require_cert=( strategy )
		self.log.debug "Setting require_cert to strategy: %p" % [ strategy ]
		numeric_opt = TLS_REQUIRE_CERT_STRATEGIES[ strategy ] or
			raise IndexError, "unknown TLS certificate-checking strategy %p" % [strategy]
		self._tls_require_cert=( numeric_opt )
	end


	### Get the current CRL check strategy (a Symbol). See #tls_crlcheck=
	### for a list of the valid return values and what they mean.
	def tls_crlcheck
		sym = TLS_CRL_CHECK_STRATEGY_NAMES[ self._tls_crlcheck ] or
			raise IndexError, "unknown TLS CRL evaluation strategy %p" % [self._tls_crlcheck]
		return sym
	end


	### Specify if the Certificate Revocation List (CRL) of the CA should be used to check
	### if the client certificates have been revoked or not. This option is ignored with GNUtls.
	### +strategy+ can be specified as one of the following:
	###
	### [:none]   No CRL checks are performed
	### [:peer]   Check the CRL of the peer certificate
	### [:all]    Check the CRL for a whole certificate chain
	###
	### If this is set to +:peer+ or +:all+, #tls_cacertdir also needs to be set.
	def tls_crlcheck=( strategy )
		numeric_opt = TLS_CRL_CHECK_STRATEGIES[ strategy ] or
			raise IndexError, "unknown TLS CRL evaluation strategy %p" % [strategy]
		self._tls_crlcheck=( numeric_opt )
	end


	### Fetch an IO object wrapped around the file descriptor the library is using to
	### communicate with the directory. Returns +nil+ if the connection hasn't yet
	### been established.
	def socket
		unless @socket
			fd = self.fdno or return nil
			@socket = IO.for_fd( fd, "rb:ascii-8bit" )
			@socket.autoclose = false
			@socket.close_on_exec = false
		end

		return @socket
	end


	### Return a String representation of the object suitable for debugging.
	def inspect
		return "#<%p:%#016x %s>"
	end


	#######
	private
	#######

	### Strip all but the schema, host, and port from the given +url+ and return it as a
	### String.
	def simplify_url( url )
		url = URI( url ) unless url.is_a?( URI )
		simpleurl = URI::Generic.build( :scheme => url.scheme, :host => url.host, :port => url.port )
		self.log.info "Simplified URL %s to: %s" % [ url, simpleurl ] if url.to_s != simpleurl.to_s

		return simpleurl.to_s
	end

end # class OpenLDAP::Connection

