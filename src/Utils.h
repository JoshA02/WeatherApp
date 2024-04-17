#include <iostream>
#include <string>

#pragma once
class Date {
public:
	Date(int day, int month, int year) : day(day), month(month), year(year) {}
	Date(std::string day, std::string month, std::string year) : day(std::stoi(day)), month(std::stoi(month)), year(std::stoi(year)) {}

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

class Utils
{
public:
	static bool validateDate(Date date);
};

