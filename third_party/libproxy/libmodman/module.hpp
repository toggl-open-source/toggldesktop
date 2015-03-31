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

#ifndef MODULE_HPP_
#define MODULE_HPP_

#include <cstdlib> // For NULL

#ifndef MM_MODULE_BUILTIN
#define MM_MODULE_BUILTIN
#endif

#ifdef _MSC_VER
#define __MM_DLL_EXPORT __declspec(dllexport)
#else
#define __MM_DLL_EXPORT __attribute__ ((visibility("default")))
#endif

#define __MM_MODULE_VERSION 2
#define __MM_MODULE_VARNAME__(suffix, name) mm_ ## name ## _ ## suffix
#define __MM_MODULE_VARNAME_(suffix, name) __MM_MODULE_VARNAME__(suffix, name)
#define __MM_MODULE_VARNAME(name) __MM_MODULE_VARNAME_(MM_MODULE_BUILTIN, name)

#define MM_MODULE_INIT(mtype, minit, mtest, msymb, msmod) \
	extern "C" __MM_DLL_EXPORT struct mm_module __MM_MODULE_VARNAME(info); \
	struct mm_module __MM_MODULE_VARNAME(info) = { \
		__MM_MODULE_VERSION, \
		# mtype, \
		mtype::base_type, \
		minit, mtest, msymb, msmod \
	}

#define MM_MODULE_INIT_EZ(clsname, mtest, msymb, msmod) \
	static libmodman::base_extension** clsname ## _init() { \
		libmodman::base_extension** retval = new libmodman::base_extension*[2]; \
		retval[0] = new clsname(); \
		retval[1] = NULL; \
		return retval; \
	} \
	static bool clsname ## _test() { return mtest; } \
	MM_MODULE_INIT(clsname, clsname ## _init, clsname ## _test, msymb, msmod)

/* Helper macro for loading builtins */

#define MM_DEF_BUILTIN(modname) \
  extern "C" struct mm_module __MM_MODULE_VARNAME_(modname,info)

#define MM_BUILTIN(modname) __MM_MODULE_VARNAME_(modname,info)


namespace libmodman {

class __MM_DLL_EXPORT base_extension {
public:
	static  const char*     base_type() { return NULL; }
	static  bool            singleton() { return false; }
	virtual           ~base_extension() {}
	virtual const char* get_base_type() const =0;
	virtual bool            operator<(const base_extension&) const =0;
};

template <class basetype, bool sngl=false>
class __MM_DLL_EXPORT extension : public base_extension {
public:
#ifdef _MSC_VER
	static  const char*     base_type() { return __FUNCSIG__; }
#else
	static  const char*     base_type() { return __PRETTY_FUNCTION__; }
#endif

	static  bool            singleton() { return sngl; }
	virtual const char* get_base_type() const { return basetype::base_type(); }
	virtual bool            operator<(const base_extension&) const { return false; };
};

}

extern "C" {

struct __MM_DLL_EXPORT mm_module
{
/* For some unknown reason, when vers is const VC++ (win32) NULLs out the whole struct.
 * This obviously breaks module loading.  I'd love to know the reason for this, so if
 * anyone knows, please tell me.  In the meantime, vers not being const is by design,
 * so don't change it. */
        unsigned int		     vers;
  const char*                    name;
  const char*				   (*type)();
  libmodman::base_extension**  (*init)();
  bool						   (*test)();
  const char*					 symb;
  const char*					 smod;
};

}

#endif /* MODULE_HPP_ */
