#include <iostream>
#include <string>
#include <vector>

#pragma once


class Date {
public:
	Date(int day, int month, int year) : day(day), month(month), year(year) {}
	Date(std::string day, std::string month, std::string year) : day(std::stoi(day)), month(std::stoi(month)), year(std::stoi(year)) {}
	Date(std::string ISO8601) {
		year = std::stoi(ISO8601.substr(0, 4));
		month = std::stoi(ISO8601.substr(5, 2));
		day = std::stoi(ISO8601.substr(8, 2));
	}

	int day;
	int month;
	int year;
	
	std::string toString(std::string format = "DD/MM/YYYY") {		
		// This is a better way to do it:
		std::string dayStr = (day < 10) ? "0" + std::to_string(day) : std::to_string(day);
		std::string monthStr = (month < 10) ? "0" + std::to_string(month) : std::to_string(month);
		std::string yearStr = std::to_string(year);

		// Use the format to determine order of day, month, year:
		std::string result = format;
		size_t dayPos = result.find("DD");
		size_t monthPos = result.find("MM");
		size_t yearPos = result.find("YYYY");
		
		if (dayPos != std::string::npos) result.replace(dayPos, 2, dayStr);
		if (monthPos != std::string::npos) result.replace(monthPos, 2, monthStr);
		if (yearPos != std::string::npos) result.replace(yearPos, 4, yearStr);

		return result;
	}
};
class Time {
public:
	Time(int hour, int minute) : hour(hour), minute(minute) {
		if (hour < 0 || hour > 23 || minute < 0 || minute > 59) throw std::invalid_argument("Invalid time");
	}
	Time(std::string ISO8601) {
		hour = std::stoi(ISO8601.substr(11, 2));
		minute = std::stoi(ISO8601.substr(14, 2));
	}

	std::string toString(std::string format = "HH:MM") {
		std::string hourStr = (hour < 10) ? "0" + std::to_string(hour) : std::to_string(hour);
		std::string minuteStr = (minute < 10) ? "0" + std::to_string(minute) : std::to_string(minute);

		std::string result = format;
		size_t hourPos = result.find("HH");
		size_t minutePos = result.find("MM");

		if (hourPos != std::string::npos) result.replace(hourPos, 2, hourStr);
		if (minutePos != std::string::npos) result.replace(minutePos, 2, minuteStr);

		return result;
	}

	int hour;
	int minute;
};

struct DateRange {
	Date start;
	Date end;
};

struct WeatherUnits {
	std::string tempUnit = "celsius";
	std::string windSpeedUnit = "kmh";
	std::string precipUnit = "mm";
	std::string timeZone = "Europe/London";
	std::string domain = "cams_europe";
};

struct weatherProperty {
	std::string key;
	std::string value;
};

struct hourData {
	Time time;
	std::vector<weatherProperty> keysAndValues;
};

struct dayData {
	Date date;
	std::vector<weatherProperty> dailyData;
	std::vector<hourData> hourlyData;
};

class Utils
{
public:
	static bool validateDate(Date date);
};

