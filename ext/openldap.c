
/*
 * Ruby-OpenLDAP -- a Ruby binding to OpenLDAP's libldap
 * $Id$
 *
 * Authors
 *
 * - Michael Granger <ged@FaerieMUD.org>
 *
 * Copyright (c) 2011 Michael Granger
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice, this
 *    list of conditions and the following disclaimer in the documentation and/or
 *    other materials provided with the distribution.
 *
 *  * Neither the name of the authors, nor the names of its contributors may be used to
 *    endorse or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 */

#include "openldap.h"

VALUE ropenldap_mOpenLDAP;
VALUE ropenldap_mOpenLDAPLoggable;

VALUE ropenldap_eOpenLDAPError;

VALUE ropenldap_rbmURI;


/* --------------------------------------------------------------
 * Logging Functions
 * -------------------------------------------------------------- */

/*
 * Log a message to the given +context+ object's logger.
 */
void
#ifdef HAVE_STDARG_PROTOTYPES
ropenldap_log_obj( VALUE context, const char *level, const char *fmt, ... )
#else
ropenldap_log_obj( VALUE context, const char *level, const char *fmt, va_dcl )
#endif
{
	char buf[BUFSIZ];
	va_list	args;
	VALUE logger = Qnil;
	VALUE message = Qnil;

	va_start( args, fmt );
	vsnprintf( buf, BUFSIZ, fmt, args );
	message = rb_str_new2( buf );

	logger = rb_funcall( context, rb_intern("log"), 0, 0 );
	rb_funcall( logger, rb_intern(level), 1, message );

	va_end( args );
}


/*
 * Log a message to the global logger.
 */
void
#ifdef HAVE_STDARG_PROTOTYPES
ropenldap_log( const char *level, const char *fmt, ... )
#else
ropenldap_log( const char *level, const char *fmt, va_dcl )
#endif
{
	char buf[BUFSIZ];
	va_list	args;
	VALUE logger = Qnil;
	VALUE message = Qnil;

	va_init_list( args, fmt );
	vsnprintf( buf, BUFSIZ, fmt, args );
	message = rb_str_new2( buf );

	logger = rb_funcall( ropenldap_mOpenLDAP, rb_intern("logger"), 0, 0 );
	rb_funcall( logger, rb_intern(level), 1, message );

	va_end( args );
}


/*
 * Raise an appropriate exception with an appropriate message for the given
 * resultcode.
 */
void
#ifdef HAVE_STDARG_PROTOTYPES
ropenldap_check_result( int resultcode, const char *func, ... )
#else
ropenldap_check_result( int resultcode, const char *func, va_dcl )
#endif
{
	char buf[BUFSIZ];
	va_list args;
	VALUE exception_class = Qnil;

	if ( resultcode == LDAP_SUCCESS ) return;

	va_init_list( args, func );
	vsnprintf( buf, BUFSIZ, func, args );

	exception_class =
		rb_funcall( ropenldap_eOpenLDAPError, rb_intern("subclass_for"), 1, INT2FIX(resultcode) );

	rb_raise( exception_class, "%s", buf );
}


/*
 * Raise an appropriate exception for the given option +resultcode+ if one is
 * warranted.
 */
void
ropenldap_check_opt_result( int optresult, const char *opt )
{
	VALUE exception_class = Qnil;

	if ( optresult == LDAP_OPT_SUCCESS ) return;
	if ( optresult == LDAP_OPT_ERROR )
		rb_raise( rb_eRuntimeError, "Failed to get/set %s option!", opt );

	exception_class =
		rb_funcall( ropenldap_eOpenLDAPError, rb_intern("subclass_for"), 1, INT2FIX(optresult) );
	rb_raise( exception_class, "while getting/setting %s", opt );
}


/*
 * Convert an array of string pointers to a Ruby Array of Strings.
 */
VALUE
ropenldap_rb_string_array( char **strings )
{
	VALUE ary = rb_ary_new();
	char **iter;

	/* If there aren't any pointers, just return the empty Array */
	if ( !strings ) return ary;

	for ( iter = strings ; *iter != NULL ; iter++ ) {
		ropenldap_log( "debug", "  adding %s to string array", *iter );
		rb_ary_push( ary, rb_str_new2(*iter) );
	}

	return ary;
}



/*
 * call-seq:
 *    OpenLDAP.split_url( str )   -> array
 *
 * Split an LDAP URL into an array of its parts:
 * - uri_scheme
 * - host
 * - port
 * - base
 * - attrs
 * - scope
 * - filter
 * - exts
 * - crit_exts
 */
