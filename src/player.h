#ifndef PLAYER_H
#define PLAYER_H

#define INV_SIZE 10
#define MAX_ITEMS 10
#define MAX_NAME_LEN 32
#define MAX_FARMER_NAME 33


typedef enum StatusEffect {
    HEALTHY, SPRAIN, POISON, VIGOR
} StatusEffect;

typedef struct Buff {
    StatusEffect type;
    int steps_left;
} Buff;

typedef struct Player {
    char name[MAX_FARMER_NAME];
    int hp, max_hp;
    int stamina, max_stamina;
    int gold;
    int x, y;
    Buff buff;
    int counts[INV_SIZE];
    int ids[INV_SIZE];
} Player;

Player* create_player(Player* previous);
void destroy_player(Player* player);
void print_player(const Player* player);
void print_status(const Player* player);
int apply_step_effect(Player* player, int base_cost);
void show_inventory(const int* counts, const char item_names[][MAX_NAME_LEN], const int* ids);
void add_item(int* counts, int* ids);
void drop_item(int* counts, int* ids);
int count_item(const int* ids, int item_id);
const char* plural_raz(int n);
void show_most_common(const int* ids, const char item_names[][MAX_NAME_LEN]);
void find_item(const int* counts, const char item_names[][MAX_NAME_LEN], const int* ids);

#endif
