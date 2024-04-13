#include "StorageManager.h"
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

std::string StorageManager::getPreference(std::string key)
{	
	std::ifstream file("user.prefs");
	if (!file.good()) throw PreferenceNotFoundException(key);

	json data = json::parse(file);

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
		std::cout << value << " " << value.is_number_integer() << std::endl;
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
		std::cout << x << std::endl;
		values.push_back(x);
	}
	

	//std::string type = getPreferenceType(key);
	//
	//json allowedValues = data[key]["allowedValues"];
	//
	//std::list<std::string> values;
	//for (json::iterator it = allowedValues.begin(); it != allowedValues.end(); ++it) {

	//	std::string x = it.value().dump();
	//	// remove quotes from string
	//	/*if (type == "string" || type == "char") {
	//		x = x.substr(1, x.length() - 2);
	//	}*/

	//	values.push_back(x);
	//	std::cout << x << std::endl;
	//}
	
	return values;
}

std::string StorageManager::getPreferenceType(std::string key)
{
	std::ifstream file("user.prefs");
	if (!file.good()) throw PreferenceNotFoundException(key);

	json data = json::parse(file);

	if(!preferenceExists(key)) throw PreferenceNotFoundException(key);
	if (data[key].find("type") == data[key].end()) throw InvalidPreferenceException("The preference key '" + key + "' does not have an expected type. Please check the file.");

	return data[key]["type"];
}

bool StorageManager::preferenceExists(std::string key)
{
	std::ifstream file("user.prefs");
	if (!file.good()) throw PreferenceNotFoundException(key);

	json data = json::parse(file);

	return data.find(key) != data.end();
}
