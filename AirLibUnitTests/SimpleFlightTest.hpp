#ifndef msr_AirLibUnitTests_SimpleFlightTest_hpp
#define msr_AirLibUnitTests_SimpleFlightTest_hpp

#include "vehicles/multirotor/MultiRotorParamsFactory.hpp"
#include "TestBase.hpp"
#include "physics/PhysicsWorld.hpp"
#include "physics/FastPhysicsEngine.hpp"
#include "vehicles/multirotor/api/MultirotorApiBase.hpp"
#include "common/SteppableClock.hpp"
#include "vehicles/multirotor/MultiRotorPhysicsBody.hpp"

namespace msr
{
namespace airlib
{

    class SimpleFlightTest : public TestBase
    {
    public:
        virtual void run() override
        {
            AirSimSettings::initializeSettings(R"({
                "SettingsVersion": 1.2,
                "SimMode": "Multirotor",
                "Vehicles": {"SimpleFlight": {"VehicleType": "SimpleFlight"}}
            })");
            AirSimSettings::singleton().load([] { return "Multirotor"; });
            testAssert(AirSimSettings::singleton().error_messages.empty(), "Invalid SimpleFlight settings");

            const auto clock = std::make_shared<SteppableClock>(3E-3);
            ClockFactory::get(clock);
            auto params = MultiRotorParamsFactory::createConfig(
                AirSimSettings::singleton().getVehicleSetting("SimpleFlight"),
                std::make_shared<SensorFactory>());
            auto api = params->createMultirotorApi();
            testAssert(api != nullptr, "SimpleFlight API was not created");

            Kinematics kinematics(Kinematics::State::zero());
            Environment::State initial_environment;
            initial_environment.position = Vector3r::Zero();
            initial_environment.geo_point = GeoPoint(47.641468, -122.140165, 122.0f);
            Environment environment(initial_environment);
            kinematics.reset();
            environment.reset();
            MultiRotorPhysicsBody vehicle(params.get(), api.get(), &kinematics, &environment);
            api->setSimulatedGroundTruth(&kinematics.getState(), &environment);
            api->reset();

            const std::vector<UpdatableObject*> vehicles = { &vehicle };
            PhysicsWorld physics_world(std::unique_ptr<PhysicsEngineBase>(new FastPhysicsEngine()),
                                       vehicles, 3000000LL, false, false);
            // This core-only test has no Unreal collision surface to establish ground contact.
            vehicle.setGrounded(true);
            std::string message;
            const bool ready = api->isReady(message);
            testAssert(ready, "SimpleFlight not ready: " + message);
            api->enableApiControl(true);
            testAssert(api->isApiControlEnabled(), "API control was not enabled");
            testAssert(api->armDisarm(true), "SimpleFlight failed to arm");

            physics_world.startAsyncUpdator();
            const bool took_off = api->takeoff(15.0f);
            physics_world.stopAsyncUpdator();
            if (!took_off) {
                std::vector<std::string> messages;
                api->getStatusMessages(messages);
                for (const auto& status : messages)
                    std::cerr << status << std::endl;
                std::cerr << "Takeoff position: " << kinematics.getState().pose.position.transpose()
                          << ", velocity: " << kinematics.getState().twist.linear.transpose()
                          << ", simulation seconds: " << clock->elapsedSince(clock->getStart()) << std::endl;
            }
            testAssert(took_off, "Takeoff did not finish within 15 simulation seconds");
            assertFinite(kinematics.getState());
            testAssert(kinematics.getState().pose.position.z() < -1.0f, "Takeoff did not gain altitude in NED coordinates");
            const Vector3r near_position = kinematics.getState().pose.position;
            testAssert(!api->moveToPosition(near_position.x(), near_position.y(), near_position.z(), 1.0f, 0.0f,
                                            DrivetrainType::MaxDegreeOfFreedom, YawMode::Zero(), -1, 0),
                       "An immediately timed-out command must not report success");

            const Vector3r target(-5, -5, -5);
            physics_world.startAsyncUpdator();
            const bool reached_target = api->moveToPosition(target.x(), target.y(), target.z(), 5.0f, 15.0f,
                                                           DrivetrainType::MaxDegreeOfFreedom, YawMode(true, 0), -1, 0);
            physics_world.stopAsyncUpdator();
            testAssert(reached_target, "Position command did not finish within 15 simulation seconds");
            assertFinite(kinematics.getState());
            testAssert((kinematics.getState().pose.position - target).norm() < 0.5f,
                       "Vehicle did not reach the target within 0.5 meters");
            testAssert(clock->elapsedSince(clock->getStart()) > 0, "Simulation clock did not advance");
            testAssert(api->armDisarm(false), "SimpleFlight failed to disarm");
            api->enableApiControl(false);
            testAssert(!api->isApiControlEnabled(), "API control was not released");

            kinematics.reset();
            environment.reset();
            api->reset();
            physics_world.reset();
            assertFinite(kinematics.getState());
            testAssert(kinematics.getState().pose.position.norm() < 1E-5f, "Reset did not restore the initial position");
            testAssert(kinematics.getState().twist.linear.norm() < 1E-5f, "Reset did not restore zero velocity");
        }

    private:
        void assertFinite(const Kinematics::State& state)
        {
            testAssert(state.pose.position.allFinite(), "Position contains non-finite values");
            testAssert(state.twist.linear.allFinite(), "Velocity contains non-finite values");
            testAssert(state.pose.orientation.coeffs().allFinite(), "Orientation contains non-finite values");
            testAssert(std::abs(state.pose.orientation.norm() - 1.0f) < 1E-3f, "Orientation is not normalized");
        }
    };
}
}
#endif
