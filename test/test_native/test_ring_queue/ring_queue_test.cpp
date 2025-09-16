#include <unity.h>
#include "native/bmmpi_native.hpp"
#include "AppConfig.h"


RingQueue<int> test_queue(0);


void setUP(void) {
    test_queue = RingQueue<int>(0);
}


void tearDown(void) {
    test_queue.flush();
}


void test_init_empty() {
    TEST_ASSERT_TRUE(test_queue.empty());
    TEST_ASSERT_EQUAL(0, test_queue.size());
    TEST_ASSERT_EQUAL(RING_QUEUE_BUFFER_SIZE, test_queue.max_size());
    TEST_ASSERT_EQUAL(RING_QUEUE_BUFFER_SIZE, 4);
}

void test_push_pop() {
    TEST_ASSERT(test_queue.pop_front() == 0); // pop from empty queue returns default value
    
    // test basic push pop operations
    TEST_ASSERT_TRUE(test_queue.push_back(1));
    TEST_ASSERT_TRUE(test_queue.push_back(2));
    TEST_ASSERT_EQUAL(2, test_queue.size());
    TEST_ASSERT_EQUAL(1, test_queue.pop_front());
    TEST_ASSERT_EQUAL(1, test_queue.size());

    TEST_ASSERT_TRUE(test_queue.push_back(3));
    TEST_ASSERT_TRUE(test_queue.push_back(4));
    TEST_ASSERT_TRUE(test_queue.push_back(5));
    TEST_ASSERT_EQUAL(4, test_queue.size());
    TEST_ASSERT_TRUE(test_queue.full());
    TEST_ASSERT_FALSE(test_queue.push_back(6)); // queue should be full now
    TEST_ASSERT_EQUAL(test_queue.pop_front(), 2);
    TEST_ASSERT_EQUAL(test_queue.pop_front(), 3);
    TEST_ASSERT_EQUAL(test_queue.pop_front(), 4);
    TEST_ASSERT_EQUAL(test_queue.pop_front(), 5);
    TEST_ASSERT_TRUE(test_queue.empty());
    TEST_ASSERT_EQUAL(test_queue.pop_front(), 0); // pop from empty queue returns default
}

void test_push_pop_repeated() {
    for(int x = 0; x < 10; x++) {
        for(int i = 0; i < 3; i++) {
            TEST_ASSERT_TRUE(test_queue.push_back(i));
            TEST_ASSERT_EQUAL(i + 1, test_queue.size());
            TEST_ASSERT_FALSE(test_queue.empty());
        }
        for(int i = 0; i < 3; i++) {
            TEST_ASSERT_EQUAL(i, test_queue.pop_front());
            TEST_ASSERT_EQUAL(2 - i, test_queue.size());
        }
        TEST_ASSERT_TRUE(test_queue.empty());
    }
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_init_empty);
    RUN_TEST(test_push_pop);
    RUN_TEST(test_push_pop_repeated);
    UNITY_END();
}

