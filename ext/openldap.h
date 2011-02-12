/* 
 * Header for openldap.c
 */

#ifndef __OPENLDAP_H__
#define __OPENLDAP_H__

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <ldap.h>

#include <ruby.h>


/* --------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------- */

extern VALUE ropenldap_mOpenLDAP;

extern VALUE ropenldap_cOpenLDAPConnection;


/* --------------------------------------------------------------
 * Typedefs
 * -------------------------------------------------------------- */

/* OpenLDAP::Connection struct */
struct ropenldap_connection {
	LDAP	*ldap;
	VALUE	connection;
};


/* --------------------------------------------------------------
 * Macros
 * -------------------------------------------------------------- */
#define IsConnection( obj ) rb_obj_is_kind_of( (obj), ropenldap_cOpenLDAPConnection )


/* --------------------------------------------------------------
 * Declarations
 * -------------------------------------------------------------- */


/* --------------------------------------------------------------
 * Initializers
 * -------------------------------------------------------------- */

void Init_openldap_ext( void );


#endif /* __OPENLDAP_H__ */

