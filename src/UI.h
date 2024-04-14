#pragma once

#include <iostream>
#include <functional>
#include "Location.h"


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
	std::vector<MenuItem> preferencesMenu;
	
	void displayMenu(std::vector<MenuItem>);
	void updatePreference(std::string key);
	void quickSearch();
	void locationData(Location& l);
	void forecastData(Location& l);
	void historicData(Location& l);

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

