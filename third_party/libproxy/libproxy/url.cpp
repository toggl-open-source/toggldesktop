/*******************************************************************************
 * libproxy - A library for proxy configuration
 * Copyright (C) 2006 Nathaniel McCallum <nathaniel@natemccallum.com>
 *
 * Based on work found in GLib GIO:
 * Copyright (C) 2006-2007 Red Hat, Inc.
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
 *
 ******************************************************************************/
#ifdef WIN32
#include <io.h>
#define open _open
#define O_RDONLY _O_RDONLY
#define close _close
#endif
#include <fcntl.h> // For ::open()
#include <cstring> // For memcpy()
#include <sstream> // For int/string conversion (using stringstream)
#include <cstdio>  // For sscanf()
#include <cstdlib>    // For atoi()
#include <sys/stat.h> // For stat()
#include <algorithm> // For transform()
#include <unistd.h>  // For read() close()

#ifdef WIN32
#include <io.h>
#define close _close
#define read _read
#define SHUT_RDWR SD_BOTH
#endif

#include "url.hpp"
using namespace libproxy;
using namespace std;

// This mime type should be reported by the web server
#define PAC_MIME_TYPE "application/x-ns-proxy-autoconfig"
// Fall back to checking for this mime type, which servers often report wrong
#define PAC_MIME_TYPE_FB "text/plain"

// This is the maximum pac size (to avoid memory attacks)
#define PAC_MAX_SIZE 102400

static inline int get_default_port(string scheme) {
	struct servent *serv;
	size_t plus = scheme.find('+');

	if (plus != string::npos)
		scheme = scheme.substr(plus + 1);

	if ((serv = getservbyname(scheme.c_str(), NULL)))
		return ntohs(serv->s_port);

	return 0;
}

template <class T>
static inline string to_string_ (const T& t) {
	stringstream ss;
	ss << t;
	return ss.str();
}

#define _copyaddr_t(type, addr) (sockaddr*) memcpy(new type, &(addr), sizeof(type))
static inline sockaddr* copyaddr(const struct sockaddr& addr) {
	switch (addr.sa_family) {
	case (AF_INET):
		return _copyaddr_t(sockaddr_in, addr);
	case (AF_INET6):
		return _copyaddr_t(sockaddr_in6, addr);
	default:
		return NULL;
	}
}

bool url::is_valid(const string url_) {
	bool rtv = true;

	try {
		url url(url_);
	} catch (parse_error&) {
		rtv = false;
	}

	return rtv;
}

string url::encode(const string &data, const string &valid_reserved) {
	ostringstream encoded;
	for (unsigned int i=0; i < data.size(); i++) {
		if (isalnum((unsigned char)data[i])
				|| valid_reserved.find(data[i]) != string::npos
				|| string("-._~").find(data[i]) != string::npos)
			encoded << data[i];
		else
			encoded << '%' 
					<< ((unsigned char)data[i] < 16 ? "0" : "")
					<< hex << (((int)data[i]) & 0xff);
	}
	return encoded.str();
}

