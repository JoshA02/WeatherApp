#pragma once

#include "iostream"
#include <list>
#include <fstream>

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

	std::string getPreference(std::string key);

	void setPreference(std::string key, std::string value);

	std::list<std::string> getPrefAllowedValues(std::string key);

private:
	std::string getPreferenceType(std::string key);
	bool preferenceExists(std::string key);
};

