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


/* --------------------------------------------------------------
 * Declarations
 * -------------------------------------------------------------- */
VALUE ropenldap_cOpenLDAPConnection;


ID id_base;
ID id_subtree;
ID id_onelevel;


/* --------------------------------------------------
 *	Memory-management functions
 * -------------------------------------------------- */

/*
 * Allocation function
 */
static struct ropenldap_connection *
ropenldap_conn_alloc( LDAP *ldp )
{
	struct ropenldap_connection *ptr = ALLOC( struct ropenldap_connection );

	ptr->ldap = ldp;

	return ptr;
}


/*
 * GC Mark function
 */
static void
ropenldap_conn_gc_mark( struct ropenldap_connection *ptr )
{
	/* No-op */
}



/*
 * GC Free function
 */
static void
ropenldap_conn_gc_free( struct ropenldap_connection *ptr )
{
	if ( ptr ) {
		ptr->ldap = NULL;

		xfree( ptr );
		ptr = NULL;
	}
}


/*
 * Object validity checker. Returns the data pointer.
 */
static struct ropenldap_connection *
check_conn( VALUE self )
{
	Check_Type( self, T_DATA );

    if ( !IsConnection(self) ) {
		rb_raise( rb_eTypeError, "wrong argument type %s (expected an OpenLDAP::Connection)",
				  rb_obj_classname( self ) );
    }

	return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static struct ropenldap_connection *
ropenldap_get_conn( VALUE self )
{
	struct ropenldap_connection *conn = check_conn( self );

	if ( !conn ) rb_fatal( "Use of uninitialized OpenLDAP::Connection" );

	return conn;
}


/*
 * Fetch the LDAP handle from the OpenLDAP::Connection object +connection+.
 */
LDAP *
ropenldap_conn_get_ldap( VALUE connection )
{
	struct ropenldap_connection *conn = check_conn( connection );
	return conn->ldap;
}


/* --------------------------------------------------------------
 * Class methods
 * -------------------------------------------------------------- */

/*
 * call-seq:
 *    OpenLDAP::Connection.allocate   -> store
 *
 * Allocate a new OpenLDAP::Connection object.
 *
 */
static VALUE
ropenldap_conn_s_allocate( VALUE klass )
{
	return Data_Wrap_Struct( klass, ropenldap_conn_gc_mark, ropenldap_conn_gc_free, 0 );
}


/* --------------------------------------------------------------
 * Instance methods
 * -------------------------------------------------------------- */

/*
 * Backend method for OpenLDAP::Connection#initialize.
 */
static VALUE
ropenldap_conn__initialize( VALUE self, VALUE urls )
{
	ropenldap_log_obj( self, "debug", "Initializing 0x%x", self );

	if ( !check_conn(self) ) {
		VALUE urlstring;
		LDAP *ldp = NULL;
		char *url = NULL;
		struct ropenldap_connection *conn;
		int result = 0;

		urlstring = rb_obj_as_string( urls );
		url = RSTRING_PTR( rb_obj_as_string(urlstring) );

		if ( !ldap_is_ldap_url(url) )
			rb_raise( rb_eArgError, "'%s' is not an LDAP url", url );

		ropenldap_log_obj( self, "info", "Creating a new %s (%s)", rb_obj_classname(self), url );
		result = ldap_initialize( &ldp, url );
		ropenldap_check_result( result, "ldap_initialize( \"%s\" )", url );

		conn = DATA_PTR( self ) = ropenldap_conn_alloc( ldp );

	} else {
		rb_raise( ropenldap_eOpenLDAPError,
				  "Cannot re-initialize a connection once it's been created." );
	}

	return Qnil;
}


/*
 * call-seq:
 *    conn.protocol_version   -> fixnum
 *
 * Get the protocol version use by the connection.
 *
 *    conn.protocol_version
 *    # => 3
 */
static VALUE
ropenldap_conn_protocol_version( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	int version = 0;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_PROTOCOL_VERSION, &version) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_PROTOCOL_VERSION" );

	return INT2FIX( version );
}


/*
 * call-seq:
 *    conn.protocol_version = version
 *
 * Set the protocol version use by the connection to +version+.
 *
 *    conn.protocol_version = 3
 *    # => 3
 */
static VALUE
ropenldap_conn_protocol_version_eq( VALUE self, VALUE version )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	int v = NUM2INT( version );

	if ( ldap_set_option(ptr->ldap, LDAP_OPT_PROTOCOL_VERSION, &v) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_PROTOCOL_VERSION" );

	return version;
}


