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

#include <vector>
#include <cstring>   // For strdup()
#include <iostream>  // For cerr
#include <stdexcept> // For exception
#include <typeinfo>  // Only for debug messages.

#include <libmodman/module_manager.hpp>

#include "extension_config.hpp"
#include "extension_ignore.hpp"
#include "extension_network.hpp"
#include "extension_pacrunner.hpp"
#include "extension_wpad.hpp"

#ifdef WIN32
#define strdup _strdup
#endif

#define X(m) MM_DEF_BUILTIN(m);
BUILTIN_MODULES
#undef X

#define X(m) &MM_BUILTIN(m),
static struct mm_module* builtin_modules[] = {
	BUILTIN_MODULES
	NULL
};
#undef X

namespace libproxy {
using namespace std;

class proxy_factory {
public:
	proxy_factory();
	~proxy_factory();
	vector<string> get_proxies(string url);

private:
	void lock();
	void unlock();

	void check_network_topology();
	void get_config(url &realurl, vector<url> &configs, string &ignore);
	bool is_ignored(url &realurl, const string &ignore);
	bool expand_wpad(const url &confurl);
	bool expand_pac(url &configurl);
	void run_pac(url &realurl, const url &confurl, vector<string> &response);
	void clear_cache();

#ifdef WIN32
	HANDLE mutex;
#else
	pthread_mutex_t mutex;
#endif
	module_manager  mm;
	char*  pac;
	url*   pacurl;
	bool   wpad;
	bool   debug;
};

static bool istringcmp(string a, string b) {
    transform( a.begin(), a.end(), a.begin(), ::tolower );
    transform( b.begin(), b.end(), b.begin(), ::tolower );
    return ( a == b );
}

// Convert the PAC formatted response into our proxy URL array response
static void format_pac_response(string response, vector<string> &retval)
{
	// Skip ahead one character if we start with ';'
	if (response[0] == ';') {
		format_pac_response(response.substr(1), retval);
		return;
	}

	// If the string contains a delimiter (';')
	if (response.find(';') != string::npos) {
		format_pac_response(response.substr(response.find(';') + 1), retval);
		response = response.substr(0, response.find(';'));
	}

	// Strip whitespace
	if (response.size() > 0)
		response = response.substr(response.find_first_not_of(" \t\n"), response.find_last_not_of(" \t\n")+1);

	// Get the method and the server
	string method = "";
	string server = "";
	if (response.find_first_of(" \t") == string::npos)
		method = response;
	else {
		method = response.substr(0, response.find_first_of(" \t"));
		server = response.substr(response.find_first_of(" \t") + 1);
	}

	// Insert our url value
	if (istringcmp(method, "proxy") && url::is_valid("http://" + server))
		retval.insert(retval.begin(), string("http://") + server);
	else if (istringcmp(method, "socks") && url::is_valid("http://" + server))
		retval.insert(retval.begin(), string("socks://") + server);
	else if (istringcmp(method, "socks4") && url::is_valid("http://" + server))
		retval.insert(retval.begin(), string("socks4://") + server);
	else if (istringcmp(method, "socks4a") && url::is_valid("http://" + server))
		retval.insert(retval.begin(), string("socks4a://") + server);
	else if (istringcmp(method, "socks5") && url::is_valid("http://" + server))
		retval.insert(retval.begin(), string("socks5://") + server);
	else if (istringcmp(method, "direct"))
		retval.insert(retval.begin(), string("direct://"));
}

proxy_factory::proxy_factory() {
	const char *module_dir;

#ifdef WIN32
	this->mutex = CreateMutex(NULL, false, NULL);
	WSADATA wsadata;
	WORD vers = MAKEWORD(2, 2);
	WSAStartup(vers, &wsadata);
#else
	pthread_mutex_init(&this->mutex, NULL);
#endif

	lock();

	this->pac    = NULL;
	this->pacurl = NULL;
	this->wpad   = false;

	// Register our types
	this->mm.register_type<config_extension>();
	this->mm.register_type<ignore_extension>();
	this->mm.register_type<network_extension>();
	this->mm.register_type<pacrunner_extension>();
	this->mm.register_type<wpad_extension>();

	// Load builtin modules
	for (int i=0 ; builtin_modules[i] ; i++)
		this->mm.load_builtin(builtin_modules[i]);

	// Load all modules
	module_dir = getenv("PX_MODULE_PATH");
	if (!module_dir)
		module_dir = MODULEDIR;
	this->mm.load_dir(module_dir);
	this->mm.load_dir(module_dir, false);

	this->debug  = (getenv("_PX_DEBUG") != NULL);

	unlock();
}

proxy_factory::~proxy_factory() {
	lock();

	if (this->pac) delete[] this->pac;
	if (this->pacurl) delete this->pacurl;
	
	unlock();

#ifdef WIN32
	WSACleanup();
	ReleaseMutex(this->mutex);
#else
	pthread_mutex_destroy(&this->mutex);
#endif
}


vector<string> proxy_factory::get_proxies(string realurl) {
	vector<string>             response;

	// Check to make sure our url is valid
	if (!url::is_valid(realurl))
		goto do_return;

	lock();

	// Let trap and forward exceptions so we don't deadlock
	try {
		vector<url> configs;
		string ignore;
		url dst(realurl);

		check_network_topology();
		get_config(dst, configs, ignore);
		
		if (debug) cerr << "Config is: " << endl;

		for (vector<url>::iterator i=configs.begin() ; i != configs.end() ; i++) {
			url confurl(*i);

			if (debug) cerr << "\t" << confurl.to_string() << endl;

			if (expand_wpad(confurl) || expand_pac(confurl)) {
				run_pac(dst, confurl, response);
			} else {
				clear_cache();
				response.push_back(confurl.to_string());
			}
		}

		unlock();
	} catch (exception &e) {
		unlock();
		throw e;
	}

do_return:
	if (response.size() == 0)
		response.push_back("direct://");
	return response;
}

void proxy_factory::check_network_topology() {
	vector<network_extension*> networks;

	// Check to see if our network topology has changed...
	networks = this->mm.get_extensions<network_extension>();
	for (vector<network_extension*>::iterator i=networks.begin() ; i != networks.end() ; i++) {
		// If it has, reset our wpad/pac setup and we'll retry our config
		if ((*i)->changed()) {
			if (debug) cerr << "Network changed" << endl;
			vector<wpad_extension*> wpads = this->mm.get_extensions<wpad_extension>();
			for (vector<wpad_extension*>::iterator j=wpads.begin() ; j != wpads.end() ; j++)
				(*j)->rewind();
			if (this->pac) delete this->pac;
			this->pac = NULL;
			break;
		}
	}
}

void proxy_factory::get_config(url &realurl, vector<url> &config, string &ignore) {
	vector<config_extension*>  configs;

	configs = this->mm.get_extensions<config_extension>();
	for (vector<config_extension*>::iterator i=configs.begin() ; i != configs.end() ; i++) {
		config_extension *configurator = *i;

		// Try to get the configuration
		try {
			ignore = configurator->get_ignore(realurl);
			if (!is_ignored(realurl, ignore))
				config = configurator->get_config(realurl);
			if (debug) {
				if (configurator) {
					cerr << "Configuration extension is: " << typeid(*configurator).name() << endl;
					cerr << "Ingored list is: " << ignore << endl;
				} else {
					cerr << "No configuration extension found." << endl;
				}
			}
			break;
		}
		catch (runtime_error e) {
			ignore = "";
		}
	}
}

bool proxy_factory::is_ignored(url &realurl, const string &ignore) {
	vector<ignore_extension*>  ignores;
	bool                       ignored = false, invign = false;
	string                     confign = ignore;

	/* Check our ignore patterns */
	ignores = this->mm.get_extensions<ignore_extension>();
	invign  = confign.size() > 0 && confign[0] == '-';
	if (invign) confign = confign.substr(1);
	for (size_t i=0 ; i < confign.size() && !ignored;) {
		size_t next = confign.find(',', i);
		if (next == string::npos) next = confign.length();
		if (next > (i+1)) {
			string ignorestr = confign.substr (i, next - i);
			ignorestr = ignorestr.substr(ignorestr.find_first_not_of(" \t\n"), ignorestr.find_last_not_of(" \t\n")+1);
			for (vector<ignore_extension*>::iterator it=ignores.begin() ; it != ignores.end() && !ignored ; it++)
				ignored = ((*it)->ignore(realurl, ignorestr));
		}
		i = next+1;
	}

	if (invign)
		return !ignored;
	else
		return ignored;
}

bool proxy_factory::expand_wpad(const url &confurl)
{
	bool rtv = false;
	if (confurl.get_scheme() == "wpad") {
		rtv = true;
		/* If the config has just changed from PAC to WPAD, clear the PAC */
		if (!this->wpad) {
			if (this->pac)    delete this->pac;
			if (this->pacurl) delete this->pacurl;
			this->pac    = NULL;
			this->pacurl = NULL;
			this->wpad = true;
		}

		/* If we have no PAC, get one */
		if (!this->pac) {
			if (debug) cerr << "Trying to find the PAC using WPAD..." << endl;
			vector<wpad_extension*> wpads = this->mm.get_extensions<wpad_extension>();
			for (vector<wpad_extension*>::iterator i=wpads.begin() ; i != wpads.end() ; i++) {
				if (debug) cerr << "WPAD search via: " << typeid(**i).name() << endl;
				if ((this->pacurl = (*i)->next(&this->pac))) {
					if (debug) cerr << "PAC found!" << endl;
					break;
				}
			}

			/* If getting the PAC fails, but the WPAD cycle worked, restart the cycle */
			if (!this->pac) {
				if (debug) cerr << "No PAC found..." << endl;
				for (vector<wpad_extension*>::iterator i=wpads.begin() ; i != wpads.end() ; i++) {
					if ((*i)->found()) {
						if (debug) cerr << "Resetting WPAD search..." << endl;

						/* Since a PAC was found at some point,
						 * rewind all the WPADS to start from the beginning */
						/* Yes, the reuse of 'i' here is intentional...
						 * This is because if *any* of the wpads have found a pac,
						 * we need to reset them all to keep consistent state. */
						for (i=wpads.begin() ; i != wpads.end() ; i++)
							(*i)->rewind();

						// Attempt to find a PAC
						for (i=wpads.begin() ; i != wpads.end() ; i++) {
							if (debug) cerr << "WPAD search via: " << typeid(**i).name() << endl;
							if ((this->pacurl = (*i)->next(&this->pac))) {
								if (debug) cerr << "PAC found!" << endl;
								break;
							}
						}
						break;
					}
				}
			}
		}
	}

	return rtv;
}

bool proxy_factory::expand_pac(url &confurl)
{
	bool rtv = false;

	// If we have a PAC config
	if (confurl.get_scheme().substr(0, 4) == "pac+") {
		rtv = true;  

		/* Save the PAC config */
		if (this->wpad)
			this->wpad = false;

		/* If a PAC already exists, but came from a different URL than the one specified, remove it */
		if (this->pac) {
			if (this->pacurl->to_string() != confurl.to_string()) {
				delete this->pacurl;
				delete this->pac;
				this->pacurl = NULL;
				this->pac    = NULL;
			}
		}

		/* Try to load the PAC if it is not already loaded */
		if (!this->pac) {
			this->pacurl = new url(confurl);
			this->pac = confurl.get_pac();
			if (debug) {
				if (!this->pac)
					cerr << "Unable to download PAC!" << endl;
				else
					cerr << "PAC received!" << endl;
			}
		}
	}

	return rtv;
}

void proxy_factory::run_pac(url &realurl, const url &confurl, vector<string> &response) {
	/* In case of either PAC or WPAD, we'll run the PAC */
	if (this->pac && (confurl.get_scheme() == "wpad" || confurl.get_scheme().substr(0, 4) == "pac+") ) {
		vector<pacrunner_extension*> pacrunners = this->mm.get_extensions<pacrunner_extension>();

		/* No PAC runner found, fall back to direct */
		if (pacrunners.size() == 0) {
			if (debug) cerr << "Unable to find a required pacrunner!" << endl;
			return;
		}

		/* Run the PAC, but only try one PACRunner */
		if (debug) cerr << "Using pacrunner: " << typeid(*pacrunners[0]).name() << endl;
		string pacresp = pacrunners[0]->get(this->pac, this->pacurl->to_string())->run(realurl);
		if (debug) cerr << "Pacrunner returned: " << pacresp << endl;
		format_pac_response(pacresp, response);
	}
}

void proxy_factory::clear_cache() {
	this->wpad = false;
	if (this->pac)    { delete this->pac;    this->pac = NULL; }
	if (this->pacurl) { delete this->pacurl; this->pacurl = NULL; }
}

void proxy_factory::lock() {
#ifdef WIN32
	WaitForSingleObject(this->mutex, INFINITE);
#else
	pthread_mutex_lock(&this->mutex);
#endif
}

void proxy_factory::unlock() {
#ifdef WIN32
	ReleaseMutex(this->mutex);
#else
	pthread_mutex_unlock(&this->mutex);
#endif
}

};

struct pxProxyFactory_ {
	libproxy::proxy_factory pf;
};

extern "C" DLL_PUBLIC struct pxProxyFactory_ *px_proxy_factory_new(void) {
	return new struct pxProxyFactory_;
}

extern "C" DLL_PUBLIC char** px_proxy_factory_get_proxies(struct pxProxyFactory_ *self, const char *url) {
	std::vector<std::string> proxies;
	char** retval;

	// Call the main method
	try {
		proxies = self->pf.get_proxies(url);
		retval  = (char**) malloc(sizeof(char*) * (proxies.size() + 1));
		if (!retval) return NULL;
	} catch (std::exception&) {
		// Return NULL on any exception
		return NULL;
	}

	// Copy the results into an array
	// Return NULL on memory allocation failure
	retval[proxies.size()] = NULL;
	for (size_t i=0 ; i < proxies.size() ; i++) {
		retval[i] = strdup(proxies[i].c_str());
		if (retval[i] == NULL) {
			for (int j=i-1 ; j >= 0 ; j--)
				free(retval[j]);
			free(retval);
			return NULL;
		}
	}
	return retval;
}

extern "C" DLL_PUBLIC void px_proxy_factory_free(struct pxProxyFactory_ *self) {
	delete self;
}
