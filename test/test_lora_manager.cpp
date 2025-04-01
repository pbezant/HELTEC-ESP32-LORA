#include <unity.h>
#include "LoRaManager.h"

// Mock pins for testing
#define TEST_LORA_CS   18
#define TEST_LORA_DIO1 23
#define TEST_LORA_RST  14
#define TEST_LORA_BUSY 33

// Test credentials
uint64_t testJoinEUI = 0x0000000000000000;
uint64_t testDevEUI = 0x0000000000000000;
uint8_t testAppKey[16] = {0};
uint8_t testNwkKey[16] = {0};

void setUp(void) {
    // Setup code before each test
}

void tearDown(void) {
    // Cleanup code after each test
}

void test_lora_initialization() {
    LoRaManager lora;
    TEST_ASSERT_TRUE(lora.begin(TEST_LORA_CS, TEST_LORA_DIO1, TEST_LORA_RST, TEST_LORA_BUSY));
}

void test_lora_credentials() {
    LoRaManager lora;
    lora.begin(TEST_LORA_CS, TEST_LORA_DIO1, TEST_LORA_RST, TEST_LORA_BUSY);
    
    // Set and verify credentials (verification is implicit since there's no getter)
    lora.setCredentials(testJoinEUI, testDevEUI, testAppKey, testNwkKey);
    
    // This is more of a compilation test since we can't verify values
    TEST_PASS();
}

void test_lora_join() {
    LoRaManager lora;
    lora.begin(TEST_LORA_CS, TEST_LORA_DIO1, TEST_LORA_RST, TEST_LORA_BUSY);
    lora.setCredentials(testJoinEUI, testDevEUI, testAppKey, testNwkKey);
    
    // Since we're using dummy credentials, we don't expect this to succeed
    // Just verify it runs without crashing
    lora.joinNetwork();
    
    // Get the error code
    int errorCode = lora.getLastErrorCode();
    
    // Verify we got a valid error code
    TEST_ASSERT_NOT_EQUAL(0, errorCode); // Should fail to join with dummy credentials
}

void test_lora_send_data() {
    LoRaManager lora;
    lora.begin(TEST_LORA_CS, TEST_LORA_DIO1, TEST_LORA_RST, TEST_LORA_BUSY);
    lora.setCredentials(testJoinEUI, testDevEUI, testAppKey, testNwkKey);
    
    // Create test payload
    uint8_t payload[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t payloadSize = sizeof(payload);
    
    // Should fail since we're not joined to a network with dummy credentials
    bool result = lora.sendData(payload, payloadSize, 1, false);
    
    // Verify we got an error
    TEST_ASSERT_NOT_EQUAL(0, lora.getLastErrorCode());
}

void test_lora_send_string() {
    LoRaManager lora;
    lora.begin(TEST_LORA_CS, TEST_LORA_DIO1, TEST_LORA_RST, TEST_LORA_BUSY);
    lora.setCredentials(testJoinEUI, testDevEUI, testAppKey, testNwkKey);
    
    // Test string method
    String testString = "Hello LoRaWAN";
    
    // Should fail since we're not joined to a network with dummy credentials
    bool result = lora.sendString(testString, 1, false);
    
    // Verify we got an error
    TEST_ASSERT_NOT_EQUAL(0, lora.getLastErrorCode());
}

void test_lora_network_joined() {
    LoRaManager lora;
    lora.begin(TEST_LORA_CS, TEST_LORA_DIO1, TEST_LORA_RST, TEST_LORA_BUSY);
    
    // With dummy credentials, we shouldn't be joined
    bool joined = lora.isNetworkJoined();
    
    // Should not be joined
    TEST_ASSERT_FALSE(joined);
}

void RUN_UNITY_TESTS() {
    UNITY_BEGIN();

    RUN_TEST(test_lora_initialization);
    RUN_TEST(test_lora_credentials);
    RUN_TEST(test_lora_join);
    RUN_TEST(test_lora_send_data);
    RUN_TEST(test_lora_send_string);
    RUN_TEST(test_lora_network_joined);

    UNITY_END();
}

int main(int argc, char **argv) {
    RUN_UNITY_TESTS();
    return 0;
} 