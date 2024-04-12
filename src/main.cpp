#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <map>
#include <iomanip>
#include "API.h"
#include <UI.h>
#include <StorageManager.h>


// ctrl + shift + / to toggle comments

int main()
{   
	/*API api;
	latlong ll = api.getLatLongFromLocation("London");
	std::cout << "Latitude: " << ll.latitude << std::endl;
	std::cout << "Longitude: " << ll.longitude << std::endl;

	// Create new UI instance:
	UI ui;*/

	StorageManager sm;
	try {
		std::cout << sm.getPreference("test") << std::endl;
		std::cout << sm.getPreference("tempUnit") << std::endl;
		
		/*sm.setPreference("test", "Hello World");
		sm.setPreference("tempUnit", "Celsius");
		sm.setPreference("tempUnit", "c");*/

		sm.setPreference("jeff", "c");
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

    return 0;
}