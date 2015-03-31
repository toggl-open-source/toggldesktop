/*******************************************************************************
 * libproxy sysconfig module
 * Copyright (C) 2010 Novell Inc.
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

#include <sys/stat.h>
#include <cstdlib>
#include <map>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>


#include "../extension_config.hpp"
using namespace libproxy;
using std::map;

enum Trim {
    NO_TRIM = 0x00,
    L_TRIM  = 0x01,
    R_TRIM  = 0x02,
    TRIM    = (L_TRIM|R_TRIM)
};

static std::string trim( const std::string & s, const Trim trim_r = TRIM ) {

	if (s.empty() || trim_r == NO_TRIM)
		return s;

	std::string ret(s);
    
	if (trim_r & L_TRIM) {
 		std::string::size_type p = ret.find_first_not_of(" \t\n");
		if (p == std::string::npos)
			return std::string();
        
		ret = ret.substr(p);
	}
    
	if (trim_r & R_TRIM) {
		std::string::size_type p = ret.find_last_not_of(" \t\n");
		if (p == std::string::npos)
			return std::string();
        
		ret = ret.substr(0, p+1);
	}
    
	return ret;
}

static map<string,string> sysconfig_read(const string &_path)	{
    
	map<string,string> ret;  
	string line;

	ifstream in(_path.c_str());
	if (in.fail()) {
		return ret;
	}

	while(getline( in, line)) {
		
		if (*line.begin() != '#') {
			
			string::size_type pos = line.find('=', 0);
            
			if (pos != string::npos) {

 				string key = trim(line.substr(0, pos));
 				string value = trim(line.substr(pos + 1, line.length() - pos - 1));

				if (value.length() >= 2
				    && *(value.begin()) == '"'
				    && *(value.rbegin()) == '"') {
					value = value.substr( 1, value.length() - 2 );
				}
                
				if (value.length() >= 2
				    && *(value.begin()) == '\''
 				    && *(value.rbegin()) == '\'' ) {
					value = value.substr( 1, value.length() - 2 );
				}
				ret[key] = value;
			} // '=' found

		} // not comment

	} // while getline
	return ret;
}

static bool should_use_sysconfig()
{
	struct stat st;
	if (stat("/etc/sysconfig", &st) == 0)
		return (getuid() == 0);
	return false;
}

class sysconfig_config_extension : public config_extension {

	map<string,string> _data;
    
public:
	sysconfig_config_extension()
		: _data(sysconfig_read("/etc/sysconfig/proxy")) {
            
	}

	~sysconfig_config_extension() {
	}
   
	vector<url> get_config(const url &dst) throw (runtime_error) {
		map<string,string>::const_iterator it = _data.find("PROXY_ENABLED");
		vector<url> response;

		if (it != _data.end() && it->second == "no") {
			response.push_back(url("direct://"));
			return response;
		}
            
		string key;
		string proxy;
            
		// If the URL is an ftp url, try to read the ftp proxy
		if (dst.get_scheme() == "ftp")
			key = "FTP_PROXY";
		else if (dst.get_scheme() == "http")
			key = "HTTP_PROXY";
		else if (dst.get_scheme() == "https")
			key = "HTTPS_PROXY";
            
		it = _data.find(key);
		if (it != _data.end())
			proxy = it->second;

		if (proxy.empty())
			throw runtime_error("Unable to read configuration");

		response.push_back(url(proxy));
		return response;
	}

	string get_ignore(const url&) {
		map<string,string>::const_iterator it = _data.find("NO_PROXY");
		if (it != _data.end())
			return it->second;
		return "";
        }
    
	// if we are running as root, and the module is used, then 
	// make sure this is the first method tried
	virtual bool operator<(const base_extension&) const {
		return true;
	}
};

MM_MODULE_INIT_EZ(sysconfig_config_extension, should_use_sysconfig(), NULL, NULL);

