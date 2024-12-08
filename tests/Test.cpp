#define BOOST_TEST_MODULE mytests
#include <boost/test/included/unit_test.hpp>
#include "StorageManager.h"
#include "Location.h"
#include "API.h"

BOOST_AUTO_TEST_CASE(invalidPreferencesTest)
{
	// Assert that StorageManager.setPreference("x") throws a PreferenceNotFoundException:
	StorageManager sm;
	BOOST_CHECK_THROW(sm.setPreference("nonExistentPreference", "some value"), PreferenceNotFoundException); // This preference key does not exist. It should throw an exception.
	BOOST_CHECK_THROW(sm.setPreference("intentionallyBrokenProperty", "some value"), InvalidPreferenceException); // This preference is missing a type. It should throw an exception.
	BOOST_CHECK_THROW(sm.setPreference("hourlyAdvanced", "hello"), InvalidPreferenceValueException); // This preference is a boolean. It must be either "true" or "false".
}
BOOST_AUTO_TEST_CASE(validPreferenceTest)
{
	StorageManager sm;
	BOOST_CHECK_NO_THROW(sm.getPreference("hourlyAdvanced"));
	BOOST_CHECK_NO_THROW(sm.setPreference("hourlyAdvanced", "false"));
	BOOST_CHECK_NO_THROW(sm.setPreference("hourlyAdvanced", "true"));
}

BOOST_AUTO_TEST_CASE(invalidLocationsTest)
{
	API api;
	BOOST_CHECK_THROW(Location nonsenseLocation("kasndkasnjdnsad"), LocationNotFoundException); // This place does not exist. It should throw an exception when trying to search the API for it.
	BOOST_CHECK_THROW(api.getCoordsFromLocationName("kasndkasnjdnsad"), LocationNotFoundException); // This place does not exist. It should throw an exception when trying to search the API for it.
}
BOOST_AUTO_TEST_CASE(locationNameTest)
{	
	BOOST_CHECK_EQUAL(Location("london").getName(), "London"); // The name should be capitalized.
	BOOST_CHECK_EQUAL(Location("lONDoN").getName(), "London"); // The name should be capitalized.
	
	BOOST_CHECK_EQUAL(Location("uNItED KinGDoM").getName(), "United kingdom"); // The name should be capitalized.
}
BOOST_AUTO_TEST_CASE(locationCoordsTest)
{
	API api;
	Location london("London");
	latlong coords = api.getCoordsFromLocationName("london");
	BOOST_CHECK_EQUAL(coords.latitude, london.getCoords().latitude);
	BOOST_CHECK_EQUAL(coords.longitude, london.getCoords().longitude);

	// Check that the coordinates are correct for a few locations around the world, allowing for a small margin of error.
	BOOST_CHECK_CLOSE(api.getCoordsFromLocationName("new york").latitude, 40.71427, 0.1);
	BOOST_CHECK_CLOSE(api.getCoordsFromLocationName("new york").longitude, -74.00597, 0.1);

	BOOST_CHECK_CLOSE(api.getCoordsFromLocationName("tokyo").latitude, 35.6895, 0.1);
	BOOST_CHECK_CLOSE(api.getCoordsFromLocationName("tokyo").longitude, 139.6917, 0.1);
	
	BOOST_CHECK_CLOSE(api.getCoordsFromLocationName("sydney").latitude, -33.8688, 0.1);
	BOOST_CHECK_CLOSE(api.getCoordsFromLocationName("sydney").longitude, 151.2093, 0.1);
}
BOOST_AUTO_TEST_CASE(renameStoredLocationTest) {
	StorageManager sm;
	sm.getStoredLocations();
	auto loc = sm.getStoredLocations()[0]; // Grab the first stored location
	
	Location newLoc(loc.getId(), "test", loc.getCoords().latitude, loc.getCoords().longitude); // Create a temporary new location with the updated name
	BOOST_CHECK_NO_THROW(sm.updateStoredLocation(newLoc)); // Update the stored location with the new location
	
	auto updatedLoc = sm.getStoredLocations()[0]; // Grab the first stored location again
	BOOST_CHECK_EQUAL(updatedLoc.getName(), "Test"); // Check that the name has been updated, and is now capitalized
}
BOOST_AUTO_TEST_CASE(renameStoredLocationAutomatedTest) {
	
	for(int i = 0; i < 20; i++) {
		BOOST_TEST_MESSAGE("renameStoredLocationAutomatedTest - Attempt: " << i);
		StorageManager sm;
		sm.getStoredLocations();
		auto loc = sm.getStoredLocations()[0]; // Grab the first stored location
		
		int randSeed = std::chrono::system_clock::now().time_since_epoch().count();
		BOOST_TEST_MESSAGE("renameStoredLocationAutomatedTest - Seed: " << randSeed);
		srand(randSeed);

		std::string randomName;
		for (int i = 0; i < 10; i++) {
			randomName += (char)(rand() % 92 + 33); // Random ASCII character between 33 and 125 (! - })
		}

		BOOST_TEST_MESSAGE("renameStoredLocationAutomatedTest - Random name: " << randomName);

		std::string randomNameCapitalized = randomName;
		randomNameCapitalized[0] = toupper(randomNameCapitalized[0]);
		for (int i = 1; i < randomNameCapitalized.size(); i++) {
			randomNameCapitalized[i] = tolower(randomNameCapitalized[i]);
		}


		Location newLoc(loc.getId(), randomName, loc.getCoords().latitude, loc.getCoords().longitude); // Create a temporary new location with the updated name
		BOOST_CHECK_NO_THROW(sm.updateStoredLocation(newLoc)); // Update the stored location with the new location
		
		auto updatedLoc = sm.getStoredLocations()[0]; // Grab the first stored location again
		BOOST_CHECK_EQUAL(updatedLoc.getName(), randomNameCapitalized); // Check that the name has been updated, and is now capitalized
	}
}