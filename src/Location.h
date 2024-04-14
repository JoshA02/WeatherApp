#pragma once

#include <iostream>

// latlong is a struct with two members: latitude and longitude
struct latlong {
	double latitude;
	double longitude;
};


class Location {
public:
	Location(std::string locName);

	latlong getCoords();
	std::string getName();

private:
	std::string name;
	latlong coords;
};