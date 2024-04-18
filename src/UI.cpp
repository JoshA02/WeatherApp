#include "UI.h"
#include <conio.h>
#include "StorageManager.h"
#include <list>
#include <string>
#include "Location.h"
#include <API.h>

UI::UI()
{
	std::cout << "This is the constructor for UI" << std::endl;

	mainMenu = {
		{"=== Main Menu ===", []() {}},
		{"Favourite New Location", [&]() { displayMenu(preferencesMenu); }},
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
		std::cout << e.what() << std::endl;
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
	
	std::string result = l.getCurrentData();
	
	std::cout << result << std::endl << std::endl;
	
	std::vector<MenuItem> locationMenu = {
		{"=== Current Data for " + l.getName() + " ===" + "\n\n" + result, []() {}}, // Because the first line isn't considered an option by displayMenu
		{"View Daily Data", [&]() { dailyData(l); }},
		{"View Forecast", [&]() { forecastData(l); }},
		{"View Historic Data", [&]() { historicData(l); }},
		{"Go back", [&]() { displayMenu(mainMenu); }}
	};
	
	return displayMenu(locationMenu);
}

void UI::dailyData(Location& l) {
	using namespace std;

	clearScreen();

	cout << "=== Daily Data for " << l.getName() << endl;

	DateRange dateRange = askForDateRange(); // This handles validation and everything. Returns a DateRange object with the start and end dates

	const string dailyDataHeader = "=== Daily Data for " + l.getName() + " from " + dateRange.start.toString() + " to " + dateRange.end.toString() + " ===";

	clearScreen();
	cout << dailyDataHeader << endl;
	cout << "Fetching data between " << dateRange.start.toString() << " and " << dateRange.end.toString() << "..." << endl;

	API api;
	vector<dayData> days;
	try {
		cout << "Fetching data, please wait..." << endl;
		days = api.getDayDataFromLocationWithinRange(l, dateRange.start, dateRange.end); // Returns a data structure containing all the data for each day, including the hourly data for each day
		cout << "Found " << days.size() << " days worth of data!" << endl;
	}
	catch (invalid_argument e) {
		cout << e.what() << endl << "Press enter to try again" << endl;
		_getch();
		return dailyData(l);
	}
	
	if (days.size() == 0) {
		cout << "No data found. Hit enter to try again." << endl;
		_getch();
		return dailyData(l);
	}

	displayDay(l, days, 0); // Display the first day's data



	

	// TODO: Append the daily data. Allow them to view hourly.
	// For hourly, display the hourly data (using cout, not displayMenu).
	// Then, finally, just use displayMenu to display the left/right arrows to go to the next hour. Those buttons will then call the displayHour function, providing the day and hour +/- 1.
	// To do this, displayMenu will need a param to say whether to clear the screen or not so the hourly data gets accompanied with the buttons, not replaced by them.
	// Later, I could have custom logic within displayMenu to look for the user pressing left/right if the label is "Go back" and the menu is a single item. But, for now, just have regular numbered options (1) - Go back one hour, (2) - Go forward one hour, (3) - Return to daily data.

	//return displayMenu(dailyMenu);
}

DateRange UI::askForDateRange() {
	std::string startDateInput;
	std::string endDateInput;

	std::cout << "Enter start date (DD-MM-YYYY): ";
	std::cin >> startDateInput;

	std::cout << "Enter end date (DD-MM-YYYY) - can be the same as start: ";
	std::cin >> endDateInput;

	// Ensure that the provided strings are in valid date format (MUST BE DD-MM-YYYY):

	// 1 - Check that the length is 10
	if (startDateInput.length() != 10 || endDateInput.length() != 10) {
		std::cout << "Invalid date(s). Press enter to try again" << std::endl;
		_getch();
		return askForDateRange();
	}

	// 2 - Check that the 2nd and 5th characters are '-'
	if (startDateInput[2] != '-' || startDateInput[5] != '-' || endDateInput[2] != '-' || endDateInput[5] != '-') {
		std::cout << "Invalid date(s). Press enter to try again" << std::endl;
		_getch();
		return askForDateRange();
	}

	// 3 - Check that the first 2 characters and the 3rd and 4th characters are numbers using std::stoi
	try {
		std::stoi(startDateInput.substr(0, 2));
		std::stoi(startDateInput.substr(3, 2));
		std::stoi(startDateInput.substr(6, 4));

		std::stoi(endDateInput.substr(0, 2));
		std::stoi(endDateInput.substr(3, 2));
		std::stoi(endDateInput.substr(6, 4));
	}
	catch (std::invalid_argument e) {
		std::cout << "Invalid date(s). Press enter to try again" << std::endl;
		_getch();
		return askForDateRange();
	}

	// Should be good, so create the Date objects:
	Date startDate = Date(startDateInput.substr(0, 2), startDateInput.substr(3, 2), startDateInput.substr(6, 4));
	Date endDate = Date(endDateInput.substr(0, 2), endDateInput.substr(3, 2), endDateInput.substr(6, 4));

	DateRange range = DateRange(startDate, endDate);

	if (!Utils::validateDate(range.start) || !Utils::validateDate(range.end)) {
		std::cout << "Invalid date(s). Press enter to try again" << std::endl;
		_getch();
		return askForDateRange();
	}

	return range;
}

// Could take a "header" string as input to show at the top of the menu
void UI::displayDay(Location& loc, std::vector<dayData> days, int dayIndex) {
	using namespace std;
	
	vector<MenuItem> dayMenu = {};

	stringstream ss;
	for (auto data : days[dayIndex].dailyData) {
		ss << data.key << ": " << data.value << endl;
	}

	dayMenu.push_back({ "=== Daily Data for " + loc.getName() + " @ x date ===\n\n" + ss.str(), []() {}});

	if (days.size() - 1 > dayIndex) dayMenu.push_back({ "Next Day", [&]() { displayDay(loc, days, dayIndex + 1); } });
	if (dayIndex > 0)				dayMenu.push_back({ "Previous Day", [&]() { displayDay(loc, days, dayIndex - 1); } });
	
	auto lastOption = dayMenu[dayMenu.size() - 1];
	lastOption.label = lastOption.label + "\n"; // Add some space after the last option
	dayMenu[dayMenu.size() - 1] = lastOption;

	dayMenu.push_back({ "Return To Location Summary", [&]() { locationData(loc); } });

	return displayMenu(dayMenu);
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