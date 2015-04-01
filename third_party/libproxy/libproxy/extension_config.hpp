/*******************************************************************************
 * libproxy - A library for proxy configuration
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

#ifndef EXTENSION_CONFIG_HPP_
#define EXTENSION_CONFIG_HPP_

#include <libmodman/module.hpp>
#include "url.hpp"

namespace libproxy {
using namespace std;
using namespace libmodman;

// Config module
class DLL_PUBLIC config_extension : public extension<config_extension> {
public:
	// Abstract methods
	virtual vector<url>      get_config(const url &dst) throw (runtime_error)=0;

	// Virtual methods
	virtual string   get_ignore(const url &dst);
	virtual bool     set_creds(const url &proxy, const string &username, const string &password);

	// Final methods
	        bool     get_valid();
	        void     set_valid(bool valid);
	virtual bool     operator<(const base_extension&) const;

private:
	bool valid;
};

}

#endif /* EXTENSION_CONFIG_HPP_ */
