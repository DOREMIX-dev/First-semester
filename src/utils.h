#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define INPUT_SIZE 256

int read_line(FILE* stream, char* text, size_t size);
int read_text(const char* prompt, char* text, size_t size);
int read_int(const char* prompt, int minimum, int maximum, int* value);

#define HOURS_PER_DAY 24
#define MORNING_HOUR 8
#define BEDTIME_HOUR 2

typedef struct GameClock {
    int day;
    int hour;
} GameClock;

void print_time(const GameClock* clock);
int hours_until_bedtime(const GameClock* clock);
int advance_clock(GameClock* clock, int hours);
int sleep_until_morning(GameClock* clock);

#endif
