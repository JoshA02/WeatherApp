#include "StorageManager.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>


using json = nlohmann::json;

std::vector<Location> StorageManager::getStoredLocations()
{
	using namespace std;
	std::ifstream file("locations.data");
	if (!file.good()) return std::vector<Location>();
	
	json data = json::parse(file);
	file.close();
	
	// The root is an array of locations, each with an ID, name, latitude, and longitude.
	if (!data.is_array()) return std::vector<Location>();
	
	std::vector<Location> locations;
	for (auto& location : data) {
		if (!location["id"].is_string()) continue;
		if (!location["name"].is_string()) continue;
		if (!location["latitude"].is_number()) continue;
		if (!location["longitude"].is_number()) continue;

		locations.push_back(Location(location["id"].get<string>(), location["name"].get<string>(), stod(location["latitude"].dump()), stod(location["longitude"].dump())));
	}

	return locations;
}

bool StorageManager::updateStoredLocation(Location& l)
{
	using namespace std;
	std::ifstream file("locations.data");
	if (!file.good()) return false;

	json data = json::parse(file);
	file.close();

	// The root is an array of locations, each with an ID, name, latitude, and longitude.
	if (!data.is_array()) return false;

	bool found = false;
	for (auto& location : data) {
		if (!location["id"].is_string()) continue;
		if (location["id"].get<string>() != l.getId()) continue;
		location["name"] = l.getName();
		location["latitude"] = l.getCoords().latitude;
		location["longitude"] = l.getCoords().longitude;
		found = true;
		break;
	}

	std::ofstream out("locations.data");
	out << data.dump(4);
	out.close();
	
	return found;
}

bool StorageManager::removeStoredLocation(Location& l)
{
	using namespace std;
	std::ifstream file("locations.data");
	if (!file.good()) return false;

	json data = json::parse(file);
	file.close();

	// The root is an array of locations, each with an ID, name, latitude, and longitude.
	if (!data.is_array()) return false;

	bool found = false;
	for (auto it = data.begin(); it != data.end(); ++it) {
		if (!(*it)["id"].is_string()) continue;
		if ((*it)["id"].get<string>() != l.getId()) continue;
		data.erase(it);
		found = true;
		break;
	}

	std::ofstream out("locations.data");
	out << data.dump(4);
	out.close();

	return found;
}

std::string StorageManager::getPreference(std::string key)
{	
	std::ifstream file("user.prefs");
	if (!file.good()) throw PreferenceNotFoundException(key);

	json data = json::parse(file);
	file.close();

	if (!preferenceExists(key)) throw PreferenceNotFoundException(key);
	if (data[key].find("value") == data[key].end()) throw InvalidPreferenceException("The preference key '" + key + "' does not have a value. Please check the file.");
	if (data[key].find("type") == data[key].end()) throw InvalidPreferenceException("The preference key '" + key + "' does not have an expected type. Please check the file.");
	
	json value = data[key]["value"];

	std::string type = getPreferenceType(key);
	
	// Check if the value is of the expected type
	if (type == "string") {
		if (!value.is_string()) throw InvalidPreferenceValueException(key, type, value);
	}
	else if (type == "int") {
		//std::cout << value << " " << value.is_number_integer() << std::endl;
		if (!value.is_number_integer()) throw InvalidPreferenceValueException(key, type, value);
	}
	else if (type == "float") {
		if (!value.is_number_float()) throw InvalidPreferenceValueException(key, type, value);
	}
	else if (type == "bool") {
		if (!value.is_boolean()) throw InvalidPreferenceValueException(key, type, value);
	}
	else if (type == "char") {
		if (!value.is_string() || value.get<std::string>().length() != 1) throw InvalidPreferenceValueException(key, type, value);
	}
	else throw InvalidPreferenceException("The preference key '" + key + "' has an invalid type. Please check the file.");
	

	std::string x = value.dump();
	// remove quotes from string
	if (type == "string" || type == "char") {
		x = x.substr(1, x.length() - 2);
	}
	
	return x;
}

