#include <unity.h>
#include "DisplayManager.h"

void setUp(void) {
    // Setup code before each test
}

void tearDown(void) {
    // Cleanup code after each test
}

void test_display_initialization() {
    DisplayManager display;
    TEST_ASSERT_TRUE(display.begin(-1, -1, DisplayManager::V3_0));
}

void test_display_update() {
    DisplayManager display;
    display.begin(-1, -1, DisplayManager::V3_0);
    TEST_ASSERT_TRUE(display.update());
}

void test_display_clear() {
    DisplayManager display;
    display.begin(-1, -1, DisplayManager::V3_0);
    display.clear();
    TEST_ASSERT_TRUE(true); // Add specific display state verification
}

void RUN_UNITY_TESTS() {
    UNITY_BEGIN();

    RUN_TEST(test_display_initialization);
    RUN_TEST(test_display_update);
    RUN_TEST(test_display_clear);

    UNITY_END();
}

int main(int argc, char **argv) {
    RUN_UNITY_TESTS();
    return 0;
} 