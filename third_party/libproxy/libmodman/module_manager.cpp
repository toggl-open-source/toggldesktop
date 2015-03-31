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

#include <algorithm>  // For sort()
#include <sys/stat.h> // For stat()
#include <iostream>
#include <typeinfo>

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>  // For dlopen(), etc...
#include <dirent.h> // For opendir(), readdir(), closedir()
#endif

#include "module_manager.hpp"
using namespace libmodman;

#include <cstdio>

#define _LOAD_FAIL -1
#define _LOAD_LAZY  0
#define _LOAD_SUCC  1

#ifdef WIN32
#define pdlmtype HMODULE
#define pdlopenl(filename) LoadLibraryEx(filename, NULL, DONT_RESOLVE_DLL_REFERENCES)
#define pdlclose(module) FreeLibrary((pdlmtype) module)
static void* pdlsym(pdlmtype mod, string sym) {
	return (void *) GetProcAddress(mod, sym.c_str());
}

static pdlmtype pdlreopen(const char* filename, pdlmtype module) {
	pdlclose(module);
	return LoadLibrary(filename);
}

static string pdlerror() {
	std::string e;
	LPTSTR msg;

	FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &msg,
			0,
			NULL);
	e = std::string((const char*) msg);
    LocalFree(msg);
    return e;
}

static bool pdlsymlinked(const char* modn, const char* symb) {
	return (GetProcAddress(GetModuleHandle(modn), symb) != NULL || \
		    GetProcAddress(GetModuleHandle(NULL), symb) != NULL);
}

static string prep_type_name(string name) {
	string prefix = "<class ";
	string suffix = ",";
	if (name.find(prefix) != name.npos)
		name = name.substr(name.find(prefix) + prefix.size());
	if (name.find(suffix) != name.npos)
		name = name.substr(0, name.find(suffix));
	return name;
}
#else
#define pdlmtype void*
#define pdlopenl(filename) dlopen(filename, RTLD_LAZY | RTLD_LOCAL)
#define pdlclose(module) dlclose((pdlmtype) module)
#define pdlreopen(filename, module) module
static void* pdlsym(pdlmtype mod, string sym) {
	return dlsym(mod, sym.c_str());
}

static string pdlerror() {
	return dlerror();
}

bool pdlsymlinked(const char* modn, const char* symb) {
	void* mod = dlopen(NULL, RTLD_LAZY | RTLD_LOCAL);
	if (mod) {
		void* sym = dlsym(mod, symb);
		dlclose(mod);
		return sym != NULL;
	}
	return false;
}

#define prep_type_name(name) name
#endif

#define _str(s) #s
#define __str(s) _str(s)

#ifndef _MOD_SUFFIX
#ifdef WIN32
#define _MOD_SUFFIX "dll"
#define CR ""
#else
#define _MOD_SUFFIX "so"
#define CR "\r"
#endif
#endif

module_manager::~module_manager() {
	// Free all extensions
	for (map<string, vector<base_extension*> >::iterator i=this->extensions.begin() ; i != this->extensions.end() ; i++) {
		for (vector<base_extension*>::iterator j=i->second.begin() ; j != i->second.end() ; j++)
			delete *j;
		i->second.clear();
	}
	this->extensions.clear();

	// Free all modules
	for (set<void*>::iterator i=this->modules.begin() ; i != this->modules.end() ; i++)
		pdlclose(*i);
	this->modules.clear();
}

