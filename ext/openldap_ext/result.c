/*
 * Ruby-OpenLDAP -- OpenLDAP::Result class
 * $Id$
 *
 * Authors
 *
 * - Michael Granger <ged@FaerieMUD.org>
 *
 * Copyright (c) 2011-2013 Michael Granger
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
VALUE ropenldap_cOpenLDAPResult;


/* --------------------------------------------------
 *	Memory-management functions
 * -------------------------------------------------- */

/*
 * Allocation function
 */
static struct ropenldap_result *
ropenldap_result_alloc( VALUE connection, int msgid )
{
	struct ropenldap_result *ptr = ALLOC( struct ropenldap_result );

	ptr->msgid      = msgid;
	ptr->connection = connection;
	ptr->abandoned  = Qfalse;

	return ptr;
}


/*
 * GC Mark function
 */
static void
ropenldap_result_gc_mark( struct ropenldap_result *ptr )
{
	if ( ptr ) {
		rb_gc_mark( ptr->connection );
	}
}



/*
 * GC Free function
 */
static void
ropenldap_result_gc_free( struct ropenldap_result *ptr )
{
	if ( ptr ) {
		ptr->msgid      = 0;
		ptr->connection = Qnil;
		ptr->abandoned  = Qfalse;

		xfree( ptr );
		ptr = NULL;
	}
}


/*
 * Object validity checker. Returns the data pointer.
 */
static struct ropenldap_result *
check_result( VALUE self )
{
	Check_Type( self, T_DATA );

    if ( !IsResult(self) ) {
		rb_raise( rb_eTypeError, "wrong argument type %s (expected an OpenLDAP::Result)",
				  rb_obj_classname( self ) );
    }

	return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static struct ropenldap_result *
ropenldap_get_result( VALUE self )
{
	struct ropenldap_result *conn = check_result( self );

	if ( !conn ) rb_fatal( "Use of uninitialized OpenLDAP::Result" );

	return conn;
}



/* --------------------------------------------------------------
 * Class methods
 * -------------------------------------------------------------- */

/*
 * call-seq:
 *    OpenLDAP::Result.allocate   -> store
 *
 * Allocate a new OpenLDAP::Result object.
 *
 */
static VALUE
ropenldap_result_s_allocate( VALUE klass )
{
	return Data_Wrap_Struct( klass, ropenldap_result_gc_mark, ropenldap_result_gc_free, 0 );
}


/* --------------------------------------------------------------
 * Instance methods
 * -------------------------------------------------------------- */

/*
 * call-seq:
 *    OpenLDAP::Result.new( connection, msgid )    -> result
 *
 * Create a new OpenLDAP::Result object for the specified +msgid+ on the given
 * +connection+.
 *
 */
static VALUE
ropenldap_result_initialize( VALUE self, VALUE connection, VALUE msgid )
{
	ropenldap_log_obj( self, "debug", "Initializing 0x%x", self );

	if ( !check_result(self) ) {
		DATA_PTR( self ) = ropenldap_result_alloc( connection, NUM2INT(msgid) );
	} else {
		rb_raise( ropenldap_eOpenLDAPError,
				  "Cannot re-initialize a result once it's been created." );
	}

	return Qnil;
}


/*
 * call-seq:
 *    result.abandon   -> true
 *
 * Abandon the operation in progress and discard any results that have been
 * queued.
 *
 */
static VALUE
ropenldap_result_abandon( int argc, VALUE *argv, VALUE self )
{
	struct ropenldap_result *ptr = ropenldap_get_result( self );
	LDAP *ldap = ropenldap_conn_get_ldap( ptr->connection );
	int res;

	/* :TODO: controls */

	res = ldap_abandon_ext( ldap, ptr->msgid, NULL, NULL );
	ropenldap_check_result( res, "ldap_abandon_ext" );

	return Qtrue;
}


/*
 * call-seq:
 *    result.fetch              -> message or nil
 *    result.fetch( timeout )   -> message or nil
 *
 * Fetch the next result if it's ready. Raises a TimeoutError if the fetch
 * times out.
 *
 */
static VALUE
ropenldap_result_fetch( int argc, VALUE *argv, VALUE self )
{
	struct ropenldap_result *ptr = ropenldap_get_result( self );
	LDAP *ldap = ropenldap_conn_get_ldap( ptr->connection );
	VALUE timeout = Qnil;
	VALUE message = Qnil;
	LDAPMessage *msg = NULL;
	struct timeval *c_timeout = NULL;
	int res = 0;

	rb_scan_args( argc, argv, "01", &timeout );

	if ( !NIL_P(timeout) ) {
		c_timeout = ALLOCA_N( struct timeval, 1 );
		double seconds = NUM2DBL( timeout );
		c_timeout->tv_sec = (time_t)floor( seconds );
		c_timeout->tv_usec = (suseconds_t)( fmod(seconds, 1.0) * MILLION_F );
	}

	// int ldap_result( LDAP *ld, int msgid, int all,
	//             struct timeval *timeout, LDAPMessage **result );
	res = ldap_result( ldap, ptr->msgid, 0, c_timeout, &msg );

	if ( res == 0 ) {
		rb_raise( rb_eRuntimeError, "timeout!" );
	}

	else if ( res < 0 ) {
		ropenldap_check_result( res, "ldap_result(%p, %d, ...)", ldap, ptr->msgid );
	}

	message = ropenldap_new_message( ptr->connection, msg );

	return message;
}



/*
 * document-class: OpenLDAP::Result
 */
void
ropenldap_init_result( void )
{
	ropenldap_log( "debug", "Initializing OpenLDAP::Result" );

#ifdef FOR_RDOC
	ropenldap_mOpenLDAP = rb_define_module( "OpenLDAP" );
#endif

	/* OpenLDAP::Result */
	ropenldap_cOpenLDAPResult =
		rb_define_class_under( ropenldap_mOpenLDAP, "Result", rb_cObject );

	rb_define_alloc_func( ropenldap_cOpenLDAPResult, ropenldap_result_s_allocate );

	rb_define_protected_method( ropenldap_cOpenLDAPResult, "initialize",
	                            ropenldap_result_initialize, 2 );

	rb_define_method( ropenldap_cOpenLDAPResult, "abandon", ropenldap_result_abandon, -1 );
	rb_define_method( ropenldap_cOpenLDAPResult, "fetch", ropenldap_result_fetch, -1 );

	rb_require( "openldap/result" );
}

