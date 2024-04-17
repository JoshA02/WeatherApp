#include "Location.h"
#include "API.h"

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
