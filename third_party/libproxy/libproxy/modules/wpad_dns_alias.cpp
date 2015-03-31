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

#include "../extension_wpad.hpp"
using namespace libproxy;

class dns_alias_wpad_extension : public wpad_extension {
public:
	dns_alias_wpad_extension() : lasturl(NULL), lastpac(NULL) { }
	bool found() { return lastpac != NULL; }
	
	void rewind() {
		if (lasturl) delete lasturl;
		if (lastpac) delete lastpac;
		lasturl = NULL;
		lastpac = NULL;
	}

	url* next(char** pac) {
		if (lasturl) return NULL;

		lasturl = new url("http://wpad/wpad.dat");
		lastpac = *pac = lasturl->get_pac();
		if (!lastpac) {
		    delete lasturl;
		    return NULL;
		}

		return lasturl;
	}

private:
	url*  lasturl;
	char* lastpac;
};

MM_MODULE_INIT_EZ(dns_alias_wpad_extension, true, NULL, NULL);
