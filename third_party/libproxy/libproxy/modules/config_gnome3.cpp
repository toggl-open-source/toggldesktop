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

#include <cstdio>         // For fileno(), fread(), pclose(), popen(), sscanf()
#include <sys/select.h>   // For select()
#include <fcntl.h>        // For fcntl()
#include <errno.h>        // For errno stuff
#include <sys/types.h>    // For stat()
#include <sys/stat.h>     // For stat()
#include <unistd.h>       // For pipe(), close(), vfork(), dup(), execl(), _exit()
#include <signal.h>       // For kill()

#include "../extension_config.hpp"
using namespace libproxy;

#define BUFFERSIZE 10240

#define PROXY_MODE			"org.gnome.system.proxy/mode"
#define PROXY_USE_AUTHENTICATION	"org.gnome.system.proxy.http/use-authentication"
#define PROXY_AUTH_PASSWORD		"org.gnome.system.proxy.http/authentication-password"
#define PROXY_AUTH_USER			"org.gnome.system.proxy.http/authentication-user"
#define PROXY_AUTOCONFIG_URL		"org.gnome.system.proxy/autoconfig-url"
#define PROXY_IGNORE_HOSTS		"org.gnome.system.proxy/ignore-hosts"
#define PROXY_HTTP_HOST			"org.gnome.system.proxy.http/host"
#define PROXY_HTTP_PORT			"org.gnome.system.proxy.http/port"
#define PROXY_FTP_HOST			"org.gnome.system.proxy.ftp/host"
#define PROXY_FTP_PORT			"org.gnome.system.proxy.ftp/port"
#define PROXY_SECURE_HOST		"org.gnome.system.proxy.https/host"
#define PROXY_SECURE_PORT		"org.gnome.system.proxy.https/port"
#define PROXY_SOCKS_HOST		"org.gnome.system.proxy.socks/host"
#define PROXY_SOCKS_PORT		"org.gnome.system.proxy.socks/port"
#define PROXY_SAME_FOR_ALL		"org.gnome.system.proxy/use-same-proxy"

static const char *all_keys[] = {
	"org.gnome.system.proxy",
	"org.gnome.system.proxy.http",
	"org.gnome.system.proxy.https",
	"org.gnome.system.proxy.ftp",
	"org.gnome.system.proxy.socks",
	NULL
};

static int popen2(const char *program, FILE** read, FILE** write, pid_t* pid) {
	if (!read || !write || !pid || !program || !*program)
		return EINVAL;
	*read  = NULL;
	*write = NULL;
	*pid   = 0;

	// Open the pipes
	int rpipe[2];
	int wpipe[2];
	if (pipe(rpipe) < 0)
		return errno;
	if (pipe(wpipe) < 0) {
		close(rpipe[0]);
		close(rpipe[1]);
		return errno;
	}

	switch (*pid = vfork()) {
	case -1: // Error
		close(rpipe[0]);
		close(rpipe[1]);
		close(wpipe[0]);
		close(wpipe[1]);
		return errno;

	case 0: // Child
		close(STDIN_FILENO);  // Close stdin
		close(STDOUT_FILENO); // Close stdout

		// Dup the read end of the write pipe to stdin
		// Dup the write end of the read pipe to stdout
		if (dup2(wpipe[0], STDIN_FILENO)  != STDIN_FILENO)  _exit(1);
		if (dup2(rpipe[1], STDOUT_FILENO) != STDOUT_FILENO) _exit(2);

		// Close unneeded fds
		for (int i = 3; i < sysconf(_SC_OPEN_MAX); i++)
			close(i);

		// Exec
		execl("/bin/sh", "sh", "-c", program, (char*) NULL);
		_exit(127);  // Whatever we do, don't return

	default: // Parent
		close(rpipe[1]);
		close(wpipe[0]);
		*read  = fdopen(rpipe[0], "r");
		*write = fdopen(wpipe[1], "w");
		if (*read == NULL || *write == NULL) {
			if (*read  != NULL) fclose(*read);
			if (*write != NULL) fclose(*write);
			return errno;
		}
		return 0;
	}
}

static inline uint16_t get_port(const string &port)
{
	uint16_t retval;

	if (sscanf(port.c_str(), "%hu", &retval) != 1)
		retval = 0;

	return retval;	
}

class gnome_config_extension : public config_extension {
public:
	gnome_config_extension() {
		// Build the command
		int count;
		struct stat st;
		string cmd = LIBEXECDIR "/pxgsettings";
		const char *pxgconf = getenv("PX_GSETTINGS");

		if (pxgconf)
			cmd = string (pxgconf);

		if (stat(cmd.c_str(), &st))
			throw runtime_error ("Unable to open gsettings helper!");

		for (count=0 ; all_keys[count] ; count++)
			cmd += string(" ", 1) + all_keys[count];

		// Get our pipes
		if (popen2(cmd.c_str(), &this->read, &this->write, &this->pid) != 0)
			throw runtime_error("Unable to run gconf helper!");

		// Read in our initial data
		this->read_data(count);

		// Set the read pipe to non-blocking
		if (fcntl(fileno(this->read), F_SETFL, O_NONBLOCK) == -1) {
			fclose(this->read);
			fclose(this->write);
			kill(this->pid, SIGTERM);
			throw runtime_error("Unable to set pipe to non-blocking!");
		}
	}

