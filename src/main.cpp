#ifndef TEST_MODE

#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <map>
#include <iomanip>
#include "API.h"
#include "UI.h"
#include "StorageManager.h"
// #include <conio.h>

int main()
{
	std::locale::global(std::locale("en_US.UTF-8"));
	try {
		UI ui;
	} catch (std::exception& e) {
		// Should be caught by the UI class, but just in case.
		std::cout << e.what() << std::endl;
		std::cout << "Press any key to try again or close the app to exit" << std::endl;
		int ch = std::cin.get();
		main();
	}

	return 0;
}

#endif // HEADER_H_