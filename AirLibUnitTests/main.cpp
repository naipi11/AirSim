#include "SettingsTest.hpp"
#include "SimpleFlightTest.hpp"
#include "QuaternionTest.hpp"
#include "CelestialTests.hpp"
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>

int main(int argc, char** argv)
{
    using namespace msr::airlib;
    int timeout_seconds = 60;
    std::string selected_test;
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string option = argv[i];
            if (option == "--test" && i + 1 < argc)
                selected_test = argv[++i];
            else if (option == "--timeout-seconds" && i + 1 < argc) {
                const std::string value = argv[++i];
                size_t parsed = 0;
                timeout_seconds = std::stoi(value, &parsed);
                if (parsed != value.size() || timeout_seconds < 1 || timeout_seconds > 3600)
                    throw std::invalid_argument("Timeout must be an integer from 1 to 3600 seconds");
            }
            else
                throw std::invalid_argument("Usage: AirLibUnitTests [--test NAME] [--timeout-seconds N]");
        }
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 2;
    }

    struct TestCase
    {
        const char* name;
        std::unique_ptr<TestBase> test;
    };
    TestCase tests[] = {
        { "Quaternion", std::unique_ptr<TestBase>(new QuaternionTest()) },
        { "Celestial", std::unique_ptr<TestBase>(new CelestialTest()) },
        { "Settings", std::unique_ptr<TestBase>(new SettingsTest()) },
        { "SimpleFlight", std::unique_ptr<TestBase>(new SimpleFlightTest()) }
    };

    int passed = 0;
    int failed = 0;
    for (auto& entry : tests) {
        if (!selected_test.empty() && selected_test != entry.name)
            continue;

        std::cout << "[ RUN      ] " << entry.name << std::endl;
        std::mutex mutex;
        std::condition_variable finished;
        bool complete = false;
        std::thread watchdog([&] {
            std::unique_lock<std::mutex> lock(mutex);
            if (!finished.wait_for(lock, std::chrono::seconds(timeout_seconds), [&] { return complete; })) {
                std::cerr << "[ TIMEOUT  ] " << entry.name << " exceeded " << timeout_seconds << " wall-clock seconds" << std::endl;
                // A stalled simulation cannot be safely unwound from another thread.
                std::_Exit(EXIT_FAILURE);
            }
        });

        try {
            entry.test->run();
            ++passed;
            std::cout << "[       OK ] " << entry.name << std::endl;
        }
        catch (const std::exception& ex) {
            ++failed;
            std::cerr << "[  FAILED  ] " << entry.name << ": " << ex.what() << std::endl;
        }
        catch (...) {
            ++failed;
            std::cerr << "[  FAILED  ] " << entry.name << ": unknown exception" << std::endl;
        }

        {
            std::lock_guard<std::mutex> lock(mutex);
            complete = true;
        }
        finished.notify_one();
        watchdog.join();
    }

    if (passed + failed == 0) {
        std::cerr << "Unknown test: " << selected_test << std::endl;
        return 2;
    }
    std::cout << "Passed: " << passed << ", failed: " << failed << std::endl;
    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
