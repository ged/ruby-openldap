#!/usr/bin/ruby -*- ruby -*-

BEGIN {
	require 'rbconfig'
	require 'pathname'
	basedir = Pathname.new( __FILE__ ).dirname.expand_path
	libdir = basedir + "lib"
	extdir = libdir + Config::CONFIG['sitearch']

	puts ">>> Adding #{libdir} to load path..."
	$LOAD_PATH.unshift( libdir.to_s )

	puts ">>> Adding #{extdir} to load path..."
	$LOAD_PATH.unshift( extdir.to_s )
}


# Try to require the 'openldap' library
begin
	$stderr.puts "Loading OpenLDAP..."
	require 'openldap'
rescue => e
	$stderr.puts "Ack! OpenLDAP library failed to load: #{e.message}\n\t" +
		e.backtrace.join( "\n\t" )
end

