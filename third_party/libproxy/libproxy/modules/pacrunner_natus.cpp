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

#include "../extension_pacrunner.hpp"
#include <unistd.h> // gethostname
using namespace libproxy;

#define I_ACKNOWLEDGE_THAT_NATUS_IS_NOT_STABLE
#include <natus/natus.h>
#include "pacutils.h"

using namespace natus;

static Value dnsResolve(Value& ths, Value& fnc, vector<Value>& arg) {
	Value exc = checkArguments(ths, arg, "s");
	if (exc.isException()) return exc;

	// Look it up
	struct addrinfo *info;
	if (getaddrinfo(arg[0].toString().c_str(), NULL, NULL, &info))
		return NULL;

	// Try for IPv4
	char* tmp = new char[INET6_ADDRSTRLEN+1];
	if (getnameinfo(info->ai_addr, info->ai_addrlen,
					tmp, INET6_ADDRSTRLEN+1,
					NULL, 0,
					NI_NUMERICHOST)) {
			freeaddrinfo(info);
			delete tmp;
			return NULL;
		}
	freeaddrinfo(info);

	// Create the return value
	Value ret = ths.newString(tmp);
	delete tmp;
	return ret;
}

static Value myIpAddress(Value& ths, Value& fnc, vector<Value>& arg) {
	char hostname[1024];
	if (!gethostname(hostname, 1023)) {
		vector<Value> dnsargs;
		dnsargs.push_back(ths.newString(hostname));
		return dnsResolve(ths, fnc, dnsargs);
	}
	return ths.newString("Unable to find hostname!").toException();
}

class natus_pacrunner : public pacrunner {
public:
	natus_pacrunner(string pac, const url& pacurl) throw (bad_alloc) : pacrunner(pac, pacurl) {
		Value exc;

		// Create the basic context
		if (!eng.initialize()) goto error;
		glb = this->eng.newGlobal();
		if (glb.isException()) goto error;

		// Add dnsResolve into the context
		if (!glb.set("dnsResolve", glb.newFunction(dnsResolve))) goto error;

		// Add myIpAddress into the context
		if (!glb.set("myIpAddress", glb.newFunction(myIpAddress))) goto error;

		// Add all other routines into the context
		exc = glb.evaluate(JAVASCRIPT_ROUTINES);
		if (exc.isException()) goto error;

		// Add the PAC into the context
		exc = glb.evaluate(pac.c_str(), pacurl.to_string());
		if (exc.isException()) goto error;
		return;

	error:
		throw bad_alloc();
	}

	string run(const url& url_) throw (bad_alloc) {
		vector<Value> args;
		args.push_back(glb.newString(url_.to_string()));
		args.push_back(glb.newString(url_.get_host()));

		Value res = glb.call("FindProxyForURL", args);
		if (res.isString() && !res.isException())
			return res.toString();
		return "";
	}

private:
	Engine eng;
	Value  glb;
};

PX_PACRUNNER_MODULE_EZ(natus, "nt_engine_init", "natus");
