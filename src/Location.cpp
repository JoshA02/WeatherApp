#include "Location.h"
#include "API.h"

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