/*
 * call-seq:
 *    conn.async_connect?   -> boolean
 *
 * Returns +true+ if the connect(2) call made by the library will be made asynchronously.
 *
 *    conn.async_connect?
 *    # => true
 */
static VALUE
ropenldap_conn_async_connect_p( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	int enabled;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_CONNECT_ASYNC, &enabled) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_CONNECT_ASYNC" );

	return enabled ? Qtrue : Qfalse;
}


/*
 * call-seq:
 *    conn.async_connect = boolean
 *
 * If set to a +true+ value, the library will call connect(2) and return without waiting for
 * a response.
 *
 *    conn.async_connect = true
 */
static VALUE
ropenldap_conn_async_connect_eq( VALUE self, VALUE boolean )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	int rv = 0;

	if ( RTEST(boolean) ) {
		ropenldap_log_obj( self, "debug", "Enabling async connect." );
		rv = ldap_set_option( ptr->ldap, LDAP_OPT_CONNECT_ASYNC, LDAP_OPT_ON );
	} else {
		ropenldap_log_obj( self, "debug", "Disabling async connect." );
		rv = ldap_set_option( ptr->ldap, LDAP_OPT_CONNECT_ASYNC, LDAP_OPT_OFF );
	}

	if ( rv != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_CONNECT_ASYNC" );

	return RTEST( boolean ) ? Qtrue : Qfalse;
}


/*
 * call-seq:
 *    conn.network_timeout   -> float or nil
 *
 * Returns the network timeout value (if it is set).
 *
 *    conn.network_timeout
 *    # => 2.5
 */
static VALUE
ropenldap_conn_network_timeout( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	struct timeval *timeout;
	double seconds = 0;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_NETWORK_TIMEOUT, &timeout) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_NETWORK_TIMEOUT" );

	if ( timeout ) {
		ropenldap_log_obj( self, "debug", "Got network timeout: %d/%d",
		                   timeout->tv_sec, timeout->tv_usec );
		seconds = ((double) timeout->tv_sec) + timeout->tv_usec / MILLION_F;
		ldap_memfree( timeout );
		return rb_float_new( seconds );
	} else {
		ropenldap_log_obj( self, "debug", "No network timeout." );
		return Qnil;
	}
}


/*
 * call-seq:
 *    conn.network_timeout = float or nil
 *
 * Set the network timeout value; the network timeout value is the number of seconds after which
 * poll(2)/select(2) following a connect(2) returns in case of no activity. Setting this to nil
 * or -1 disables it.
 *
 *    conn.network_timeout = 1.0
 */
static VALUE
ropenldap_conn_network_timeout_eq( VALUE self, VALUE arg )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	double seconds;
	struct timeval timeout;

	if ( NIL_P(arg) ) {
		seconds = -1.0;
	} else {
		seconds = NUM2DBL( arg );
	}

	ropenldap_log_obj( self, "debug", "Setting network timeout to %0.5f seconds", seconds );
	timeout.tv_sec = (time_t)floor( seconds );
	timeout.tv_usec = (suseconds_t)( fmod(seconds, 1.0) * MILLION_F );

	if ( ldap_set_option(ptr->ldap, LDAP_OPT_NETWORK_TIMEOUT, (const void *)&timeout) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_NETWORK_TIMEOUT" );

	return arg;
}


/*
 * call-seq:
 *    conn.search_timeout   -> float or nil
 *
 * Returns the value that defines the time limit after which a search operation
 * should be terminated by the server.
 *
 *    conn.search_timeout
 *    # => 2.5
 */
static VALUE
ropenldap_conn_search_timeout( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	int timeout = 0;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_TIMELIMIT, &timeout) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_TIMELIMIT" );

	if ( timeout ) {
		ropenldap_log_obj( self, "debug", "Got  timeout: %d", timeout );
		return INT2FIX( timeout );
	} else {
		ropenldap_log_obj( self, "debug", "No timeout." );
		return Qnil;
	}
}


/*
 * call-seq:
 *    conn.search_timeout = int or nil
 *
 * Set the the value that defines the time limit after which a search operation
 * should be terminated by the server. Setting this to nil or -1 disables it.
 *
 *    conn.search_timeout = 10
 */
static VALUE
ropenldap_conn_search_timeout_eq( VALUE self, VALUE arg )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	int seconds = 0;

	if ( NIL_P(arg) ) {
		seconds = -1;
	} else {
		seconds = NUM2INT( arg );
	}

	ropenldap_log_obj( self, "debug", "Setting network timeout to %d seconds", seconds );
	if ( ldap_set_option(ptr->ldap, LDAP_OPT_TIMELIMIT, &seconds) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_TIMELIMIT" );

	return arg;
}


