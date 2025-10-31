#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Include your decoder header
#include "decoder.h"
#include "decoder.c"

// Test result tracking
typedef struct {
    int total;
    int passed;
    int failed;
} TestResults;

TestResults results = {0, 0, 0};

// Helper function to print test results
void assert_test(const char* test_name, bool condition) {
    results.total++;
    if (condition) {
        results.passed++;
        printf("[PASS] %s\n", test_name);
    } else {
        results.failed++;
        printf("[FAIL] %s\n", test_name);
    }
}

// Helper to create test input
Decoder_FSM_Val create_input(uint16_t freq, uint16_t diff_time) {
    Decoder_FSM_Val input;
    input.freq = freq;
    input.diff_time = diff_time;
    return input;
}

// Test 1: Valid Robot36 transmission sequence
void test_robot36_valid_sequence() {
    printf("\n=== Test 1: Valid Robot36 Transmission ===\n");
    
    // Robot36 VIS code: 0001000 (binary) = 8 (decimal)
    // Expected sequence: SYNC1 -> HOLD -> SYNC2 -> START -> 7 CODE bits -> PARITY -> STOP
    
    Decoder_FSM_Val inputs[] = {
        create_input(1900, 300),  // SYNC1
        create_input(1200, 10),   // HOLD
        create_input(1900, 300),  // SYNC2
        create_input(1200, 30),   // START BIT
        create_input(1300, 30),   // Bit 0 (MSB): 0
        create_input(1300, 30),   // Bit 1: 0
        create_input(1300, 30),   // Bit 2: 0
        create_input(1100, 30),   // Bit 3: 1
        create_input(1300, 30),   // Bit 4: 0
        create_input(1300, 30),   // Bit 5: 0
        create_input(1300, 30),   // Bit 6 (LSB): 0
        create_input(1100, 30),   // PARITY (odd parity, 1 one-bit)
        create_input(1200, 30)    // STOP BIT
    };
    
    int result = 0;
    for (int i = 0; i < 13; i++) {
        result = header(inputs[i]);
    }
    
    assert_test("Robot36 VIS code decoded correctly", result == 8);
}

// Test 2: Valid Robot72 transmission sequence
void test_robot72_valid_sequence() {
    printf("\n=== Test 2: Valid Robot72 Transmission ===\n");
    
    // Robot72 VIS code: 0001100 (binary) = 12 (decimal)
    
    Decoder_FSM_Val inputs[] = {
        create_input(1900, 300),  // SYNC1
        create_input(1200, 10),   // HOLD
        create_input(1900, 300),  // SYNC2
        create_input(1200, 30),   // START BIT
        create_input(1300, 30),   // Bit 0: 0
        create_input(1300, 30),   // Bit 1: 0
        create_input(1300, 30),   // Bit 2: 0
        create_input(1100, 30),   // Bit 3: 1
        create_input(1100, 30),   // Bit 4: 1
        create_input(1300, 30),   // Bit 5: 0
        create_input(1300, 30),   // Bit 6: 0
        create_input(1300, 30),   // PARITY (even parity, 2 one-bits)
        create_input(1200, 30)    // STOP BIT
    };
    
    int result = 0;
    for (int i = 0; i < 13; i++) {
        result = header(inputs[i]);
    }
    
    assert_test("Robot72 VIS code decoded correctly", result == 12);
}

// Test 3: Frequency threshold boundary test
void test_frequency_threshold() {
    printf("\n=== Test 3: Frequency Threshold Boundaries ===\n");
    
    // Test SYNC1 at upper threshold boundary (1900 + 50 = 1950 Hz)
    bool test1 = threshold(1950, 1900, 50);
    assert_test("SYNC1 upper frequency boundary", test1);
    
    // Test SYNC1 at lower threshold boundary (1900 - 50 = 1850 Hz)
    bool test2 = threshold(1850, 1900, 50);
    assert_test("SYNC1 lower frequency boundary", test2);
    
    // Test SYNC1 just outside upper boundary (1900 + 51 = 1951 Hz)
    bool test3 = !threshold(1951, 1900, 50);
    assert_test("SYNC1 exceeds upper frequency boundary", test3);
    
    // Test SYNC1 just outside lower boundary (1900 - 51 = 1849 Hz)
    bool test4 = !threshold(1849, 1900, 50);
    assert_test("SYNC1 exceeds lower frequency boundary", test4);
}

// Test 4: Time threshold boundary test
void test_time_threshold() {
    printf("\n=== Test 4: Time Threshold Boundaries ===\n");
    
    // Test SYNC1 at upper time boundary (300 + 5 = 305 ms)
    bool test1 = threshold(305, 300, 5);
    assert_test("SYNC1 upper time boundary", test1);
    
    // Test SYNC1 at lower time boundary (300 - 5 = 295 ms)
    bool test2 = threshold(295, 300, 5);
    assert_test("SYNC1 lower time boundary", test2);
    
    // Test time just outside boundaries
    bool test3 = !threshold(306, 300, 5);
    assert_test("SYNC1 exceeds upper time boundary", test3);
}

