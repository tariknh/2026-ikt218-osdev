#include "./string.h"

int32_t strlen(const char string[]) {
    int32_t length = 0;

    while (string[length] != '\0') {
        length++;
    }

    return length;
}

int16_t strcmp(const char string_a[], const char string_b[]) {
    while (*string_a && (*string_a == *string_b)) {
        string_a++;
        string_b++;
    }

    int16_t result = (unsigned char)*string_a - (unsigned char)*string_b;
    return result;
}


static int int_to_str(int32_t value, char *buf) {
    // handle zero explicitly
    if (value == 0) {
        buf[0] = '0';
        return 1;
    }

    int is_negative = 0;
    int i = 0;

    if (value < 0) {
        is_negative = 1;
        // careful: handle INT32_MIN
        // convert using unsigned to avoid overflow
        uint32_t u = (uint32_t)(-(value + 1)) + 1;

        while (u > 0) {
            buf[i++] = '0' + (u % 10);
            u /= 10;
        }

        if (is_negative) {
            buf[i++] = '-';
        }

        // reverse
        for (int l = 0, r = i - 1; l < r; l++, r--) {
            char tmp = buf[l];
            buf[l] = buf[r];
            buf[r] = tmp;
        }

        return i;
    }
    else {
        uint32_t u = (uint32_t)value;

        while (u > 0) {
            buf[i++] = '0' + (u % 10);
            u /= 10;
        }

        // reverse
        for (int l = 0, r = i - 1; l < r; l++, r--) {
            char tmp = buf[l];
            buf[l] = buf[r];
            buf[r] = tmp;
        }

        return i;
    }
}

char* format_string(char input_string[], int32_t value) {
    if (!input_string) {
        return NULL;
    }

    uint16_t input_size = strlen(input_string);

    // worst case: input + int32 max + null
    char* out = (char*)malloc(input_size + 12);
    if (!out) {
        return NULL;
    }

    uint16_t j = 0;

    for (uint16_t i = 0; i < input_size && input_string[i] != '\0'; i++) {

        if (input_string[i] == '%' && (i + 1) < input_size && input_string[i + 1] == 'd') {
            char num_buf[12];
            int len = int_to_str(value, num_buf);

            for (int k = 0; k < len; k++) {
                out[j] = num_buf[k];
                j++;
            }

            i++; // skip 'd'
        }
        else {
            out[j++] = input_string[i];
        }
    }

    out[j] = '\0';
    return out;
}