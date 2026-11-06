#ifndef msr_AirLibUnitTests_CelestialTest_hpp
#define msr_AirLibUnitTests_CelestialTest_hpp

#include "TestBase.hpp"
#include "common/EarthCelestial.hpp"

namespace msr
{
namespace airlib
{

    class CelestialTest : public TestBase
    {
    public:
        virtual void run() override
        {
            dateParsingTest();

            // 2018-02-22 15:24 in Seattle (PST), expressed as a fixed UTC instant.
            const uint64_t t = 1519341840;
            const auto c_sun = EarthCelestial::getSunCoordinates(t, 47.673988, -122.121513);
            const auto c_moon = EarthCelestial::getMoonCoordinates(t, 47.673988, -122.121513);
            const auto c_moon_phase = EarthCelestial::getMoonPhase(t);

            testAssert(Utils::isApproximatelyEqual(c_sun.altitude, 19.67, 0.1), "Sun altitude is not correct");
            testAssert(Utils::isApproximatelyEqual(c_moon.altitude, 45.02, 0.1), "Moon altitude is not correct");
            testAssert(Utils::isApproximatelyEqual(c_moon_phase.fraction, 0.47, 0.1), "Moon fraction is not correct");
        }

    private:
        void dateParsingTest()
        {
            const auto t = Utils::to_time_t("2018-02-22 15:24:00");
            testAssert(Utils::to_string(t, "%Y-%m-%d %H:%M:%S") == "2018-02-22 15:24:00",
                       "Numeric date/time did not round-trip in local time");
            testAssert(Utils::to_time_t("2018-February-22 15:24:00", false, "%Y-%B-%d %H:%M:%S") == t,
                       "Explicit month-name format was not respected");
            testAssert(Utils::to_time_t("22/02/2018 15:24:00", false, "%d/%m/%Y %H:%M:%S") == t,
                       "Custom date/time format was not respected");
            testAssert(Utils::to_time_t("2018-02-22 15:24:00  ") == t,
                       "Trailing whitespace should be accepted");

            for (const auto* invalid : { "", "not-a-date", "2018-February-22 15:24:00",
                                        "2018-13-22 15:24:00", "2018-02-22 25:24:00",
                                        "2018-02-22 15:24:00junk" }) {
                bool rejected = false;
                try {
                    Utils::to_time_t(invalid);
                }
                catch (const std::invalid_argument&) {
                    rejected = true;
                }
                testAssert(rejected, std::string("Invalid date/time was accepted: ") + invalid);
            }
        }
    };
}
}

#endif
