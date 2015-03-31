/*******************************************************************************
 * libmodman - A library for extending applications
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

#ifndef MAIN_HPP_
#define MAIN_HPP_

#include <typeinfo>

#include "../module_manager.hpp"

using namespace std;
using namespace libmodman;

class __MM_DLL_EXPORT singleton_extension : public extension<singleton_extension, true> {};

class __MM_DLL_EXPORT sorted_extension    : public extension<sorted_extension> {
public:
	virtual bool operator<(const base_extension& other) const {
		return string(typeid(*this).name()) > string(typeid(other).name());
	}
};

class __MM_DLL_EXPORT symbol_extension    : public extension<symbol_extension> {};
class __MM_DLL_EXPORT condition_extension : public extension<condition_extension> {};
class __MM_DLL_EXPORT builtin_extension   : public extension<builtin_extension> {};

#endif /* MAIN_HPP_ */
