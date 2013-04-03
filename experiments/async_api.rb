# -*- ruby -*-
#encoding: utf-8

require 'openldap'

# Split API
# Each function has two variants: synchronous and async:

result = conn.search( base_dn, filter, scope )
result = conn.search_async( base_dn, filter, scope )

# Single API
# Each function can be used asynchronously by passing a block:

result = conn.search( base_dn, filter, scope )
conn.search( base_dn, filter, scope ) {|result| ... }

# Single API, no async
result = conn.search( base_dn, filter, scope )

thr = Thread.new { conn.search( base_dn, filter, scope ) }
result = thr.value
