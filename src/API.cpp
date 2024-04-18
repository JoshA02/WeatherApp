#include "API.h"
#include <sstream>
#include <nlohmann/json.hpp>
#include "curl_easy.h"
#include "curl_form.h"
#include "curl_ios.h"
#include "curl_exception.h"
#include <algorithm>
#include "StorageManager.h"


using namespace std::string_literals;
using json = nlohmann::json;

std::stringstream API::get_response(std::string_view url)
{
    std::stringstream str;
    curl::curl_ios<std::stringstream> writer(str);

    curl::curl_easy easy(writer);

    easy.add<CURLOPT_URL>(url.data());
    easy.add<CURLOPT_FOLLOWLOCATION>(1L);
    try
    {
        easy.perform();
    }
    catch (curl::curl_easy_exception error)
    {
        auto errors = error.get_traceback();
        error.print_traceback();
    }

    return str;
}


std::string API::getCurrentDataFromLocation(Location& loc)
{
    WeatherUnits units = getUnits();

    auto url = std::format("https://api.open-meteo.com/v1/forecast?latitude={}&longitude={}&current=temperature_2m,relative_humidity_2m,precipitation,cloud_cover,wind_speed_10m,wind_direction_10m,wind_gusts_10m&temperature_unit={}&wind_speed_unit={}&precipitation_unit={}", std::to_string(loc.getCoords().latitude), std::to_string(loc.getCoords().longitude), units.tempUnit, units.windSpeedUnit, units.precipUnit);

    try {
        auto jsonString = get_response(url);

        auto j = json::parse(jsonString.str());
        //std::cout << j.dump(4) << std::endl;

        // Get the 'results' json array
        auto results = j["current"];

        auto unitsJson = j["current_units"];


        std::stringstream ss;

        for (auto& [key, value] : results.items()) {
            if (ignoreKey(key)) continue;
            const std::string thisUnit = ignoreUnit(key) ? "" : unitsJson[key].get<std::string>();
            ss << responseNameToFriendly(key) << ": " << value << thisUnit << "\n";
        }

        return ss.str();
    }
    catch (std::exception e) {
        std::cout << "Error grabbing current data" << std::endl;
        return "";
    }
};


// Returns data for each day requested, including hourly data for each day.
std::vector<dayData> API::getDayDataFromLocationWithinRange(Location& loc, Date startDate, Date endDate, std::list<std::string> dailyKeysToInclude, std::list<std::string> hourlyKeysToInclude){

    //WeatherUnits units = getUnits();
    // TODO: Add units and more daily data options


    std::string url = std::format("https://api.open-meteo.com/v1/forecast?latitude={}&longitude={}&hourly=temperature_2m,relative_humidity_2m&daily=weather_code,temperature_2m_max,temperature_2m_min,apparent_temperature_max,apparent_temperature_min,sunrise,sunset&start_date={}&end_date={}", std::to_string(loc.getCoords().latitude), std::to_string(loc.getCoords().longitude), startDate.toString("YYYY-MM-DD"), endDate.toString("YYYY-MM-DD"));

    auto jsonString = get_response(url);

    auto j = json::parse(jsonString.str());

	// The 'reason' will only be present if there was an error:
	if (j.find("reason") != j.end()) throw std::invalid_argument("Error: " + j["reason"].get<std::string>());
    
    std::vector<dayData> days;

    try {
        std::stringstream ss;

        auto dailyData = j["daily"];
        auto hourlyData = j["hourly"];
        //auto unitsJson = j["daily_units"];

		for (int dayIndex = 0; dayIndex < dailyData["time"].size(); dayIndex++) {
            dayData day;
            day.dailyData = std::vector<weatherProperty>();
            day.hourlyData = std::vector<hourData>();

            //std::cout << "Hourly data start index " << 24 * dayIndex << ". End index " << (24 * dayIndex) + 23 << std::endl;
            for (int hourIndex = (24 * dayIndex); hourIndex <= (24 * dayIndex) + 23; hourIndex++) {
                hourData hour;
                hour.keysAndValues = std::vector<weatherProperty>();

                std::cout << hourIndex << std::endl;
                for (auto key : hourlyKeysToInclude) {
                    weatherProperty p;
                    p.key = responseNameToFriendly(key);
                    if (!hourlyData[key][hourIndex].is_string()) p.value = hourlyData[key][hourIndex].dump();
                    else p.value = hourlyData[key][hourIndex].get<std::string>();

                    std::cout << "Found hourly data: " << p.key << " = " << p.value << std::endl;

                    hour.keysAndValues.push_back(p);
                }

                day.hourlyData.push_back(hour);
            }


            for (auto key : dailyKeysToInclude) {
                weatherProperty p;
                p.key = responseNameToFriendly(key);
                if (!dailyData[key][dayIndex].is_string()) p.value = dailyData[key][dayIndex].dump();
                else p.value = dailyData[key][dayIndex].get<std::string>();
                //std::cout << p.key << ": " << p.value << std::endl;
                day.dailyData.push_back(p);
            }
			days.push_back(day);
            std::cout << std::endl << std::endl;
		}

        std::cout << "Made " << days.size() << " days" << std::endl;
        for (auto day : days) {
            for (weatherProperty prop : day.dailyData)
            {
                std::cout << prop.key << ": " << prop.value << std::endl;
            }
            std::cout << std::endl << std::endl;
        }

        return days;
    }
    catch (std::exception e) {
        //std::cout << e.what() << std::endl;
        return {};
    }
}