/*
 * call-seq:
 *    conn.bind( bind_dn, password )   -> result
 *    conn.bind( bind_dn, password ) {|result| ... }
 *
 * Bind to the directory using a simple +bind_dn+ and a +password+.
 *
 */
static VALUE
ropenldap_conn_bind( int argc, VALUE *argv, VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	VALUE bind_dn = Qnil, password = Qnil;
	int res    = 0;
	char *who  = NULL;
	struct berval cred = BER_BVNULL;
	int msgid  = 0;

	rb_scan_args( argc, argv, "02", &bind_dn, &password );

	if ( bind_dn != Qnil ) {
		who = StringValueCStr( bind_dn );
	}

	if ( password != Qnil ) {
		VALUE pass_string = rb_obj_as_string( password );
		cred.bv_val = ber_strdup( RSTRING_PTR(pass_string) );
		cred.bv_len = RSTRING_LEN( pass_string );
	}

	/* TODO:  async for block form, sync otherwise */

	/* TODO: SASL interactive, ANONYMOUS (RFC2245?) */
	// int ldap_sasl_bind(LDAP *ld, const char *dn, const char *mechanism,
	//               struct berval *cred, LDAPControl *sctrls[],
	//               LDAPControl *cctrls[], int *msgidp);
	res = ldap_sasl_bind( ptr->ldap, who, LDAP_SASL_SIMPLE, &cred, NULL, NULL, &msgid );
	if ( !BER_BVISNULL(&cred) ) {
		ber_memfree( cred.bv_val );
		BER_BVZERO( &cred );
	}
	ropenldap_log_obj( self, "debug", "Rval from ldap_sasl_bind_a: %d", res );
	ropenldap_check_result( res, "ldap_sasl_bind_s" );

	return Qtrue;
}


/*
 * Start TLS synchronously; called from ropenldap_conn__start_tls after
 * the GVL is released.
 */
static void *
ropenldap_conn__start_tls_blocking( void *ptr )
{
	LDAP *ld = ptr;
	int rval = ldap_start_tls_s( ld, NULL, NULL );
	return (void *)(VALUE)rval;
}


/*
 * #_start_tls: backend of the #start_tls method.
 */
static VALUE
ropenldap_conn__start_tls( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );

	int result;

	ropenldap_log_obj( self, "debug", "Starting TLS..." );
	result = (int)(VALUE)rb_thread_call_without_gvl( ropenldap_conn__start_tls_blocking,
	                                                 (void *)ptr->ldap, RUBY_UBF_IO, NULL );
	ropenldap_check_result( result, "ldap_start_tls_s" );
	ropenldap_log_obj( self, "debug", "  TLS started." );

	return Qtrue;
}


/*
 * call-seq:
 *    conn.tls_inplace?   -> true or false
 *
 * Returns +true+ if TLS handlers have been installed on the session.
 *
 */
static VALUE
ropenldap_conn_tls_inplace_p( VALUE self )
{
#ifdef HAVE_LDAP_TLS_INPLACE
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );

	if ( ldap_tls_inplace(ptr->ldap) ) {
		return Qtrue;
	} else {
		return Qfalse;
	}
#else
	rb_raise( rb_eNotImpError, "not implemented in your version of libldap" );
#endif /* HAVE_LDAP_TLS_INPLACE */
}


/*
 * call-seq:
 *    conn.tls_cacertfile    -> string
 *
 * Get the full path of the CA certificate file as a String.
 */
static VALUE
ropenldap_conn_tls_cacertfile( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	char *path;
	VALUE pathstring = Qnil;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_X_TLS_CACERTFILE, &path) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_X_TLS_CACERTFILE" );
	if ( !path )
		return Qnil;

	pathstring = rb_str_new2( path );
	ldap_memfree( path );

	return pathstring;
}

/*
 * call-seq:
 *    conn.tls_cacertfile = string
 *
 * Set the full path of the CA certificate file as a String.
 */
static VALUE
ropenldap_conn_tls_cacertfile_eq( VALUE self, VALUE path )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	VALUE pathstring = rb_obj_as_string( path );
	const char *pathopt = StringValuePtr( pathstring );

	if ( ldap_set_option(ptr->ldap, LDAP_OPT_X_TLS_CACERTFILE, pathopt) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_X_TLS_CACERTFILE" );

	return path;
}


/*
 * call-seq:
 *    conn.tls_cacertdir    -> string
 *
 * Get the full path of the directory containing CA certificates as a String.
 */
