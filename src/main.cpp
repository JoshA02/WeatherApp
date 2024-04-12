#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <map>
#include <iomanip>
#include "API.h"
#include <UI.h>


// ctrl + shift + / to toggle comments

int main()
{   
    API api;
	latlong ll = api.getLatLongFromLocation("London");
	std::cout << "Latitude: " << ll.latitude << std::endl;
	std::cout << "Longitude: " << ll.longitude << std::endl;

	// Create new UI instance:
	UI ui;

    return 0;
}