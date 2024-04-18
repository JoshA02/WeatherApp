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
std::vector<dayData> API::getDaysFromLocationAndRange(Location& loc, Date startDate, Date endDate, std::list<std::string> dailyKeysToInclude, std::list<std::string> hourlyKeysToInclude, std::string urlPrefix){
    using namespace std;

    WeatherUnits units = getUnits();

    string urlLatLong = format("latitude={}&longitude={}", std::to_string(loc.getCoords().latitude), std::to_string(loc.getCoords().longitude));
    string urlHourlyDataPoints = format("hourly=temperature_2m,relative_humidity_2m,dew_point_2m,apparent_temperature,precipitation_probability,precipitation,rain,showers,snowfall,snow_depth,weather_code,pressure_msl,surface_pressure,cloud_cover,cloud_cover_low,cloud_cover_mid,cloud_cover_high,visibility,evapotranspiration,et0_fao_evapotranspiration,vapour_pressure_deficit,wind_speed_10m,wind_speed_80m,wind_speed_120m,wind_speed_180m,wind_direction_10m,wind_direction_80m,wind_direction_120m,wind_direction_180m,wind_gusts_10m,temperature_80m,temperature_120m,temperature_180m,soil_temperature_0cm,soil_temperature_6cm,soil_temperature_18cm,soil_temperature_54cm,soil_moisture_0_to_1cm,soil_moisture_1_to_3cm,soil_moisture_3_to_9cm,soil_moisture_9_to_27cm,soil_moisture_27_to_81cm");
    string urlDailyDataPoints = format("daily=temperature_2m_max,temperature_2m_min,apparent_temperature_max,apparent_temperature_min,sunrise,sunset,uv_index_max,rain_sum");
    string urlOptions = format("temperature_unit={}&wind_speed_unit={}&precipitation_unit={}&timezone=Europe%2FLondon", units.tempUnit, units.windSpeedUnit, units.precipUnit);
    string urlStartDate = format("start_date={}", startDate.toString("YYYY-MM-DD"));
    string urlEndDate = format("end_date={}", endDate.toString("YYYY-MM-DD"));

    string urlSuffix = ("?" + urlLatLong + "&" + urlHourlyDataPoints + "&" + urlDailyDataPoints + "&" + urlOptions + "&" + urlStartDate + "&" + urlEndDate);
	std::string url = urlPrefix + urlSuffix;

    auto jsonString = get_response(url);

    auto j = json::parse(jsonString.str());

	// The 'reason' will only be present if there was an error:
    if (j.find("reason") != j.end()) {
		
        // If the data requested is too old for the forecast API, try using the historic one.
		if (j["reason"].get<std::string>().find("'start_date' is out of allowed range") != std::string::npos) {
            if (urlPrefix == "https://api.open-meteo.com/v1/forecast") return getDaysFromLocationAndRange(loc, startDate, endDate, dailyKeysToInclude, hourlyKeysToInclude, "https://archive-api.open-meteo.com/v1/archive");
            else throw invalid_argument("The start date you provided is outside the allowed range.");
;		}

        if (j["reason"].get<string>().find("'end_date' is out of allowed range") != string::npos) {
            throw invalid_argument("The end date you provided is too far in the future. Try, at most, 16 days ahead.");
        }

        throw std::invalid_argument("Error: " + j["reason"].get<std::string>());
    }
    
    std::vector<dayData> days;

    try {
        std::stringstream ss;

        auto dailyData = j["daily"];
        auto hourlyData = j["hourly"];
        auto dailyUnitsJson = j["daily_units"];
		auto hourlyUnitsJson = j["hourly_units"];

		for (int dayIndex = 0; dayIndex < dailyData["time"].size(); dayIndex++) {
            dayData day = dayData(Date(dailyData["time"][dayIndex]));
            day.dailyData = std::vector<weatherProperty>();
            day.hourlyData = std::vector<hourData>();

            for (int hourIndex = (24 * dayIndex); hourIndex <= (24 * dayIndex) + 23; hourIndex++) {
                hourData hour;
                hour.keysAndValues = std::vector<weatherProperty>();

                for (auto key : hourlyKeysToInclude) {
                    weatherProperty p;
                    json jsonValue = hourlyData[key][hourIndex];
                    if (jsonValue.is_null() && urlPrefix == "https://api.open-meteo.com/v1/forecast") return getDaysFromLocationAndRange(loc, startDate, endDate, dailyKeysToInclude, hourlyKeysToInclude, "https://archive-api.open-meteo.com/v1/archive"); // Try the historic API
                    p.key = responseNameToFriendly(key);
                    if (!jsonValue.is_string()) p.value = jsonValue.dump();
                    else p.value = jsonValue.get<std::string>();
                    if (!ignoreUnit(key)) p.value = p.value + hourlyUnitsJson[key].get<std::string>();

                    hour.keysAndValues.push_back(p);
                }

                day.hourlyData.push_back(hour);
            }

            for (auto key : dailyKeysToInclude) {
                weatherProperty p;
                p.key = responseNameToFriendly(key);
                json jsonValue = dailyData[key][dayIndex];
                if(jsonValue.is_null() && urlPrefix == "https://api.open-meteo.com/v1/forecast") return getDaysFromLocationAndRange(loc, startDate, endDate, dailyKeysToInclude, hourlyKeysToInclude, "https://archive-api.open-meteo.com/v1/archive"); // Try the historic API
                if (!jsonValue.is_string()) p.value = jsonValue.dump();
                else p.value = jsonValue.get<std::string>();
                if (!ignoreUnit(key)) p.value = p.value + dailyUnitsJson[key].get<std::string>();
                day.dailyData.push_back(p);
            }
			days.push_back(day);
		}
        return days;
    }
    catch (std::exception e) {
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