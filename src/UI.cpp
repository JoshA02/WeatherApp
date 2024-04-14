#include "UI.h"
#include <conio.h>
#include "StorageManager.h"
#include <list>
#include <string>
#include "Location.h"
#include <API.h>

UI::UI()
{
	StorageManager sm;

	std::cout << "This is the constructor for UI" << std::endl;

	mainMenu = {
		{"=== Main Menu ===", []() {}},
		{"Add New Location", [&]() { displayMenu(preferencesMenu); }},
		{"Quick Search", [&]() { quickSearch(); } },
		{"Preferences", [&]() { displayMenu(preferencesMenu); }},
		{"Go to Second Menu", [&]() { displayMenu(secondMenu); }},
		{"Quit", [&]() { }}
	};

	secondMenu = {
		{"=== Second Menu ===", []() {}},
		{"Option 1", []() { std::cout << "You selected option 1" << std::endl; }},
		{"Go to Main Menu", [&]() { displayMenu(mainMenu); }}
	};
	
	preferencesMenu = {
		{"=== User Preferences ===\nThese will only affect future searches", []() {}},
		{"Temperature Unit (%PREF_tempUnit%)", [&]() { updatePreference("tempUnit"); }},
		{"Wind Speed Unit (%PREF_windSpeedUnit%)", [&]() { updatePreference("windSpeedUnit"); }},
		{"Pressure Unit (%PREF_pressureUnit%)", [&]() { updatePreference("pressureUnit"); }},
		{"Precipitation Unit (%PREF_precipitationUnit%)", [&]() { updatePreference("precipitationUnit"); }},
		{"Time Format (%PREF_timeFormat%)", [&]() { updatePreference("timeFormat"); }},
		{"Past Days (%PREF_pastDays%)", [&]() { updatePreference("pastDays"); }},
		{"Forecast Days (%PREF_forecastDays%)", [&]() { updatePreference("forecastDays"); }},
		{"Time Zone (%PREF_timeZone%)", [&]() { updatePreference("timeZone"); }},
		{"Go to Main Menu", [&]() { displayMenu(mainMenu); }}
	};
	
	displayMenu(mainMenu);
}

void UI::displayMenu(std::vector<MenuItem> menuItems) {
	clearScreen();

	for (int i = 0; i < menuItems.size(); i++) {
		std::string label = menuItems[i].label;
		
		if(i == 0) std::cout << label << std::endl;
		else {
			// check if label contains a %
			if (label.find("%") != std::string::npos) {
				// Replace placeholder strings, such as %PREF_tempUnit% with the actual preference value
				StorageManager sm;
				std::string prefValue = sm.getPreference(label.substr(label.find("%") + 6, label.find("%", label.find("%") + 1) - label.find("%") - 6));
				label.replace(label.find("%"), label.find("%", label.find("%") + 1) - label.find("%") + 1, prefValue);
			}


			std::cout << i << ". " << label << std::endl;
		}
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
	
	std::string introText = "=== Updating " + key + " - currently " + sm.getPreference(key) + " ===";

	std::vector<MenuItem> thisMenu = {};
	if (values.size() > 0) thisMenu.push_back({ introText + "\nPossible values : ", []() {} });


	if (values.size() == 0) {
		std::cout << introText << std::endl;
		std::cout << "'back' to go back" << std::endl << std::endl;
		
		std::cout << "Enter new value: ";
		std::string value;
		std::cin >> value;

		try {
			if (value == "back") {
				return displayMenu(preferencesMenu);
			}
			
			if (value == "" || value.size() == 0) throw std::exception(); // TODO: In future, check if within min/max range if applicable

			sm.setPreference(key, value);
			return displayMenu(preferencesMenu);
		}
		catch (std::exception e) {
			std::cout << "Invalid value. Press enter to dismiss" << std::endl;
			_getch();
			updatePreference(key); // Try again
		}
		displayMenu(mainMenu);
		return;
	}

	for (std::string &value : values) {
		thisMenu.push_back({ value, [&]() { sm.setPreference(key, value); displayMenu(preferencesMenu); }});
	}
	thisMenu.push_back({ "Go back", [&]() { displayMenu(preferencesMenu); } });

	displayMenu(thisMenu);
	return;
}

void UI::quickSearch() {
	clearScreen();
	std::cout << "=== Quick Search ===" << std::endl << std::endl;
	
	std::cout << "Enter location name: ";
	std::string locationName;
	std::cin >> locationName;

	clearScreen();
	std::cout << "=== Quick Search ===" << std::endl << std::endl;
	std::cout << "Searching for " << locationName << "..." << std::endl << std::endl;

	try {
		Location loc = Location(locationName);

		return locationData(loc);

		//clearScreen();
		//std::cout << "=== Quick Search ===" << std::endl << std::endl;
		//std::cout << "Results for " << loc.getName() << ":" << std::endl << std::endl;
		//std::cout << result << std::endl;
	}
	catch (LocationNotFoundException e) {
		std::cout << "Location not found. Press enter to return to main menu" << std::endl;
		
		_getch();
		displayMenu(mainMenu);

		return;
	}	
}

void UI::locationData(Location& l)
{
	clearScreen();
	
	API api = API();
	std::string result = api.getCurrentDataFromLocation(l);
	
	std::cout << result << std::endl << std::endl;
	
	std::vector<MenuItem> locationMenu = {
		{"=== Current Data for " + l.getName() + " ===" + "\n\n" + result, []() {}}, // Because the first line isn't considered an option by displayMenu
		{"View Forecast", [&]() { forecastData(l); }},
		{"View Historic Data", [&]() { historicData(l); }},
		{"Go back", [&]() { displayMenu(mainMenu); }}
	};
	
	return displayMenu(locationMenu);
}

void UI::forecastData(Location& l) {
	std::vector<MenuItem> forecastMenu = {
		{"=== Forecast Data for " + l.getName() + " ===", []() {}},
		{"Go back", [&]() { locationData(l); }}
	};

	return displayMenu(forecastMenu);
}

void UI::historicData(Location& l) {
	std::vector<MenuItem> historicMenu = {
		{"=== Historic Data for " + l.getName() + " ===", []() {}},
		{"Go back", [&]() { locationData(l); }}
	};

	return displayMenu(historicMenu);
}