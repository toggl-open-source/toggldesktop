#include <cassert>
#include <iostream>
#include <sstream>
#include <string>


#include "url.hpp"

using namespace libproxy;

void assert_equal_str (const std::string &what,
					   const std::string &in,
					   const std::string &value,
					   const std::string &ref)
{
	if (value != ref)
		throw logic_error(what + " in URL " + in + " is '"
							+ value + "' instead of '" + ref + "'");
}

void assert_equal_int (const std::string &what,
					   const std::string &in,
					   int value,
					   int ref)
{
	if (value != ref) {
		ostringstream s;
		s << what << " in URL " << in << " is "
			<< value << " instead of " << ref;
		throw logic_error(s.str());
	}
}

void try_url (std::string link, bool &rtv,
			  const std::string &scheme,
			  const std::string &user,
			  const std::string &pass,
			  const std::string &host,
			  int port,
			  const std::string &path)
{
  try {
      url u(link);

	  assert_equal_str ("Scheme", u.to_string(), u.get_scheme(), scheme);
	  assert_equal_str ("User", u.to_string(), u.get_username(), user);
	  assert_equal_str ("Pass", u.to_string(), u.get_password(), pass);
	  assert_equal_str ("Host", u.to_string(), u.get_host(), host);
	  assert_equal_int ("Port", u.to_string(), u.get_port(), port);
	  assert_equal_str ("Path", u.to_string(), u.get_path(), path);
  }
  catch (exception &e) {
      std::cerr << e.what() << std::endl;
      rtv = false;
  }
}

int main()
{
  bool rtv = true;

#ifdef WIN32
  WSADATA wsadata;
  WORD vers = MAKEWORD(2, 2);
  WSAStartup(vers, &wsadata);
#endif

  try_url ("file:///allo", rtv,
		  "file",
		  "", "",
		  "", 0,
		  "/allo");

  try_url ("http://test.com", rtv,
		  "http",
		  "", "",
		  "test.com", 80,
		  "");

  try_url ("http://test.com/", rtv,
		  "http",
		  "", "",
		  "test.com", 80,
		  "/");

  try_url ("http://test.com#", rtv,
		  "http",
		  "", "",
		  "test.com", 80,
		  "");

  try_url ("http://test.com?", rtv,
		  "http",
		  "", "",
		  "test.com", 80,
		  "");

  try_url ("http://nicolas@test.com", rtv,
		  "http",
		  "nicolas", "",
		  "test.com", 80,
		  "");

  try_url ("http://nicolas:@test.com", rtv,
		  "http",
		  "nicolas", "",
		  "test.com", 80,
		  "");

  try_url ("http://nicolas:secret@test.com", rtv,
		  "http",
		  "nicolas", "secret",
		  "test.com", 80,
		  "");

  try_url ("http://:secret@test.com", rtv,
		  "http",
		  "", "secret",
		  "test.com", 80,
		  "");

  try_url ("http+ssh://:secret@test.com", rtv,
		  "http+ssh",
		  "", "secret",
		  "test.com", 22,
		  "");

  try_url ("HtTp://TeSt.CoM/ALLO", rtv,
		  "http",
		  "", "",
		  "test.com", 80,
		  "/ALLO");

  /* This is a very uncommon but valid case that use to cause crash */
  try_url ("www.google.com:80", rtv,
		   "www.google.com",
		   "", "",
		   "", 0,
		   "80");

  /* Handle the case where there is nothing other than the scheme */
  try_url ("http://", rtv,
		   "http",
		   "", "",
		   "", 80,
		   "");

  /* Check windows style URL */
  try_url ("file:///E:/dir/file.pac", rtv,
		  "file",
		  "", "",
		  "", 0,
#ifdef WIN32
          "E:\\dir\\file.pac");
#else
          "/E:/dir/file.pac");
#endif

  /* UNC windows URL */
  try_url ("file://server/dir/file.pac", rtv,
		  "file",
		  "", "",
		  "server", 0,
#ifdef WIN32
          "\\\\server\\dir\\file.pac");
#else
          "/dir/file.pac");
#endif

#ifdef WIN32
  WSACleanup();
#endif
  
  return !rtv;
}
