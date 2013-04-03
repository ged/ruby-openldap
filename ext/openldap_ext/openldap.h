/*
 * Header for openldap.c
 */

#ifndef __OPENLDAP_H__
#define __OPENLDAP_H__

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include <ldap.h>

#include <ruby.h>
#include <ruby/thread.h>

#include "extconf.h"

/* --------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------- */

/* Reference to the URI module */
extern VALUE ropenldap_rbmURI;

extern VALUE ropenldap_mOpenLDAP;

extern VALUE ropenldap_cOpenLDAPConnection;
extern VALUE ropenldap_cOpenLDAPResult;

extern VALUE ropenldap_eOpenLDAPError;


/* --------------------------------------------------------------
 * Typedefs
 * -------------------------------------------------------------- */

/* OpenLDAP::Connection struct */
struct ropenldap_connection {
    LDAP    *ldap;
};

/* OpenLDAP::Result struct */
struct ropenldap_result {
	LDAP        *ldap;
	int         msgid;
	LDAPMessage *message;
	VALUE       connection;
	VALUE       abandoned;
};

/* --------------------------------------------------------------
 * Macros
 * -------------------------------------------------------------- */
#define IsConnection( obj ) rb_obj_is_kind_of( (obj), ropenldap_cOpenLDAPConnection )
#define IsResult( obj ) rb_obj_is_kind_of( (obj), ropenldap_cOpenLDAPResult )

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif


// BER stuff stolen from OpenLDAP's lber_pvt.h
#define BER_BVC(s)		{ STRLENOF(s), (char *)(s) }
#define BER_BVNULL		{ 0L, NULL }
#define BER_BVZERO(bv) \
	do { \
		(bv)->bv_len = 0; \
		(bv)->bv_val = NULL; \
	} while (0)
#define BER_BVSTR(bv,s)	\
	do { \
		(bv)->bv_len = STRLENOF(s); \
		(bv)->bv_val = (s); \
	} while (0)
#define BER_BVISNULL(bv)	((bv)->bv_val == NULL)
#define BER_BVISEMPTY(bv)	((bv)->bv_len == 0)


/* --------------------------------------------------------------
 * Declarations
 * -------------------------------------------------------------- */

#ifdef HAVE_STDARG_PROTOTYPES
#include <stdarg.h>
#define va_init_list(a,b) va_start(a,b)
void ropenldap_log_obj( VALUE, const char *, const char *, ... );
void ropenldap_log( const char *, const char *, ... );
void ropenldap_check_result( int, const char *, ... );
#else
#include <varargs.h>
#define va_init_list(a,b) va_start(a)
void ropenldap_log_obj( VALUE, const char *, const char *, va_dcl );
void ropenldap_log( const char *, const char *, va_dcl );
void ropenldap_check_result( int, va_dcl );
#endif

VALUE ropenldap_rb_string_array         _(( char ** ));


/* --------------------------------------------------------------
 * Initializers
 * -------------------------------------------------------------- */

void Init_openldap_ext                  _(( void ));
void ropenldap_init_connection          _(( void ));
void ropenldap_init_result              _(( void ));

struct ropenldap_connection *ropenldap_get_conn _(( VALUE ));


#endif /* __OPENLDAP_H__ */

