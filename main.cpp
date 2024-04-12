#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <map>

#include "curl_easy.h"
#include "curl_form.h"
#include "curl_ios.h"
#include "curl_exception.h"

#include <nlohmann/json.hpp>
#include <iostream>
#include <iomanip>

using json = nlohmann::json;


std::stringstream get_response(std::string_view url)
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

int main()
{
    std::cout << std::setw(4) << json::meta() << std::endl;
    using namespace std::string_literals;
   
    auto location = "London"s;
    auto url = "https://geocoding-api.open-meteo.com/v1/search?name=" + location + "&count=1&language=en&format=json";

    auto jsonString = get_response(url);

    std::cout << jsonString.str() << std::endl;

	auto j = json::parse(jsonString.str());
    
    // Get the 'results' json array
	auto results = j["results"];

	// Get the first element of the array
	auto firstResult = results[0];
    
	// Get the 'latitude' and 'longitude' values
	auto latitude = firstResult["latitude"];
	auto longitude = firstResult["longitude"];

	std::cout << "Latitude: " << latitude << std::endl;
	std::cout << "Longitude: " << longitude << std::endl;

	// dump(4) means indent with 4 spaces
	std::cout << j.dump(4) << std::endl;
   
    return 0;
}