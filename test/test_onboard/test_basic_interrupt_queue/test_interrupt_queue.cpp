#include <unity.h>
#include "board/bmmpi.hpp"
#include "AppConfig.h"


InterruptQueue<int, 8> test_queue1(-1);
InterruptQueue<int, 16> test_queue2(-1);
#define MAX_COUNT = 10
int glob_counter = 0;


void src_push_isr() {
    hw_clear_bits(&timer1_hw->intr, 1);
    if(glob_counter < MAX_COUNT) {
        test_queue1.push_back(glob_counter);
        glob_counter++;
    }

    timer1_hw->alarm[0] = timer1_hw->timerawl + 3000;
}

void inter_push_isr() {
    bool result;
    int popped = test_queue1.pop_front(&result);
    if(result) {
        test_queue2.push_back(popped);
    }

    timer_hw->alarm[0] = (uint32_t) timer_hw->timerawl + 2500;
}

void setUpSourceIRQ(void) {
    hw_set_bits(&timer1_hw->inte, 1 << 0);

    irq_set_exclusive_handler(TIMER1_IRQ_0, src_push_isr);
    irq_set_enabled(TIMER1_IRQ_0, true);

    timer1_hw->alarm[0] = (uint32_t) (timer1_hw->timerawl + 3000);
}

void tearDownSourceIRQ(void) {
    irq_remove_handler(TIMER1_IRQ_0, src_push_isr);
}

void setUpInterIRQ(void) {

}

void setUp(void) {
    setUpSourceIRQ();
}

void tearDown(void) {
    tearDownSourceIRQ();
}

void test_basic_push_pull() {
    int internal_counter;
    bool success = false;
    int value;

    for(internal_counter = 0; internal_counter < MAX_COUNT; internal_counter++) {
        while(!success) {
            value = test_queue1.pop_front(&success);
        }
        TEST_ASSERT_TRUE(value, internal_counter);
        success = false;
    }
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_basic_push_pull);
    UNITY_END();

    return 0;
}