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

#ifdef __linux__
#include <sys/types.h>
#include <sys/wait.h>
#include <spawn.h>
#endif

namespace toggl {

error Netconf::autodetectProxy(
    const std::string encoded_url,
    std::vector<std::string> *proxy_strings) {

    poco_assert(proxy_strings);

    proxy_strings->clear();

    if (encoded_url.empty()) {
        return noError;
    }

#ifdef _WIN32
    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ie_config = { 0 };
    if (!WinHttpGetIEProxyConfigForCurrentUser(&ie_config)) {
        std::stringstream ss;
        ss << "WinHttpGetIEProxyConfigForCurrentUser error: "
           << GetLastError();
        return ss.str();
    }
    if (ie_config.lpszProxy) {
        std::wstring proxy_url_wide(ie_config.lpszProxy);
        std::string s("");
        Poco::UnicodeConverter::toUTF8(proxy_url_wide, s);
        proxy_strings->push_back(s);
    }
#endif

    // Inspired by VLC source code
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
                proxy_strings->push_back(std::string(buffer));
            }
        }

        CFRelease(dicRef);
    }
#endif

    // Inspired by VLC source code
    // https://github.com/videolan/vlc/blob/master/src/posix/netconf.c
#ifdef __linux__
    /* FIXME:
        posix_spawn_file_actions_t actions;
        posix_spawn_file_actions_init(&actions);
        posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, "/dev/null",
                                         O_RDONLY, 0644);
        int fd[2];
        posix_spawn_file_actions_adddup2(&actions, fd[1], STDOUT_FILENO);

        posix_spawnattr_t attr;
        posix_spawnattr_init(&attr);
        {
            sigset_t set;
            sigemptyset(&set);

            posix_spawnattr_setsigmask(&attr, &set);
            sigaddset(&set, SIGPIPE);
            posix_spawnattr_setsigdefault(&attr, &set);
            posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGDEF
                                     | POSIX_SPAWN_SETSIGMASK);
        }

        pid_t pid;
        char *argv[3] = { const_cast<char *>("proxy"),
                          const_cast<char *>(encoded_url.c_str()), NULL
                        };
        if (posix_spawnp(&pid, "proxy", &actions, &attr, argv, environ)) {
            pid = -1;
        }

        posix_spawnattr_destroy(&attr);
        posix_spawn_file_actions_destroy(&actions);
        close(fd[1]);

        if (-1 == pid) {
            close(fd[0]);
            return error("Failed to run proxy command");
        }

        char buf[1024];
        size_t len = 0;
        do {
            ssize_t val = read(fd[0], buf + len, sizeof (buf) - len);
            if (val <= 0) {
                break;
            }
            len += val;
        } while (len < sizeof (buf));

        close(fd[0]);

        while (true) {
            int status = {0};
            if (-1 != waitpid(pid, &status, 0)) {
                break;
            }
        }

        if (len >= 9 && !strncasecmp(buf, "direct://", 9)) {
            return noError;
        }

        char *end = static_cast<char *>(memchr(buf, '\n', len));
        if (end != NULL) {
            *end = '\0';
            proxy_strings->push_back(std::string(buf));
        }
    */
#endif

    return noError;
}

error Netconf::ConfigureProxy(
    const std::string encoded_url,
    Poco::Net::HTTPSClientSession *session) {

    Poco::Logger &logger = Poco::Logger::get("ConfigureProxy");

    std::string proxy_url("");
    if (HTTPSClient::Config.AutodetectProxy) {
        if (Poco::Environment::has("HTTPS_PROXY")) {
            proxy_url = Poco::Environment::get("HTTPS_PROXY");
        }
        if (Poco::Environment::has("https_proxy")) {
            proxy_url = Poco::Environment::get("https_proxy");
        }
        if (Poco::Environment::has("HTTP_PROXY")) {
            proxy_url = Poco::Environment::get("HTTP_PROXY");
        }
        if (Poco::Environment::has("http_proxy")) {
            proxy_url = Poco::Environment::get("http_proxy");
        }
        if (proxy_url.empty()) {
            std::vector<std::string> proxy_strings;
            error err = autodetectProxy(encoded_url, &proxy_strings);
            if (err != noError) {
                return err;
            }
            if (!proxy_strings.empty()) {
                proxy_url = proxy_strings[0];
            }
        }

        if (!proxy_url.empty()) {
            if (proxy_url.find("://") == std::string::npos) {
                proxy_url = "http://" + proxy_url;
            }
            Poco::URI proxy_uri(proxy_url);

            std::stringstream ss;
            ss << "Using proxy URI=" + proxy_uri.toString()
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
