/*******************************************************************************
 * libproxy - A library for proxy configuration
 * Copyright (C) 2006 Nathaniel McCallum <nathaniel@natemccallum.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 ******************************************************************************/

#include <cstdio>
#include <cstring>

#include "../extension_ignore.hpp"
using namespace libproxy;

static inline bool
sockaddr_equals(const struct sockaddr *ip_a, const struct sockaddr *ip_b, const struct sockaddr *nm)
{
	if (!ip_a || !ip_b) return false;
	if (ip_a->sa_family != ip_b->sa_family) return false;
	if (nm && ip_a->sa_family != nm->sa_family) return false;

	/* Setup the arrays */
	uint8_t bytes = 0, *a_data = NULL, *b_data = NULL, *nm_data = NULL;
	if (ip_a->sa_family == AF_INET)
	{
		bytes   = 32 / 8;
		a_data  = (uint8_t *) &((struct sockaddr_in *) ip_a)->sin_addr;
		b_data  = (uint8_t *) &((struct sockaddr_in *) ip_b)->sin_addr;
		nm_data = nm ? (uint8_t *) &((struct sockaddr_in *) nm)->sin_addr : NULL;
	}
	else if (ip_a->sa_family == AF_INET6)
	{
		bytes   = 128 / 8;
		a_data  = (uint8_t *) &((struct sockaddr_in6 *) ip_a)->sin6_addr;
		b_data  = (uint8_t *) &((struct sockaddr_in6 *) ip_b)->sin6_addr;
		nm_data = nm ? (uint8_t *) &((struct sockaddr_in6 *) nm)->sin6_addr : NULL;
	}
	else
		return false;

	for (int i=0 ; i < bytes ; i++)
	{
		if (nm && (a_data[i] & nm_data[i]) != (b_data[i] & nm_data[i]))
			return false;
		else if (!nm && (a_data[i] != b_data[i]))
			return false;
	}
	return true;
}

static inline sockaddr *
sockaddr_from_string(string ip)
{
	struct sockaddr *result = NULL;

	/* Try to parse */
	struct addrinfo *info = NULL;
	struct addrinfo flags;
	flags.ai_family = AF_UNSPEC;
	flags.ai_socktype = 0;
	flags.ai_protocol = 0;
	flags.ai_flags = AI_NUMERICHOST;
	if (getaddrinfo(ip.c_str(), NULL, &flags, &info) != 0 || !info) return result;

	/* Copy the results into our buffer */
	result = (sockaddr *) new char[info->ai_addrlen];
	if (!result) {
		freeaddrinfo(info);
		return result;
	}
	memcpy(result, info->ai_addr, info->ai_addrlen);
	freeaddrinfo(info);
	return result;
}

static inline sockaddr *
sockaddr_from_cidr(sa_family_t af, uint8_t cidr)
{
	/* IPv4 */
	if (af == AF_INET)
	{
		sockaddr_in *mask = (sockaddr_in*) new char[sizeof(sockaddr_in)];
		mask->sin_family = af;
		mask->sin_addr.s_addr = htonl(~0 << (32 - (cidr > 32 ? 32 : cidr)));

		return (struct sockaddr *) mask;
	}

	/* IPv6 */
	else if (af == AF_INET6)
	{
		sockaddr_in6 *mask = (sockaddr_in6*) new char[sizeof(sockaddr_in6)];
		mask->sin6_family = af;
		for (uint8_t i=0 ; i < sizeof(mask->sin6_addr) ; i++)
			mask->sin6_addr.s6_addr[i] = ~0 << (8 - (8*i > cidr ? 0 : cidr-8*i < 8 ? cidr-8*i : 8) );

		return (sockaddr *) mask;
	}

	return NULL;
}

class ip_ignore_extension : public ignore_extension {
public:
	virtual bool ignore(url& url, const string &ignore) {
		bool result   = false;
		uint16_t port = 0;
		const struct sockaddr *dst_ip = url.get_ips(false) ? url.get_ips(false)[0] : NULL;
		struct sockaddr *ign_ip   = NULL, *net_ip = NULL;

		/*
		 * IPv4
		 * IPv6
		 */
		if ((ign_ip = sockaddr_from_string(ignore)))
			goto out;

		/*
		 * IPv4/CIDR
		 * IPv4/IPv4
		 * IPv6/CIDR
		 * IPv6/IPv6
		 */
		if (ignore.find('/') != string::npos)
		{
			ign_ip = sockaddr_from_string(ignore.substr(0, ignore.find('/')));

			uint32_t cidr = 0;
			string mask = ignore.substr(ignore.find('/') + 1);

			if (mask.find('.') != string::npos)
			{
				/* A dotted netmask was used */
				net_ip = sockaddr_from_string(mask);
			}
			else
			{
				/* If CIDR notation was used, get the netmask */
				if (sscanf(mask.c_str(), "%d", &cidr) == 1)
					net_ip = sockaddr_from_cidr(ign_ip->sa_family, cidr);
			}

			if (ign_ip && net_ip && ign_ip->sa_family == net_ip->sa_family)
				goto out;

			delete[] ign_ip;
			delete[] net_ip;
			ign_ip = NULL;
			net_ip = NULL;
		}

		/*
		 * IPv4:port
		 * [IPv6]:port
		 */
		if (ignore.rfind(':') != string::npos && sscanf(ignore.substr(ignore.rfind(':')).c_str(), ":%hu", &port) == 1 && port > 0)
		{
			ign_ip = sockaddr_from_string(ignore.substr(ignore.rfind(':')).c_str());

			/* Make sure this really is just a port and not just an IPv6 address */
			if (ign_ip && (ign_ip->sa_family != AF_INET6 || ignore[0] == '['))
				goto out;

			delete[] ign_ip;
			ign_ip = NULL;
			port   = 0;
		}

	out:
		result = sockaddr_equals(dst_ip, ign_ip, net_ip);
		delete[] ign_ip;
		delete[] net_ip;
		return port != 0 ? (port == url.get_port() && result) : result;
	}
};

MM_MODULE_INIT_EZ(ip_ignore_extension, true, NULL, NULL);
