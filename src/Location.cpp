#include "Location.h"
#include "API.h"
#include "StorageManager.h"

using namespace std::string_literals;
using json = nlohmann::json;

Location::Location(std::string locName)
{
	for (char& c : locName)
	{
		c = tolower(c);
	}

	name = locName;
	
	API api;
	coords = api.getCoordsFromLocationName(locName);
}

Location::Location(int id)
{
	
}

latlong Location::getCoords()
{
	return coords;
}

std::string Location::getName()
{
	std::string presentableName = name;
	for (int i = 0; i < presentableName.size(); i++)
	{
		if (i == 0) presentableName[i] = toupper(presentableName[i]);
		else presentableName[i] = tolower(presentableName[i]);
	}
	
	return presentableName;
}

std::string Location::getCurrentData()
{
	API api;
	std::string result = api.getCurrentDataFromLocation(*this);
	return result;
}

//										1		1		2024	
std::string Location::getDailyData(int day, int month, int year)
{
	return "STUB FUNCTION. IMPLEMENT THIS";
}

//										1		1		2024		0-23
std::string Location::getHourData(int day, int month, int year, int hour24)
{
	return "STUB FUNCTION. IMPLEMENT THIS";
}


std::vector<dayData> Location::getDaysInRange(Date startDate, Date endDate) {
	using namespace std;
	API api;
	StorageManager sm;

	bool hourlyAdvanced = sm.getPreference("hourlyAdvanced") == "true";
	bool dailyAdvanced = sm.getPreference("dailyAdvanced") == "true";

	
	// The defaults
	vector<string> dailyKeysToInclude = {
		"temperature_2m_max",
		"temperature_2m_min",
		"temperature_2m_mean",
		"sunrise",
		"sunset"
	};
	
	// The defaults
	vector<string> hourlyKeysToInclude = {
		"temperature_2m",
		"relative_humidity_2m",
		"apparent_temperature",
		"precipitation",
		"pressure_msl",
		"surface_pressure",
		"cloud_cover",
		"wind_speed_10m",
		"wind_direction_10m",
		"wind_gusts_10m"
	};

	// The advanced keys:
	vector<string> allDailyKeys = {
		"weather_code",
		"temperature_2m_max",
		"temperature_2m_min",
		"apparent_temperature_max",
		"apparent_temperature_min",
		"sunrise",
		"sunset",
		"daylight_duration",
		"sunshine_duration",
		"uv_index_max",
		"uv_index_clear_sky_max",
		"precipitation_sum",
		"rain_sum",
		"showers_sum",
		"snowfall_sum",
		"precipitation_hours",
		"precipitation_probability_max",
		"wind_speed_10m_max",
		"wind_gusts_10m_max",
		"wind_direction_10m_dominant",
		"shortwave_radiation_sum",
		"et0_fao_evapotranspiration"
	};

	// The advanced keys:
	vector<string> allHourlyKeys = {
		"temperature_2m",
		"relative_humidity_2m",
		"dew_point_2m",
		"apparent_temperature",
		"precipitation_probability",
		"precipitation",
		"rain",
		"showers",
		"snowfall",
		"snow_depth",
		"weather_code",
		"pressure_msl",
		"surface_pressure",
		"cloud_cover",
		"cloud_cover_low",
		"cloud_cover_mid",
		"cloud_cover_high",
		"visibility",
		"evapotranspiration",
		"et0_fao_evapotranspiration",
		"vapour_pressure_deficit",
		"wind_speed_10m",
		"wind_speed_80m",
		"wind_speed_120m",
		"wind_speed_180m",
		"wind_direction_10m",
		"wind_direction_80m",
		"wind_direction_120m",
		"wind_direction_180m",
		"wind_gusts_10m",
		"temperature_80m",
		"temperature_120m",
		"temperature_180m",
		"soil_temperature_0cm",
		"soil_temperature_6cm",
		"soil_temperature_18cm",
		"soil_temperature_54cm",
		"soil_moisture_0_to_1cm",
		"soil_moisture_1_to_3cm",
		"soil_moisture_3_to_9cm",
		"soil_moisture_9_to_27cm",
		"soil_moisture_27_to_81cm"
	};

	if (dailyAdvanced) dailyKeysToInclude = allDailyKeys;
	if (hourlyAdvanced) hourlyKeysToInclude = allHourlyKeys;

	return api.getDaysFromLocationAndRange(*this, startDate, endDate, dailyKeysToInclude, hourlyKeysToInclude);
}