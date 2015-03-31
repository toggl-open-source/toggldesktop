#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "url.hpp"

using namespace libproxy;

class TestServer {
	public:
		TestServer(in_port_t port)
			: m_port(port)
			  ,m_sock(-1)

		{
			m_pipe[0] = -1;
			m_pipe[1] = -1;
		}

		void start()
		{
			struct sockaddr_in addr = {0};
			int ret;
			int i = 1;

			addr.sin_family = AF_INET;
			inet_aton("127.0.0.1", &addr.sin_addr);
			addr.sin_port = ntohs(m_port);

			if (m_sock != -1)
				return;

			m_sock = socket(AF_INET, SOCK_STREAM, 0);
			assert(m_sock > 0);

			setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

			ret = bind(m_sock, (sockaddr*)&addr, sizeof (struct sockaddr_in));
			assert(!ret);

			ret = listen(m_sock, 1);
			assert(!ret);

			ret = pipe(m_pipe);
			assert(!ret);

			ret = pthread_create(&m_thread, NULL, TestServer::server_thread, this);
			assert(!ret);
		}

		void stop()
		{
			assert (write(m_pipe[1], (void*)"q", 1) == 1);
			pthread_join (m_thread, NULL);
			close(m_pipe[1]);
			m_pipe[1] = -1;
			close(m_sock);
			m_sock = -1;
		}

	private:
		static void * server_thread(void *data)
		{
			TestServer *server = (TestServer*)data;
			while (server->loop()) {}
			return NULL;
		}

		bool loop()
		{
			int ret;
			fd_set fds;
			struct sockaddr_in addr;
			int lf_count = 0;
			bool done = false;
			char buffer[1024];
			char *ptr;
			int csock;
			socklen_t len = sizeof (struct sockaddr_in);

			FD_ZERO (&fds);
			FD_SET (m_pipe[0], &fds);
			FD_SET (m_sock, &fds);

			memset(buffer, 0, 1024);

			// Wait for connection
			ret = select(max(m_pipe[0], m_sock) + 1, &fds, NULL, NULL, NULL);
			assert(ret > 0);

			if (FD_ISSET (m_pipe[0], &fds)) {
				close(m_pipe[0]);
				m_pipe[0] = -1;

				return false;
			}

			csock = accept(m_sock, (sockaddr*) &addr, &len);
			assert(csock > 0);

			// Read request
			ptr = buffer;
			do {
				ret = recv (csock, ptr, 1, 0);
				if (ret <= 0)
					goto done;

				if (*ptr == '\n') {
					lf_count++;
				} else if (*ptr != '\r') {
					lf_count = 0;
				}

				if (lf_count == 2)
					done = true;

				ptr++;
				assert((int) (ptr - buffer) < 1024);
			} while (!done);

			if (strstr(buffer, "basic")) {
				sendBasic(csock);
			} else if (strstr(buffer, "truncated")) {
				sendTruncated(csock);
			} else if (strstr(buffer, "overflow")) {
				sendOverflow(csock);
			} else if (strstr(buffer, "chunked")) {
				sendChunked(csock);
			} else {
				assert(!"Unsupported request");
			}

done:
			close(csock);
			return true;
		}

		void sendBasic(int csock)
		{
			int ret;
			const char *basic =
				"HTTP/1.1 200 OK\n" \
				"Content-Type: text/plain\n" \
				"Content-Length: 10\n" \
				"\n" \
				"0123456789";
			ret = send(csock, (void*)basic, strlen(basic), 0);
			assert(ret == strlen(basic));
			shutdown(csock, SHUT_RDWR);
			close(ret);
		}

		void sendTruncated(int csock)
		{
			int ret;
			const char *basic =
				"HTTP/1.1 200 OK\n" \
				"Content-Type: text/plain\n" \
				"Content-Length: 10\n" \
				"\n" \
				"01234";
			ret = send(csock, (void*)basic, strlen(basic), 0);
			assert(ret == strlen(basic));
			shutdown(csock, SHUT_RDWR);
			close(ret);
		}

		void sendOverflow(int csock)
		{
			int ret;
			int size = 500000;
			char *buf = new char[size];
			memset(buf, 1, size);

			const char *basic =
				"HTTP/1.1 200 OK\n" \
				"Content-Type: text/plain\n" \
				"Content-Length: 500000\n" \
				"\n";
			ret = send(csock, (void*)basic, strlen(basic), 0);
			assert(ret == strlen(basic));
			ret = send(csock, (void*)buf, size, 0);
			assert(ret != size);
			delete[] buf;
			shutdown(csock, SHUT_RDWR);
			close(ret);
		}

		void sendChunked(int csock)
		{
			int ret;
			const char *chunked =
				"HTTP/1.1 200 OK\n" \
				"Content-Type: text/plain\n" \
				"Transfer-Encoding: chunked\n" \
				"\n" \
				"5\n" \
				"01234" \
				"\n" \
				"5\n" \
				"56789" \
				"\n";
			ret = send(csock, (void*)chunked, strlen(chunked), 0);
			assert(ret == strlen(chunked));
			shutdown(csock, SHUT_RDWR);
			close(ret);
		}

		in_port_t m_port;
		int m_sock;
		int m_pipe[2];
		pthread_t m_thread;
};


int main()
{
	TestServer server(1983);
	int rtv = 0;
	char *pac;

	url basic("http://localhost:1983/basic.js");
	url truncated("http://localhost:1983/truncated.js");
	url overflow("http://localhost:1983/overflow.js");
	url chunked("http://localhost:1983/chunked.js");

	server.start();

	pac = basic.get_pac();
	assert(pac != NULL);
	assert(strlen(pac) == 10);
	assert(!strcmp("0123456789", pac));
	delete[] pac;

	pac = truncated.get_pac();
	assert(pac == NULL);

	pac = overflow.get_pac();
	assert(pac == NULL);

	pac = chunked.get_pac();
	assert(pac != NULL);
	assert(strlen(pac) == 10);
	assert(!strcmp("0123456789", pac));
	delete[] pac;

	server.stop();

	return 0;
}
