# -*- ruby -*-
#encoding: utf-8

require 'openldap'

BIND_DN   = 'cn=admin,dc=example,dc=com'
BIND_PASS = 'the_password'


conn = OpenLDAP.connect( 'ldap://example.com' )

# Anonymous bind
conn.bind

# Simple bind
conn.bind( BIND_DN, BIND_PASS )

# SASL bind
