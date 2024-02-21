#include "Tests.h"

#include "TestMessages.h"

#include <iostream>

namespace Tests
{
void RunAllTests()
{
    std::cout << "Running all tests...\n";
    MessageTests();
    std::cout << "All tests successfully passed\n";
}
}
