#include "player.h"
#include "utils.h"
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define START_HP 100
#define START_STAMINA 50

void destroy_player(Player* player)
{
    if (!player) return;
    printf("Память структуры по адресу %p освобождена\n", (void*)player);
    free(player);
}

Player* create_player(Player* previous)
{
    char name[MAX_FARMER_NAME];
    int confirm;
    Player* player;

    if (previous && (!read_int("Удалить прежнего героя? 1 — да, 0 — нет: ",
        0, 1, &confirm) || !confirm)) return previous;
    if (!read_text("Введите имя фермера: ", name, sizeof(name))) return previous;
    player = calloc(1, sizeof(*player));
    if (!player) {
        puts("Не удалось выделить память для персонажа.");
        return previous;
    }
    strcpy(player->name, name);
    player->hp = player->max_hp = START_HP;
    player->stamina = player->max_stamina = START_STAMINA;
    destroy_player(previous);
    return player;
}

void print_player(const Player* player)
{
    printf("%s | HP: %d/%d | Выносливость: %d/%d | Золото: %d\n",
        player->name, player->hp, player->max_hp,
        player->stamina, player->max_stamina, player->gold);
}

#define EFFECT_CHANCE_PERCENT 5
#define EFFECT_DURATION 10
#define PERCENT_TOTAL 100
#define SPRAIN_COST_MULTIPLIER 2
#define POISON_DAMAGE 2

static const char* status_name(StatusEffect effect)
{
    switch (effect) {
    case SPRAIN: return "Растяжение";
    case POISON: return "Отравление";
    case VIGOR: return "Бодрость";
    case HEALTHY: return "Здоров";
    }
    return "Здоров";
}

void print_status(const Player* player)
{
    printf("Состояние: %s", status_name(player->buff.type));
    if (player->buff.steps_left > 0) printf(" (%d ходов)", player->buff.steps_left);
    putchar('\n');
}

int apply_step_effect(Player* player, int base_cost)
{
    Buff* buff = &player->buff;
    int cost = base_cost;

    switch (buff->type) {
    case SPRAIN: cost *= SPRAIN_COST_MULTIPLIER; break;
    case POISON:
        player->hp = player->hp > POISON_DAMAGE ? player->hp - POISON_DAMAGE : 0;
        break;
    case VIGOR: cost = 0; break;
    case HEALTHY: break;
    }

    if (buff->steps_left > 0 && --buff->steps_left == 0) buff->type = HEALTHY;

    if (rand() % PERCENT_TOTAL < EFFECT_CHANCE_PERCENT) {
        buff->type = (StatusEffect)(SPRAIN + rand() % (VIGOR - SPRAIN + 1));
        buff->steps_left = EFFECT_DURATION;
        printf("%s. Новый эффект: %s (%d ходов).\n",
            buff->type == SPRAIN ? "Герой наступил на колючку" : "Герой съел странную ягоду",
            status_name(buff->type), buff->steps_left);
    }
    return cost;
}

void show_inventory(const int* counts, const char item_names[][MAX_NAME_LEN], const int* ids)
{
    int i;

    for (i = 0; i < INV_SIZE; i++) {
        printf("Слот %d: [%d] - %s\n", i, counts[i], item_names[ids[i]]);
    }
}

void add_item(int* counts, int* ids)
{
    int slot, item;

    if (!read_int("Введите индекс слота: ", 0, INV_SIZE - 1, &slot) ||
        !read_int("Введите ID предмета: ", 0, MAX_ITEMS - 1, &item)) return;
    if (counts[slot] == INT_MAX) {
        printf("Слот переполнен.\n");
        return;
    }

    if (slot >= 0 && slot < INV_SIZE && item >= 0 && item < MAX_ITEMS) {
        counts[slot] = counts[slot] + 1;
        ids[slot] = item;
    }
    else {
        printf("Введен неверный индекс или ID\n");
    }
}

void drop_item(int* counts, int* ids)
{
    int slot;

    if (!read_int("Введите индекс слота с которого хотите выбросить предметы: ",
        0, INV_SIZE - 1, &slot)) return;

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

void find_item(const int* counts, const char item_names[][MAX_NAME_LEN], const int* ids)
{
    char search[MAX_NAME_LEN];
    int id = -1;
    int i;
    int found_slot = 0;
    if (!read_text("Введите название предмета для поиска: ", search, sizeof(search))) return;

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