// Test 5: Transmission error - wrong frequency in SYNC1
void test_error_wrong_sync1_frequency() {
    printf("\n=== Test 5: Error - Wrong SYNC1 Frequency ===\n");
    
    Decoder_FSM_Val input = create_input(1500, 300); // Wrong frequency
    int result = header(input);
    
    // Should return NULL or error indicator (check your implementation)
    printf("Result for wrong SYNC1 frequency: %d\n", result);
}

// Test 6: Transmission error - timing too long
void test_error_timing_too_long() {
    printf("\n=== Test 6: Error - Timing Too Long ===\n");
    
    Decoder_FSM_Val inputs[] = {
        create_input(1900, 300),  // SYNC1
        create_input(1200, 50)    // HOLD with timing way too long (should be ~10ms)
    };
    
    for (int i = 0; i < 2; i++) {
        header(inputs[i]);
    }
    
    printf("Timing error test completed\n");
}

// Test 7: Initialize mode function tests
void test_initialize_mode() {
    printf("\n=== Test 7: Initialize Mode Function ===\n");
    
    sstv_mode_t mode1 = initializeMode(ROBOT_36);
    assert_test("Initialize Robot36 mode", mode1.decMode == ROBOT_36);
    assert_test("Robot36 has correct row count", mode1.row == 320);
    assert_test("Robot36 has correct color format", mode1.color == YCRCB);
    
    sstv_mode_t mode2 = initializeMode(MARTIN_2);
    assert_test("Initialize Martin2 mode", mode2.decMode == MARTIN_2);
    assert_test("Martin2 has correct row count", mode2.row == 256);
    
    sstv_mode_t mode3 = initializeMode(PD_90);
    assert_test("Initialize PD90 mode", mode3.decMode == PD_90);
    assert_test("PD90 has correct transfer time", mode3.tranTime == 90);
    
    sstv_mode_t mode_invalid = initializeMode(999); // Invalid code
    assert_test("Invalid code returns NULL_Mode", mode_invalid.decMode == NULL);
}

// Test 8: Parity check test
void test_parity_checking() {
    printf("\n=== Test 8: Parity Checking ===\n");
    
    // Test with correct parity for code 0001000 (1 bit set, odd parity = 1)
    // This would need to be tested within the header function context
    printf("Parity check tests require full FSM context\n");
}

// Test 9: Martin2 valid sequence
void test_martin2_valid_sequence() {
    printf("\n=== Test 9: Valid Martin2 Transmission ===\n");
    
    // Martin2 VIS code: 0101000 (binary) = 40 (decimal)
    
    Decoder_FSM_Val inputs[] = {
        create_input(1900, 300),  // SYNC1
        create_input(1200, 10),   // HOLD
        create_input(1900, 300),  // SYNC2
        create_input(1200, 30),   // START BIT
        create_input(1300, 30),   // Bit 0: 0
        create_input(1100, 30),   // Bit 1: 1
        create_input(1300, 30),   // Bit 2: 0
        create_input(1100, 30),   // Bit 3: 1
        create_input(1300, 30),   // Bit 4: 0
        create_input(1300, 30),   // Bit 5: 0
        create_input(1300, 30),   // Bit 6: 0
        create_input(1300, 30),   // PARITY (even, 2 one-bits)
        create_input(1200, 30)    // STOP BIT
    };
    
    int result = 0;
    for (int i = 0; i < 13; i++) {
        result = header(inputs[i]);
    }
    
    assert_test("Martin2 VIS code decoded correctly", result == 40);
}

// Test 10: Edge case - multiple HOLD pulses (overlapping case)
void test_multiple_hold_pulses() {
    printf("\n=== Test 10: Multiple HOLD Pulses ===\n");
    
    Decoder_FSM_Val inputs[] = {
        create_input(1900, 300),  // SYNC1
        create_input(1200, 10),   // HOLD 1
        create_input(1200, 10),   // HOLD 2 (overlapping)
        create_input(1900, 300),  // SYNC2
    };
    
    for (int i = 0; i < 4; i++) {
        header(inputs[i]);
    }
    
    printf("Multiple HOLD pulse test completed\n");
}

// Main test runner
int main() {
    printf("========================================\n");
    printf("   SSTV Decoder Testbench\n");
    printf("========================================\n");
    
    // Run all tests
    test_frequency_threshold();
    test_time_threshold();
    test_initialize_mode();
    test_robot36_valid_sequence();
    test_robot72_valid_sequence();
    test_martin2_valid_sequence();
    test_error_wrong_sync1_frequency();
    test_error_timing_too_long();
    test_parity_checking();
    test_multiple_hold_pulses();
    
    // Print summary
    printf("\n========================================\n");
    printf("   Test Summary\n");
    printf("========================================\n");
    printf("Total Tests:  %d\n", results.total);
    printf("Passed:       %d\n", results.passed);
    printf("Failed:       %d\n", results.failed);
    printf("Success Rate: %.1f%%\n", 
           results.total > 0 ? (100.0 * results.passed / results.total) : 0.0);
    printf("========================================\n");
    
    return results.failed > 0 ? 1 : 0;
}