static VALUE
ropenldap_conn_tls_cacertdir( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	char *path;
	VALUE pathstring = Qnil;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_X_TLS_CACERTDIR, &path) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_X_TLS_CACERTDIR" );
	if ( !path )
		return Qnil;

	pathstring = rb_str_new2( path );
	ldap_memfree( path );

	return pathstring;
}

/*
 * call-seq:
 *    conn.tls_cacertdir = string
 *
 * Set the path of the directory containing CA certificates as a String.
 */
static VALUE
ropenldap_conn_tls_cacertdir_eq( VALUE self, VALUE path )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	VALUE pathstring = rb_obj_as_string( path );
	const char *pathopt = StringValuePtr( pathstring );

	if ( ldap_set_option(ptr->ldap, LDAP_OPT_X_TLS_CACERTDIR, pathopt) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_X_TLS_CACERTDIR" );

	return path;
}


/*
 * call-seq:
 *    conn.tls_cacertfile    -> string
 *
 * Get the full path of the certificate file as a String.
 */
static VALUE
ropenldap_conn_tls_certfile( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	char *path;
	VALUE pathstring = Qnil;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_X_TLS_CERTFILE, &path) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_X_TLS_CERTFILE" );
	if ( !path )
		return Qnil;

	pathstring = rb_str_new2( path );
	ldap_memfree( path );

	return pathstring;
}

/*
 * call-seq:
 *    conn.tls_certfile = string
 *
 * Set the full path of the certificate file as a String.
 */
static VALUE
ropenldap_conn_tls_certfile_eq( VALUE self, VALUE path )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	VALUE pathstring = rb_obj_as_string( path );
	const char *pathopt = StringValuePtr( pathstring );

	if ( ldap_set_option(ptr->ldap, LDAP_OPT_X_TLS_CERTFILE, pathopt) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_X_TLS_CERTFILE" );

	return path;
}


/*
 * call-seq:
 *    conn.tls_keyfile    -> string
 *
 * Get the full path of the file that contains the private key that matches the certificate stored
 * in the #tls_certfile.
 */
static VALUE
ropenldap_conn_tls_keyfile( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	char *path;
	VALUE pathstring = Qnil;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_X_TLS_KEYFILE, &path) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_X_TLS_KEYFILE" );
	if ( !path )
		return Qnil;

	pathstring = rb_str_new2( path );
	ldap_memfree( path );

	return pathstring;
}

/*
 * call-seq:
 *    conn.tls_keyfile = newvalue
 *
 * Set the full path to the file that contains the private key that matches the certificate stored
 * in the #tls_certfile.
 */
static VALUE
ropenldap_conn_tls_keyfile_eq( VALUE self, VALUE path )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	VALUE pathstring = rb_obj_as_string( path );
	const char *pathopt = StringValuePtr( pathstring );

	if ( ldap_set_option(ptr->ldap, LDAP_OPT_X_TLS_KEYFILE, pathopt) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_X_TLS_KEYFILE" );

	return path;
}


/*
 * call-seq:
 *    conn.tls_cipher_suite    -> cipherstring
 *
 * Get the allowed cipher suite. See http://www.openssl.org/docs/apps/ciphers.html for the
 * allowed format of the +cipherstring+.
 */
static VALUE
ropenldap_conn_tls_cipher_suite( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	char *path;
	VALUE pathstring = Qnil;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_X_TLS_CIPHER_SUITE, &path) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_X_TLS_CIPHER_SUITE" );
	if ( !path )
		return Qnil;

	pathstring = rb_str_new2( path );
	ldap_memfree( path );

	return pathstring;
}

/*
 * call-seq:
 *    conn.tls_cipher_suite = cipherstring
 *
 * Set the allowed cipher suite to +cipherstring+; see http://www.openssl.org/docs/apps/ciphers.html
 * for more about the format of this string.
 */
static VALUE
ropenldap_conn_tls_cipher_suite_eq( VALUE self, VALUE cipherstring )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	VALUE string = rb_obj_as_string( cipherstring );
	const char *cipheropt = StringValuePtr( string );

	if ( ldap_set_option(ptr->ldap, LDAP_OPT_X_TLS_CIPHER_SUITE, cipheropt) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_X_TLS_CIPHER_SUITE" );

	return string;
}


/*
 * call-seq:
 *    conn.tls_random_file    -> true or false
 *
 * Get the path to the random file that will be used when /dev/random and /dev/urandom are
 * not available.
 */
static VALUE
ropenldap_conn_tls_random_file( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	char *path;
	VALUE pathstring = Qnil;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_X_TLS_RANDOM_FILE, &path) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_X_TLS_RANDOM_FILE" );
	if ( !path )
		return Qnil;

	pathstring = rb_str_new2( path );
	ldap_memfree( path );

	return pathstring;
}

