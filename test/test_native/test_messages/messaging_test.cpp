#include <unity.h>
#include "native/n_bmmpi.hpp"
#include "AppConfig.h"


ReceiverRouter fifo_router_test;
int global_flag_0 = 0;

void test_irq_0(FIFOMessage msg) {
    global_flag_0 += msg.data;
}

void test_irq_1(FIFOMessage msg) {
    global_flag_0 -= msg.data;
}

void tearDown() {

}

void setUp() {
    fifo_router_test = ReceiverRouter();
    fifo_router_test.register_handler(0, test_irq_0);
    fifo_router_test.register_handler(1, test_irq_1);
    global_flag_0 = 0;
}

void test_register_handler() {
    // Attempt to register a handler to an already-registered ID
    TEST_ASSERT_FALSE(fifo_router_test.register_handler(0, test_irq_1));
    // Attempt to register a handler to an invalid ID
    TEST_ASSERT_FALSE(fifo_router_test.register_handler(FIFO_DEST_ID_MAX + 1, test_irq_1));
    // Register a handler to a valid, unregistered ID
    TEST_ASSERT_TRUE(fifo_router_test.register_handler(2, test_irq_1));
    TEST_ASSERT_EQUAL(3, fifo_router_test.get_registered_count());
}

void test_route_message() {
    FIFOMessage msg;
    construct_message(0, 0, 5, &msg);
    TEST_ASSERT_TRUE(fifo_router_test.route_message(msg));
    TEST_ASSERT_EQUAL(5, global_flag_0);

    construct_message(1, 0, 3, &msg);
    TEST_ASSERT_TRUE(fifo_router_test.route_message(msg));
    TEST_ASSERT_EQUAL(2, global_flag_0);

    // Attempt to route a message to an unregistered ID
    construct_message(3, 0, 10, &msg);
    TEST_ASSERT_FALSE(fifo_router_test.route_message(msg));
    TEST_ASSERT_EQUAL(2, global_flag_0);

    // Attempt to route a message to an invalid ID
    construct_message(FIFO_DEST_ID_MAX + 1, 0, 10, &msg);
    TEST_ASSERT_FALSE(fifo_router_test.route_message(msg));
    TEST_ASSERT_EQUAL(2, global_flag_0);
}


int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_register_handler);
    UNITY_END();
}