static VALUE
ropenldap_s_split_url( VALUE UNUSED(module), VALUE urlstring )
{
	const char *url = StringValueCStr( urlstring );
	LDAPURLDesc *urldesc;
	VALUE rval = Qnil, obj = Qnil;

	if ( !ldap_is_ldap_url(url) )
		rb_raise( rb_eArgError, "Not an LDAP URL." );

	/* Parse the URL */
	if ( ldap_url_parse(url, &urldesc) != 0 )
		rb_raise( rb_eRuntimeError, "Error parsing %s as an LDAP URL!", url );

	rval = rb_ary_new2( 9 );

	/* Scheme */
	if ( urldesc->lud_scheme ) {
		ropenldap_log( "debug", "  parsed scheme: %s", urldesc->lud_scheme );
		obj = rb_str_new2( urldesc->lud_scheme );
		OBJ_INFECT( obj, urlstring );
		rb_ary_store( rval, 0L, obj );
	}

	/* LDAP host to contact */
	if ( urldesc->lud_host ) {
		ropenldap_log( "debug", "  parsed host: %s", urldesc->lud_host );
		obj = rb_str_new2( urldesc->lud_host );
		OBJ_INFECT( obj, urlstring );
		rb_ary_store( rval, 1L, obj );
	}

	/* Port */
	rb_ary_store( rval, 2L, INT2FIX(urldesc->lud_port) );

	/* Base DN */
	if ( urldesc->lud_dn ) {
		ropenldap_log( "debug", "  parsed DN: %s", urldesc->lud_dn );
		obj = rb_str_new2( urldesc->lud_dn );
		OBJ_INFECT( obj, urlstring );
		rb_ary_store( rval, 3L, obj );
	}

	/* Attributes */
	rb_ary_store( rval, 4L, ropenldap_rb_string_array(urldesc->lud_attrs) );

	/* Numeric scope (LDAP_SCOPE_*) */
	rb_ary_store( rval, 5L, INT2FIX(urldesc->lud_scope) );

	/* Filter */
	if ( urldesc->lud_filter ) {
		ropenldap_log( "debug", "  parsed filter: %s", urldesc->lud_filter );
		obj = rb_str_new2( urldesc->lud_filter );
		OBJ_INFECT( obj, urlstring );
		rb_ary_store( rval, 6L, obj );
	}

	/* lists of LDAP extensions */
	rb_ary_store( rval, 7L, ropenldap_rb_string_array(urldesc->lud_exts) );

	/* Critical extension/s flag */
	rb_ary_store( rval, 8L, urldesc->lud_crit_exts ? Qtrue : Qfalse );

	ldap_free_urldesc( urldesc );

	return rval;
}


/*
 * call-seq:
 *    OpenLDAP.err2string( resultcode )   -> string
 *
 * Return a short description of the +resultcode+ returned by routines in this library.
 *
 */
static VALUE
ropenldap_s_err2string( VALUE UNUSED(module), VALUE resultcode )
{
	int err = FIX2INT( resultcode );
	char *string = ldap_err2string( err );

	return rb_str_new2( string );
}


/* Check to be sure the library that's dynamically-linked is the same
 * one it was compiled against. */
static void
ropenldap_check_link()
{
	LDAPAPIInfo api;
	api.ldapai_info_version = LDAP_API_INFO_VERSION;

	if ( ldap_get_option(NULL, LDAP_OPT_API_INFO, &api) != LDAP_OPT_SUCCESS ) {
		rb_warn( "ldap_get_option(API_INFO) failed" );
		return;
	}

	if ( api.ldapai_info_version != LDAP_API_INFO_VERSION ) {
		rb_warn( "LDAP APIInfo version mismatch: library %d, header %d",
		         api.ldapai_info_version, LDAP_API_INFO_VERSION );
	}

	if ( api.ldapai_api_version != LDAP_API_VERSION ) {
		rb_warn( "LDAP API version mismatch: library %d, header %d",
		         api.ldapai_api_version, LDAP_API_VERSION );
	}

	if ( strcmp(api.ldapai_vendor_name, LDAP_VENDOR_NAME ) != 0 ) {
		rb_warn( "LDAP vendor name mismatch: library %s, header %s\n",
		         api.ldapai_vendor_name, LDAP_VENDOR_NAME );
	}

	if( api.ldapai_vendor_version != LDAP_VENDOR_VERSION ) {
		rb_warn( "LDAP vendor version mismatch: library %d, header %d\n",
		         api.ldapai_vendor_version, LDAP_VENDOR_VERSION );
	}

	ropenldap_log( "info", "LDAP library: %s %d",
		           LDAP_VENDOR_NAME, LDAP_VENDOR_VERSION );

	ldap_memfree( api.ldapai_vendor_name );
	ber_memvfree( (void **)api.ldapai_extensions );
}