void StorageManager::setPreference(std::string key, std::string value)
{
	std::ifstream file("user.prefs");
	if (!file.good()) throw PreferenceNotFoundException(key);

	json data = json::parse(file);
	file.close();

	if (!preferenceExists(key)) throw PreferenceNotFoundException(key);
	if (data[key].find("value") == data[key].end()) throw InvalidPreferenceException("The preference key '" + key + "' does not have a value. Please check the file.");
	
	std::string type = getPreferenceType(key);

	// Check if the value is of the expected type and catch any exceptions during conversion
	try{
		if (type == "string") {
			data[key]["value"] = value;
		}
		else if (type == "int") {
			data[key]["value"] = std::stoi(value);
		}
		else if (type == "float") {
			data[key]["value"] = std::stof(value);
		}
		else if (type == "bool") {
			transform(value.begin(), value.end(), value.begin(), ::tolower);
			if (value != "true" && value != "false") throw InvalidPreferenceValueException(key, type, value);
			data[key]["value"] = (value == "true");
		}
		else if (type == "char") {
			if (value.length() != 1) throw InvalidPreferenceValueException(key, type, value);
			data[key]["value"] = value;
		}
		else throw InvalidPreferenceException("The preference key '" + key + "' has an invalid type. Please check the file.");
	}
	catch (std::exception e) {
		throw InvalidPreferenceValueException(key, type, value);
	}

	std::ofstream out("user.prefs");
	out << data.dump(4);
	out.close();
}

std::list<std::string> StorageManager::getPrefAllowedValues(std::string key)
{
	std::ifstream file("user.prefs");
	if (!file.good()) throw PreferenceNotFoundException(key);
	
	json data = json::parse(file);
	file.close();
	
	if (!preferenceExists(key)) throw PreferenceNotFoundException(key);
	
	if (data[key].find("allowedValues") == data[key].end()) return std::list<std::string>();

	std::list<std::string> values;
	
	std::string type = getPreferenceType(key);
	json allowedValues = data[key]["allowedValues"];
	for (json::iterator it = allowedValues.begin(); it != allowedValues.end(); ++it) {
		std::string x = it.value().dump();
		// remove quotes from string
		if (type == "string" || type == "char") {
			x = x.substr(1, x.length() - 2);
		}
		//std::cout << x << std::endl;
		values.push_back(x);
	}
	
	return values;
}

std::string StorageManager::getPreferenceType(std::string key)
{
	std::ifstream file("user.prefs");
	if (!file.good()) throw PreferenceNotFoundException(key);

	json data = json::parse(file);
	file.close();

	if(!preferenceExists(key)) throw PreferenceNotFoundException(key);
	if (data[key].find("type") == data[key].end()) throw InvalidPreferenceException("The preference key '" + key + "' does not have an expected type. Please check the file.");

	return data[key]["type"];
}

std::vector<std::string> StorageManager::getWeatherKeys(std::string type, bool all)
{
	std::ifstream file("weatherkeys.keyinfo");
	if (!file.good()) throw PreferenceNotFoundException("weatherkeys.keyinfo");
	
	json data = json::parse(file);
	file.close();
	
	if (data.find(type) == data.end()) throw PreferenceNotFoundException(type);
	
	if (all) {
		if (data[type].find("all") == data[type].end()) return std::vector<std::string>();
		return data[type]["all"].get<std::vector<std::string>>();
	}
	else {
		if (data[type].find("basic") == data[type].end()) return std::vector<std::string>();
		return data[type]["basic"].get<std::vector<std::string>>();
	}
}

bool StorageManager::preferenceExists(std::string key)
{
	// Could utilize the getPreferenceType function to check if the key exists, but this is more efficient

	std::ifstream file("user.prefs");
	if (!file.good()) throw PreferenceNotFoundException(key);

	json data = json::parse(file);
	file.close();

	return data.find(key) != data.end();
}
