#include "UI.h"
#include <conio.h>
#include "StorageManager.h"
#include <list>
#include <string>

UI::UI()
{
	StorageManager sm;

	std::cout << "This is the constructor for UI" << std::endl;

	mainMenu = {
		{"=== Main Menu ===", []() {}},
		{"Set Preferences", [&]() { displayMenu(preferencesMenu); }},
		{"Go to Second Menu", [&]() { displayMenu(secondMenu); }}
	};

	secondMenu = {
		{"=== Second Menu ===", []() {}},
		{"Option 1", []() { std::cout << "You selected option 1" << std::endl; }},
		{"Go to Main Menu", [&]() { displayMenu(mainMenu); }}
	};

	preferencesMenu = {
		{"=== User Preferences ===", []() {}},
		{"Temperature Unit (" + sm.getPreference("tempUnit") + ")", [&]() { updatePreference("tempUnit"); }},
		{"Wind Speed Unit (" + sm.getPreference("windSpeedUnit") + ")", [&]() { updatePreference("windSpeedUnit"); }},
		{"Pressure Unit (" + sm.getPreference("pressureUnit") + ")", [&]() { updatePreference("pressureUnit"); }},
		{"Precipitation Unit (" + sm.getPreference("precipitationUnit") + ")", [&]() { updatePreference("precipitationUnit"); }},
		{"Time Format (" + sm.getPreference("timeFormat") + ")", [&]() { updatePreference("timeFormat"); }},
		{"Past Days (" + sm.getPreference("pastDays") + ")", [&]() { updatePreference("pastDays"); }},
		{"Forecast Days (" + sm.getPreference("forecastDays") + ")", [&]() { updatePreference("forecastDays"); }},
		{"Time Zone (" + sm.getPreference("timeZone") + ")", [&]() { updatePreference("timeZone"); }},
		{"Go to Main Menu", [&]() { displayMenu(mainMenu); }}
	};
	
	displayMenu(mainMenu);
}

void UI::displayMenu(std::vector<MenuItem> menuItems) {
	clearScreen();
	
	for (int i = 0; i < menuItems.size(); i++) {
		if(i == 0) std::cout << menuItems[i].label << std::endl;
		else std::cout << i << ". " << menuItems[i].label << std::endl;
	}

	std::string choiceStr;
	std::cout << "Enter your choice: ";
	std::cin >> choiceStr;
	
	try {
		int choice = std::stoi(choiceStr);
		
		if (choice > 0 && choice <= menuItems.size() - 1) menuItems[choice].action();
		else throw std::exception();
	}
	catch (std::exception e) {
		std::cout << "Invalid choice. Press enter to dismiss" << std::endl;
		_getch();
		displayMenu(menuItems);
	}
}

void UI::updatePreference(std::string key) {
	StorageManager sm;
	
	std::list<std::string> values = sm.getPrefAllowedValues(key);

	std::vector<MenuItem> thisMenu = {
		{"=== Updating " + key + " ===\nPossible values: ", []() {}}
		//{"Celsius (c)", [&sm, &key]() { sm.setPreference(key, "c"); }},
		//{"Fahrenheit (f)", [&sm, &key]() { sm.setPreference(key, "f"); }}
	};

	for (std::string &value : values) {
		thisMenu.push_back({ value, [&sm, &key, &value]() { sm.setPreference(key, value); } });
	}

	displayMenu(thisMenu);
}