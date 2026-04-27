#include "kernel/util.h"
// converts an int to sacii
void int_to_ascii(int n, char str[]) {
  int i, sign;
  if ((sign = n) < 0)
    n = -n;
  i = 0;
  do {
    str[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);

  if (sign < 0)
    str[i++] = '-';
  str[i] = '\0';

  int start = 0;
  int end = i - 1;
  while (start < end) {
    char tmp = str[start];
    str[start] = str[end];
    str[end] = tmp;
    start++;
    end--;
  }
}
