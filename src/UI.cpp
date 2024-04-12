#include "UI.h"
#include <conio.h>

UI::UI()
{
	std::cout << "This is the constructor for UI" << std::endl;

	mainMenu = {
		{"=== Main Menu ===", []() {}},
		{"Option 1", []() { std::cout << "You selected option 1" << std::endl; }},
		{"Go to Second Menu", [&]() { displayMenu(secondMenu); }}
	};

	secondMenu = {
		{"=== Second Menu ===", []() {}},
		{"Option 1", []() { std::cout << "You selected option 1" << std::endl; }},
		{"Go to Main Menu", [&]() { displayMenu(mainMenu); }}
	};
	
	displayMenu(mainMenu);
}

void UI::displayMenu(std::vector<MenuItem> menuItems) {
	std::cout << menuItems.size() << std::endl;

	clearScreen();
	
	for (int i = 0; i < menuItems.size(); i++) {
		if(i == 0) std::cout << menuItems[i].label << std::endl;
		else std::cout << i << ". " << menuItems[i].label << std::endl;
	}

	int choice;
	std::cout << "Enter your choice: ";
	std::cin >> choice;
	
	if (choice > 0 && choice <= menuItems.size() - 1) {
		menuItems[choice].action();
	}
	else {	
		std::cout << "Invalid choice. Press enter to dismiss" << std::endl;
		_getch();
		

		displayMenu(menuItems);
	}
}