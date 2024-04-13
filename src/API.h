#pragma once

#include <iostream>
#include <Location.h>

class API
{
private:
	std::stringstream get_response(std::string_view);
public:
	latlong getLatLongFromLocation(std::string);
	std::string getCurrentDataFromLocation(Location& loc);
};