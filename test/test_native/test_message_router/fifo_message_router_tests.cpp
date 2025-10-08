#include <unity.h>
#include "native/n_bmmpi.hpp"
#include "AppConfig.h"

ReceiverRouter test_router;
FIFOMessage test_message;
bool handler_called = false;  // Global variable to track if handler was called

// Define the handler as a regular function instead of a lambda
void test_handler(FIFOMessage msg) {
    handler_called = true;
}

void setUp(void) {
    test_router = ReceiverRouter();
    test_message = FIFOMessage();
    handler_called = false;  // Reset the flag before each test
}

void tearDown(void) {
}

void test_receiver_router() {
    // Test registration
    TEST_ASSERT_TRUE(test_router.register_handler(1, test_handler));
    TEST_ASSERT_EQUAL(1, test_router.get_registered_count());
    
    // Test duplicate registration
    TEST_ASSERT_FALSE(test_router.register_handler(1, test_handler));
    
    // Test invalid registration
    TEST_ASSERT_FALSE(test_router.register_handler(FIFO_DEST_ID_MAX + 1, test_handler));
    
    // Test routing
    FIFOMessage msg;
    construct_message(1, 0, 0, &msg);
    TEST_ASSERT_TRUE(test_router.route_message(msg));
    TEST_ASSERT_TRUE(handler_called);
    
    // Test invalid routing
    construct_message(2, 0, 0, &msg);
    TEST_ASSERT_FALSE(test_router.route_message(msg));
}

// ... rest of your test file ...

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_receiver_router);
    // ... other tests ...
    UNITY_END();
}