/*
 * call-seq:
 *    OpenLDAP.api_info   -> hash
 *
 * Return a Hash describing the API version, vendor, extensions, etc.
 *
 *    conn.api_info
 *    # => {:api_version=>3001, :protocol_version=>3, :extensions=>["X_OPENLDAP"], 
 *          :vendor_name=>"OpenLDAP", :vendor_version=>20424} (using ==)
 */
static VALUE
ropenldap_api_info( VALUE self )
{
	VALUE rval = rb_hash_new();
	LDAPAPIInfo info;
	info.ldapai_info_version = LDAP_API_INFO_VERSION;

	if ( ldap_get_option(NULL, LDAP_OPT_API_INFO, &info) != LDAP_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "ldap_get_option(API_INFO) failed." );

	rb_hash_aset( rval, ID2SYM(rb_intern("api_version")), INT2FIX(info.ldapai_api_version) );
	rb_hash_aset( rval, ID2SYM(rb_intern("protocol_version")), INT2FIX(info.ldapai_protocol_version) );
	rb_hash_aset( rval, ID2SYM(rb_intern("extensions")), ropenldap_rb_string_array(info.ldapai_extensions) );
	rb_hash_aset( rval, ID2SYM(rb_intern("vendor_name")), rb_str_new2(info.ldapai_vendor_name) );
	rb_hash_aset( rval, ID2SYM(rb_intern("vendor_version")), INT2FIX(info.ldapai_vendor_version) );

	ldap_memfree( info.ldapai_vendor_name );
	ber_memvfree( (void **)info.ldapai_extensions );

	return rval;
}


/*
 * call-seq:
 *    OpenLDAP.api_feature_info   -> hash
 *
 * Returns a hash of the versions of the extensions in .api_info[:extensions]
 *
 *    conn.api_feature_info
 *    # => 
 */
static VALUE
ropenldap_api_feature_info( VALUE self )
{
	VALUE rval = rb_hash_new();
	int i;
	LDAPAPIInfo info;
	info.ldapai_info_version = LDAP_API_INFO_VERSION;

	if ( ldap_get_option(NULL, LDAP_OPT_API_INFO, &info) != LDAP_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "ldap_get_option(API_INFO) failed." );

	for( i=0; info.ldapai_extensions[i] != NULL; i++ ) {
		LDAPAPIFeatureInfo fi;
		fi.ldapaif_info_version = LDAP_FEATURE_INFO_VERSION;
		fi.ldapaif_name = info.ldapai_extensions[i];
		fi.ldapaif_version = 0;

		if ( ldap_get_option(NULL, LDAP_OPT_API_FEATURE_INFO, &fi) == LDAP_SUCCESS ) {
			if(fi.ldapaif_info_version == LDAP_FEATURE_INFO_VERSION) {
				rb_hash_aset( rval, rb_str_new2(fi.ldapaif_name), INT2FIX(fi.ldapaif_version) );
			} else {
				ropenldap_log( "warn", "Feature info version mismatch for %s; expected %d, got %d",
				               fi.ldapaif_name, LDAP_FEATURE_INFO_VERSION, fi.ldapaif_info_version );
				rb_hash_aset( rval, rb_str_new2(fi.ldapaif_name), Qnil );
			}
		} else {
			ldap_memfree( info.ldapai_vendor_name );
			ber_memvfree( (void **)info.ldapai_extensions );
			rb_raise( ropenldap_eOpenLDAPError, "ldap_get_option(API_FEATURE_INFO) failed." );
		}
	}

	ldap_memfree( info.ldapai_vendor_name );
	ber_memvfree( (void **)info.ldapai_extensions );

	return rval;
}





