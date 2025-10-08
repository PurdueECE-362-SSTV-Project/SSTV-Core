#include <unity.h>
#include "native/n_bmmpi.hpp"

// Testing only static methods of BaseQueue
void test_empty() {
    TEST_ASSERT_TRUE((BaseQueue<int, 4>::empty(0, 0)));  // Empty queue
    TEST_ASSERT_FALSE((BaseQueue<int, 4>::empty(0, 1))); // One element
    TEST_ASSERT_FALSE((BaseQueue<int, 4>::empty(1, 2))); // One element
    TEST_ASSERT_TRUE((BaseQueue<int, 4>::empty(2, 2)));  // Empty queue
    TEST_ASSERT_TRUE((BaseQueue<int, 4>::empty(3, 3)));  // Empty queue
}

void test_full() {
    TEST_ASSERT_FALSE((BaseQueue<int, 4>::full(0, 0)));  // Empty queue
    TEST_ASSERT_FALSE((BaseQueue<int, 4>::full(0, 1)));  // One element
    TEST_ASSERT_FALSE((BaseQueue<int, 4>::full(0, 2)));  // Two elements
    TEST_ASSERT_TRUE((BaseQueue<int, 4>::full(0, 3)));  // Three elements
    TEST_ASSERT_TRUE((BaseQueue<int, 4>::full(3, 2)));
}

void test_size() {
    TEST_ASSERT_EQUAL(0, (BaseQueue<int, 4>::size(0, 0)));  // Empty queue
    TEST_ASSERT_EQUAL(2, (BaseQueue<int, 4>::size(0, 1)));  // One element
    TEST_ASSERT_EQUAL(3, (BaseQueue<int, 4>::size(0, 2)));  // Two elements
    TEST_ASSERT_EQUAL(4, (BaseQueue<int, 4>::size(0, 3)));  // Three elements
    
    // Test wraparound cases
    TEST_ASSERT_EQUAL(2, (BaseQueue<int, 4>::size(3, 0)));  // Wrapped one element
    TEST_ASSERT_EQUAL(3, (BaseQueue<int, 4>::size(3, 1)));  // Wrapped two elements
    TEST_ASSERT_EQUAL(4, (BaseQueue<int, 4>::size(3, 2)));  // Wrapped three elements
}

void test_wraparound_increment() {
    TEST_ASSERT_EQUAL(1, (BaseQueue<int, 4>::wraparound_increment(0)));
    TEST_ASSERT_EQUAL(2, (BaseQueue<int, 4>::wraparound_increment(1)));
    TEST_ASSERT_EQUAL(3, (BaseQueue<int, 4>::wraparound_increment(2)));
    TEST_ASSERT_EQUAL(0, (BaseQueue<int, 4>::wraparound_increment(3)));  // Wrap back to 0
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_empty);
    RUN_TEST(test_full);
    RUN_TEST(test_size);
    RUN_TEST(test_wraparound_increment);
    UNITY_END();
    
    return 0;
}