#pragma once

#include <iostream>
#include <nlohmann/json.hpp>

// latlong is a struct with two members: latitude and longitude
struct latlong {
	double latitude;
	double longitude;
};


class Location {
public:
	Location(std::string locName);
	Location(int id);

	latlong getCoords();
	std::string getName();

	std::string getCurrentData();
	std::string getDailyData(int day, int month, int year);
	std::string getHourData(int day, int month, int year, int hour24);

private:
	std::string name;
	latlong coords;
};