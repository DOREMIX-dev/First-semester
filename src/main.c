#include "map.h"
#include "player.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>

#define MAX_RECORDS 200
#define MAX_NPC_NAME 64

#define HOURS_PER_DAY 24
#define MAX_MENU_CHOICE 11
#define SCHEDULE_TOKEN_SIZE 1024

int load_items(char item_names[][MAX_NAME_LEN])
{
    FILE* f;
    char line[MAX_NAME_LEN];
    int line_no = 0;
    int result;

    f = fopen("items.txt", "r");
    if (!f) {
        printf("Ошибка: файл items.txt не найден. Программа будет остановлена.\n");
        return 0;
    }
    while ((result = read_line(f, line, sizeof(line))) != 0) {
        if (result < 0 || line[0] == '\0' || line_no >= MAX_ITEMS) {
            printf("Ошибка items.txt: нужны %d непустых названий длиной до %d байт.\n",
                MAX_ITEMS, MAX_NAME_LEN - 1);
            fclose(f);
            return 0;
        }
        strcpy(item_names[line_no++], line);
    }
    if (ferror(f) || line_no != MAX_ITEMS) {
        printf("Ошибка: не удалось прочитать все %d названий из items.txt.\n", MAX_ITEMS);
        fclose(f);
        return 0;
    }
    fclose(f);

    printf("Каталог предметов загружен из items.txt (%d названий)\n", line_no);
    return 1;
}

void print_menu(void)
{
    printf("\n---------------- МЕНЮ ----------------\n");
    printf(" 1 - Создать или пересоздать персонажа\n");
    printf(" 2 - Карточка персонажа\n");
    printf(" 3 - Посмотреть инвентарь\n");
    printf(" 4 - Положить предмет в слот\n");
    printf(" 5 - Выбросить предмет из слота\n");
    printf(" 6 - Найти самый частый предмет\n");
    printf(" 7 - Поиск предмета в рюкзаке по названию\n");
    printf(" 8 - Записать состояние в дневник (diary.txt)\n");
    printf(" 9 - Расписание жителей деревни (input.txt -> output.txt)\n");
    printf("10 - Отправиться в приключение\n");
    printf("11 - Потратить часы на работу\n");
    printf(" 0 - Выйти из программы\n");
    printf("--------------------------------------\n");
}

// 1
void print_time(int day, int hour)
{
    printf("Текущее время: %d, %d:00 \n", day, hour);
}

// 2
void work(int* day, int* hour)
{
    int spend;

    if (!read_int("Сколько хотите потратить часов на работу? ", 0, INT_MAX - *hour, &spend)) return;
    if ((spend + *hour) / HOURS_PER_DAY > INT_MAX - *day) {
        printf("Слишком большое количество дней.\n");
        return;
    }
    *day += (spend + *hour) / HOURS_PER_DAY;
    *hour = (spend + *hour) % HOURS_PER_DAY;
}

// 8
void write_diary(const char* farmer_name, int day, int hour,
    const int* counts, const char item_names[][MAX_NAME_LEN], const int* ids)
{
    FILE* f;
    int i;

    f = fopen("diary.txt", "a");
    if (!f) {
        printf("Ошибка: не удалось открыть diary.txt для записи.\n");
        return;
    }

    fprintf(f, "=== Дневник фермера %s ===\n", farmer_name);
    fprintf(f, "День %d, %d:00\n", day, hour);
    fprintf(f, "Инвентарь:\n");
    for (i = 0; i < INV_SIZE; i++) {
        fprintf(f, "  Слот %d: [%d] - %s\n", i, counts[i], item_names[ids[i]]);
    }
    fprintf(f, "\n");

    fclose(f);
    printf("Состояние записано в diary.txt\n");
}

int parse_hour(const char* text, int maximum, int* hour)
{
    char* end;
    long value;
    errno = 0;
    value = strtol(text, &end, 10);
    if (end == text) return 0;
    while (isspace((unsigned char)*end)) end++;
    if (errno != 0 || *end != '\0' || value < 0 || value > maximum) return 0;
    *hour = (int)value;
    return 1;
}

int parse_entry(char* token,
    char names[][MAX_NPC_NAME], int* starts, int* ends,
    char locs[][MAX_NPC_NAME], int* count)
{
    char* fields[4];
    int i, start, end;
    fields[0] = token;
    for (i = 1; i < 4; i++) {
        char* separator = strchr(fields[i - 1], ';');
        if (!separator) return 0;
        *separator = '\0';
        fields[i] = separator + 1;
    }
    if (strchr(fields[3], ';')) return 0;
    /* Убираем пробелы вокруг полей, в том числе после запятой между записями. */
    for (i = 0; i < 4; i++) {
        size_t length;
        while (isspace((unsigned char)*fields[i])) fields[i]++;
        length = strlen(fields[i]);
        while (length > 0 && isspace((unsigned char)fields[i][length - 1]))
            fields[i][--length] = '\0';
        if (length == 0) return 0;
    }
    if (*count >= MAX_RECORDS || strlen(fields[0]) >= MAX_NPC_NAME ||
        strlen(fields[3]) >= MAX_NPC_NAME ||
        !parse_hour(fields[1], HOURS_PER_DAY - 1, &start) ||
        !parse_hour(fields[2], HOURS_PER_DAY, &end)) return 0;
    strcpy(names[*count], fields[0]);
    strcpy(locs[*count], fields[3]);
    starts[*count] = start;
    ends[*count] = end;
    (*count)++;
    return 1;
}

