# AirLib core tests

These tests run without Unreal Engine, external flight controllers, or a user settings file. Build `AirLibUnitTests.vcxproj` using the same x64 configuration as AirLib, MavLinkCom and rpclib.

## Running

From the repository root in Git Bash:

```bash
./AirLibUnitTests/build/x64/Release/AirLibUnitTests.exe
```

```bash
./AirLibUnitTests/build/x64/Debug/AirLibUnitTests.exe
```

Select a case with `--test Quaternion`, `--test Celestial`, `--test Settings`, or `--test SimpleFlight`:

```bash
./AirLibUnitTests/build/x64/Release/AirLibUnitTests.exe --test SimpleFlight
```

Each case has a default 60-second wall-clock watchdog, independent of the simulation clock. Override it with `--timeout-seconds N` (1 through 3600). A timeout reports the case and terminates the process with a failure code; it does not attempt unsafe thread cancellation or cleanup of a stalled simulation.

Exit codes: 0 means all selected cases passed, 1 means an assertion/exception or watchdog timeout, and 2 means invalid arguments or an unknown test name.

## Coverage

- Quaternion: rotation composition, normalization and inverse transformation.
- Celestial: fixed UTC reference values, valid/custom local date parsing, and rejection of malformed input. Production date conversion retains local-time/DST semantics.
- Settings: malformed JSON, missing vehicle settings, vehicle initialization and the existing SimpleFlight fallback.
- SimpleFlight: explicit settings and ground-truth initialization, arming, takeoff, position command, finite/normalized state, disarming and reset. Command completion has simulation-time limits; the watchdog limits wall time.

The SimpleFlight fixture starts grounded without Unreal collision surfaces. It does not validate landing collisions, rendering, external firmware, RPC transport, or repeatable real-world dynamics. Settings are created in memory; no user settings file is read or written.

The current machine's build and verification records are in [the AirsimX documentation](../docs/airsimx/index.md).
