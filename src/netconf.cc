// Copyright 2015 Toggl Desktop developers.

#include "../src/netconf.h"

#include <string>
#include <sstream>

#include "./https_client.h"

#include "Poco/Environment.h"
#include "Poco/Logger.h"
#include "Poco/Net/HTTPCredentials.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/URI.h"
#include "Poco/UnicodeConverter.h"

#ifdef __MACH__
#include <CoreFoundation/CoreFoundation.h>  // NOLINT
#include <CoreServices/CoreServices.h>  // NOLINT
#endif

#ifdef _WIN32
#include <winhttp.h>
#pragma comment(lib, "winhttp")
#endif

namespace toggl {

error Netconf::autodetectProxy(
    const std::string encoded_url,
    std::string *proxy_url) {

    *proxy_url = "";

    if (encoded_url.empty()) {
        return noError;
    }

#ifdef _WIN32
    HINTERNET session_handle = WinHttpOpen(
        NULL,
        WINHTTP_ACCESS_TYPE_NO_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);
    if (!session_handle) {
        return error("Failed to start winhttp session");
    }

    WinHttpSetTimeouts(session_handle, 10000, 10000, 5000, 5000);

    WINHTTP_AUTOPROXY_OPTIONS options = { 0 };
    options.fAutoLogonIfChallenged = FALSE;
    options.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
    // FIXME: set PAC url with options.lpszAutoConfigUrl

    WINHTTP_PROXY_INFO info = { 0 };

    std::wstring encoded_url_wide;
    Poco::UnicodeConverter::toUTF16(encoded_url, encoded_url_wide);
    if (!WinHttpGetProxyForUrl(
        session_handle,
        encoded_url_wide.c_str(),
        &options,
        &info)) {

        DWORD errcode = GetLastError();
        std::stringstream ss;
        ss << "WinHttpGetProxyForUrl error " << errcode;

        WinHttpCloseHandle(session_handle);
        session_handle = 0;

        return ss.str();
    }

    WinHttpCloseHandle(session_handle);
    session_handle = 0;

    if (WINHTTP_ACCESS_TYPE_NAMED_PROXY == info.dwAccessType) {
        std::wstring proxy_url_wide(info.lpszProxy);
        std::string s;
        Poco::UnicodeConverter::toUTF8(proxy_url_wide, s);
        *proxy_url = s;
    }

    if (info.lpszProxy) {
        free(info.lpszProxy);
    }
    if (info.lpszProxyBypass) {
        free(info.lpszProxyBypass);
    }
#endif

    // Inspider by VLC source code
    // https://github.com/videolan/vlc/blob/master/src/darwin/netconf.c
#ifdef __MACH__
    CFDictionaryRef dicRef = CFNetworkCopySystemProxySettings();
    if (NULL != dicRef) {
        const CFStringRef proxyCFstr = (const CFStringRef)CFDictionaryGetValue(
            dicRef, (const void*)kCFNetworkProxiesHTTPProxy);
        const CFNumberRef portCFnum = (const CFNumberRef)CFDictionaryGetValue(
            dicRef, (const void*)kCFNetworkProxiesHTTPPort);
        if (NULL != proxyCFstr && NULL != portCFnum) {
            int port = 0;
            if (!CFNumberGetValue(portCFnum, kCFNumberIntType, &port)) {
                CFRelease(dicRef);
                return noError;
            }

            const std::size_t kBufsize(4096);
            char host_buffer[kBufsize];
            memset(host_buffer, 0, sizeof(host_buffer));
            if (CFStringGetCString(proxyCFstr, host_buffer, sizeof(host_buffer)
                                   - 1, kCFStringEncodingUTF8)) {
                char buffer[kBufsize];
                snprintf(buffer, kBufsize, "%s:%d", host_buffer, port);
                *proxy_url = std::string(buffer);
            }
        }

        CFRelease(dicRef);
    }
#endif

    return noError;
}

error Netconf::ConfigureProxy(
    const std::string encoded_url,
    Poco::Net::HTTPSClientSession *session) {

    Poco::Logger &logger = Poco::Logger::get("ConfigureProxy");

    std::string proxy_url("");
    if (HTTPSClient::Config.AutodetectProxy) {
        if (Poco::Environment::has("HTTP_PROXY")) {
            proxy_url = Poco::Environment::get("HTTP_PROXY");
        }
        if (proxy_url.empty()) {
            error err = autodetectProxy(encoded_url, &proxy_url);
            if (err != noError) {
                return err;
            }
        }
        if (proxy_url.find("://") == std::string::npos) {
            proxy_url = "http://" + proxy_url;
        }
        Poco::URI proxy_uri(proxy_url);

        std::stringstream ss;
        ss << "Proxy detected URI=" + proxy_uri.toString()
           << " host=" << proxy_uri.getHost()
           << " port=" << proxy_uri.getPort();
        logger.debug(ss.str());

        session->setProxy(
            proxy_uri.getHost(),
            proxy_uri.getPort());

        if (!proxy_uri.getUserInfo().empty()) {
            Poco::Net::HTTPCredentials credentials;
            credentials.fromUserInfo(proxy_uri.getUserInfo());
            session->setProxyCredentials(
                credentials.getUsername(),
                credentials.getPassword());

            logger.debug("Proxy credentials detected username="
                         + credentials.getUsername());
        }
    }

    // Try to use user-configured proxy
    if (proxy_url.empty() && HTTPSClient::Config.UseProxy &&
            HTTPSClient::Config.ProxySettings.IsConfigured()) {
        session->setProxy(
            HTTPSClient::Config.ProxySettings.Host(),
            static_cast<Poco::UInt16>(
                HTTPSClient::Config.ProxySettings.Port()));

        std::stringstream ss;
        ss << "Proxy configured "
           << " host=" << HTTPSClient::Config.ProxySettings.Host()
           << " port=" << HTTPSClient::Config.ProxySettings.Port();
        logger.debug(ss.str());

        if (HTTPSClient::Config.ProxySettings.HasCredentials()) {
            session->setProxyCredentials(
                HTTPSClient::Config.ProxySettings.Username(),
                HTTPSClient::Config.ProxySettings.Password());

            logger.debug("Proxy credentials configured username="
                         + HTTPSClient::Config.ProxySettings.Username());
        }
    }

    return noError;
}

}   // namespace toggl