/*
 * call-seq:
 *    conn.tls_random_file = newvalue
 *
 * Set the path to the random file that will be used when /dev/random and /dev/urandom are
 * not available.
 */
static VALUE
ropenldap_conn_tls_random_file_eq( VALUE self, VALUE path )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	VALUE pathstring = rb_obj_as_string( path );
	const char *pathopt = StringValuePtr( pathstring );

	if ( ldap_set_option(ptr->ldap, LDAP_OPT_X_TLS_RANDOM_FILE, pathopt) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_X_TLS_RANDOM_FILE" );

	return path;
}


/*
 * call-seq:
 *    conn.tls_dhfile    -> string
 *
 * Path to PEM encoded Diffie-Hellman parameter file.
 */
static VALUE
ropenldap_conn_tls_dhfile( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	char *path;
	VALUE pathstring = Qnil;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_X_TLS_DHFILE, &path) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_X_TLS_DHFILE" );
	if ( !path )
		return Qnil;

	pathstring = rb_str_new2( path );
	ldap_memfree( path );

	return pathstring;
}

/*
 * call-seq:
 *    conn.tls_dhfile = newvalue
 *
 * Set the path to a PEM-encoded Diffie-Hellman parameter file. If this is specified, DH key
 * exchange will be used for the ephemeral keying.
 *
 * You can use the 'openssl' command-line tool to generate the file like so:
 *
 *   openssl dhparam -outform PEM -out dh1024.pem -5 1024
 */
static VALUE
ropenldap_conn_tls_dhfile_eq( VALUE self, VALUE path )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	VALUE pathstring = rb_obj_as_string( path );
	const char *pathopt = StringValuePtr( pathstring );

	if ( ldap_set_option(ptr->ldap, LDAP_OPT_X_TLS_DHFILE, pathopt) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_X_TLS_DHFILE" );

	return path;
}


/*
 * call-seq:
 *    conn.tls_crlfile    -> string
 *
 * Get the current path to the file containing a Certificate Revocation List used for verifying that
 * certificates have not been revoked.
 */
static VALUE
ropenldap_conn_tls_crlfile( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	char *path;
	VALUE pathstring = Qnil;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_X_TLS_CRLFILE, &path) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_X_TLS_CRLFILE" );
	if ( !path )
		return Qnil;

	pathstring = rb_str_new2( path );
	ldap_memfree( path );

	return pathstring;
}

/*
 * call-seq:
 *    conn.tls_crlfile = path
 *
 * Set the path to the file containing a Certificate Revocation List used for verifying that
 * certificates have not been revoked. This value is only used when LDAP has been compiled
 * to use GnuTLS.
 */
static VALUE
ropenldap_conn_tls_crlfile_eq( VALUE self, VALUE path )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	VALUE pathstring = rb_obj_as_string( path );
	const char *pathopt = StringValuePtr( pathstring );

	if ( ldap_set_option(ptr->ldap, LDAP_OPT_X_TLS_CRLFILE, pathopt) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_X_TLS_CRLFILE" );

	return path;
}


/*
 * call-seq:
 *    conn._tls_require_cert    -> fixnum
 *
 * Backend method for #tls_require_cert
 */
static VALUE
ropenldap_conn__tls_require_cert( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	int opt;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_X_TLS_REQUIRE_CERT, &opt) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_X_TLS_REQUIRE_CERT" );

	return INT2FIX( opt );
}

/*
 * call-seq:
 *    conn._tls_require_cert = fixnum
 *
 * Backend method for #tls_require_cert=.
 */
static VALUE
ropenldap_conn__tls_require_cert_eq( VALUE self, VALUE opt )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	const int optval = NUM2INT( opt );

	ropenldap_log_obj( self, "debug", "Setting LDAP_OPT_X_TLS_REQUIRE_CERT to %d", optval );

	if ( ldap_set_option(ptr->ldap, LDAP_OPT_X_TLS_REQUIRE_CERT, &optval) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_X_TLS_REQUIRE_CERT" );

	return opt;
}


/*
 * call-seq:
 *    conn._tls_crlcheck    -> fixnum
 *
 * Backend method for #tls_crlcheck.
 */
static VALUE
ropenldap_conn__tls_crlcheck( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	int opt;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_X_TLS_CRLCHECK, &opt) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_X_TLS_CRLCHECK" );

	return INT2FIX( opt );
}

/*
 * call-seq:
 *    conn._tls_crlcheck = fixnum
 *
 * Backend method for #tls_crlcheck=.
 */
