#include "StorageManager.h"
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

std::string StorageManager::getPreference(std::string key)
{	
	std::ifstream file("user.prefs");
	if (!file.good()) throw PreferenceNotFoundException(key);

	json data = json::parse(file);

	if (data.find(key) == data.end()) throw PreferenceNotFoundException(key);
	if (data[key].find("value") == data[key].end()) throw InvalidPreferenceException("The preference key '" + key + "' does not have a value. Please check the file.");
	if (data[key].find("type") == data[key].end()) throw InvalidPreferenceException("The preference key '" + key + "' does not have an expected type. Please check the file.");
	
	json value = data[key]["value"];

	json type = data[key]["type"];
	
	// Check if the value is of the expected type
	if (type == "string") {
		if (!value.is_string()) throw InvalidPreferenceValueException(key, type);
	}
	else if (type == "int") {
		if (!value.is_number_integer()) throw InvalidPreferenceValueException(key, type);
	}
	else if (type == "float") {
		if (!value.is_number_float()) throw InvalidPreferenceValueException(key, type);
	}
	else if (type == "bool") {
		if (!value.is_boolean()) throw InvalidPreferenceValueException(key, type);
	}
	else if (type == "char") {
		if (!value.is_string() || value.get<std::string>().length() != 1) throw InvalidPreferenceValueException(key, type);
	}
	else throw InvalidPreferenceException("The preference key '" + key + "' has an invalid type. Please check the file.");
	

	return value.get<std::string>();
}

void StorageManager::setPreference(std::string key, std::string value)
{
	std::ifstream file("user.prefs");
	if (!file.good()) throw PreferenceNotFoundException(key);

	json data = json::parse(file);

	if (data.find(key) == data.end()) throw PreferenceNotFoundException(key);
	if (data[key].find("value") == data[key].end()) throw InvalidPreferenceException("The preference key '" + key + "' does not have a value. Please check the file.");
	if (data[key].find("type") == data[key].end()) throw InvalidPreferenceException("The preference key '" + key + "' does not have an expected type. Please check the file.");

	json type = data[key]["type"];

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
			if (value != "true" && value != "false") throw InvalidPreferenceValueException(key, type);
			data[key]["value"] = (value == "true");
		}
		else if (type == "char") {
			if (value.length() != 1) throw InvalidPreferenceValueException(key, type);
			data[key]["value"] = value[0];
		}
		else throw InvalidPreferenceException("The preference key '" + key + "' has an invalid type. Please check the file.");
	}
	catch (std::exception e) {
		throw InvalidPreferenceValueException(key, type);
	}

	std::ofstream out("user.prefs");
	out << data.dump(4);
	out.close();
}
