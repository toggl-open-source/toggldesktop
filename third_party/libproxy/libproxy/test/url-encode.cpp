#include <cassert>
#include <iostream>
#include <string>


#include "url.hpp"

using namespace libproxy;

#define test(cond,rtv) _test(#cond, (cond), (rtv))
void _test (string exp, bool condition, bool &rtv)
{
	if (!condition) {
		cerr << "Failed: " << exp << endl;
		rtv = false;
	}
}

int main()
{
	bool rtv = true;
	string valid = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~";

	test(url::encode(valid) == valid, rtv);
	test(url::encode("é") == "%c3%a9", rtv);
	test(url::encode("+!@#$^\r%", "!#^") == "%2b!%40#%24^%0d%25", rtv);

	return !rtv;
}
