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
    using namespace std;
    WeatherUnits units = getUserOptions();

    string urlOptions = format("temperature_unit={}&wind_speed_unit={}&precipitation_unit={}&timezone={}", units.tempUnit, units.windSpeedUnit, units.precipUnit, units.timeZone);
    auto url = std::format("https://api.open-meteo.com/v1/forecast?latitude={}&longitude={}&current=temperature_2m,relative_humidity_2m,precipitation,cloud_cover,wind_speed_10m,wind_direction_10m,wind_gusts_10m,is_day&", std::to_string(loc.getCoords().latitude), std::to_string(loc.getCoords().longitude));
    url += urlOptions;
    
    string airUrlOptions = format("latitude={}&longitude={}&current=european_aqi,us_aqi,ozone,dust,uv_index&timezone={}", to_string(loc.getCoords().latitude), to_string(loc.getCoords().longitude), units.timeZone);
    airUrlOptions += (units.domain != "both" ? format("&domains={}", units.domain) : "");
	auto airUrl = "https://air-quality-api.open-meteo.com/v1/air-quality?" + airUrlOptions;

    try {
        auto jsonString = get_response(url);
        auto airJsonString = get_response(airUrl);

        auto j = json::parse(jsonString.str());
        auto airJ = json::parse(airJsonString.str());

		// Merge j and airJ
		for (auto& [key, value] : airJ["current"].items()) {
			if (j["current"].find(key) != j["current"].end()) continue;
			j["current"][key] = value;
		}
        for (auto& [key, value] : airJ["current_units"].items()) {
            if (j["current_units"].find(key) != j["current_units"].end()) continue;
            j["current_units"][key] = value;
        }

        // Get the 'results' json array
        auto results = j["current"];

        auto unitsJson = j["current_units"];


        std::stringstream ss;

        for (auto& [key, value] : results.items()) {
            if (ignoreKey(key)) continue;
            if (results[key].is_null()) value = "N/A";
            const std::string thisUnit = ignoreUnit(key) ? "" : unitsJson[key].get<std::string>();
            string valueStr = value.is_string() ? value.get<string>() : value.dump();
            if (key == "time") valueStr = Date(results[key].get<string>()).toString() + " - " + Time(results[key].get<string>()).toString();
            if (key == "is_day") valueStr = valueStr == "0" ? "Night" : "Day";
            
            ss << responseNameToFriendly(key) << ": " << valueStr << thisUnit << "\n";
        }

        return ss.str();
    }
    catch (std::exception e) {
        std::cout << "Error grabbing current data" << std::endl;
        return "";
    }
};


