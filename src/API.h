#pragma once

#include <iostream>
#include <Location.h>


class LocationNotFoundException : public std::exception {
private:
	std::string locationName;
public:
	LocationNotFoundException(std::string locationName) {
		this->locationName = locationName;
	}
	const char* what() const throw() {
		return ("Location '" + locationName + "' not found").c_str();
	}
};

class API
{
private:
	std::stringstream get_response(std::string_view);
public:
	latlong getCoordsFromLocationName(std::string);
	std::string getCurrentDataFromLocation(Location& loc);
};