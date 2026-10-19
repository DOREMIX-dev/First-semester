#include <stdio.h>

#define INV_SIZE 10
#define ITEM_TYPES 10

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
void show_inventory(const int* counts, char** names, const int* ids)
{
    int i;

    for (i = 0; i < INV_SIZE; i++) {
        if (names[ids[i]][0] == '\0') {
            printf("Слот %d: [0]\n", i);
        }
        else {
            printf("Слот %d: [%d] (%s)\n",
                i, counts[i], names[ids[i]]);
        }
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

    if (slot >= 0 && slot < INV_SIZE && item >= 0 && item < ITEM_TYPES) {
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

// 6
void show_most_common(const int* ids)
{
    int item;
    int best_item = 0;
    int best_count = 0;

    for (item = 1; item < ITEM_TYPES; item++) {
        int c = count_item(ids, item);
        if (best_count < c) {
            best_count = c;
            best_item = item;
        }
    }

    if (best_count == 1 || (best_count >= 5 && best_count <= 10)) {
        printf("Самый частовстречаемый предмет: %d, он встречается %d раз.\n",
            best_item, best_count);
    }
    else {
        printf("Самый частовстречаемый предмет: %d, он встречается %d раза.\n",
            best_item, best_count);
    }
}

void handle_choice(int choice,
    int* day, int* hour,
    int* counts, char** names, int* ids)
{
    switch (choice) {
    case 1:
        print_time(*day, *hour);
        break;
    case 2:
        work(day, hour);
        break;
    case 3:
        show_inventory(counts, names, ids);
        break;
    case 4:
        add_item(counts, ids);
        break;
    case 5:
        drop_item(counts, ids);
        break;
    case 6:
        show_most_common(ids);
        break;
    case 0:
        break;
    default:
        printf("Есть лишь 6 команд!\n");
        break;
    }
}

int main(void)
{
    int current_day = 1;
    int current_hour = 8;
    int inventory[INV_SIZE] = { 0 }; // Сколько находится?
    int inventoryx[INV_SIZE] = { 0 }; // Что именно находится?
    char* inventoryv[ITEM_TYPES] = {
        "", "Дерево", "Камень", "Семена", "Железо",
        "Золото", "Серебро", "Алмазы", "Обсидиан", "Безумруды"
    }; // Библиотека существующих вещей.
    int x = 7;

    while (x != 0) {
        printf("Введите номер действия:\n");
        scanf("%d", &x);

        handle_choice(x, &current_day, &current_hour,
            inventory, inventoryv, inventoryx);
    }

    return 0;
}