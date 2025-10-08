
#include <unity.h>
#include "native/templates/to_uint32_t.tpp"

void setUp(void) {
}

void tearDown(void) {
}

void test_uint32_conversion() {
    struct TestStruct {
        uint8_t a;
        uint8_t b;
        uint16_t c;
    };
    
    TestStruct test = {1, 2, 0x0304};
    uint32_t result = to_uint32(test);
    
    TEST_ASSERT_EQUAL(0x03040201, result);
}

void test_uint32_conversion_simple() {
    uint16_t test = 0x1234;
    uint32_t result = to_uint32(test);
    
    TEST_ASSERT_EQUAL(0x1234, result);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_uint32_conversion);
    RUN_TEST(test_uint32_conversion_simple);
    UNITY_END();
}