static VALUE
ropenldap_conn__tls_crlcheck_eq( VALUE self, VALUE opt )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	const int optval = NUM2INT( opt );

	if ( ldap_set_option(ptr->ldap, LDAP_OPT_X_TLS_CRLCHECK, &optval) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_X_TLS_CRLCHECK" );

	return opt;
}


/*
 * call-seq:
 *    conn.tls_protocol_min    -> fixnum
 *
 * Gets the current minimum protocol version.
 */
static VALUE
ropenldap_conn_tls_protocol_min( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	int version;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_X_TLS_PROTOCOL_MIN, &version) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_X_TLS_PROTOCOL_MIN" );

	return INT2FIX( version );
}


/*
 * call-seq:
 *    conn.tls_protocol_min = fixnum
 *
 * Set the minimum protocol version.
 */
static VALUE
ropenldap_conn_tls_protocol_min_eq( VALUE self, VALUE opt )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	const int optval = NUM2INT( opt );

	if ( ldap_set_option(ptr->ldap, LDAP_OPT_X_TLS_PROTOCOL_MIN, &optval) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_X_TLS_PROTOCOL_MIN" );

	return opt;
}


/*
 * call-seq:
 *    conn.tls_package    -> string
 *
 * :FIXME: I can't find any docs on what this does.
 */
static VALUE
ropenldap_conn_tls_package( VALUE self )
{
#ifdef LDAP_OPT_X_TLS_PACKAGE
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	char *package;
	VALUE pkgstring = Qnil;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_X_TLS_PACKAGE, &package) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_X_TLS_PACKAGE" );
	if ( !package )
		return Qnil;

	pkgstring = rb_str_new2( package );
	ldap_memfree( package );

	return pkgstring;
#else
	rb_raise( rb_eNotImpError, "not implemented in your version of libldap." );
#endif /* LDAP_OPT_X_TLS_PACKAGE */
}


/*
 * call-seq:
 *    conn.create_new_tls_context    -> true or false
 *
 * Instructs the library to create a new TLS library context.
 */
static VALUE
ropenldap_conn_create_new_tls_context( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	const int flag = 1;

	if ( ldap_set_option(ptr->ldap, LDAP_OPT_X_TLS_NEWCTX, &flag) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't set option: LDAP_OPT_X_TLS_NEWCTX" );

	return Qtrue;
}


/*
 * Turn a STRING_T into a URI object via URI::parse.
 */
static VALUE
ropenldap_parse_uri( VALUE string )
{
	StringValue( string );
	return rb_funcall( ropenldap_rbmURI, rb_intern("parse"), 1, string );
}


/*
 * call-seq:
 *     connection.uris   -> array
 *
 * Gets an Array of URIs to be contacted by the library when trying to establish
 * a connection.
 *
 */
static VALUE
ropenldap_conn_uris( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	char *uris;
	VALUE uristring, uriarray;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_URI, &uris) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_URI" );

	/* Convert to strings first, then collect them into URI objects */
	uristring = rb_str_new2( uris );
	ldap_memfree( uris );

	uriarray = rb_funcall( uristring, rb_intern("split"), 0 );
	rb_block_call( uriarray, rb_intern("collect!"), 0, NULL,
	               ropenldap_parse_uri, Qnil );

	return uriarray;
}


/*
 * call-seq:
 *    connection.fdno   -> fixnum
 *
 * Return the file descriptor of the underlying socket. If the socket isn't connected yet,
 * this method returns +nil+.
 *
 *    ldapsock = IO.for_fd( conn.fdno, "w+" )
 */
static VALUE
ropenldap_conn_fdno( VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	int fdno = 0;

	if ( ldap_get_option(ptr->ldap, LDAP_OPT_DESC, &fdno) != LDAP_OPT_SUCCESS )
		rb_raise( ropenldap_eOpenLDAPError, "couldn't get option: LDAP_OPT_DESC" );

	/* If the socket isn't set up yet, return nil */
	if ( fdno < 0 ) return Qnil;

	return INT2FIX( fdno );
}


/* Return the corresponding constant for the scope Symbol +scope+ */
static int
ropenldap_scope_symbol_to_int( VALUE scope )
{
	if ( SYM2ID(scope) == id_base ) {
		return LDAP_SCOPE_BASE;
	}

	else if ( SYM2ID(scope) == id_subtree ) {
		return LDAP_SCOPE_SUBTREE;
	}

	else if ( SYM2ID(scope) == id_onelevel ) {
		return LDAP_SCOPE_ONELEVEL;
	}

	else {
		rb_raise( rb_eArgError, "Invalid scope %s", RSTRING_PTR(rb_inspect(scope)) );
	}
}


