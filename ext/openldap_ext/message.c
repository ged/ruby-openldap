/*
 * Ruby-OpenLDAP -- OpenLDAP::Message class
 * $Id$
 *
 * Authors
 *
 * - Michael Granger <ged@FaerieMUD.org>
 * - Mahlon E. Smith <mahlon@martini.nu>
 *
 * Copyright (c) 2013 Michael Granger and Mahlon E. Smith
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
VALUE ropenldap_cOpenLDAPMessage;



/* --------------------------------------------------------------
 * Global functions
 * -------------------------------------------------------------- */

static void ropenldap_message_gc_mark( struct ropenldap_message * );
static void ropenldap_message_gc_free( struct ropenldap_message * );

/*
 * Message constructor
 */
VALUE
ropenldap_new_message( VALUE conn, LDAPMessage *msg )
{
	struct ropenldap_message *ptr = ALLOC( struct ropenldap_message );

	ptr->connection = conn;
	ptr->msg        = msg;

	return Data_Wrap_Struct( ropenldap_cOpenLDAPMessage, ropenldap_message_gc_free,
	                         ropenldap_message_gc_mark, ptr );
}


/* --------------------------------------------------
 *	Memory-management functions
 * -------------------------------------------------- */

/*
 * GC Mark function
 */
static void
ropenldap_message_gc_mark( struct ropenldap_message *ptr )
{
	if ( ptr ) rb_gc_mark( ptr->connection );
}



/*
 * GC Free function
 */
static void
ropenldap_message_gc_free( struct ropenldap_message *ptr )
{
	if ( ptr ) {
		ptr->connection = Qnil;
		ptr->msg        = NULL;

		xfree( ptr );
		ptr = NULL;
	}
}


/*
 * Object validity checker. Returns the data pointer.
 */
static struct ropenldap_message *
check_message( VALUE self )
{
	Check_Type( self, T_DATA );

    if ( !IsMessage(self) ) {
		rb_raise( rb_eTypeError, "wrong argument type %s (expected an OpenLDAP::Message)",
				  rb_obj_classname( self ) );
    }

	return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static struct ropenldap_message *
ropenldap_get_message( VALUE self )
{
	struct ropenldap_message *msg = check_message( self );

	if ( !msg ) rb_fatal( "Use of uninitialized OpenLDAP::Message" );

	return msg;
}



/* --------------------------------------------------------------
 * Instance methods
 * -------------------------------------------------------------- */

static VALUE
ropenldap_message_count( VALUE self )
{
	struct ropenldap_message *ptr = ropenldap_get_message( self );
	LDAP *ldap = ropenldap_conn_get_ldap( ptr->connection );
	int count = 0;

	count = ldap_count_messages( ldap, ptr->msg );
	if ( count == -1 )
		ropenldap_check_result( count, "ldap_count_messages" );

	return INT2FIX( count );
}


/*
 * document-class: OpenLDAP::Message
 */
void
ropenldap_init_message( void )
{
	ropenldap_log( "debug", "Initializing OpenLDAP::Message" );

#ifdef FOR_RDOC
	ropenldap_mOpenLDAP = rb_define_module( "OpenLDAP" );
#endif

	/* OpenLDAP::Result */
	ropenldap_cOpenLDAPMessage =
		rb_define_class_under( ropenldap_mOpenLDAP, "Message", rb_cObject );

	rb_define_method( ropenldap_cOpenLDAPMessage, "count", ropenldap_message_count, 0 );

	rb_require( "openldap/message" );
}