int load_schedule(char names[][MAX_NPC_NAME], int* starts, int* ends,
    char locs[][MAX_NPC_NAME], int* count)
{
    FILE* f;
    char token[SCHEDULE_TOKEN_SIZE];
    size_t length = 0;
    int c;
    int valid = 1;
    *count = 0;
    f = fopen("input.txt", "r");
    if (!f) return 0;
    while ((c = fgetc(f)) != EOF) {
        if (c == ',' || c == '\n' || c == '\r') {
            if (length > 0) {
                token[length] = '\0';
                if (!parse_entry(token, names, starts, ends, locs, count)) {
                    valid = 0;
                    break;
                }
                length = 0;
            }
        }
        else {
            if (length >= sizeof(token) - 1) {
                valid = 0;
                break;
            }
            token[length++] = (char)c;
        }
    }
    if (valid && length > 0) {
        token[length] = '\0';
        valid = parse_entry(token, names, starts, ends, locs, count);
    }
    if (ferror(f) || *count == 0) valid = 0;
    fclose(f);
    return valid;
}

int hour_in_interval(int hour, int start, int end)
{
    if (end > start) {
        return hour >= start && hour < end;
    }
    if (end < start) {
        return hour >= start || hour < end;
    }
    return 0;
}

int find_location(const char names[][MAX_NPC_NAME], const int* starts, const int* ends,
    const char locs[][MAX_NPC_NAME], int count,
    const char* npc, int hour, char* out_loc)
{
    int i;

    for (i = 0; i < count; i++) {
        if (strcmp(names[i], npc) == 0) {
            if (hour_in_interval(hour, starts[i], ends[i])) {
                snprintf(out_loc, MAX_NPC_NAME, "%s", locs[i]);
                return 1;
            }
        }
    }
    return 0;
}

// 9
void decode_variant(int hour)
{
    char names[MAX_RECORDS][MAX_NPC_NAME];
    char locs[MAX_RECORDS][MAX_NPC_NAME];
    int starts[MAX_RECORDS];
    int ends[MAX_RECORDS];
    int count = 0;
    char unique[MAX_RECORDS][MAX_NPC_NAME];
    int unique_count = 0;
    FILE* out;
    char cur_loc[MAX_NPC_NAME];
    int i, j;

    if (!load_schedule(names, starts, ends, locs, &count)) {
        printf("Ошибка: не удалось прочитать корректное расписание из input.txt.\n");
        return;
    }

    for (i = 0; i < count; i++) {
        int seen = 0;
        for (j = 0; j < unique_count; j++) {
            if (strcmp(unique[j], names[i]) == 0) {
                seen = 1;
                break;
            }
        }
        if (!seen) {
            snprintf(unique[unique_count], MAX_NPC_NAME, "%s", names[i]);
            unique_count++;
        }
    }

    out = fopen("output.txt", "w");
    if (!out) {
        printf("Ошибка: не удалось создать output.txt.\n");
        return;
    }

    printf("Расписание жителей на %d:00\n", hour);
    printf("--------------------------------------\n");
    fprintf(out, "Расписание жителей на %d:00\n", hour);
    fprintf(out, "--------------------------------------\n");

    for (j = 0; j < unique_count; j++) {
        if (find_location(names, starts, ends, locs, count, unique[j], hour, cur_loc)) {
            printf("%-20s %s\n", unique[j], cur_loc);
            fprintf(out, "%-20s %s\n", unique[j], cur_loc);
        }
        else {
            printf("%-20s [Спит дома]\n", unique[j]);
            fprintf(out, "%-20s [Спит дома]\n", unique[j]);
        }
    }

    fclose(out);
    printf("\nРезультат также записан в output.txt\n");
}

typedef enum MenuChoice {
    EXIT_GAME, CREATE_PLAYER, PLAYER_CARD, INVENTORY, ADD_ITEM, DROP_ITEM,
    COMMON_ITEM, FIND_ITEM, DIARY, SCHEDULE, ADVENTURE, WORK
} MenuChoice;

static void handle_choice(MenuChoice choice, Player* player, int* day, int* hour,
    char item_names[][MAX_NAME_LEN])
{
    if (!player && choice != SCHEDULE && choice != EXIT_GAME) {
        puts("Сначала создайте персонажа.");
        return;
    }

    switch (choice) {
    case PLAYER_CARD:
        print_player(player);
        print_time(*day, *hour);
        show_inventory(player->counts, item_names, player->ids);
        break;
    case WORK: work(day, hour); break;
    case INVENTORY: show_inventory(player->counts, item_names, player->ids); break;
    case ADD_ITEM: add_item(player->counts, player->ids); break;
    case DROP_ITEM: drop_item(player->counts, player->ids); break;
    case COMMON_ITEM: show_most_common(player->ids, item_names); break;
    case FIND_ITEM: find_item(player->counts, item_names, player->ids); break;
    case DIARY:
        write_diary(player->name, *day, *hour, player->counts, item_names, player->ids);
        break;
    case SCHEDULE: decode_variant(*hour); break;
    case ADVENTURE: adventure(); break;
    case EXIT_GAME:
    case CREATE_PLAYER: break;
    }
}

int main(void)
{
    Player* player = NULL;
    char item_names[MAX_ITEMS][MAX_NAME_LEN];
    int current_day = 1;
    int current_hour = 8;
    int choice;

    srand((unsigned int)time(NULL));
    if (!load_items(item_names)) return 1;

    while (1) {
        print_menu();
        if (!read_int("Введите номер действия: ", EXIT_GAME, MAX_MENU_CHOICE, &choice) ||
            choice == EXIT_GAME) break;
        if (choice == CREATE_PLAYER) player = create_player(player);
        else handle_choice((MenuChoice)choice, player, &current_day, &current_hour, item_names);
    }

    destroy_player(player);
    puts("До свидания!");
    return 0;
}
