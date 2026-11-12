#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

/* 1: строка прочитана, 0: конец ввода, -1: строка слишком длинная. */
int read_line(FILE* stream, char* text, size_t size)
{
    size_t length;
    int ch;
    if (!fgets(text, (int)size, stream)) return 0;
    length = strlen(text);
    if (length > 0 && text[length - 1] == '\n') {
        text[--length] = '\0';
    }
    else {
        ch = fgetc(stream);
        if (ch != '\n' && ch != EOF) {
            while (ch != '\n' && ch != EOF) ch = fgetc(stream);
            return -1;
        }
    }
    if (length > 0 && text[length - 1] == '\r') text[length - 1] = '\0';
    return 1;
}

int read_text(const char* prompt, char* text, size_t size)
{
    int result;
    for (;;) {
        printf("%s", prompt);
        result = read_line(stdin, text, size);
        if (result == 0) return 0;
        if (result == 1 && text[0] != '\0') return 1;
        printf("Ошибка: введите непустую строку длиной до %zu байт.\n", size - 1);
    }
}

int read_int(const char* prompt, int minimum, int maximum, int* value)
{
    char text[INPUT_SIZE];
    char* end;
    long number;
    for (;;) {
        if (!read_text(prompt, text, sizeof(text))) return 0;
        errno = 0;
        number = strtol(text, &end, 10);
        if (end == text) {
            printf("Ошибка: требуется целое число.\n");
            continue;
        }
        while (isspace((unsigned char)*end)) end++;
        if (errno == 0 && *end == '\0' && number >= minimum && number <= maximum) {
            *value = (int)number;
            return 1;
        }
        printf("Ошибка: введите целое число от %d до %d.\n", minimum, maximum);
    }
}
