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

#ifndef MODULE_MANAGER_HPP_
#define MODULE_MANAGER_HPP_

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cassert>

#include "module.hpp"

namespace libmodman {
using namespace std;

class __MM_DLL_EXPORT module_manager {
public:
	~module_manager();
	bool load_builtin(mm_module *mod);
	bool load_file(string filename, bool symbreq=true);
	bool load_dir(string dirname, bool symbreq=true);

	template <class T> vector<T*> get_extensions() const {
		struct pcmp {
			static bool cmp(T* x, T* y) { return *x < *y; }
		};

		map<string, vector<base_extension*> >::const_iterator it = this->extensions.find(T::base_type());
		vector<T*> retlist;

		if (it != this->extensions.end()) {
			vector<base_extension*> extlist = it->second;

			for (size_t i=0 ; i < extlist.size() ; i++) {
				T* obj = dynamic_cast<T*>(extlist[i]);
				if (obj)
					retlist.push_back(obj);
				else
					assert (obj != NULL);
			}

			sort(retlist.begin(), retlist.end(), &pcmp::cmp);
		}

		return retlist;
	}

	template <class T> bool register_type() {
		if (T::singleton()) {
			if (!this->singletons.insert(T::base_type()).second)
				return false;
		}
		else
			this->singletons.erase(T::base_type());
		this->extensions[T::base_type()];
		return true;
	}

private:
	set<void*>                            modules;
	set<string>                           singletons;
	map<string, vector<base_extension*> > extensions;
};

}

#endif /* MODULE_MANAGER_HPP_ */
