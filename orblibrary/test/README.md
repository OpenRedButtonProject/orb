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
   - Invalid XML input (should return INVALID_APP_ID)
   - Valid XML AIT input (should return valid app ID)
   - DVB-I flag usage
   - Custom scheme parameter

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

The `MockApplicationSessionCallback` provides a mock implementation of the `ApplicationSessionCallback` interface, allowing tests to run without requiring actual browser or platform integration.
