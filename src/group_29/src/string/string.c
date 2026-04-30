#include "./string.h"

int32_t strlength(const char string[]) {
    int32_t length = 0;

    while (string[length] != '\0') {
        length++;
    }

    return length;
}

int16_t strcompare(const char string_a[], const char string_b[]) {
    while (*string_a && (*string_a == *string_b)) {
        string_a++;
        string_b++;
    }

    int16_t result = (unsigned char)*string_a - (unsigned char)*string_b;
    return result;
}


int int_to_str(int32_t value, char *buf) {
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

int32_t str_to_int_checked(const char string[], int* ok) {
    if (!string) {
        if (ok) *ok = 0;
        return 0;
    }

    const char *start = string;
    int32_t result = 0;
    int32_t sign = 1;

    // skip whitespace
    while (*string == ' ' || *string == '\t' || *string == '\n' ||
           *string == '\r' || *string == '\v' || *string == '\f') {
        string++;
    }

    // sign
    if (*string == '-') {
        sign = -1;
        string++;
    } else if (*string == '+') {
        string++;
    }

    // must have at least one digit
    if (*string < '0' || *string > '9') {
        if (ok) *ok = 0;
        return 0;
    }

    int has_digits = 0;

    while (*string >= '0' && *string <= '9') {
        has_digits = 1;

        result = result * 10 + (*string - '0');
        string++;
    }

    // ensure full consumption (no trailing junk)
    if (*string != '\0' && *string != '\n') {
        if (ok) *ok = 0;
        return 0;
    }

    if (!has_digits) {
        if (ok) *ok = 0;
        return 0;
    }

    if (ok) *ok = 1;
    return result * sign;
}

char* format_string(char input_string[], int32_t value) {
    if (!input_string) {
        return NULL;
    }

    uint16_t input_size = strlength(input_string);

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

int16_t string_copy(const char input[], char output[], int16_t output_length) {
    int input_length = strlength(input);

    if (input_length >= output_length) {
        return -1;
    }

    for (int i = 0; i <= input_length; i++) {
        output[i] = input[i];
    }

    return 0;
}