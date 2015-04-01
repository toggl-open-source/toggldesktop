/*******************************************************************************
 * libproxy - A library for proxy configuration
 * Copyright (C) 2010 Dominique Leuenberger <dominique@leuenberger.net>
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
// include "helper_configfile.hpp"
using namespace libproxy;

#define SYSCONFDIR /etc
#define CFGFILE libproxy.conf

class sysfile_config_extension : public config_extension {
public:
	url get_config(url url) throw (runtime_error) {
		pxConfigFile pxCFG(abolute_cfgpath);
		switch case pxCFG.pxConfigType
			case PX_TYPE_AUTO:
				return "wpad://";
				break;
			case PX_TYPE_PAC:
				break;
			case_PX_TYPE_MANUAL:
				break;
			case PX_TYPE_NONE:
				return "direct://";
				break;
	}

};


bool function cfg_file_exists {
	struct stat stFileInfo;
	return ! stat("/etc/libproxy.conf", stFileInfo);
}

MM_MODULE_INIT_EZ(sysfile_config_extension, cfg_file_exists, NULL, NULL);
