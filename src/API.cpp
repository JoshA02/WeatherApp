#include "API.h"
#include <sstream>
#include <nlohmann/json.hpp>
#include "curl_easy.h"
#include "curl_form.h"
#include "curl_ios.h"
#include "curl_exception.h"


std::stringstream API::get_response(std::string_view url)
{
    std::stringstream str;
    curl::curl_ios<std::stringstream> writer(str);

    curl::curl_easy easy(writer);

    easy.add<CURLOPT_URL>(url.data());
    easy.add<CURLOPT_FOLLOWLOCATION>(1L);
    try
    {
        easy.perform();
    }
    catch (curl::curl_easy_exception error)
    {
        auto errors = error.get_traceback();
        error.print_traceback();
    }

    return str;
}


// TODO: Return JSON object instead of string to make saving data easier when implementing. Then again, I won't be saving current weather data, just forecast data, so maybe it's not necessary.
std::string API::getCurrentDataFromLocation(Location& loc)
{
    //
    
    using namespace std::string_literals;
    using json = nlohmann::json;

	auto url = std::format("https://api.open-meteo.com/v1/forecast?latitude={}&longitude={}&current=temperature_2m,relative_humidity_2m,precipitation,cloud_cover,wind_speed_10m,wind_direction_10m,wind_gusts_10m", std::to_string(loc.getCoords().latitude), std::to_string(loc.getCoords().longitude));

    try {
        auto jsonString = get_response(url);

        auto j = json::parse(jsonString.str());
		std::cout << j.dump(4) << std::endl;

        // Get the 'results' json array
        auto results = j["current"];

        std::stringstream ss;

		ss << "Temperature: " << results["temperature_2m"] << "°C\n";
		ss << "Humidity: " << results["relative_humidity_2m"] << "%\n";
		ss << "Precipitation: " << results["precipitation"] << "mm\n";
		ss << "Cloud Cover: " << results["cloud_cover"] << "%\n";
		ss << "Wind Speed: " << results["wind_speed_10m"] << "m/s\n";
		ss << "Wind Direction: " << results["wind_direction_10m"] << "°\n";
		ss << "Wind Gusts: " << results["wind_gusts_10m"] << "m/s\n";
        
		return ss.str();
	}
	catch (std::exception e) {
		std::cout << "Error grabbing current data" << std::endl;
		return "";
	}

    //    // Get the 'latitude' and 'longitude' values
    //    auto latitude = firstResult["latitude"];
    //    auto longitude = firstResult["longitude"];

    //    ll.latitude = latitude;
    //    ll.longitude = longitude;
    //    return ll;
    //}
    //catch (std::exception e) {
    //    ll.latitude = 0;
    //    ll.longitude = 0;
    //    std::cout << "Error grabbing location data" << std::endl;

    //    // TODO: Consider throwing a custom exception if the location is 0, 0
    //    return ll;
    //}
    //
    //return std::string();
}
;


latlong API::getLatLongFromLocation(std::string location) {
    using namespace std::string_literals;
    using json = nlohmann::json;

    auto url = "https://geocoding-api.open-meteo.com/v1/search?name=" + location + "&count=1&language=en&format=json";
    latlong ll;

    try {
        auto jsonString = get_response(url);

        //std::cout << jsonString.str() << std::endl;

        auto j = json::parse(jsonString.str());

        // Get the 'results' json array
        auto results = j["results"];

        // Get the first element of the array
        auto firstResult = results[0];

        // Get the 'latitude' and 'longitude' values
        auto latitude = firstResult["latitude"];
        auto longitude = firstResult["longitude"];

        ll.latitude = latitude;
        ll.longitude = longitude;
        return ll;
    }
    catch (std::exception e) {
        ll.latitude = 0;
        ll.longitude = 0;
        std::cout << "Error grabbing location data" << std::endl;

        // TODO: Consider throwing a custom exception if the location is 0, 0
        return ll;
    }
};