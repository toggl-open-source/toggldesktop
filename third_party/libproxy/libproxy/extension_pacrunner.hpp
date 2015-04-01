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

#ifndef EXTENSION_PACRUNNER_HPP_
#define EXTENSION_PACRUNNER_HPP_

#include <libmodman/module.hpp>
#include "url.hpp"

#define PX_PACRUNNER_MODULE_EZ(name, symb, smod) \
	class name ## _pacrunner_extension : public pacrunner_extension { \
	protected: \
		virtual pacrunner* create(string pac, const url& pacurl) throw (bad_alloc) { \
			return new name ## _pacrunner(pac, pacurl); \
		} \
	}; \
	MM_MODULE_INIT_EZ(name ## _pacrunner_extension, true, symb, smod)

namespace libproxy {
using namespace std;
using namespace libmodman;

// PACRunner module
class DLL_PUBLIC pacrunner {
public:
	pacrunner(string pac, const url& pacurl);
	virtual ~pacrunner() {};
	virtual string run(const url& url) throw (bad_alloc)=0;
};

class DLL_PUBLIC pacrunner_extension : public extension<pacrunner_extension, true> {
public:
	// Virtual methods
	virtual pacrunner* get(string pac, const url& pacurl) throw (bad_alloc);
	virtual ~pacrunner_extension();

	// Final methods
	pacrunner_extension();

protected:
	// Abstract methods
	virtual pacrunner* create(string pac, const url& pacurl) throw (bad_alloc)=0;

private:
	pacrunner* pr;
	string     last;
};

}

#endif /* EXTENSION_PACRUNNER_HPP_ */