latlong API::getCoordsFromLocationName(std::string name) {
    using namespace std::string_literals;
    using json = nlohmann::json;

    auto url = "https://geocoding-api.open-meteo.com/v1/search?name=" + name + "&count=1&language=en&format=json";
    latlong ll = {};

    try {
        auto jsonString = get_response(url);

        //std::cout << jsonString.str() << std::endl;

        auto j = json::parse(jsonString.str());

        // Get the 'results' json array
        auto results = j["results"];

        // Get the first element of the array
        auto firstResult = results[0];

        // Get the 'latitude' and 'longitude' values
        auto latitude = firstResult["latitude"];
        auto longitude = firstResult["longitude"];

        ll.latitude = latitude;
        ll.longitude = longitude;
        return ll;
    }
    catch (std::exception e) {
        throw LocationNotFoundException(name);
    }
}
WeatherUnits API::getUnits()
{
    StorageManager sm;
    // TODO: Ask the StorageManager for the data.
    WeatherUnits units;
    units.precipUnit = sm.getPreference("precipitationUnit");
    units.tempUnit = sm.getPreference("tempUnit");
    units.windSpeedUnit = sm.getPreference("windSpeedUnit");

    return units;
};

std::string API::responseNameToFriendly(std::string name) {
    using namespace std;
    for (char& c : name) {
        std::tolower(c);
    }


	std::unordered_map<std::string, std::string> friendlyNames = {
		{"temperature_2m", "Temperature"},
		{"relative_humidity_2m", "Humidity"},
		{"precipitation", "Precipitation"},
		{"cloud_cover", "Cloud Cover"},
		{"wind_speed_10m", "Wind Speed"},
		{"wind_direction_10m", "Wind Direction"},
		{"wind_gusts_10m", "Wind Gusts"},
        {"time", "Time"},
        {"weather_code", "Weather Code"},
        {"temperature_2m_max", "Max Temperature"},
        {"temperature_2m_min", "Min Temperature"},
        {"apparent_temperature_max", "Max Apparent Temperature"},
        {"apparent_temperature_min", "Min Apparent Temperature"},
	};
	if (friendlyNames.find(name) == friendlyNames.end()) {
		name[0] = toupper(name[0]); // Can't find it so just capitalize the first letter and hope it looks okay.
        return name;
	}

	return friendlyNames[name];
}

bool API::ignoreKey(std::string key) {
    return std::find(ignoredKeys.begin(), ignoredKeys.end(), key) != ignoredKeys.end();
}
bool API::ignoreUnit(std::string key) {
    return std::find(ignoredUnits.begin(), ignoredUnits.end(), key) != ignoredUnits.end();
}