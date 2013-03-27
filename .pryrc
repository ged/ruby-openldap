# -*- ruby -*-

puts ">>> Adding 'lib' to load path..."
$LOAD_PATH.unshift( "lib", "ext" )

# Try to require the 'openldap' library
begin
	$stderr.puts "Loading OpenLDAP..."
	require 'openldap'
rescue => e
	$stderr.puts "Ack! OpenLDAP failed to load: #{e.message}\n\t" +
		e.backtrace.join( "\n\t" )
end


