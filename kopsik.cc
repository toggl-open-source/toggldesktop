#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
	char* buf = getenv("TOGGL_API_TOKEN");
	if (!buf) {
		std::cerr << "Please set TOGGL_API_TOKEN in environment" << std::endl;
		return 1;
	}
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " COMMAND" << std::endl;
		return 1;
	}
	return 0;
}