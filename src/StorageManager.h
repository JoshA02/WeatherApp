#pragma once

#include "iostream"
#include <list>
#include <fstream>
#include "Location.h"

class PreferenceNotFoundException : public std::exception {
private:
	std::string message;
public:
	PreferenceNotFoundException(std::string key) {
		this->message = "The preference key '" + key + "' was not found.";
	}
	const char* what() const throw() {
		return message.c_str();
	}
};

class InvalidPreferenceException : public std::exception {
private:
	std::string message;
public:
	InvalidPreferenceException(std::string msg) {
		this->message = msg;
	}
	const char* what() const throw() {
		return message.c_str();
	}
};

class InvalidPreferenceValueException : public std::exception {
private:
	std::string message;
public:
	InvalidPreferenceValueException(std::string key, std::string expectedType, std::string value) {
		this->message = "The value for preference key '" + key + "'(" + value + ") is not a valid " + expectedType + ".";
	}
	const char* what() const throw() {
		return message.c_str();
	}
};


class StorageManager
{
public:
	/*StorageManager();
	~StorageManager();*/

	bool storeLocation(Location& l); // Adds a location to the stored locations file, using its name and ID.
	std::vector<Location> getStoredLocations(); // Returns a list of all stored locations.
	bool updateStoredLocation(Location& l); // Updates a stored location with new data.
	bool removeStoredLocation(Location& l); // Removes a location from the stored locations file.

	std::string getPreference(std::string key); // Returns the value of a preference key as a string.
	void setPreference(std::string key, std::string value); // Sets a preference key to a value, only if the key exists in the file already.
	std::list<std::string> getPrefAllowedValues(std::string key); // Returns a list of allowed values for a preference key.
	std::string getPreferenceType(std::string key); // Returns the type of a preference key (string, int, float, bool, char, etc.)

	// Type can be "hourly" or "daily", and all can be set to true to get all keys.
	std::vector<std::string> getWeatherKeys(std::string type, bool all = false);

private:
	bool preferenceExists(std::string key); // Checks if a preference key exists in the file.
};

