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

#ifdef __APPLE__
// JavaScriptCore.h requires CoreFoundation
// This is only found on Mac OS X
#include <JavaScriptCore/JavaScriptCore.h>
#else
#include <JavaScriptCore/JavaScript.h>
#endif
#include "pacutils.h"

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif

static char *jstr2str(JSStringRef str, bool release) throw (bad_alloc)
{
	char *tmp = new char[JSStringGetMaximumUTF8CStringSize(str)+1];
	JSStringGetUTF8CString(str, tmp, JSStringGetMaximumUTF8CStringSize(str)+1);
	if (release) JSStringRelease(str);
	return tmp;
}

static JSValueRef dnsResolve(JSContextRef ctx, JSObjectRef /*func*/, JSObjectRef /*self*/, size_t argc, const JSValueRef argv[], JSValueRef* /*exception*/)
{
	if (argc != 1)                         return NULL;
	if (!JSValueIsString(ctx, argv[0]))    return NULL;

	// Get hostname argument
	char *tmp = jstr2str(JSValueToStringCopy(ctx, argv[0], NULL), true);

	// Look it up
	struct addrinfo *info;
	if (getaddrinfo(tmp, NULL, NULL, &info))
		return NULL;
	delete tmp;

	// Try for IPv4
	tmp = new char[INET6_ADDRSTRLEN+1];
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
	JSStringRef str = JSStringCreateWithUTF8CString(tmp);
	JSValueRef  ret = JSValueMakeString(ctx, str);
	JSStringRelease(str);
	delete tmp;

	return ret;
}

static JSValueRef myIpAddress(JSContextRef ctx, JSObjectRef func, JSObjectRef self, size_t /*argc*/, const JSValueRef[] /*argv*/, JSValueRef* /*exception*/)
{
	char hostname[1024];
	if (!gethostname(hostname, 1023)) {
		JSStringRef str = JSStringCreateWithUTF8CString(hostname);
		JSValueRef  val = JSValueMakeString(ctx, str);
		JSStringRelease(str);
		JSValueRef ip = dnsResolve(ctx, func, self, 1, &val, NULL);
		return ip;
	}
	return NULL;
}

class webkit_pacrunner : public pacrunner {
public:
	~webkit_pacrunner() {
		JSGarbageCollect(this->jsctx);
		JSGlobalContextRelease(this->jsctx);
	}

	webkit_pacrunner(string pac, const url& pacurl) throw (bad_alloc) : pacrunner(pac, pacurl) {
		JSStringRef str  = NULL;
		JSObjectRef func = NULL;

		// Create the basic context
		if (!(this->jsctx = JSGlobalContextCreate(NULL))) goto error;

		// Add dnsResolve into the context
		str = JSStringCreateWithUTF8CString("dnsResolve");
		func = JSObjectMakeFunctionWithCallback(this->jsctx, str, dnsResolve);
		JSObjectSetProperty(this->jsctx, JSContextGetGlobalObject(this->jsctx), str, func, kJSPropertyAttributeNone, NULL);
		JSStringRelease(str);

		// Add myIpAddress into the context
		str = JSStringCreateWithUTF8CString("myIpAddress");
		func = JSObjectMakeFunctionWithCallback(this->jsctx, str, myIpAddress);
		JSObjectSetProperty(this->jsctx, JSContextGetGlobalObject(this->jsctx), str, func, kJSPropertyAttributeNone, NULL);
		JSStringRelease(str);

		// Add all other routines into the context
		str = JSStringCreateWithUTF8CString(JAVASCRIPT_ROUTINES);
		if (!JSCheckScriptSyntax(this->jsctx, str, NULL, 0, NULL)) goto error;
		JSEvaluateScript(this->jsctx, str, NULL, NULL, 1, NULL);
		JSStringRelease(str);

		// Add the PAC into the context
		str = JSStringCreateWithUTF8CString(pac.c_str());
		if (!JSCheckScriptSyntax(this->jsctx, str, NULL, 0, NULL)) goto error;
		JSEvaluateScript(this->jsctx, str, NULL, NULL, 1, NULL);
		JSStringRelease(str);
		return;

	error:
		if (str) JSStringRelease(str);
		if (this->jsctx) {
			JSGarbageCollect(this->jsctx);
			JSGlobalContextRelease(this->jsctx);
		}
		throw bad_alloc();
	}

	string run(const url& url_) throw (bad_alloc) {
		JSStringRef str = NULL;
		JSValueRef  val = NULL;
		string      tmp;

		// Run the PAC
		tmp = string("FindProxyForURL(\"") + url_.to_string() + string("\", \"") + url_.get_host() + "\");";
		str = JSStringCreateWithUTF8CString(tmp.c_str());
		if (!str) throw bad_alloc();
		if (!JSCheckScriptSyntax(this->jsctx, str, NULL, 0, NULL))            goto error;
		if (!(val = JSEvaluateScript(this->jsctx, str, NULL, NULL, 1, NULL))) goto error;
		if (!JSValueIsString(this->jsctx, val))                               goto error;
		JSStringRelease(str);

		// Convert the return value to a string
		return jstr2str(JSValueToStringCopy(this->jsctx, val, NULL), true);

	error:
		JSStringRelease(str);
		return "";
	}

private:
	JSGlobalContextRef jsctx;
};

PX_PACRUNNER_MODULE_EZ(webkit, "JSObjectMakeFunctionWithCallback", "webkit");
