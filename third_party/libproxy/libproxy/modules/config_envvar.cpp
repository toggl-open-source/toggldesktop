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

#include <cstdlib>

#include "../extension_config.hpp"
using namespace libproxy;

class envvar_config_extension : public config_extension {
public:
	vector<url> get_config(const url &dst) throw (runtime_error) {
		const char *proxy = NULL;
                vector<url> response;

		// If the URL is an ftp url, try to read the ftp proxy
		if (dst.get_scheme() == "ftp") {
			if (!(proxy = getenv("ftp_proxy")))
				proxy = getenv("FTP_PROXY");
		}

		// If the URL is an https url, try to read the https proxy
		if (dst.get_scheme() == "https") {
			if (!(proxy = getenv("https_proxy")))
				proxy = getenv("HTTPS_PROXY");
		}

		// If the URL is not ftp or no ftp_proxy was found, get the http_proxy
		if (!proxy) {
			if (!(proxy = getenv("http_proxy")))
				proxy = getenv("HTTP_PROXY");
		}

		if (!proxy)
			throw runtime_error("Unable to read configuration");

                response.push_back(url(proxy));
		return response;
	}

	string get_ignore(const url&) {
		char *ignore = getenv("no_proxy");
		      ignore = ignore ? ignore : getenv("NO_PROXY");
		return string(ignore ? ignore : "");
	}

	// Make sure that envvar is pushed to the back behind all other config extensions
	virtual bool operator<(const base_extension&) const {
		return false;
	}
};

MM_MODULE_INIT_EZ(envvar_config_extension, true, NULL, NULL);
