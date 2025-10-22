#include <unity.h>
#include "native/n_types.hpp"
#include "AppConfig.h"


static Optional<int> test_option;


void tearDown() {
    test_option = Optional<int>();
}

void setUp() {
    
}

void test_none_option() {
    test_option = Optional<int>();
    TEST_ASSERT_FALSE(test_option.is_some());
    TEST_ASSERT_TRUE(test_option.get_ref() == nullptr);
}

void test_some_option() {
    test_option = Optional<int>(5);
    TEST_ASSERT_TRUE(test_option.is_some());
    TEST_ASSERT_TRUE(test_option.get_ref() != nullptr);
    TEST_ASSERT_EQUAL(5, *(test_option.get_ref()));
    TEST_ASSERT_EQUAL(5, test_option.take());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_none_option);
    RUN_TEST(test_some_option);
    UNITY_END();
}