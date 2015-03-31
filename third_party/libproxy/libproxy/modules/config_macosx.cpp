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

#include <sstream>

#include "../extension_config.hpp"
using namespace libproxy;

#include <SystemConfiguration/SystemConfiguration.h>

class str : public string {
public:
	str(CFStringRef s) : string() {
		if (!s) return;
		const char* tmp = CFStringGetCStringPtr(s, CFStringGetFastestEncoding(s));
		*this += tmp ? tmp : "";
	}

	str(CFArrayRef a) : string() {
		if (!a) return;
		for (CFIndex i=0 ; i < CFArrayGetCount(a) ; i++) {
			CFStringRef s = (CFStringRef) CFArrayGetValueAtIndex(a, i);
			*this += str(s);
			if (i+1 < CFArrayGetCount(a))
				*this += ",";
		}
	}
};

template <class T>
static T getobj(CFDictionaryRef settings, string key) {
	if (!settings) return NULL;
	CFStringRef k = CFStringCreateWithCString(NULL, key.c_str(), kCFStringEncodingMacRoman);
	if (!k) return NULL;
	T retval = (T) CFDictionaryGetValue(settings, k);
	CFRelease(k);
	return retval;
}

static bool getint(CFDictionaryRef settings, string key, int64_t& answer) {
	CFNumberRef n = getobj<CFNumberRef>(settings, key);
	if (!n) return false;
	if (!CFNumberGetValue(n, kCFNumberSInt64Type, &answer))
		return false;
	return true;
}

static bool getbool(CFDictionaryRef settings, string key, bool dflt=false) {
	int64_t i;
	if (!getint(settings, key, i)) return dflt;
	return i != 0;
}

static bool protocol_url(CFDictionaryRef settings, string protocol, string& config) {
	int64_t port;
	string  host;

	// Check ProtocolEnabled
	if (!getbool(settings, protocol + "Enable"))
		return false;

	// Get ProtocolPort
	if (!getint(settings, protocol + "Port", port))
		return false;

	// Get ProtocolProxy
	if ((host = str(getobj<CFStringRef>(settings, protocol + "Proxy"))) == "")
		return false;

	stringstream ss;
	if (protocol == "HTTP" || protocol == "HTTPS" || protocol == "FTP" || protocol == "Gopher")
		ss << "http://";
	else if (protocol == "RTSP")
		ss << "rtsp://";
	else if (protocol == "SOCKS")
		ss << "socks://";
	else
		return false;
	ss << host;
	ss << ":";
	ss << port;

	config = ss.str();
	return true;
}

static string toupper(string str) {
	string tmp;
	for (unsigned int i=0 ; str.c_str()[i] ; i++)
		tmp += toupper(str.c_str()[i]);
	return tmp;
}

static string capitalize(string str) {
	char c = toupper(str.c_str()[0]);
	return string(&c, 1) + str.substr(1);
}

class macosx_config_extension : public config_extension {
public:
	vector<url> get_config(const url &url) throw (runtime_error) {
		string tmp;
		CFDictionaryRef proxies = SCDynamicStoreCopyProxies(NULL);
		vector<url> response;

		if (!proxies) throw runtime_error("Unable to fetch proxy configuration");

		// wpad://
		if (getbool(proxies, "ProxyAutoDiscoveryEnable")) {
			CFRelease(proxies);
			response.push_back(url("wpad://"));
		}

		// pac+http://...
		else if (getbool(proxies, "ProxyAutoConfigEnable") &&
		    (tmp = str(getobj<CFStringRef>(proxies, "ProxyAutoConfigURLString"))) != "" &&
        	    url::is_valid(tmp)) {
			CFRelease(proxies);
			response.push_back(url(string("pac+") + tmp));
		}

		// http:// or socks:// (TODO: gopher:// and rtsp:// ???)
		else if ((protocol_url(proxies, toupper(url.get_scheme()), tmp)    && url::is_valid(tmp)) ||
		    (protocol_url(proxies, capitalize(url.get_scheme()), tmp) && url::is_valid(tmp)) ||
		    (protocol_url(proxies, toupper("http"), tmp)              && url::is_valid(tmp)) ||
	            (protocol_url(proxies, toupper("socks"), tmp)             && url::is_valid(tmp))) {
			CFRelease(proxies);
			response.push_back(url(tmp));
		}
		else {
			// direct://
			CFRelease(proxies);
			response.push_back(url("direct://"));
		}

		return response;
	}

	string get_ignore(const url&) {
		// Get config dict
		CFDictionaryRef proxies = SCDynamicStoreCopyProxies(NULL);
		if (!proxies) return "";

		// Get ignores
		string tmp = str(getobj<CFArrayRef>(proxies, "ExceptionsList"));
		if (getbool(proxies, "ExcludeSimpleHostnames"))
			tmp += (tmp == "" ? string("") : string(",")) + "<local>";

		CFRelease(proxies);
		return tmp;
	}
};

MM_MODULE_INIT_EZ(macosx_config_extension, true, NULL, NULL);

