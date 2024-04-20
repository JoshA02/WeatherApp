#pragma once

#include <iostream>
#include <functional>
#include "Location.h"
#include "Utils.h"
#include <vector>
#include "API.h"


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
	std::vector<MenuItem> preferencesMenu;
	std::vector<MenuItem> dataPreferencesMenu;
	
	void displayMenuNoInput(std::vector<MenuItem> menuItems);
	int getChoice(std::vector<MenuItem> menuItems);
	void displayMenu(std::vector<MenuItem>);

	void updatePreference(std::string key, std::vector<MenuItem>);
	void quickSearch();
	
	Location selectFavourite(std::string header, std::vector<MenuItem> menu);
	void showFavourites();
	void manageFavourites();
	void addFavourite();
	void changeFavouriteName(Location l);
	
	void locationData(Location& l);
	void dailyData(Location& l);
	void forecastData(Location& l);
	void historicData(Location& l);

	void displayDay(Location& l, std::vector<dayData> days, int dayIndex);
	void displayHour(Location& l, std::vector<dayData> days, int dayIndex, int hourIndex); // day index is needed to get the date, and to return to the correct day when the user goes back.

	DateRange askForDateRange();

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