/* Utility method to derive a scope constant from a ruby scope argument
   (e.g., :base, :onelevel, or their Fixnum equivalents) */
static int
ropenldap_get_scope( VALUE scope )
{
	switch( TYPE(scope) ) {
		case T_SYMBOL:
		  return ropenldap_scope_symbol_to_int( scope );
		  break;

		case T_FIXNUM:
		  return FIX2INT( scope );

		default:
		  rb_raise( rb_eTypeError, "expected Fixnum or Symbol, got a %s",
		            rb_obj_classname(scope) );
	}

	// Unreached
	return 0;
}


/*
 * call-seq:
 *    conn.search( base, scope=:subtree, ... )   -> <return value>
 *
 * Execute a search given the args.
 *
 *    example code
 */
static VALUE
ropenldap_conn_search( int argc, VALUE *argv, VALUE self )
{
	struct ropenldap_connection *ptr = ropenldap_get_conn( self );
	VALUE rb_base        = Qnil,
	      rb_scope       = Qnil,
	      rb_filter      = Qnil,
	      rb_attrs       = Qnil,
	      rb_attrsonly   = Qnil,
	      rb_serverctrls = Qnil,
	      rb_clientctrls = Qnil,
	      rb_timeout     = Qnil,
	      rb_sizelimit   = Qnil;
	char *base = NULL;
	int scope = LDAP_SCOPE_SUBTREE;
	char *filter = NULL;
	char *attrs[] = {};
	int attrsonly = 0;
	LDAPControl *serverctrls[] = {}, *clientctrls[] = {};
	struct timeval timeout = { 0, 0 };
	int sizelimit = -1;
	rb_encoding *utf8 = rb_utf8_encoding();

	// Result
	int rval = -1;
	int msgid = 0;

	ropenldap_log_obj( self, "debug", "Searching:" );
	rb_scan_args( argc, argv, "18",
	              &rb_base, &rb_scope, &rb_filter, &rb_attrs, &rb_attrsonly,
				  &rb_serverctrls, &rb_clientctrls, &rb_timeout, &rb_sizelimit );

	// Base
	SafeStringValue( rb_base );
	rb_base = rb_str_encode( rb_filter, rb_enc_from_encoding(utf8), 0, Qnil );
	rb_gc_register_address( &rb_base );
	base = StringValueCStr( rb_base );
	ropenldap_log_obj( self, "debug", "  search base set to '%s'", base );

	// Scope
	if ( !NIL_P(rb_scope) ) scope = ropenldap_get_scope( rb_scope );
	ropenldap_log_obj( self, "debug", "  search scope set to %d", scope );

	// Filter
	if ( !NIL_P(rb_filter) ) {
		SafeStringValue( rb_filter );
		rb_filter = rb_str_encode( rb_filter, rb_enc_from_encoding(utf8), 0, Qnil );
		rb_gc_register_address( &rb_filter );
		filter = StringValueCStr( rb_filter );
	}

	// Do the search
	rval = ldap_search_ext( ptr->ldap, base, scope, filter, attrs, attrsonly,
	                        serverctrls, clientctrls, &timeout, sizelimit,
	                        &msgid );

	// Release all of the strings we were using
	rb_gc_unregister_address( &rb_base );
	rb_gc_unregister_address( &rb_filter );

	// Check the results of the search and raise if there was a problem
	ropenldap_check_result( rval, "ldap_search_ext( %s, %d, %s )", base, scope, filter );

	// Some other stuff

	return Qnil;
}



/*
 * document-class: OpenLDAP::Connection
 */
