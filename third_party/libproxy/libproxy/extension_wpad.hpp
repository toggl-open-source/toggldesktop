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

#ifndef EXTENSION_WPAD_HPP_
#define EXTENSION_WPAD_HPP_

#include <libmodman/module.hpp>
#include "url.hpp"

namespace libproxy {
using namespace std;
using namespace libmodman;

// WPAD module
class DLL_PUBLIC wpad_extension : public extension<wpad_extension> {
public:
	// Abstract methods
	virtual bool found()=0;
	virtual url* next(char** pac)=0;
	virtual void rewind()=0;

	// Virtual methods
	virtual bool operator<(const wpad_extension& module) const;
	using extension<wpad_extension>::operator<;
};

}

#endif /* EXTENSION_WPAD_HPP_ */
