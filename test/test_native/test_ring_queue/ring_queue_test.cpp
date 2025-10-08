#include <unity.h>
#include "native/n_bmmpi.hpp"
#include "AppConfig.h"

constexpr int QUEUE_SIZE = 4;
RingQueue<int, QUEUE_SIZE> test_queue(0);

void setUp(void) {
    test_queue = RingQueue<int, QUEUE_SIZE>(0);
}

void tearDown(void) {
    test_queue.flush();
}

void test_init_empty() {
    TEST_ASSERT_TRUE(test_queue.empty());
    TEST_ASSERT_EQUAL(0, test_queue.size());
    TEST_ASSERT_EQUAL(QUEUE_SIZE, test_queue.max_size());
}

void test_push_pop() {
    // Test pop from empty queue
    bool success;
    TEST_ASSERT_EQUAL(0, test_queue.pop_front(&success));
    TEST_ASSERT_FALSE(success);
    
    // Test basic push operations
    TEST_ASSERT_TRUE(test_queue.push_back(1));
    TEST_ASSERT_EQUAL(1, test_queue.size());
    TEST_ASSERT_FALSE(test_queue.empty());
    
    TEST_ASSERT_TRUE(test_queue.push_back(2));
    TEST_ASSERT_EQUAL(2, test_queue.size());
    
    // Test basic pop operations
    TEST_ASSERT_EQUAL(1, test_queue.pop_front(&success));
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(1, test_queue.size());
    
    TEST_ASSERT_EQUAL(2, test_queue.pop_front(&success));
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_TRUE(test_queue.empty());
}

void test_queue_full() {
    bool success;
    // Fill queue to capacity
    for(int i = 1; i <= QUEUE_SIZE; i++) {
        TEST_ASSERT_TRUE(test_queue.push_back(i));
    }
    
    // Queue should be full
    TEST_ASSERT_TRUE(test_queue.full());
    TEST_ASSERT_FALSE(test_queue.push_back(5));
    TEST_ASSERT_EQUAL(QUEUE_SIZE, test_queue.size());
    
    // Verify contents
    for(int i = 1; i <= QUEUE_SIZE; i++) {
        TEST_ASSERT_EQUAL(i, test_queue.pop_front(&success));
        TEST_ASSERT_TRUE(success);
    }
}

void test_wrap_around() {
    bool success;
    // Fill and partially empty the queue
    TEST_ASSERT_TRUE(test_queue.push_back(1));
    TEST_ASSERT_TRUE(test_queue.push_back(2));
    TEST_ASSERT_EQUAL(1, test_queue.pop_front(&success));
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(2, test_queue.pop_front(&success));
    TEST_ASSERT_TRUE(success);
    
    // Add more elements to test wrap-around
    for(int i = 3; i <= QUEUE_SIZE + 2; i++) {
        TEST_ASSERT_TRUE(test_queue.push_back(i));
    }
    
    // Verify wrapped contents
    for(int i = 3; i <= QUEUE_SIZE + 2; i++) {
        TEST_ASSERT_EQUAL(i, test_queue.pop_front(&success));
        TEST_ASSERT_TRUE(success);
    }
}

void test_flush() {
    bool success;
    // Fill queue partially
    TEST_ASSERT_TRUE(test_queue.push_back(1));
    TEST_ASSERT_TRUE(test_queue.push_back(2));
    TEST_ASSERT_EQUAL(2, test_queue.size());
    
    // Test flush
    test_queue.flush();
    TEST_ASSERT_TRUE(test_queue.empty());
    TEST_ASSERT_EQUAL(0, test_queue.size());
    
    // Verify queue still works after flush
    TEST_ASSERT_TRUE(test_queue.push_back(3));
    TEST_ASSERT_EQUAL(1, test_queue.size());
    TEST_ASSERT_EQUAL(3, test_queue.pop_front(&success));
    TEST_ASSERT_TRUE(success);
}

void test_push_pop_repeated() {
    bool success;
    // Test repeated push/pop cycles
    for(int x = 0; x < 10; x++) {
        // Fill queue partially
        for(int i = 0; i < QUEUE_SIZE - 1; i++) {
            TEST_ASSERT_TRUE(test_queue.push_back(i));
            TEST_ASSERT_EQUAL(i + 1, test_queue.size());
            TEST_ASSERT_FALSE(test_queue.empty());
        }
        
        // Empty queue and verify contents
        for(int i = 0; i < QUEUE_SIZE - 1; i++) {
            TEST_ASSERT_EQUAL(i, test_queue.pop_front(&success));
            TEST_ASSERT_TRUE(success);
            TEST_ASSERT_EQUAL(QUEUE_SIZE - 2 - i, test_queue.size());
        }
        TEST_ASSERT_TRUE(test_queue.empty());
    }
}

void test_edge_cases() {
    bool success;
    // Test pushing to full queue
    for(int i = 0; i < QUEUE_SIZE; i++) {
        TEST_ASSERT_TRUE(test_queue.push_back(i));
    }
    TEST_ASSERT_FALSE(test_queue.push_back(100));
    
    // Test popping from empty queue
    test_queue.flush();
    TEST_ASSERT_EQUAL(0, test_queue.pop_front(&success));
    TEST_ASSERT_FALSE(success);
    
    // Test alternating push/pop at boundaries
    TEST_ASSERT_TRUE(test_queue.push_back(1));
    TEST_ASSERT_EQUAL(1, test_queue.pop_front(&success));
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_TRUE(test_queue.push_back(2));
    TEST_ASSERT_EQUAL(2, test_queue.pop_front(&success));
    TEST_ASSERT_TRUE(success);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_init_empty);
    RUN_TEST(test_push_pop);
    RUN_TEST(test_queue_full);
    RUN_TEST(test_wrap_around);
    RUN_TEST(test_flush);
    RUN_TEST(test_push_pop_repeated);
    RUN_TEST(test_edge_cases);
    UNITY_END();
    
    return 0;
}