url::url(const string &url) throw(parse_error)
	: m_orig(url), m_port(0), m_ips(NULL) {
	size_t idx = 0;
	size_t hier_part_start, hier_part_end;
	size_t path_start, path_end;
	string hier_part;

	/* From RFC 3986 Decodes:
	 * URI         = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
	 */

	idx = 0;

	/* Decode scheme:
	 * scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
	 */

	if (!isalpha(url[idx]))
		throw parse_error("Invalid URL: " + url);

	while (1) {
		char c = url[idx++];

		if (c == ':') break;

		if (!(isalnum(c) ||
					c == '+' ||
					c == '-' ||
					c == '.'))
			throw parse_error("Invalid URL: " + url);
	}

	m_scheme = url.substr(0, idx - 1);
	transform(m_scheme.begin(), m_scheme.end(), m_scheme.begin(), ::tolower);

	hier_part_start = idx;
	hier_part_end = url.find('?', idx);
	if (hier_part_end == string::npos)
		hier_part_end = url.find('#', idx);

	hier_part = url.substr(hier_part_start,
							hier_part_end == string::npos ?
								string::npos : hier_part_end - hier_part_start);

	/*  3:
	 *	hier-part   = "//" authority path-abempty
	 *		      / path-absolute
	 *		      / path-rootless
	 *		      / path-empty
	 */

	if (hier_part.size() >= 2 && hier_part[0] == '/' && hier_part[1] == '/') {
		size_t authority_start, authority_end;
		size_t userinfo_start, userinfo_end;
		size_t host_start, host_end;

		authority_start = 2;
		/* authority is always followed by / or nothing */
		authority_end = hier_part.find('/', authority_start);
		if (authority_end == string::npos)
			authority_end = hier_part.size();
		path_start = authority_end;

		/* 3.2:
		 *      authority   = [ userinfo "@" ] host [ ":" port ]
		 */

		/* Get user and password */
		userinfo_start = authority_start;
		userinfo_end = hier_part.find('@', authority_start);
		if (userinfo_end != string::npos) {
			size_t user_end;

			user_end = hier_part.rfind(':', userinfo_end);
			if (user_end == string::npos)
				user_end = userinfo_end;
			else
				m_pass = hier_part.substr(user_end + 1, userinfo_end - (user_end + 1));

			m_user = hier_part.substr(userinfo_start, user_end - userinfo_start);
		}

		/* Get hostname */
		if (userinfo_end == string::npos)
			host_start = authority_start;
		else
			host_start = userinfo_end + 1;

		/* Check for IPv6 IP */
		if (host_start < hier_part.size()
				&& hier_part[host_start] == '[') {
			host_end = hier_part.find(']', host_start);
			if (host_end == string::npos)
				throw parse_error("Invalid URL: " + url);
			host_end++;
			if (hier_part[host_end] == '\0')
				host_end = string::npos;
		} else {
			host_end = hier_part.find(':', host_start);
			if (path_start < host_end)
				host_end = path_start;
		}

		/* If not port, host ends where path starts */
		if (host_end == string::npos)
			host_end = path_start;

		m_host = hier_part.substr(host_start, host_end - host_start);
		transform(m_host.begin(), m_host.end(), m_host.begin(), ::tolower);

		/* Get port */
		m_port = get_default_port(m_scheme);

		if (host_end < hier_part.size()
			&& hier_part[host_end] == ':') {
			size_t port_start = host_end + 1;
			m_port = atoi(hier_part.c_str() + port_start);
		}
	} else {
		path_start = 0;
	}

	/* Get path */
	if (path_start != string::npos)
	{
		path_end = hier_part_end;
		if (path_end == string::npos)
			m_path = hier_part.substr(path_start);
		else
			m_path = hier_part.substr(path_start, path_end - path_start);
	}
}

url::url(const url &url) : m_ips(NULL) {
	*this = url;
}

url::~url() {
	empty_cache();
}

bool url::operator==(const url& url) const {
	return m_orig == url.to_string();
}

url& url::operator=(const url& url) {
	// Ensure these aren't the same objects
	if (&url == this)
		return *this;

	m_host   = url.m_host;
	m_orig   = url.m_orig;
	m_pass   = url.m_pass;
	m_path   = url.m_path;
	m_port   = url.m_port;
	m_scheme = url.m_scheme;
	m_user   = url.m_user;

	empty_cache();

	if (url.m_ips) {
		int i;

		// Copy the new ip cache
		for (i=0 ; url.m_ips[i] ; i++) {};
		m_ips = new sockaddr*[i];
		for (i=0 ; url.m_ips[i] ; i++)
			m_ips[i] = copyaddr(*url.m_ips[i]);
	}

	return *this;
}

url& url::operator=(string strurl) throw (parse_error) {
	url tmp(strurl);
	*this = tmp;
	return *this;
}

string url::get_host() const {
	return m_host;
}

sockaddr const* const* url::get_ips(bool usedns) {
	// Check the cache
	if (m_ips)
		return m_ips;

	// Check without DNS first
	if (usedns && get_ips(false))
		return m_ips;

	// Check DNS for IPs
	struct addrinfo* info;
	struct addrinfo flags;
	memset(&flags, '\0', sizeof(addrinfo));
	flags.ai_family   = AF_UNSPEC;
	flags.ai_socktype = 0;
	flags.ai_protocol = 0;
	flags.ai_flags    = AI_NUMERICHOST;
	if (!getaddrinfo(m_host.c_str(), NULL, usedns ? NULL : &flags, &info)) {
		struct addrinfo* first = info;
		unsigned int i = 0;

		// Get the size of our array
		for (info = first ; info ; info = info->ai_next)
			i++;

		// Return NULL if no IPs found
		if (i == 0)
			return m_ips = NULL;

		// Create our array since we actually have a result
		m_ips = new sockaddr*[++i];
		memset(m_ips, '\0', sizeof(sockaddr*)*i);

		// Copy the sockaddr's into m_ips
		for (i = 0, info = first ; info ; info = info->ai_next) {
			if (info->ai_addr->sa_family == AF_INET || info->ai_addr->sa_family == AF_INET6) {
				m_ips[i] = copyaddr(*(info->ai_addr));
				if (!m_ips[i]) break;
				((sockaddr_in **) m_ips)[i++]->sin_port = htons(m_port);
			}
		}

		freeaddrinfo(first);
		return m_ips;
	}

	// No addresses found
	return NULL;
}