void
ropenldap_init_connection( void )
{
	ropenldap_log( "debug", "Initializing OpenLDAP::Connection" );

#ifdef FOR_RDOC
	ropenldap_mOpenLDAP = rb_define_module( "OpenLDAP" );
#endif

	id_base     = rb_intern_const( "base" );
	id_subtree  = rb_intern_const( "subtree" );
	id_onelevel = rb_intern_const( "onelevel" );

	/* OpenLDAP::Connection */
	ropenldap_cOpenLDAPConnection =
		rb_define_class_under( ropenldap_mOpenLDAP, "Connection", rb_cObject );

	rb_define_alloc_func( ropenldap_cOpenLDAPConnection, ropenldap_conn_s_allocate );

	rb_define_protected_method( ropenldap_cOpenLDAPConnection, "_initialize",
	                            ropenldap_conn__initialize, 1 );

	rb_define_method( ropenldap_cOpenLDAPConnection, "uris", ropenldap_conn_uris, 0 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "fdno", ropenldap_conn_fdno, 0 );
	rb_define_alias(  ropenldap_cOpenLDAPConnection, "fileno", "fdno" );
	rb_define_method( ropenldap_cOpenLDAPConnection, "bind", ropenldap_conn_bind, -1 );

	rb_define_method( ropenldap_cOpenLDAPConnection, "search", ropenldap_conn_search, 1 );
	rb_define_alias ( ropenldap_cOpenLDAPConnection, "search_ext", "search" );

	/* Options */
	rb_define_method( ropenldap_cOpenLDAPConnection, "protocol_version",
	                  ropenldap_conn_protocol_version, 0 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "protocol_version=",
	                  ropenldap_conn_protocol_version_eq, 1 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "async_connect?",
	                  ropenldap_conn_async_connect_p, 0 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "async_connect=",
	                  ropenldap_conn_async_connect_eq, 1 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "network_timeout",
	                  ropenldap_conn_network_timeout, 0 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "network_timeout=",
	                  ropenldap_conn_network_timeout_eq, 1 );

	rb_define_method( ropenldap_cOpenLDAPConnection, "search_timeout",
	                  ropenldap_conn_search_timeout, 0 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "search_timeout=",
	                  ropenldap_conn_search_timeout_eq, 1 );
	rb_define_alias ( ropenldap_cOpenLDAPConnection, "timelimit", "search_timeout" );
	// rb_define_method( ropenldap_cOpenLDAPConnection, "result_timeout",
	//                   ropenldap_conn_result_timeout, 0 );
	// rb_define_method( ropenldap_cOpenLDAPConnection, "result_timeout=",
	//                   ropenldap_conn_result_timeout_eq, 1 );

	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_inplace?",
	                  ropenldap_conn_tls_inplace_p, 0 );

	/* TLS Options */
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_cacertfile",
	                  ropenldap_conn_tls_cacertfile, 0 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_cacertfile=",
	                  ropenldap_conn_tls_cacertfile_eq, 1 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_cacertdir",
	                  ropenldap_conn_tls_cacertdir, 0 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_cacertdir=",
	                  ropenldap_conn_tls_cacertdir_eq, 1 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_certfile",
	                  ropenldap_conn_tls_certfile, 0 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_certfile=",
	                  ropenldap_conn_tls_certfile_eq, 1 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_keyfile",
	                  ropenldap_conn_tls_keyfile, 0 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_keyfile=",
	                  ropenldap_conn_tls_keyfile_eq, 1 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_cipher_suite",
	                  ropenldap_conn_tls_cipher_suite, 0 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_cipher_suite=",
	                  ropenldap_conn_tls_cipher_suite_eq, 1 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_random_file",
	                  ropenldap_conn_tls_random_file, 0 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_random_file=",
	                  ropenldap_conn_tls_random_file_eq, 1 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_dhfile",
	                  ropenldap_conn_tls_dhfile, 0 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_dhfile=",
	                  ropenldap_conn_tls_dhfile_eq, 1 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_crlfile",
	                  ropenldap_conn_tls_crlfile, 0 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_crlfile=",
	                  ropenldap_conn_tls_crlfile_eq, 1 );

	/* Integer options */
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_protocol_min",
	                  ropenldap_conn_tls_protocol_min, 0 );
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_protocol_min=",
	                  ropenldap_conn_tls_protocol_min_eq, 1 );

	/* Read-only options */
	rb_define_method( ropenldap_cOpenLDAPConnection, "tls_package",
	                  ropenldap_conn_tls_package, 0 );

	rb_define_method( ropenldap_cOpenLDAPConnection, "create_new_tls_context",
	                  ropenldap_conn_create_new_tls_context, 0 );



	/* Methods with Ruby front-ends */
	rb_define_protected_method( ropenldap_cOpenLDAPConnection, "_start_tls",
	                            ropenldap_conn__start_tls, 0 );
	rb_define_protected_method( ropenldap_cOpenLDAPConnection, "_tls_require_cert",
	                            ropenldap_conn__tls_require_cert, 0 );
	rb_define_protected_method( ropenldap_cOpenLDAPConnection, "_tls_require_cert=",
	                            ropenldap_conn__tls_require_cert_eq, 1 );
	rb_define_protected_method( ropenldap_cOpenLDAPConnection, "_tls_crlcheck",
	                            ropenldap_conn__tls_crlcheck, 0 );
	rb_define_protected_method( ropenldap_cOpenLDAPConnection, "_tls_crlcheck=",
	                            ropenldap_conn__tls_crlcheck_eq, 1 );

	rb_require( "openldap/connection" );
}