	~gnome_config_extension() {
		fclose(this->read);
		fclose(this->write);
		kill(this->pid, SIGTERM);
	}

	void store_response(const string &type,
						const string &host,
						const string &port,
						bool auth,
						const string &username,
						const string &password,
						vector<url> &response) {
		if (host != "" && get_port(port) != 0) {
			string tmp = type + "://";
			if (auth)
			  tmp += username + ":" + password + "@";
			tmp += host + ":" + port;
			response.push_back(url(tmp));
		}
	}

	vector<url> get_config(const url &dest) throw (runtime_error) {
		// Check for changes in the config
		fd_set rfds;
		struct timeval timeout = { 0, 0 };
		vector<url> response;

		FD_ZERO(&rfds);
		FD_SET(fileno(this->read), &rfds);
		if (select(fileno(this->read)+1, &rfds, NULL, NULL, &timeout) > 0)
			this->read_data();

		// Mode is wpad:// or pac+http://...
		if (this->data[PROXY_MODE] == "auto") {
			string pac = this->data[PROXY_AUTOCONFIG_URL];
			response.push_back(url::is_valid(pac) ? url(string("pac+") + pac) : url("wpad://"));
			return response;
		}

		// Mode is http://... or socks://...
		else if (this->data[PROXY_MODE] == "manual") {
			bool       auth = this->data[PROXY_USE_AUTHENTICATION] == "true";
			string username = url::encode(this->data[PROXY_AUTH_USER], URL_ALLOWED_IN_USERINFO_ELEMENT);
			string password = url::encode(this->data[PROXY_AUTH_PASSWORD], URL_ALLOWED_IN_USERINFO_ELEMENT);
			
			// Get the per-scheme proxy settings
			if (dest.get_scheme() == "http")
				store_response("http", this->data[PROXY_HTTP_HOST],
					this->data[PROXY_HTTP_PORT], auth, username, password, response);
			else if (dest.get_scheme() == "https")
				// It is expected that the configured server is an
				// HTTP server that support CONNECT method.
				store_response("http", this->data[PROXY_SECURE_HOST],
					this->data[PROXY_SECURE_PORT], auth, username, password, response);
			else if (dest.get_scheme() == "ftp")
				// It is expected that the configured server is an
				// HTTP server that handles proxying FTP URLs 
				// (e.g. request with header "Host: ftp://ftp.host.org")
				store_response("http", this->data[PROXY_FTP_HOST],
					this->data[PROXY_FTP_PORT], auth, username, password, response);

			store_response("socks", this->data[PROXY_SOCKS_HOST],
				this->data[PROXY_SOCKS_PORT], auth, username, password, response);

			// In case nothing matched, try HTTP Connect and fallback to direct.
			// If there is not secure HTTP proxy, this will only add direct:// to
			// the response
			if (response.size() == 0 && dest.get_scheme() != "http") {
				store_response("http", this->data[PROXY_SECURE_HOST],
					this->data[PROXY_SECURE_PORT], auth, username, password, response);
				response.push_back(url("direct://"));
			}
		}

		return response;
	}

	string get_ignore(const url&) {
		return this->data[PROXY_IGNORE_HOSTS];
	}

	bool set_creds(url /*proxy*/, string username, string password) {
		string auth = PROXY_USE_AUTHENTICATION "\ttrue\n";
		string user = string(PROXY_AUTH_USER "\t") + username + "\n";
		string pass = string(PROXY_AUTH_PASSWORD "\t") + password + "\n";

		return (fwrite(auth.c_str(), 1, auth.size(), this->write) == auth.size() &&
			fwrite(user.c_str(), 1, user.size(), this->write) == user.size() &&
			fwrite(pass.c_str(), 1, pass.size(), this->write) == pass.size());
	}

private:
	FILE* read;
	FILE* write;
	pid_t pid;
	map<string, string> data;

	bool read_data(int num=-1) {
		if (num == 0)    return true;
		if (!this->read) return false; // We need the pipe to be open

		for (char l[BUFFERSIZE] ; num != 0 && fgets(l, BUFFERSIZE, this->read) != NULL ; ) {
			string line = l;
			line        = line.substr(0, line.rfind('\n'));
			string key  = line.substr(0, line.find("\t"));
			string val  = line.substr(line.find("\t")+1);
			this->data[key] = val;
			if (num > 0) num--;
		}

		return (num <= 0);
	}
};

static base_extension** gnome_config_extension_init() {
	base_extension** retval = new base_extension*[2];
	retval[1] = NULL;
	try {
		retval[0] = new gnome_config_extension();
		return retval;
	}
	catch (runtime_error) {
		delete[] retval;
		return NULL;
	}
}

static bool gnome_config_extension_test() {
	return (getenv("GNOME_DESKTOP_SESSION_ID")
			|| (getenv("DESKTOP_SESSION")
				&& string(getenv("DESKTOP_SESSION")) == "gnome"));
}

MM_MODULE_INIT(gnome_config_extension,
			   gnome_config_extension_init,
			   gnome_config_extension_test,
			   NULL, NULL);
