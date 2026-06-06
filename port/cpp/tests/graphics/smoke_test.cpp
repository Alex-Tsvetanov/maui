// Smoke test (M0 Step 1a): proves the CMake + vcpkg + GoogleTest + ctest wiring
// is green before any real porting begins. Replaced/joined by the ported
// behavioral tests as each Graphics slice lands.
#include <gtest/gtest.h>

TEST(smoke, builds_and_runs)
{
    SUCCEED();
}
