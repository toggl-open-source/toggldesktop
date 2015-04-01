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

#include "extension_pacrunner.hpp"
using namespace libproxy;

pacrunner::pacrunner(string, const url&) {}

pacrunner_extension::pacrunner_extension() {
	this->pr = NULL;
}

pacrunner_extension::~pacrunner_extension() {
	if (this->pr) delete this->pr;
}

pacrunner* pacrunner_extension::get(string pac, const url& pacurl) throw (bad_alloc) {
	if (this->pr) {
		if (this->last == pac)
			return this->pr;
		delete this->pr;
	}

	return this->pr = this->create(pac, pacurl);
}
