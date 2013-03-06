#!/usr/bin/env ruby

require 'openldap' unless defined?( OpenLDAP )


module OpenLDAP

	# A map of result codes to the corresponding exception class
	RESULT_EXCEPTION_CLASS = {}

	# The base class for all OpenLDAP exceptions
	#
	# The exception class hierarchy follows the error constants specified by the OpenLDAP
	# client library, and looks like this:
	#
	# * OpenLDAP::Error
	#   * Referral
	#   * OperationsError
	#   * ProtocolError
	#   * TimelimitExceeded
	#   * SizelimitExceeded
	#   * CompareFalse
	#   * CompareTrue
	#   * AuthMethodNotSupported
	#   * StrongAuthRequired
	#   * PartialResults
	#   * AdminlimitExceeded
	#   * UnavailableCriticalExtension
	#   * ConfidentialityRequired
	#   * SASLBindInProgress
	#   * AttrError
	#     * NoSuchAttribute
	#     * UndefinedType
	#     * InappropriateMatching
	#     * ConstraintViolation
	#     * TypeOrValueExists
	#     * InvalidSyntax
	#   * NameError
	#     * NoSuchObject
	#     * AliasProblem
	#     * InvalidDNSyntax
	#     * IsLeaf
	#     * AliasDerefProblem
	#   * SecurityError
	#     * XProxyAuthzFailure
	#     * InappropriateAuth
	#     * InvalidCredentials
	#     * InsufficientAccess
	#   * ServiceError
	#     * Busy
	#     * Unavailable
	#     * UnwillingToPerform
	#     * LoopDetect
	#   * UpdateError
	#     * NamingViolation
	#     * ObjectClassViolation
	#     * NotAllowedOnNonleaf
	#     * NotAllowedOnRdn
	#     * AlreadyExists
	#     * NoObjectClassMods
	#     * ResultsTooLarge
	#     * AffectsMultipleDSAs
	#     * VLVError
	#   * OtherError
	#   * APIError
	#     * ServerDown
	#     * LocalError
	#     * EncodingError
	#     * DecodingError
	#     * Timeout
	#     * AuthUnknown
	#     * FilterError
	#     * UserCancelled
	#     * ParamError
	#     * NoMemory
	#     * ConnectError
	#     * NotSupported
	#     * ControlNotFound
	#     * NoResultsReturned
	#     * MoreResultsToReturn
	#     * ClientLoop
	#     * ReferralLimitExceeded
	#     * XConnecting
	class Error < RuntimeError

		# The result code that corresponds to the exception type
		@result_code = nil
		class << self; attr_accessor :result_code; end

		### Inheritance hook -- Initialize the result code class instance variable
		### for inheriting exception classes.
		def self::inherited( subclass )
			subclass.instance_variable_set( :@result_code, nil )
		end

		### Return the appropriate Exception class for the given +resultcode+.
		### @param [Integer] resultcode  the result code from an ldap_* call.
		### @return [Class]
		def self::subclass_for( resultcode )
			return OpenLDAP::RESULT_EXCEPTION_CLASS[ resultcode ]
		end

	end # class Error


	### Define a new Exception class named +classname+ for the specified +result_code+
	### and inheriting from +superclass+.
	def self::def_ldap_exception( classname, result_code, superclass=OpenLDAP::Error )
		eclass = Class.new( superclass ) do
			def initialize( message=nil ) # :nodoc:
				ldapmsg = OpenLDAP.err2string( self.class.result_code )
				ldapmsg += ': ' + message if message
				super( ldapmsg )
			end
		end
		eclass.result_code = result_code

		const_set( classname, eclass )
		RESULT_EXCEPTION_CLASS[ result_code ] = eclass
	end


	# The LDAP referral class -- raised when the target LDAP directory instructs
	# the client to refer to another directory
	class Referral < OpenLDAP::Error

		### Create a new referral to the specified +url+.
		def initialize( url )
			super( "Referral to #{url}" )
			@url = url
		end

		######
		public
		######

		# The URL of the directory to refer to
		attr_reader :url

	end # class Referral


	def_ldap_exception :OperationsError, LDAP_OPERATIONS_ERROR
	def_ldap_exception :ProtocolError, LDAP_PROTOCOL_ERROR
	def_ldap_exception :TimelimitExceeded, LDAP_TIMELIMIT_EXCEEDED
	def_ldap_exception :SizelimitExceeded, LDAP_SIZELIMIT_EXCEEDED
	def_ldap_exception :CompareFalse, LDAP_COMPARE_FALSE
	def_ldap_exception :CompareTrue, LDAP_COMPARE_TRUE
	def_ldap_exception :AuthMethodNotSupported, LDAP_AUTH_METHOD_NOT_SUPPORTED
	def_ldap_exception :StrongAuthRequired, LDAP_STRONG_AUTH_REQUIRED
	def_ldap_exception :PartialResults, LDAP_PARTIAL_RESULTS
	def_ldap_exception :AdminlimitExceeded, LDAP_ADMINLIMIT_EXCEEDED
	def_ldap_exception :UnavailableCriticalExtension, LDAP_UNAVAILABLE_CRITICAL_EXTENSION
	def_ldap_exception :ConfidentialityRequired, LDAP_CONFIDENTIALITY_REQUIRED
	def_ldap_exception :SASLBindInProgress, LDAP_SASL_BIND_IN_PROGRESS

	#define LDAP_ATTR_ERROR(n)	LDAP_RANGE((n),0x10,0x15) /* 16-21 */
	class AttrError < OpenLDAP::Error # :nodoc:
	end

	def_ldap_exception :NoSuchAttribute, LDAP_NO_SUCH_ATTRIBUTE, OpenLDAP::AttrError
	def_ldap_exception :UndefinedType, LDAP_UNDEFINED_TYPE, OpenLDAP::AttrError
	def_ldap_exception :InappropriateMatching, LDAP_INAPPROPRIATE_MATCHING, OpenLDAP::AttrError
	def_ldap_exception :ConstraintViolation, LDAP_CONSTRAINT_VIOLATION, OpenLDAP::AttrError
	def_ldap_exception :TypeOrValueExists, LDAP_TYPE_OR_VALUE_EXISTS, OpenLDAP::AttrError
	def_ldap_exception :InvalidSyntax, LDAP_INVALID_SYNTAX, OpenLDAP::AttrError

	#define LDAP_NAME_ERROR(n)	LDAP_RANGE((n),0x20,0x24) /* 32-34,36 */
	class NameError < OpenLDAP::Error # :nodoc:
	end

	def_ldap_exception :NoSuchObject, LDAP_NO_SUCH_OBJECT, OpenLDAP::NameError
	def_ldap_exception :AliasProblem, LDAP_ALIAS_PROBLEM, OpenLDAP::NameError
	def_ldap_exception :InvalidDNSyntax, LDAP_INVALID_DN_SYNTAX, OpenLDAP::NameError
	def_ldap_exception :IsLeaf, LDAP_IS_LEAF, OpenLDAP::NameError
	def_ldap_exception :AliasDerefProblem, LDAP_ALIAS_DEREF_PROBLEM, OpenLDAP::NameError

	#define LDAP_SECURITY_ERROR(n)	LDAP_RANGE((n),0x2F,0x32) /* 47-50 */
	class SecurityError < OpenLDAP::Error # :nodoc:
	end

	def_ldap_exception :XProxyAuthzFailure, LDAP_X_PROXY_AUTHZ_FAILURE, OpenLDAP::SecurityError
	def_ldap_exception :InappropriateAuth, LDAP_INAPPROPRIATE_AUTH, OpenLDAP::SecurityError
	def_ldap_exception :InvalidCredentials, LDAP_INVALID_CREDENTIALS, OpenLDAP::SecurityError
	def_ldap_exception :InsufficientAccess, LDAP_INSUFFICIENT_ACCESS, OpenLDAP::SecurityError

	#define LDAP_SERVICE_ERROR(n)	LDAP_RANGE((n),0x33,0x36) /* 51-54 */
	class ServiceError < OpenLDAP::Error # :nodoc:
	end

	def_ldap_exception :Busy, LDAP_BUSY, OpenLDAP::ServiceError
	def_ldap_exception :Unavailable, LDAP_UNAVAILABLE, OpenLDAP::ServiceError
	def_ldap_exception :UnwillingToPerform, LDAP_UNWILLING_TO_PERFORM, OpenLDAP::ServiceError
	def_ldap_exception :LoopDetect, LDAP_LOOP_DETECT, OpenLDAP::ServiceError

	#define LDAP_UPDATE_ERROR(n)	LDAP_RANGE((n),0x40,0x47) /* 64-69,71 */
	class UpdateError < OpenLDAP::Error # :nodoc:
	end

	def_ldap_exception :NamingViolation, LDAP_NAMING_VIOLATION, OpenLDAP::UpdateError
	def_ldap_exception :ObjectClassViolation, LDAP_OBJECT_CLASS_VIOLATION, OpenLDAP::UpdateError
	def_ldap_exception :NotAllowedOnNonleaf, LDAP_NOT_ALLOWED_ON_NONLEAF, OpenLDAP::UpdateError
	def_ldap_exception :NotAllowedOnRdn, LDAP_NOT_ALLOWED_ON_RDN, OpenLDAP::UpdateError
	def_ldap_exception :AlreadyExists, LDAP_ALREADY_EXISTS, OpenLDAP::UpdateError
	def_ldap_exception :NoObjectClassMods, LDAP_NO_OBJECT_CLASS_MODS, OpenLDAP::UpdateError
	def_ldap_exception :ResultsTooLarge, LDAP_RESULTS_TOO_LARGE, OpenLDAP::UpdateError
	def_ldap_exception :AffectsMultipleDSAs, LDAP_AFFECTS_MULTIPLE_DSAS, OpenLDAP::UpdateError

	def_ldap_exception :VLVError, LDAP_VLV_ERROR if defined?( OpenLDAP::LDAP_VLV_ERROR )

	# Implementation-specific errors
	class OtherError < OpenLDAP::Error # :nodoc:
	end
	RESULT_EXCEPTION_CLASS.default = OpenLDAP::OtherError

	# API Error Codes
	#
	# Based on draft-ietf-ldap-c-api-xx
	# but with new negative code values
	#
	class APIError < OpenLDAP::Error # :nodoc:
	end

	def_ldap_exception :ServerDown, LDAP_SERVER_DOWN, OpenLDAP::APIError
	def_ldap_exception :LocalError, LDAP_LOCAL_ERROR, OpenLDAP::APIError
	def_ldap_exception :EncodingError, LDAP_ENCODING_ERROR, OpenLDAP::APIError
	def_ldap_exception :DecodingError, LDAP_DECODING_ERROR, OpenLDAP::APIError
	def_ldap_exception :Timeout, LDAP_TIMEOUT, OpenLDAP::APIError
	def_ldap_exception :AuthUnknown, LDAP_AUTH_UNKNOWN, OpenLDAP::APIError
	def_ldap_exception :FilterError, LDAP_FILTER_ERROR, OpenLDAP::APIError
	def_ldap_exception :UserCancelled, LDAP_USER_CANCELLED, OpenLDAP::APIError
	def_ldap_exception :ParamError, LDAP_PARAM_ERROR, OpenLDAP::APIError
	def_ldap_exception :NoMemory, LDAP_NO_MEMORY, OpenLDAP::APIError
	def_ldap_exception :ConnectError, LDAP_CONNECT_ERROR, OpenLDAP::APIError
	def_ldap_exception :NotSupported, LDAP_NOT_SUPPORTED, OpenLDAP::APIError
	def_ldap_exception :ControlNotFound, LDAP_CONTROL_NOT_FOUND, OpenLDAP::APIError
	def_ldap_exception :NoResultsReturned, LDAP_NO_RESULTS_RETURNED, OpenLDAP::APIError
	def_ldap_exception :MoreResultsToReturn, LDAP_MORE_RESULTS_TO_RETURN, OpenLDAP::APIError
	def_ldap_exception :ClientLoop, LDAP_CLIENT_LOOP, OpenLDAP::APIError
	def_ldap_exception :ReferralLimitExceeded, LDAP_REFERRAL_LIMIT_EXCEEDED, OpenLDAP::APIError
	def_ldap_exception :XConnecting, LDAP_X_CONNECTING, OpenLDAP::APIError


end # module OpenLDAP


