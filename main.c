#include <stdio.h>
#include <string.h>

#define INV_SIZE 10
#define MAX_ITEMS 10
#define MAX_NAME_LEN 32
#define MAX_FARMER_NAME 33

#define MAX_RECORDS 200
#define MAX_NPC_NAME 64

int load_items(char item_names[][MAX_NAME_LEN])
{
    FILE* f;
    char line[256];
    int line_no = 0;
    int i;

    for (i = 0; i < MAX_ITEMS; i++) {
        item_names[i][0] = '\0';
    }

    f = fopen("items.txt", "r");
    if (!f) {
        printf("Ошибка: файл items.txt не найден. Программа будет остановлена.\n");
        return 0;
    }

    while (fgets(line, sizeof(line), f) != NULL && line_no < MAX_ITEMS) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }
        if (len == 0) {
            printf("Ошибка: в файле items.txt обнаружена пустая строка (строка %d). Программа будет остановлена.\n",
                line_no + 1);
            fclose(f);
            return 0;
        }
        snprintf(item_names[line_no], MAX_NAME_LEN, "%s", line);
        line_no++;
    }

    fclose(f);

    if (line_no == 0) {
        printf("Ошибка: файл items.txt пуст. Программа будет остановлена.\n");
        return 0;
    }

    if (line_no < MAX_ITEMS) {
        printf("Ошибка: в items.txt только %d названий из %d. Нужны все %d. Программа будет остановлена.\n",
            line_no, MAX_ITEMS, MAX_ITEMS);
        return 0;
    }

    printf("Каталог предметов загружен из items.txt (%d названий)\n", line_no);
    return 1;
}

void print_menu(void)
{
    printf("\n---------------- МЕНЮ ----------------\n");
    printf(" 1 - Узнать текущее время\n");
    printf(" 2 - Потратить часы на работу\n");
    printf(" 3 - Посмотреть инвентарь\n");
    printf(" 4 - Положить предмет в слот\n");
    printf(" 5 - Выбросить предмет из слота\n");
    printf(" 6 - Найти самый частый предмет\n");
    printf(" 7 - Поиск предмета в рюкзаке по названию\n");
    printf(" 8 - Записать состояние в дневник (diary.txt)\n");
    printf(" 9 - Расписание жителей деревни (input.txt -> output.txt)\n");
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

    printf("Сколько хотите потратить часов на работу? ");
    scanf("%d", &spend);

    if (*hour + spend < 24) {
        *hour = *hour + spend;
    }
    else {
        *day = *day + 1;
        *hour = *hour + spend - 24;
        while (*hour >= 24) {
            *day = *day + 1;
            *hour = *hour - 24;
        }
    }
}

// 3
void show_inventory(const int* counts, const char item_names[][MAX_NAME_LEN], const int* ids)
{
    int i;

    for (i = 0; i < INV_SIZE; i++) {
        printf("Слот %d: [%d] - %s\n", i, counts[i], item_names[ids[i]]);
    }
}

// 4
void add_item(int* counts, int* ids)
{
    int slot, item;

    printf("Введите индекс слота: \n");
    scanf("%d", &slot);
    printf("Введите ID предмета: \n");
    scanf("%d", &item);

    if (slot >= 0 && slot < INV_SIZE && item >= 0 && item < MAX_ITEMS) {
        counts[slot] = counts[slot] + 1;
        ids[slot] = item;
    }
    else {
        printf("Введен неверный индекс или ID\n");
    }
}

// 5
void drop_item(int* counts, int* ids)
{
    int slot;

    printf("Введите индекс слота с которого хотите выбросить предметы\n");
    scanf("%d", &slot);

    if (slot >= 0 && slot < INV_SIZE) {
        counts[slot] = 0;
        ids[slot] = 0;
    }
    else {
        printf("Введен неверный индекс слота!\n");
    }
}

int count_item(const int* ids, int item_id)
{
    int i, total = 0;

    for (i = 0; i < INV_SIZE; i++) {
        if (ids[i] == item_id) {
            total++;
        }
    }
    return total;
}

const char* plural_raz(int n)
{
    int n10 = n % 10;
    int n100 = n % 100;
    if (n100 >= 11 && n100 <= 14) return "раз";
    if (n10 == 1) return "раз";
    if (n10 >= 2 && n10 <= 4) return "раза";
    return "раз";
}

// 6
void show_most_common(const int* ids, const char item_names[][MAX_NAME_LEN])
{
    int item;
    int best_item = 0;
    int best_count = 0;

    for (item = 1; item < MAX_ITEMS; item++) {
        int c = count_item(ids, item);
        if (best_count < c) {
            best_count = c;
            best_item = item;
        }
    }

    if (best_count == 0) {
        printf("В инвентаре нет предметов.\n");
        return;
    }

    printf("Самый частовстречаемый предмет: %s (ID %d), он встречается %d %s.\n",
        item_names[best_item], best_item, best_count, plural_raz(best_count));
}

