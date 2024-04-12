#pragma once

#include <iostream>
#include <functional>


class UI
{
public:
	UI();
private:
	struct MenuItem {
		std::string label;
		std::function<void()> action;
	};

	std::vector<MenuItem> mainMenu;
	std::vector<MenuItem> secondMenu;
	
	void displayMenu(std::vector<MenuItem>);

	#ifdef _WIN32
	// Windows
		void clearScreen() {
			system("cls");
		}
	#else
	// Unix
		void clearScreen() {
			system("clear");
		}
	#endif
};

