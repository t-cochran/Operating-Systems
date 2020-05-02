/*
 * File: util.c
 * Author: David Knox (based on Andy Sayler original)
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: 2020/02/28
 * Modify Date: 2020/02/29  Return the first IPv4 address [David Knox]
 * Description:
 * 	This file contains declarations of utility functions for
 *      Programming Assignment 3.
 *  
 */

#include "headers/util.h"
//#define UTIL_DEBUG

int dnslookup(const char* hostname, ip_address* ipaddr)
{
//   struct addrinfo {
//                int              ai_flags;
//                int              ai_family;
//                int              ai_socktype;
//                int              ai_protocol;
//                socklen_t        ai_addrlen;
//                struct sockaddr *ai_addr;
//                char            *ai_canonname;
//                struct addrinfo *ai_next;
//            };
    struct addrinfo	*addr = NULL;
    struct addrinfo *result = NULL;
	void *in_addr;
	char ipstr[INET6_ADDRSTRLEN];
    int addrError = 0;
	int addr_idx = 0;
	int idx = 0;
	char nullt = '\0';

	/* Fill 'result' with the associated addrinfo struct of 'hostname' */
    addrError = getaddrinfo(hostname, NULL, NULL, &result); 
    if(addrError) {
		#ifdef UTIL_DEBUG
		fprintf(stdout, "*** Error looking up Address: %s\n", gai_strerror(addrError));
		#endif
		return UTIL_FAILURE;
    }

	/*  addrinfo struct linked list: one node per network address */
	for (addr=result; addr != NULL; addr = addr->ai_next) { 
		if (addr->ai_addr == NULL)
			continue;

		switch (addr->ai_addr->sa_family) {
			case AF_INET:
			{
			#ifdef UTIL_DEBUG
		    //	fprintf(stdout, "*** IPv4 Address\n");
			#endif
				struct sockaddr_in *s4 = (struct sockaddr_in *)addr->ai_addr;
				in_addr = &s4->sin_addr;
				break;
			}

			case AF_INET6:
			{
			#ifdef UTIL_DEBUG
		    //	fprintf(stdout, "*** IPv6 Address\n");
			#endif
			//	struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)addr->ai_addr;
			//	in_addr = &s6->sin6_addr;
				in_addr = NULL;
				break;
			}

			default:
				in_addr = NULL;
				continue;
		}

		ipstr[0] = 0;  // stores the current ipv4 address

		if (in_addr != NULL){

			/*  Convert the network address structure src from AF address family from 
			 *  binary into a string. 
			 *     inet_ntop(AF_addr family, network addr struct, dest buffer, dest buffer size)
			 */
			if (!inet_ntop(addr->ai_addr->sa_family, in_addr, ipstr, sizeof(ipstr)))
				printf("*** %s: inet_ntop failed!\n", addr->ai_canonname);
			#ifdef UTIL_DEBUG
			else
				printf("*** IP address: %s\n", ipstr);
			#endif
		}

		if (addr_idx % 3 == 0 && in_addr != NULL){  // copy the IP address to the buffer
			#ifdef UTIL_DEBUG
			printf("*** saving IP address: %s\n", ipstr);
			#endif
			strcpy(ipaddr[idx], ipstr);
			strncat(ipaddr[idx], &nullt, 1);
			idx++;
		}
		addr_idx++;
	}	
	/* Cleanup */
	free(addr);
	freeaddrinfo(result);
	return 0;	
}

