#pragma once

#include <iostream>
#include "Location.h"
#include <list>
#include "Utils.h"


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
	std::string responseNameToFriendly(std::string name);
	bool ignoreKey(std::string key);
	bool ignoreUnit(std::string key);
	
	WeatherUnits getUserOptions();

	std::list<std::string> ignoredKeys = {
		"interval"
	};

	// The keys who's unit not to display.
	std::list<std::string> ignoredUnits = {
		"time",
		"sunrise",
		"sunset",
		"weather_code"
	};

public:
	latlong getCoordsFromLocationName(std::string);
	std::string getCurrentDataFromLocation(Location& loc); // Returns the current weather data for a location. Could change this to return a struct with all the data, like getDayDataFromLocationWithinRange
	
	// Returns each day requested, including the daily and hourly data
	std::vector<dayData> getDaysFromLocationAndRange(Location& loc, Date startDate, Date endDate,
		std::vector<std::string> dailyKeysToInclude = {
			"temperature_2m_max",
			"temperature_2m_min",
			"apparent_temperature_max",
			"apparent_temperature_min",
			"sunrise",
			"sunset"
		},
		std::vector<std::string> hourlyKeysToInclude = {
			"temperature_2m",
			"relative_humidity_2m"
		}, std::string urlPrefix = "https://api.open-meteo.com/v1/forecast"
	);
};