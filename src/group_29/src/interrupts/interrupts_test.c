#include "unity.h"
#include "interrupts.h"
#include "stdint.h"

void setUp(){}
void tearDown(){}

void test_create_gate () {
    struct idt_gate o = create_idt_gate(0xFFFF1111, 0, 0b11101110);
    TEST_ASSERT_EQUAL_UINT16(0x1111, o.low_offset);
    TEST_ASSERT_EQUAL_UINT16(0, o.selector);
    TEST_ASSERT_EQUAL_UINT8(0, o.RESERVED_DO_NOT_USE);
    TEST_ASSERT_EQUAL_UINT8(0b11101110, o.attributes);
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, o.hi_offset);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_gate);
    return UNITY_END();
}