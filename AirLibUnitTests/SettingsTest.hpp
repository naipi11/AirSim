#ifndef msr_AirLibUnitTests_SettingsTest_hpp
#define msr_AirLibUnitTests_SettingsTest_hpp

#include "TestBase.hpp"
#include "common/AirSimSettings.hpp"

namespace msr
{
namespace airlib
{

    class SettingsTest : public TestBase
    {
    public:
        virtual void run() override
        {
            AirSimSettings empty_settings;
            bool rejected_missing_vehicle = false;
            try {
                empty_settings.getVehicleSetting("missing");
            }
            catch (const std::invalid_argument&) {
                rejected_missing_vehicle = true;
            }
            testAssert(rejected_missing_vehicle, "Missing vehicle settings must produce an exception");

            bool rejected_invalid_json = false;
            try {
                Settings::loadJSonString("{invalid json");
            }
            catch (const std::exception&) {
                rejected_invalid_json = true;
            }
            testAssert(rejected_invalid_json, "Malformed settings JSON was accepted");

            AirSimSettings::initializeSettings(R"({
                "SettingsVersion": 1.2,
                "SimMode": "Multirotor",
                "Vehicles": {"SimpleFlight": {"VehicleType": "SimpleFlight"}}
            })");
            testAssert(Settings::singleton().isLoadSuccess(), "Settings JSON was not loaded");
            testAssert(Settings::singleton().getString("SimMode", "") == "Multirotor", "SimMode was not parsed");
            auto& settings = AirSimSettings::singleton();
            settings.load([] { return "Multirotor"; });
            testAssert(settings.error_messages.empty(), "Vehicle settings contain errors");
            testAssert(settings.vehicles.size() == 1, "Expected exactly one configured vehicle");
            const auto* vehicle = settings.getVehicleSetting("SimpleFlight");
            testAssert(vehicle->vehicle_type == AirSimSettings::kVehicleTypeSimpleFlight, "Incorrect vehicle type");
            testAssert(settings.getVehicleSetting("unlisted-pawn") == vehicle, "SimpleFlight fallback was not preserved");
        }
    };
}
}
#endif
