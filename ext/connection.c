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



/* --------------------------------------------------------------
 * Class methods
 * -------------------------------------------------------------- */

/*
 *  call-seq:
 *     OpenLDAP::Connection.allocate   -> store
 *
 *  Allocate a new OpenLDAP::Connection object.
 *
 */
static VALUE
ropenldap_conn_s_allocate( VALUE klass ) {
	return Data_Wrap_Struct( klass, ropenldap_conn_gc_mark, ropenldap_conn_gc_free, 0 );
}


/* --------------------------------------------------------------
 * Instance methods
 * -------------------------------------------------------------- */

/*
 *  call-seq:
 *     OpenLDAP::Connection.new( url )           -> conn
 *
 *  Create a new OpenLDAP::Connection object using the given +uri+.
 *
 */
static VALUE
ropenldap_conn_initialize( VALUE self, VALUE uristring ) {
	ropenldap_log_obj( self, "debug", "Initializing 0x%x", self );

	if ( !check_conn(self) ) {
		LDAP *ldp = NULL;
		char *uri = StringValueCStr( uristring );
		struct ropenldap_connection *conn;
		int result = 0;

		result = ldap_initialize( &ldp, uri );
		ropenldap_check_result( ldp, result );
		ropenldap_log_obj( self, "info", "Created a new %s (%s)", rb_obj_classname(self), uri );

		conn = DATA_PTR( self ) = ropenldap_conn_alloc( ldp );

	} else {
		rb_raise( ropenldap_eOpenLDAPError,
				  "Cannot re-initialize a store once it's been created." );
	}

	return Qnil;
}





/*
 * OpenLDAP Connection class
 */
void
ropenldap_init_connection( void ) {
	ropenldap_log( "debug", "Initializing OpenLDAP::Connection" );

#ifdef FOR_RDOC
	ropenldap_mOpenLDAP = rb_define_module( "OpenLDAP" );
#endif

	/* OpenLDAP::Connection */
	ropenldap_cOpenLDAPConnection =
		rb_define_class_under( ropenldap_mOpenLDAP, "Connection", rb_cObject );
	rb_define_alloc_func( ropenldap_cOpenLDAPConnection, ropenldap_conn_s_allocate );

	rb_define_method( ropenldap_cOpenLDAPConnection, "initialize", ropenldap_conn_initialize, 1 );

}