string url::get_password() const {
	return m_pass;
}

string url::get_path() const {
	return m_path;
}

uint16_t url::get_port() const {
	return m_port;
}

string url::get_scheme() const {
	return m_scheme;
}

string url::get_username() const {
	return m_user;
}

string url::to_string() const {
	return m_orig;
}

static inline string recvline(int fd) {
	// Read a character.
	// If we don't get a character, return empty string.
	// If we are at the end of the line, return empty string.
	char c = '\0';
	
	if (recv(fd, &c, 1, 0) != 1 || c == '\n')
		return "";

	return string(1, c) + recvline(fd);
}

char* url::get_pac() {
	int sock = -1;
	bool chunked = false;
	unsigned long int content_length = 0, status = 0;
	char* buffer = NULL;
	string request;

	// In case of a file:// url we open the file and read it
	if (m_scheme == "file" || m_scheme == "pac+file") {
		struct stat st;
		if ((sock = ::open(m_path.c_str(), O_RDONLY)) < 0)
			return NULL;

		if (!fstat(sock, &st) && st.st_size < PAC_MAX_SIZE) {
			buffer = new char[st.st_size+1];
			memset(buffer, 0, st.st_size+1);
			if (read(sock, buffer, st.st_size) == 0) {
				delete[] buffer;
				buffer = NULL;
			}
		}
		return buffer;
	}

	// DNS lookup of host
	if (!get_ips(true))
		return NULL;

	// Iterate through each IP trying to make a connection
	// Stop at the first one
	for (int i=0 ; m_ips[i] ; i++) {
		sock = socket(m_ips[i]->sa_family, SOCK_STREAM, 0);
		if (sock < 0) continue;

		if (m_ips[i]->sa_family == AF_INET &&
			!connect(sock, m_ips[i], sizeof(struct sockaddr_in)))
			break;
		else if (m_ips[i]->sa_family == AF_INET6 &&
			!connect(sock, m_ips[i], sizeof(struct sockaddr_in6)))
			break;

		close(sock);
		sock = -1;
	}

	// Test our socket
	if (sock < 0) return NULL;

	// Build the request string
	request  = "GET " + (m_path.size() > 0 ? m_path : "/") + " HTTP/1.1\r\n";
	request += "Host: " + m_host + "\r\n";
	request += "Accept: " + string(PAC_MIME_TYPE) + "\r\n";
	request += "Connection: close\r\n";
	request += "\r\n";

	// Send HTTP request
	if ((size_t) send(sock, request.c_str(), request.size(), 0) != request.size()) {
		close(sock);
		return NULL;
	}

	/* Verify status line */
	string line = recvline(sock);
	if (sscanf(line.c_str(), "HTTP/1.%*d %lu", &status) == 1 && status == 200) {
		/* Check for correct mime type and content length */
		for (line = recvline(sock) ; line != "\r" && line != "" ; line = recvline(sock)) {
			// Check for chunked encoding
			if (line.find("Content-Transfer-Encoding: chunked") == 0 || line.find("Transfer-Encoding: chunked") == 0)
				chunked = true;

			// Check for content length
			else if (content_length == 0)
				sscanf(line.c_str(), "Content-Length: %lu", &content_length);
		}

		// Get content
		unsigned int recvd = 0;
		buffer = new char[PAC_MAX_SIZE];
		memset(buffer, 0, PAC_MAX_SIZE);
		do {
			unsigned int chunk_length;

			if (chunked) {
				// Discard the empty line if we received a previous chunk
				if (recvd > 0) recvline(sock);

				// Get the chunk-length line as an integer
				if (sscanf(recvline(sock).c_str(), "%x", &chunk_length) != 1 || chunk_length == 0) break;

				// Add this chunk to our content length,
				// ensuring that we aren't over our max size
				content_length += chunk_length;
			}

			if (content_length >= PAC_MAX_SIZE) break;

			while (recvd != content_length) {
				int r = recv(sock, buffer + recvd, content_length - recvd, 0);
				if (r <= 0) {
					chunked = false;
					break;
				}
				recvd += r;
			}
		} while (chunked);

		if (string(buffer).size() != content_length) {
			delete[] buffer;
			buffer = NULL;
		}
	}

	// Clean up
	shutdown(sock, SHUT_RDWR);
	return buffer;
}

void url::empty_cache()
{
	if (m_ips) {
		// Free any existing ip cache
		for (int i=0 ; m_ips[i] ; i++)
			delete m_ips[i];
		delete[] m_ips;
		m_ips = NULL;
	}
}