void
Init_openldap_ext( void )
{
	rb_require( "uri" );
	ropenldap_rbmURI = rb_const_get( rb_cObject, rb_intern("URI") );

	rb_require( "openldap" );
	ropenldap_mOpenLDAP = rb_define_module( "OpenLDAP" );

	rb_require( "openldap/mixins" );
	ropenldap_mOpenLDAPLoggable = rb_define_module_under( ropenldap_mOpenLDAP, "Loggable" );
	ropenldap_eOpenLDAPError = rb_define_class_under( ropenldap_mOpenLDAP, "Error", rb_eRuntimeError );

	/* Constants */

	/* versions */
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_API_VERSION", INT2FIX(LDAP_API_VERSION) );

	/* search scopes */
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_SCOPE_BASE", INT2FIX(LDAP_SCOPE_BASE) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_SCOPE_BASEOBJECT", INT2FIX(LDAP_SCOPE_BASEOBJECT) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_SCOPE_ONELEVEL", INT2FIX(LDAP_SCOPE_ONELEVEL) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_SCOPE_ONE", INT2FIX(LDAP_SCOPE_ONE) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_SCOPE_SUBTREE", INT2FIX(LDAP_SCOPE_SUBTREE) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_SCOPE_SUB", INT2FIX(LDAP_SCOPE_SUB) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_SCOPE_SUBORDINATE", INT2FIX(LDAP_SCOPE_SUBORDINATE) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_SCOPE_CHILDREN", INT2FIX(LDAP_SCOPE_CHILDREN) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_SCOPE_DEFAULT", INT2FIX(LDAP_SCOPE_DEFAULT) );

	/* result codes */
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_SUCCESS", INT2FIX(LDAP_SUCCESS) );

 	rb_define_const( ropenldap_mOpenLDAP, "LDAP_OPERATIONS_ERROR", INT2FIX(LDAP_OPERATIONS_ERROR) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_PROTOCOL_ERROR", INT2FIX(LDAP_PROTOCOL_ERROR) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_TIMELIMIT_EXCEEDED", INT2FIX(LDAP_TIMELIMIT_EXCEEDED) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_SIZELIMIT_EXCEEDED", INT2FIX(LDAP_SIZELIMIT_EXCEEDED) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_COMPARE_FALSE", INT2FIX(LDAP_COMPARE_FALSE) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_COMPARE_TRUE", INT2FIX(LDAP_COMPARE_TRUE) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_AUTH_METHOD_NOT_SUPPORTED", INT2FIX(LDAP_AUTH_METHOD_NOT_SUPPORTED) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_STRONG_AUTH_NOT_SUPPORTED", INT2FIX(LDAP_STRONG_AUTH_NOT_SUPPORTED) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_STRONG_AUTH_REQUIRED", INT2FIX(LDAP_STRONG_AUTH_REQUIRED) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_STRONGER_AUTH_REQUIRED", INT2FIX(LDAP_STRONGER_AUTH_REQUIRED) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_PARTIAL_RESULTS", INT2FIX(LDAP_PARTIAL_RESULTS) );

	rb_define_const( ropenldap_mOpenLDAP, "LDAP_REFERRAL", INT2FIX(LDAP_REFERRAL) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_ADMINLIMIT_EXCEEDED", INT2FIX(LDAP_ADMINLIMIT_EXCEEDED) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_UNAVAILABLE_CRITICAL_EXTENSION", INT2FIX(LDAP_UNAVAILABLE_CRITICAL_EXTENSION) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_CONFIDENTIALITY_REQUIRED", INT2FIX(LDAP_CONFIDENTIALITY_REQUIRED) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_SASL_BIND_IN_PROGRESS", INT2FIX(LDAP_SASL_BIND_IN_PROGRESS) );

	rb_define_const( ropenldap_mOpenLDAP, "LDAP_NO_SUCH_ATTRIBUTE", INT2FIX(LDAP_NO_SUCH_ATTRIBUTE) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_UNDEFINED_TYPE", INT2FIX(LDAP_UNDEFINED_TYPE) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_INAPPROPRIATE_MATCHING", INT2FIX(LDAP_INAPPROPRIATE_MATCHING) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_CONSTRAINT_VIOLATION", INT2FIX(LDAP_CONSTRAINT_VIOLATION) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_TYPE_OR_VALUE_EXISTS", INT2FIX(LDAP_TYPE_OR_VALUE_EXISTS) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_INVALID_SYNTAX", INT2FIX(LDAP_INVALID_SYNTAX) );

	rb_define_const( ropenldap_mOpenLDAP, "LDAP_NO_SUCH_OBJECT", INT2FIX(LDAP_NO_SUCH_OBJECT) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_ALIAS_PROBLEM", INT2FIX(LDAP_ALIAS_PROBLEM) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_INVALID_DN_SYNTAX", INT2FIX(LDAP_INVALID_DN_SYNTAX) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_IS_LEAF", INT2FIX(LDAP_IS_LEAF) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_ALIAS_DEREF_PROBLEM", INT2FIX(LDAP_ALIAS_DEREF_PROBLEM) );

	rb_define_const( ropenldap_mOpenLDAP, "LDAP_X_PROXY_AUTHZ_FAILURE", INT2FIX(LDAP_X_PROXY_AUTHZ_FAILURE) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_INAPPROPRIATE_AUTH", INT2FIX(LDAP_INAPPROPRIATE_AUTH) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_INVALID_CREDENTIALS", INT2FIX(LDAP_INVALID_CREDENTIALS) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_INSUFFICIENT_ACCESS", INT2FIX(LDAP_INSUFFICIENT_ACCESS) );

	rb_define_const( ropenldap_mOpenLDAP, "LDAP_BUSY", INT2FIX(LDAP_BUSY) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_UNAVAILABLE", INT2FIX(LDAP_UNAVAILABLE) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_UNWILLING_TO_PERFORM", INT2FIX(LDAP_UNWILLING_TO_PERFORM) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_LOOP_DETECT", INT2FIX(LDAP_LOOP_DETECT) );

	rb_define_const( ropenldap_mOpenLDAP, "LDAP_NAMING_VIOLATION", INT2FIX(LDAP_NAMING_VIOLATION) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_OBJECT_CLASS_VIOLATION", INT2FIX(LDAP_OBJECT_CLASS_VIOLATION) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_NOT_ALLOWED_ON_NONLEAF", INT2FIX(LDAP_NOT_ALLOWED_ON_NONLEAF) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_NOT_ALLOWED_ON_RDN", INT2FIX(LDAP_NOT_ALLOWED_ON_RDN) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_ALREADY_EXISTS", INT2FIX(LDAP_ALREADY_EXISTS) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_NO_OBJECT_CLASS_MODS", INT2FIX(LDAP_NO_OBJECT_CLASS_MODS) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_RESULTS_TOO_LARGE", INT2FIX(LDAP_RESULTS_TOO_LARGE) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_AFFECTS_MULTIPLE_DSAS", INT2FIX(LDAP_AFFECTS_MULTIPLE_DSAS) );

#ifdef LDAP_VLV_ERROR
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_VLV_ERROR", INT2FIX(LDAP_VLV_ERROR) );
#endif

	rb_define_const( ropenldap_mOpenLDAP, "LDAP_OPT_SUCCESS", INT2FIX(LDAP_OPT_SUCCESS) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_OPT_ERROR", INT2FIX(LDAP_OPT_ERROR) );

	rb_define_const( ropenldap_mOpenLDAP, "LDAP_OTHER", INT2FIX(LDAP_OTHER) );

	rb_define_const( ropenldap_mOpenLDAP, "LDAP_SERVER_DOWN", INT2FIX(LDAP_SERVER_DOWN) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_LOCAL_ERROR", INT2FIX(LDAP_LOCAL_ERROR) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_ENCODING_ERROR", INT2FIX(LDAP_ENCODING_ERROR) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_DECODING_ERROR", INT2FIX(LDAP_DECODING_ERROR) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_TIMEOUT", INT2FIX(LDAP_TIMEOUT) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_AUTH_UNKNOWN", INT2FIX(LDAP_AUTH_UNKNOWN) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_FILTER_ERROR", INT2FIX(LDAP_FILTER_ERROR) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_USER_CANCELLED", INT2FIX(LDAP_USER_CANCELLED) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_PARAM_ERROR", INT2FIX(LDAP_PARAM_ERROR) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_NO_MEMORY", INT2FIX(LDAP_NO_MEMORY) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_CONNECT_ERROR", INT2FIX(LDAP_CONNECT_ERROR) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_NOT_SUPPORTED", INT2FIX(LDAP_NOT_SUPPORTED) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_CONTROL_NOT_FOUND", INT2FIX(LDAP_CONTROL_NOT_FOUND) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_NO_RESULTS_RETURNED", INT2FIX(LDAP_NO_RESULTS_RETURNED) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_MORE_RESULTS_TO_RETURN", INT2FIX(LDAP_MORE_RESULTS_TO_RETURN) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_CLIENT_LOOP", INT2FIX(LDAP_CLIENT_LOOP) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_REFERRAL_LIMIT_EXCEEDED", INT2FIX(LDAP_REFERRAL_LIMIT_EXCEEDED) );
	rb_define_const( ropenldap_mOpenLDAP, "LDAP_X_CONNECTING", INT2FIX(LDAP_X_CONNECTING) );

	/* Module functions */
	rb_define_singleton_method( ropenldap_mOpenLDAP, "split_url", ropenldap_s_split_url, 1 );
	rb_define_singleton_method( ropenldap_mOpenLDAP, "err2string", ropenldap_s_err2string, 1 );
	rb_define_singleton_method( ropenldap_mOpenLDAP, "api_info", ropenldap_api_info, 0 );
	rb_define_singleton_method( ropenldap_mOpenLDAP, "api_feature_info", ropenldap_api_feature_info, 0 );

	/* Initialize the other parts of the extension */
	ropenldap_init_connection();

	/* Detect mismatched linking */
	ropenldap_check_link();
}

