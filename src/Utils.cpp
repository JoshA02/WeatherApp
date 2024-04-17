#include "Utils.h"

bool Utils::validateDate(Date date)
{
	int day = date.day;
	int month = date.month;
	int year = date.year;
	
	if (year < 0) return false;
	if (month < 1 || month > 12) return false;
	if (day < 1 || day > 31) return false;

	if (month == 2)
	{
		if (year % 4 == 0)
		{
			if (day > 29) return false;
		}
		else
		{
			if (day > 28) return false;
		}
	}
	else if (month == 4 || month == 6 || month == 9 || month == 11)
	{
		if (day > 30) return false;
	}

	return true;
}
