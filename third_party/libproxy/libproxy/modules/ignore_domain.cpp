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

#include "../extension_ignore.hpp"
using namespace libproxy;

class domain_ignore_extension : public ignore_extension {
public:
	virtual bool ignore(url& url, const string &ignorestr) {
		/* Get our URL's hostname and port */
		string host = url.get_host();
		int    port = url.get_port();

		/* Get our ignore pattern's hostname and port */
		string ihost = ignorestr;
		int    iport = 0;
		if (ihost.find(':') != string::npos) {
				if (sscanf(ignorestr.substr(ihost.find(':')+1).c_str(), "%d", &iport) == 1)
						ihost = ihost.substr(0, ihost.find(':'));
				else
						iport = 0;
		}

		/* Hostname match (domain.com or domain.com:80) */
		if (host == ihost)
			return (iport == 0 || port == iport);

		/* Endswith (.domain.com or .domain.com:80) */
		if (ihost[0] == '.' && host.find(ihost) == host.size() - ihost.size() && host.size() >= ihost.size())
			return (iport == 0 || port == iport);

		/* Glob (*.domain.com or *.domain.com:80) */
		if (ihost[0] == '*' && host.find(ihost.substr(1)) == host.size() - ihost.substr(1).size() && host.size() >= ihost.substr(1).size()) 
			return (iport == 0 || port == iport);

		/* No match was found */
		return false;
	}
};

MM_MODULE_INIT_EZ(domain_ignore_extension, true, NULL, NULL);
