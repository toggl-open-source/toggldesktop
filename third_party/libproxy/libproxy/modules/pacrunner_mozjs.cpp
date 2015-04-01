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

#include <cstring> // ?
#include <unistd.h> // gethostname

#include "../extension_pacrunner.hpp"
using namespace libproxy;

// Work around a mozjs include bug
#ifndef JS_HAS_FILE_OBJECT
#define JS_HAS_FILE_OBJECT 0
#endif
#ifdef WIN32
#ifndef XP_WIN
#define XP_WIN
#endif
#endif
#include <jsapi.h>
#include "pacutils.h"

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif

static JSBool dnsResolve_(JSContext *cx, jsval hostname, jsval *vp) {
	// Get hostname argument
	char *tmp = JS_EncodeString(cx, JS_ValueToString(cx, hostname));

	// Set the default return value
	JS_SET_RVAL(cx, vp, JSVAL_NULL);

	// Look it up
	struct addrinfo *info = NULL;
	if (getaddrinfo(tmp, NULL, NULL, &info))
		goto out;

	// Allocate the IP address
	JS_free(cx, tmp);
	tmp = (char *) JS_malloc(cx, INET6_ADDRSTRLEN+1);
	memset(tmp, 0, INET6_ADDRSTRLEN+1);

	// Try for IPv4 and IPv6
	if (getnameinfo(info->ai_addr, info->ai_addrlen,
					tmp, INET6_ADDRSTRLEN+1,
					NULL, 0,
					NI_NUMERICHOST)) goto out;

	// We succeeded
	JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(JS_NewStringCopyN(cx, tmp, strlen(tmp))));
	tmp = NULL;

	out:
		if (info) freeaddrinfo(info);
		JS_free(cx, tmp);
		return true;
}

static JSBool dnsResolve(JSContext *cx, uintN /*argc*/, jsval *vp) {
	jsval *argv = JS_ARGV(cx, vp);
	return dnsResolve_(cx, argv[0], vp);
}

static JSBool myIpAddress(JSContext *cx, uintN /*argc*/, jsval *vp) {
	char *hostname = (char *) JS_malloc(cx, 1024);
	if (!gethostname(hostname, 1023)) {
		JSString *myhost = JS_NewStringCopyN(cx, hostname, strlen(hostname));
		jsval arg = STRING_TO_JSVAL(myhost);
		return dnsResolve_(cx, arg, vp);
	}
	JS_free(cx, hostname);
	JS_SET_RVAL(cx, vp, JSVAL_NULL);
	return true;
}

// Setup Javascript global class
// This MUST be a static global
static JSClass cls = {
		"global", JSCLASS_GLOBAL_FLAGS,
		JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
		JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

class mozjs_pacrunner : public pacrunner {
public:
	mozjs_pacrunner(string pac, const url& pacurl) throw (bad_alloc) : pacrunner(pac, pacurl) {
		jsval     rval;

		// Set defaults
		this->jsrun = NULL;
		this->jsctx = NULL;

		// Initialize Javascript runtime environment
		if (!(this->jsrun = JS_NewRuntime(1024 * 1024)))                  goto error;
		if (!(this->jsctx = JS_NewContext(this->jsrun, 1024 * 1024)))     goto error;
	    //JS_SetOptions(this->jsctx, JSOPTION_VAROBJFIX);
	    //JS_SetVersion(this->jsctx, JSVERSION_LATEST);
	    //JS_SetErrorReporter(cx, reportError);
		if (!(this->jsglb = JS_NewCompartmentAndGlobalObject(this->jsctx, &cls, NULL))) goto error;
		if (!JS_InitStandardClasses(this->jsctx, this->jsglb))            goto error;

		// Define Javascript functions
		JS_DefineFunction(this->jsctx, this->jsglb, "dnsResolve", dnsResolve, 1, 0);
		JS_DefineFunction(this->jsctx, this->jsglb, "myIpAddress", myIpAddress, 0, 0);
		JS_EvaluateScript(this->jsctx, this->jsglb, JAVASCRIPT_ROUTINES,
				        strlen(JAVASCRIPT_ROUTINES), "pacutils.js", 0, &rval);

		// Add PAC to the environment
		JS_EvaluateScript(this->jsctx, this->jsglb, pac.c_str(),
							strlen(pac.c_str()), pacurl.to_string().c_str(), 0, &rval);
		return;

		error:
			if (this->jsctx) JS_DestroyContext(this->jsctx);
			if (this->jsrun) JS_DestroyRuntime(this->jsrun);
			throw bad_alloc();
	}

	~mozjs_pacrunner() {
		if (this->jsctx) JS_DestroyContext(this->jsctx);
		if (this->jsrun) JS_DestroyRuntime(this->jsrun);
		// JS_ShutDown()?
	}

	string run(const url& url_) throw (bad_alloc) {
		// Build arguments to the FindProxyForURL() function
		char *tmpurl  = JS_strdup(this->jsctx, url_.to_string().c_str());
		char *tmphost = JS_strdup(this->jsctx, url_.get_host().c_str());
		if (!tmpurl || !tmphost) {
			if (tmpurl) JS_free(this->jsctx, tmpurl);
			if (tmphost) JS_free(this->jsctx, tmphost);
			throw bad_alloc();
		}
		jsval args[2] = {
			STRING_TO_JSVAL(JS_NewStringCopyN(this->jsctx, tmpurl, strlen(tmpurl))),
			STRING_TO_JSVAL(JS_NewStringCopyN(this->jsctx, tmphost, strlen(tmphost)))
		};

		// Find the proxy (call FindProxyForURL())
		jsval rval;
		JSBool result = JS_CallFunctionName(this->jsctx, this->jsglb, "FindProxyForURL", 2, args, &rval);
		if (!result) return "";
		
		char * tmpanswer = JS_EncodeString(this->jsctx, JS_ValueToString(this->jsctx, rval));
		string answer = string(tmpanswer);
		JS_free(this->jsctx, tmpanswer);

		if (answer == "undefined") return "";
		return answer;
	}

private:
	JSRuntime *jsrun;
	JSContext *jsctx;
	JSObject  *jsglb;
};

PX_PACRUNNER_MODULE_EZ(mozjs, "JS_DefineFunction", "mozjs");
