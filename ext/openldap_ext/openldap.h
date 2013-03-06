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

#include "extconf.h"

/* --------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------- */

/* Reference to the URI module */
extern VALUE ropenldap_rbmURI;

extern VALUE ropenldap_mOpenLDAP;
extern VALUE ropenldap_mOpenLDAPLoggable;

extern VALUE ropenldap_cOpenLDAPConnection;

extern VALUE ropenldap_eOpenLDAPError;


/* --------------------------------------------------------------
 * Typedefs
 * -------------------------------------------------------------- */

/* OpenLDAP::Connection struct */
struct ropenldap_connection {
    LDAP    *ldap;
    VALUE   connection;
};


/* --------------------------------------------------------------
 * Macros
 * -------------------------------------------------------------- */
#define IsConnection( obj ) rb_obj_is_kind_of( (obj), ropenldap_cOpenLDAPConnection )

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif


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


#endif /* __OPENLDAP_H__ */

