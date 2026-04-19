#ifndef TYPINGTEST_H
#define TYPINGTEST_H

/*
 * Typing Speed Test for lazarOS.
 * Presents words to the user and measures typing speed in WPM.
 * Uses PIT ticks for precise millisecond timing and keyboard
 * input for character-by-character comparison.
 */

void typingtest_run(void);

#endif /* TYPINGTEST_H */
