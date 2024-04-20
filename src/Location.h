#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include "Utils.h"
#include <list>
#include <vector>


// latlong is a struct with two members: latitude and longitude
struct latlong {
	double latitude;
	double longitude;
};


class Location {
public:
	Location(std::string locName);
	//Location(int id);
	Location(std::string _id, std::string locName, double lat, double lon);

	latlong getCoords() { return coords; }
	std::string getName();
	std::string getId() { return id; }

	std::string getCurrentData();
	std::string getDailyData(int day, int month, int year);
	std::string getHourData(int day, int month, int year, int hour24);
	
	// Returns data for each day requested, including hourly data for each day.
	std::vector<dayData> getDaysInRange(Date startDate, Date endDate);

private:
	std::string name;
	latlong coords;
	std::string id = "";
};