#include "unity.h"
#include "interrupts.h"
#include "stdint.h"

void setUp() {}
void tearDown() {}

void test_create_attributes() {
    uint8_t o = create_idt_attributes(true, 3, idt_type_interrupt);
    TEST_ASSERT_EQUAL_UINT8(0b11101110, o);
    //                        ^^ ^^
    //                        || |Interrupt gate (4 bits)
    //                        || Allways 0 (1 bit)
    //                        |Ring (2 bits)
    //                        Present (1 bit)
    o = create_idt_attributes(false, 2, idt_type_task_gate);
    TEST_ASSERT_EQUAL_UINT8(0b01000101, o);
}

void test_create_gate() {
    struct idt_gate o = create_idt_gate(0xFFFF1111, 0, 0b11101110);
    TEST_ASSERT_EQUAL_UINT16(0x1111, o.low_offset);
    TEST_ASSERT_EQUAL_UINT16(0, o.selector);
    TEST_ASSERT_EQUAL_UINT8(0, o.RESERVED_DO_NOT_USE);
    TEST_ASSERT_EQUAL_UINT8(0b11101110, o.attributes);
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, o.hi_offset);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_attributes);
    RUN_TEST(test_create_gate);
    return UNITY_END();
}