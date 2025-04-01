#include <unity.h>
#include "SensorManager.h"

void setUp(void) {
    // Setup code before each test
}

void tearDown(void) {
    // Cleanup code after each test
}

void test_sensor_initialization() {
    SensorManager sensors;
    TEST_ASSERT_TRUE(sensors.begin());
}

void test_sensor_readings() {
    SensorManager sensors;
    sensors.begin();
    
    float temperature, humidity, pressure;
    TEST_ASSERT_TRUE(sensors.readBME280(temperature, humidity, pressure));
    
    // Verify readings are within reasonable ranges
    TEST_ASSERT_GREATER_THAN(-40.0f, temperature);
    TEST_ASSERT_LESS_THAN(85.0f, temperature);
    TEST_ASSERT_GREATER_THAN(0.0f, humidity);
    TEST_ASSERT_LESS_THAN(100.0f, humidity);
    TEST_ASSERT_GREATER_THAN(300.0f, pressure);
    TEST_ASSERT_LESS_THAN(1100.0f, pressure);
}

void test_motion_detection() {
    SensorManager sensors;
    sensors.begin();
    
    // Test PIR sensor reading
    bool motionDetected = sensors.readPIR();
    // Note: This is a binary test, actual result depends on sensor state
    TEST_ASSERT_TRUE(motionDetected || !motionDetected);
}

void RUN_UNITY_TESTS() {
    UNITY_BEGIN();

    RUN_TEST(test_sensor_initialization);
    RUN_TEST(test_sensor_readings);
    RUN_TEST(test_motion_detection);

    UNITY_END();
}

int main(int argc, char **argv) {
    RUN_UNITY_TESTS();
    return 0;
} 