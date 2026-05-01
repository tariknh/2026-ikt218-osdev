#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int putchar(int ic);
int printf(const char *__restrict__ format, ...);
void terminal_initialize(void);
void terminal_refresh(void); // Lagt til etterpå
void terminal_set_color(uint8_t color);
uint8_t terminal_get_color(void);
void terminal_scroll_line_up(void);
void terminal_scroll_line_down(void);
void terminal_scroll_page_up(void);
void terminal_scroll_page_down(void);
void terminal_scroll_to_top(void);
void terminal_scroll_to_bottom(void);

#ifdef __cplusplus
}
#endif