static int load(map<string, vector<base_extension*> >& extensions,
                             set<string>&              singletons,
                             mm_module                *mod,
                             bool                      lazy,
                             bool                      symbreq) {
	const char* debug = getenv("_MM_DEBUG");

	if (!mod || mod->vers != __MM_MODULE_VERSION || !mod->type || !mod->init) {
		if (debug)
			cerr << "failed!" << endl
			     << "\tUnable to find basic module info!" << endl;
		return _LOAD_FAIL;
	}

	// Get the module type
	string types = mod->type();

	// Make sure the type is registered
	if (extensions.find(types) == extensions.end()) {
		if (debug)
			cerr << "failed!" << endl
				 << "\tUnknown extension type: " << prep_type_name(types) << endl;
		return _LOAD_FAIL;
	}

	// If this is a singleton and we already have an instance, don't instantiate
	if (singletons.find(types) != singletons.end() &&
		extensions[types].size() > 0) {
		if (debug)
			cerr << "failed!" << endl
			     << "\tNot loading subsequent singleton for: " << prep_type_name(types) << endl;
		return _LOAD_FAIL;
	}

	// If a symbol is defined, we'll search for it in the main process
	if (mod->symb && mod->smod && !pdlsymlinked(mod->smod, mod->symb)) {
		// If the symbol is not found and the symbol is required, error
		if (symbreq) {
			if (debug)
				cerr << "failed!" << endl
					 << "\tUnable to find required symbol: "
					 << mod->symb << endl;
			return _LOAD_FAIL;
		}

		// If the symbol is not found and not required, we'll load only
		// if there are no other modules of this type
		else if (extensions[types].size() > 0) {
			if (debug)
				cerr << "failed!" << endl
					 << "\tUnable to find required symbol: "
					 << mod->symb << endl;
			return _LOAD_FAIL;
		}
	}

	// We've passed all the tests this far, do it again in non-lazy mode
	if (lazy) return _LOAD_LAZY;

	// If our execution test succeeds, call init()
	if ((mod->test && mod->test()) || !mod->test) {
		base_extension** exts = mod->init();
		if (!exts) {
			if (debug)
				cerr << "failed!" << endl
					 << "\tinit() returned no extensions!" << endl;
			return _LOAD_FAIL;
		}

		if (debug)
			cerr << "success" << endl;

		// init() returned extensions we need to register
		for (unsigned int i=0 ; exts[i] ; i++) {
			if (debug)
				cerr << "\tRegistering "
					 << typeid(*exts[i]).name() << "("
					 << prep_type_name(exts[i]->get_base_type()) << ")" << endl;
			extensions[exts[i]->get_base_type()].push_back(exts[i]);
		}
		delete[] exts;
		return _LOAD_SUCC;
	}

	if (debug)
		cerr << "failed!" << endl
			 << "\tTest execution failed." << endl;
	return _LOAD_FAIL;
}

bool module_manager::load_builtin(mm_module *mod) {
	const char* debug = getenv("_MM_DEBUG");
	if (debug)
			cerr << "loading : builtin module " << mod->name << CR;

	// Do the load with the specified prefix
	int status = load(this->extensions, this->singletons, mod, false, false);
	return status == _LOAD_SUCC;
}

bool module_manager::load_file(string filename, bool symbreq) {
	const char* debug = getenv("_MM_DEBUG");

	// Stat the file to make sure it is a file
	struct stat st;
	if (stat(filename.c_str(), &st) != 0) return false;
	if ((st.st_mode & S_IFMT) != S_IFREG) return false;

	if (debug)
		cerr << "loading : " << filename << CR;

	// Open the module
	pdlmtype dlobj = pdlopenl(filename.c_str());
	if (!dlobj) {
		if (debug)
			cerr << "failed!" << endl
			     << "\t" << pdlerror() << endl;
		return false;
	}

	// If we have already loaded this module, return true
	if (this->modules.find((void*) dlobj) != this->modules.end()) {
		if (debug)
			cerr << "preload" << endl;
		pdlclose(dlobj);
		return true;
	}

	// Try and finish the load
	struct mm_module *mod_info = (mm_module*) pdlsym(dlobj, __str(__MM_MODULE_VARNAME(info)));
	int status = load(this->extensions, this->singletons, mod_info, true, symbreq);
	if (status == _LOAD_LAZY) { // Reload the module in non-lazy mode
		dlobj = pdlreopen(filename.c_str(), dlobj);
		if (!dlobj) {
			if (debug)
				cerr << "failed!" << endl
					 << "\tUnable to reload module: " << pdlerror() << endl;
			return false;
		}
		mod_info = (mm_module*) pdlsym(dlobj, __str(__MM_MODULE_VARNAME(info)));
		status = load(this->extensions, this->singletons, mod_info, false, symbreq);
	}
	if (status == _LOAD_FAIL) {
		pdlclose(dlobj);
		return false;
	}

	// Add the dlobject to our known modules
	this->modules.insert((void*) dlobj);

	// Yay, we did it!
	return true;
}

bool module_manager::load_dir(string dirname, bool symbreq) {
	vector<string> files;

#ifdef WIN32
	WIN32_FIND_DATA fd;
	HANDLE search;

	string srch = dirname + "\\*." + _MOD_SUFFIX;
	search = FindFirstFile(srch.c_str(), &fd);
	if (search != INVALID_HANDLE_VALUE) {
		do {
			files.push_back(dirname + "\\" + fd.cFileName);
		} while (FindNextFile(search, &fd));
		FindClose(search);
	}
#else
	struct dirent *ent;

	DIR *moduledir = opendir(dirname.c_str());
	if (moduledir) {
		while((ent = readdir(moduledir))) {
			string tmp = ent->d_name;
			if (tmp.find(_MOD_SUFFIX, tmp.size() - string(_MOD_SUFFIX).size()) != tmp.npos)
				files.push_back(dirname + "/" + tmp);
		}
		closedir(moduledir);
	}
#endif

	// Perform our load alphabetically
	sort(files.begin(), files.end());

	// Try to do the load
	bool loaded = false;
	for (vector<string>::iterator it = files.begin() ; it != files.end() ; it++)
		loaded = this->load_file(*it, symbreq) || loaded;
	return loaded;
}
