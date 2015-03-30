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

#ifdef __MACH__
#include <CoreFoundation/CoreFoundation.h>  // NOLINT
#include <CoreServices/CoreServices.h>  // NOLINT
#endif

#ifdef _WIN32
//#include <winhttp.h>
#endif

namespace toggl {

error Netconf::autodetectProxy(
    const std::string &encoded_url,
    std::string *proxy_url) {

    *proxy_url = "";

    // Inspired from Stack Overflow
    // http://stackoverflow.com/questions/202547/how-do-i-find-out-the-browsers-proxy-settings
#ifdef _WIN32
	/*
	if (WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig)) {
        if (ieProxyConfig.fAutoDetect) {
            fAutoProxy = TRUE;
        }

        if (ieProxyConfig.lpszAutoConfigUrl != NULL) {
            fAutoProxy = TRUE;
            autoProxyOptions.lpszAutoConfigUrl =
                ieProxyConfig.lpszAutoConfigUrl;
        }
    } else {
        // use autoproxy
        fAutoProxy = TRUE;
    }

    if (fAutoProxy) {
        if (autoProxyOptions.lpszAutoConfigUrl != NULL) {
            autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
        } else {
            autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
            autoProxyOptions.dwAutoDetectFlags =
                WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
        }

        // basic flags you almost always want
        autoProxyOptions.fAutoLogonIfChallenged = TRUE;

        // here we reset fAutoProxy in case an auto-proxy isn't actually
        // configured for this url
        fAutoProxy = WinHttpGetProxyForUrl(
            hiOpen, pwszUrl, &autoProxyOptions, &autoProxyInfo);
    }

    if (fAutoProxy) {
        // set proxy options for libcurl based on autoProxyInfo
    } else {
        if (ieProxyConfig.lpszProxy != NULL) {
            // IE has an explicit proxy. set proxy options for libcurl here
            // based on ieProxyConfig
            //
            // note that sometimes IE gives just a single or double colon
            // for proxy or bypass list, which means "no proxy"
        } else {
            // there is no auto proxy and no manually configured proxy
        }
    }
	*/
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

void Netconf::ConfigureProxy(
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
}

}   // namespace toggl
