#!/usr/bin/env ruby

require 'mkmf'
require 'fileutils'

if ENV['MAINTAINER_MODE']
	$stderr.puts "** Maintainer mode enabled. **"
	$CFLAGS << ' -Wall' << ' -ggdb' << ' -DDEBUG'
end


dir_config( 'openldap' )

find_header( 'ldap.h' ) or
	abort( "missing ldap.h; do you need to install a developer package?" )
find_library( 'ldap', 'ldap_initialize' ) or
	abort( "Could not find LDAP library (http://openldap.org/)." )
have_const( 'LDAP_API_VERSION', 'ldap.h' ) or
	abort "no LDAP_API_VERSION constant defined"

create_makefile( 'openldap_ext' )
