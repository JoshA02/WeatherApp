#include "Location.h"
#include "API.h"

Location::Location(std::string locName)
{
	name = locName;
	
	API api;
	coords = api.getLatLongFromLocation(locName);
}

latlong Location::getCoords()
{
	return coords;
}