// 7
void find_item(const int* counts, const char item_names[][MAX_NAME_LEN], const int* ids)
{
    char search[MAX_NAME_LEN];
    int id = -1;
    int i;
    int found_slot = 0;
    size_t len;

    printf("Введите название предмета для поиска: ");
    if (scanf(" %31[^\n]", search) != 1) {
        printf("Ошибка ввода названия.\n");
        return;
    }

    len = strlen(search);
    while (len > 0 && (search[len - 1] == '\r' || search[len - 1] == '\n')) {
        search[--len] = '\0';
    }

    for (i = 0; i < MAX_ITEMS; i++) {
        if (strcmp(item_names[i], search) == 0) {
            id = i;
            break;
        }
    }

    if (id == -1) {
        printf("Предмет '%s' не найден в каталоге.\n", search);
        return;
    }

    if (id == 0) {
        printf("'%s' — это пустой слот, а не предмет.\n", search);
        return;
    }

    printf("ID предмета '%s': %d\n", search, id);
    printf("Слоты с этим предметом:\n");
    for (i = 0; i < INV_SIZE; i++) {
        if (ids[i] == id && counts[i] > 0) {
            printf("  Слот %d: %d шт.\n", i, counts[i]);
            found_slot = 1;
        }
    }
    if (!found_slot) {
        printf("  В инвентаре этого предмета нет.\n");
    }
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

void parse_entry(const char* token,
    char names[][MAX_NPC_NAME], int* starts, int* ends,
    char locs[][MAX_NPC_NAME], int* count)
{
    char name[MAX_NPC_NAME];
    char loc[MAX_NPC_NAME];
    int s, e;

    if (*count >= MAX_RECORDS) {
        return;
    }

    sscanf(token, "%63[^;];%d;%d;%63[^\n]", name, &s, &e, loc);

    snprintf(names[*count], MAX_NPC_NAME, "%s", name);
    starts[*count] = s;
    ends[*count] = e;
    snprintf(locs[*count], MAX_NPC_NAME, "%s", loc);
    (*count)++;
}

int load_schedule(char names[][MAX_NPC_NAME], int* starts, int* ends,
    char locs[][MAX_NPC_NAME], int* count)
{
    FILE* f;
    char token[1024];
    int tlen = 0;
    int c;

    *count = 0;
    f = fopen("input.txt", "r");
    if (!f) {
        return 0;
    }

    while ((c = fgetc(f)) != EOF) {
        if (c == ',' || c == '\n' || c == '\r') {
            if (tlen > 0) {
                token[tlen] = '\0';
                parse_entry(token, names, starts, ends, locs, count);
                tlen = 0;
            }
        }
        else {
            token[tlen++] = (char)c;
        }
    }
    if (tlen > 0) {
        token[tlen] = '\0';
        parse_entry(token, names, starts, ends, locs, count);
    }

    fclose(f);
    return 1;
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
        printf("Ошибка: не удалось открыть input.txt.\n");
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

void handle_choice(int choice,
    const char* farmer_name,
    int* day, int* hour,
    int* counts,
    char item_names[][MAX_NAME_LEN],
    int* ids)
{
    switch (choice) {
    case 1:
        print_time(*day, *hour);
        break;
    case 2:
        work(day, hour);
        break;
    case 3:
        show_inventory(counts, item_names, ids);
        break;
    case 4:
        add_item(counts, ids);
        break;
    case 5:
        drop_item(counts, ids);
        break;
    case 6:
        show_most_common(ids, item_names);
        break;
    case 7:
        find_item(counts, item_names, ids);
        break;
    case 8:
        write_diary(farmer_name, *day, *hour, counts, item_names, ids);
        break;
    case 9:
        decode_variant(*hour);
        break;
    case 0:
        break;
    default:
        printf("Есть лишь 9 команд!\n");
        break;
    }
}

int main(void)
{
    char farmer_name[MAX_FARMER_NAME];
    char item_names[MAX_ITEMS][MAX_NAME_LEN];
    int current_day = 1;
    int current_hour = 8;
    int inventory[INV_SIZE] = { 0 }; // Сколько находится?
    int inventoryx[INV_SIZE] = { 0 }; // Что именно находится?
    int x = 7;

    if (!load_items(item_names)) {
        return 1;
    }

    {
        int name_ok = 0;
        while (!name_ok) {
            printf("Введите имя фермера (до 32 символов): ");
            if (fgets(farmer_name, sizeof(farmer_name), stdin) == NULL) {
                printf("\nОшибка: не удалось прочитать имя (конец ввода). Программа будет остановлена.\n");
                return 1;
            }

            {
                size_t len = strlen(farmer_name);
                int got_newline = 0;
                int too_long = 0;

                while (len > 0 && (farmer_name[len - 1] == '\n' || farmer_name[len - 1] == '\r')) {
                    farmer_name[--len] = '\0';
                    got_newline = 1;
                }

                if (!got_newline) {
                    int ch = getchar();
                    if (ch != '\n' && ch != EOF) {
                        too_long = 1;
                        while (ch != '\n' && ch != EOF) {
                            ch = getchar();
                        }
                    }
                }

                if (too_long) {
                    printf("Ошибка: имя слишком длинное (максимум 32 символа). Попробуйте снова.\n");
                    continue;
                }

                if (farmer_name[0] == '\0') {
                    printf("Ошибка: имя не может быть пустым. Попробуйте снова.\n");
                    continue;
                }

                name_ok = 1;
            }
        }
    }
    printf("Привет, %s!\n", farmer_name);

    print_menu();

    while (x != 0) {
        printf("\n=== %s | День %d | %d:00 ===\n", farmer_name, current_day, current_hour);
        printf("Введите номер действия:\n");
        scanf("%d", &x);

        handle_choice(x, farmer_name, &current_day, &current_hour,
            inventory, item_names, inventoryx);
    }

    printf("До свидания, %s!\n", farmer_name);
    return 0;
}