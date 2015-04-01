/*******************************************************************************
 * Copyright (C) 2009 Nathaniel McCallum <nathaniel@natemccallum.com>
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

#include "../extension_config.hpp"
using namespace libproxy;

#define W32REG_OFFSET_PAC  (1 << 2)
#define W32REG_OFFSET_WPAD (1 << 3)
#define W32REG_BASEKEY "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"
#define W32REG_BUFFLEN 1024

static bool get_registry(const char *key, const char *name, char **sval, uint32_t *slen, uint32_t *ival) {
	HKEY  hkey;
	LONG  result;
	DWORD type;
	DWORD buflen = W32REG_BUFFLEN;
	BYTE  buffer[W32REG_BUFFLEN];

	// Don't allow the caller to specify both sval and ival
	if (sval && ival)
		return false;

	// Open the key
	if (RegOpenKeyExA(HKEY_CURRENT_USER, key, 0, KEY_READ, &hkey) != ERROR_SUCCESS)
		return false;

	// Read the value
	result = RegQueryValueExA(hkey, name, NULL, &type, buffer, &buflen);

	// Close the key
	RegCloseKey(hkey);

	// Evaluate
	if (result != ERROR_SUCCESS)
		return false;
	switch (type)
	{
		case REG_BINARY:
		case REG_EXPAND_SZ:
		case REG_SZ:
			if (!sval) return false;
			if (slen) *slen = buflen;
			*sval = new char[buflen];
			return memcpy(*sval, buffer, buflen) != NULL;
		case REG_DWORD:
			if (ival) return memcpy(ival, buffer, buflen < sizeof(uint32_t) ? buflen : sizeof(uint32_t)) != NULL;
	}
	return false;
}

static bool is_enabled(uint8_t type) {
	char    *data = NULL;
	uint32_t dlen = 0;
	bool   result = false;

	// Get the binary value DefaultConnectionSettings
	if (!get_registry(W32REG_BASEKEY "\\Connections", "DefaultConnectionSettings", &data, &dlen, NULL))
		return false;

	// WPAD and PAC are contained in the 9th value
	if (dlen >= 9)
		result = (data[8] & type) == type; // Check to see if the bit is set

	delete data;
	return result;
}

static map<string, string> parse_manual(string data) {
	// ProxyServer comes in two formats:
	//   1.2.3.4:8080 or ftp=1.2.3.4:8080;https=1.2.3.4:8080...
	map<string, string> rval;

	// If we have the second format, do recursive parsing,
	// then handle just the first entry
	if (data.find(";") != string::npos) {
		rval = parse_manual(data.substr(data.find(";")+1));
		data = data.substr(0, data.find(";"));
	}

	// If we have the first format, just assign HTTP and we're done
	if (data.find("=") == string::npos) {
		rval["http"] = string("http://") + data;
		return rval;
	}

	// Otherwise set the value for this single entry and return
	string protocol = data.substr(0, data.find("="));
	try { rval[protocol] = url(protocol + "://" + data.substr(data.find("=")+1)).to_string(); }
	catch (parse_error&) {}

	return rval;
}

class w32reg_config_extension : public config_extension {
public:
	vector<url> get_config(const url &dst) throw (runtime_error) {
		char        *tmp = NULL;
		uint32_t enabled = 0;
		vector<url> response;

		// WPAD
		if (is_enabled(W32REG_OFFSET_WPAD)) {
			response.push_back(url("wpad://"));
			return response;
		}

		// PAC
		if (is_enabled(W32REG_OFFSET_PAC) &&
			get_registry(W32REG_BASEKEY, "AutoConfigURL", &tmp, NULL, NULL) &&
			url::is_valid(string("pac+") + tmp)) {
			response.push_back(url(string("pac+") + tmp));
			delete tmp;
			return response;
		}

		// Manual proxy
		// Check to see if we are enabled and get the value of ProxyServer
		if (get_registry(W32REG_BASEKEY, "ProxyEnable", NULL, NULL, &enabled) && enabled &&
			get_registry(W32REG_BASEKEY, "ProxyServer", &tmp, NULL, NULL)) {
			map<string, string> manual = parse_manual(tmp);
			delete tmp;

			// First we look for an exact match
			if (manual.find(dst.get_scheme()) != manual.end())
				response.push_back(manual[dst.get_scheme()]);

			// Next we look for http
			else if (manual.find("http") != manual.end())
				response.push_back(manual["http"]);

			// Last we look for socks
			else if (manual.find("socks") != manual.end())
				response.push_back(manual["socks"]);

			return response;
		}

		// Direct
		response.push_back(url("direct://"));
		return response;
	}

	string get_ignore(const url &dst) {
		char *tmp;
		if (get_registry(W32REG_BASEKEY, "ProxyOverride", &tmp, NULL, NULL)) {
			string po = tmp;
			delete tmp;
			if (po == "<local>")
				return po;
		}
		return "";
	}
};

MM_MODULE_INIT_EZ(w32reg_config_extension, true, NULL, NULL);