// Returns data for each day requested, including hourly data for each day.
std::vector<dayData> API::getDaysFromLocationAndRange(Location& loc, Date startDate, Date endDate, std::vector<std::string> dailyKeysToInclude, std::vector<std::string> hourlyKeysToInclude, std::string urlPrefix){
    using namespace std;

    WeatherUnits units = getUserOptions();

    string urlLatLong = format("latitude={}&longitude={}", std::to_string(loc.getCoords().latitude), std::to_string(loc.getCoords().longitude));
    string urlHourlyDataPoints = "hourly=";
    string urlDailyDataPoints = "daily=";

    // Add the provided keys
    for (int x = 0; x < hourlyKeysToInclude.size(); x++) urlHourlyDataPoints += (hourlyKeysToInclude[x] + (x == hourlyKeysToInclude.size() - 1 ? "" : ","));
    for (int x = 0; x < dailyKeysToInclude.size(); x++) urlDailyDataPoints += (dailyKeysToInclude[x] + (x == dailyKeysToInclude.size() - 1 ? "" : ","));

    string urlOptions = format("temperature_unit={}&wind_speed_unit={}&precipitation_unit={}&timezone={}", units.tempUnit, units.windSpeedUnit, units.precipUnit, units.timeZone);
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
                hourData hour = hourData(Time(hourlyData["time"][hourIndex]));
                hour.keysAndValues = std::vector<weatherProperty>();

                for (auto key : hourlyKeysToInclude) {
                    weatherProperty p;

                    // Check if the key exists in hourlyData:
                    if (hourlyData.find(key) == hourlyData.end()) continue;

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
                // Check if the key exists in dailyDate:
                if (dailyData.find(key) == dailyData.end()) continue;

                p.key = responseNameToFriendly(key);
                json jsonValue = dailyData[key][dayIndex];
                if(jsonValue.is_null() && urlPrefix == "https://api.open-meteo.com/v1/forecast") return getDaysFromLocationAndRange(loc, startDate, endDate, dailyKeysToInclude, hourlyKeysToInclude, "https://archive-api.open-meteo.com/v1/archive"); // Try the historic API
                if (!jsonValue.is_string()) p.value = jsonValue.dump();
                else p.value = jsonValue.get<std::string>();

                if (key == "sunrise" || key == "sunset") p.value = Time(p.value).toString();

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
    
	std::replace(name.begin(), name.end(), ' ', '+');

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
WeatherUnits API::getUserOptions()
{
    StorageManager sm;
    WeatherUnits units;
    units.precipUnit = sm.getPreference("precipitationUnit");
    units.tempUnit = sm.getPreference("tempUnit");
    units.windSpeedUnit = sm.getPreference("windSpeedUnit");
    units.timeZone = sm.getPreference("timeZone");
    units.domain = sm.getPreference("domain");

    return units;
};

std::string API::responseNameToFriendly(std::string name) {
    using namespace std;
    for (char& c : name) c = std::tolower(c);


	std::unordered_map<std::string, std::string> friendlyNames = {
		{"temperature_2m", "Temperature (2m)"},
        {"temperature_80m", "Temperature (80m)"},
        {"temperature_120m", "Temperature (120m)"},
        {"temperature_180m", "Temperature (180m)"},
		{"relative_humidity_2m", "Humidity"},
		{"precipitation", "Precipitation"},
		{"cloud_cover", "Cloud Cover"},
        {"cloud_cover_low", "Cloud Cover (low)"},
        {"cloud_cover_mid", "Cloud Cover (mid)"},
        {"cloud_cover_high", "Cloud Cover (high)"},
		{"wind_speed_10m", "Wind Speed (10m)"},
        {"wind_speed_80m", "Wind Speed (80m)"},
        {"wind_speed_120m", "Wind Speed (120m)"},
        {"wind_speed_180m", "Wind Speed (180m)"},
		{"wind_direction_10m", "Wind Direction (10m)"},
        {"wind_direction_80m", "Wind Direction (80m)"},
        {"wind_direction_120m", "Wind Direction (120m)"},
        {"wind_direction_180m", "Wind Direction (180m)"},
		{"wind_gusts_10m", "Wind Gusts"},
        {"time", "Time Of Reading"},
        {"weather_code", "Weather Code"},
        {"temperature_2m_max", "Max Temperature"},
        {"temperature_2m_min", "Min Temperature"},
        {"apparent_temperature_max", "Max Apparent Temperature"},
        {"apparent_temperature_min", "Min Apparent Temperature"},
        {"surface_pressure", "Surface Pressure"},
        {"et0_fao_evapotranspiration", "Evapotranspiration"},
        {"vapour_pressure_deficit", "Vapour Pressure Deficit"},
        {"soil_temperature_0cm", "Soil Temperature (0cm)"},
        {"soil_temperature_6cm", "Soil Temperature (6cm)"},
        {"soil_temperature_18cm", "Soil Temperature (18cm)"},
        {"soil_temperature_54cm", "Soil Temperature (54cm)"},
        {"soil_moisture_0_to_1cm", "Soil Moisture (0-1cm)"},
        {"soil_moisture_1_to_3cm", "Soil Moisture (1-3cm)"},
        {"soil_moisture_3_to_9cm", "Soil Moisture (3-9cm)"},
        {"soil_moisture_9_to_27cm", "Soil Moisture (9-27cm)"},
        {"soil_moisture_27_to_81cm", "Soil Moisture (27-81cm)"},
        {"daylight_duration", "Daylight Duration"},
        {"sunshine_duration", "Sunshine Duration"},
        {"uv_index_clear_sky_max", "Max UV Index (clear sky)"},
        {"uv_index_max", "Max UV Index"},
        {"precipitation_sum", "Precipitation Sum"},
        {"rain_sum", "Rain Sum"},
        {"showers_sum", "Showers Sum"},
        {"snowfall_sum", "Snowfall Sum"},
        {"snow_depth", "Snow Depth"},
        {"dew_point_2m", "Dew Point (2m)"},
        {"precipitation_hours", "Precipitation Hours"},
        {"precipitation_probability_max", "Max Precipitation Probability"},
        {"wind_speed_10m_max", "Max Wind Speed (10m)"},
        {"wind_gusts_10m_max", "Max Wind Speed (10m)"},
        {"wind_direction_10m_dominant", "Dominant Wind Direction (10m)"},
        {"shortwave_radiation_sum", "Shortwave Radiation (sum)"},
        {"european_aqi", "European Air Quality Index"},
        {"us_aqi", "US Air Quality Index" },
        {"uv_index", "UV Index" },
        {"precipitation_probability", "Precipitation Probability" },
        {"apparent_temperature", "Apparent Temperature" },
        {"is_day", "Currently Day or Night?" },
        {"temperature_2m_mean", "Mean Temperature" }
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