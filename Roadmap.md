# ruby-openldap Function Roadmap


ldap_initialize(3)  			initialize the LDAP library without opening a connection to a server
ldap_result(3)      			wait for the result from an asynchronous operation
ldap_abandon_ext(3) 			abandon (abort) an asynchronous operation
ldap_add_ext(3)     			asynchronously add an entry
ldap_add_ext_s(3)   			synchronously add an entry
ldap_sasl_bind(3)   			asynchronously bind to the directory
ldap_sasl_bind_s(3) 			synchronously bind to the directory
ldap_unbind_ext(3)  			synchronously unbind from the LDAP server and close the connection
    ldap_unbind(3)          		
ldap_unbind_s(3)				equivalent to ldap_unbind_ext(3)
ldap_memfree(3)     			dispose of memory allocated by LDAP routines.
ldap_compare_ext(3) 			asynchronously compare to a directory entry
ldap_compare_ext_s(3)			synchronously compare to a directory entry
ldap_delete_ext(3)  			asynchronously delete an entry
ldap_delete_ext_s(3)			synchronously delete an entry
ld_errno(3)         			LDAP error indication
ldap_errlist(3)     			list of LDAP errors and their meanings
ldap_err2string(3)  			convert LDAP error indication to a string
ldap_extended_operation(3)		asynchronously perform an arbitrary extended operation
ldap_extended_operation_s(3)	synchronously perform an arbitrary extended operation
ldap_first_attribute(3)			return first attribute name in an entry
ldap_next_attribute(3)			return next attribute name in an entry
ldap_first_entry(3) 			return first entry in a chain of search results
ldap_next_entry(3)  			return next entry in a chain of search results
ldap_count_entries(3)			return number of entries in a search result
ldap_get_dn(3)      			extract the DN from an entry
ldap_get_values_len(3)			return an attribute's values with lengths
ldap_value_free_len(3)			free memory allocated by ldap_get_values_len(3)
ldap_count_values_len(3)		return number of values
ldap_modify_ext(3)  			asynchronously modify an entry
ldap_modify_ext_s(3)			synchronously modify an entry
ldap_mods_free(3)   			free array of pointers to mod structures used by ldap_modify_ext(3)
ldap_rename(3)      			asynchronously rename an entry
ldap_rename_s(3)    			synchronously rename an entry
ldap_msgfree(3)     			free results allocated by ldap_result(3)
ldap_msgtype(3)     			return the message type of a message from ldap_result(3)
ldap_msgid(3)       			return the message id of a message from ldap_result(3)
ldap_search_ext(3)  			asynchronously search the directory
ldap_search_ext_s(3)			synchronously search the directory
ldap_is_ldap_url(3) 			check a URL string to see if it is an LDAP URL
ldap_url_parse(3)   			break up an LDAP URL string into its components
ldap_sort_entries(3)			sort a list of search results
ldap_sort_values(3) 			sort a list of attribute values
ldap_sort_strcasecmp(3)			case insensitive string comparison


OpenLDAP::Connection

OpenLDAP::Entry

OpenLDAP::Result

