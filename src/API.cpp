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
};


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