# ApplicationManager Unit Tests

This directory contains unit tests for the ApplicationManager class.

## Files

- `application_manager_unittest.cpp` - Main unit test file for ApplicationManager
- `MockApplicationSessionCallback.h` - Mock implementation of ApplicationSessionCallback for testing

## Test Coverage

The tests cover:

1. **Singleton Pattern**: Tests that the `instance()` method returns the same singleton instance
2. **ProcessXmlAit Method**: Tests various scenarios:
   - Empty XML input (should return INVALID_APP_ID)
   - Mock XML parser failure (should return INVALID_APP_ID)
   - Mock XML parser success (should return valid app ID)
3. **Callback Management**: Tests for `RegisterCallback()` and `SetCurrentInterface()` methods
4. **State Queries**: Tests for `GetRunningAppIds()`, `GetOrganizationId()`, and `GetCurrentAppNames()`
5. **Error Handling**: Tests for invalid parameters and null callbacks

## Architecture

The tests use an interface-based approach to isolate XML parsing:

- **IXmlParser Interface**: Defines the contract for XML parsing
- **RealXmlParser**: Production implementation that uses the real XML parser
- **MockXmlParser**: Test implementation that allows controlled behavior for testing

This approach allows the unit tests to run without depending on `//third_party/libxml` while maintaining full functionality in production builds.

## Running the Tests

To build and run the tests:

```bash
# Build the test target
ninja -C out/Debug test_orb_application_manager

# Run the tests
./out/Debug/test_orb_application_manager
```

## Test Structure

The tests use Google Test (gtest) framework and follow the Given-When-Then pattern:

- **Given**: Set up the test environment and initial state
- **When**: Execute the method under test
- **Then**: Verify the expected behavior

## Mock Objects

The test suite uses two types of mocks:

1. **MockApplicationSessionCallback**: Provides a mock implementation of the `ApplicationSessionCallback` interface
2. **MockXmlParser**: Provides a mock implementation of the `IXmlParser` interface with controllable behavior

## Dependencies

This test suite has minimal dependencies:
- `:orb` - The main ORB library
- `//testing/gtest` - Google Test framework
- `//third_party/jsoncpp` - JSON handling (used by the main library)

Note: The test does not depend on `//third_party/libxml` or XML parsing functionality, focusing on testing the ApplicationManager logic without requiring full XML infrastructure.
