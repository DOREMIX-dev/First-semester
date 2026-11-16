#include "map.h"
#include "utils.h"
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>

#define MAP_MIN_SIZE 5
#define MAP_MAX_SIZE 30
#define OBSTACLE_PERCENT 20
#define PERCENT_TOTAL 100
#define TILE_WALL '#'
#define TILE_FLOOR '.'
#define TILE_TREE 'T'
#define TILE_ROCK 'R'
#define TILE_UP 'U'
#define TILE_DOWN 'D'
#define TILE_HERO '@'
#define TILE_UNKNOWN '?'
#define VISIBILITY_RADIUS 4

typedef struct Map {
    char** cells;
    int width;
    int height;
    int depth;
} Map;

static void clear_cells(Map* dungeon)
{
    int y;
    if (dungeon->cells) {
        for (y = 0; y < dungeon->height; y++) free(dungeon->cells[y]);
        free(dungeon->cells);
    }
    dungeon->cells = NULL;
    dungeon->width = 0;
    dungeon->height = 0;
}

static void destroy_map(Map* map)
{
    clear_cells(map);
    printf("Память структуры по адресу %p освобождена\n", (void*)map);
    free(map);
}

int allocate_map(Map* dungeon, int width, int height)
{
    int y;
    if (dungeon->cells || width < MAP_MIN_SIZE || width > MAP_MAX_SIZE ||
        height < MAP_MIN_SIZE || height > MAP_MAX_SIZE) return 0;
    dungeon->width = width;
    dungeon->height = height;
    dungeon->cells = calloc((size_t)height, sizeof(*dungeon->cells));
    if (!dungeon->cells) {
        clear_cells(dungeon);
        return 0;
    }
    for (y = 0; y < height; y++) {
        dungeon->cells[y] = malloc((size_t)width * sizeof(*dungeon->cells[y]));
        if (!dungeon->cells[y]) {
            clear_cells(dungeon);
            return 0;
        }
    }
    return 1;
}

int random_between(int minimum, int maximum)
{
    return minimum + rand() % (maximum - minimum + 1);
}

/* Проход от старта до D: сначала по горизонтали, затем по вертикали. */
int is_stair_path(int x, int y, int start_x, int start_y, int down_x, int down_y)
{
    int min_x = start_x < down_x ? start_x : down_x;
    int max_x = start_x > down_x ? start_x : down_x;
    int min_y = start_y < down_y ? start_y : down_y;
    int max_y = start_y > down_y ? start_y : down_y;
    return (y == start_y && x >= min_x && x <= max_x) ||
        (x == down_x && y >= min_y && y <= max_y);
}

int generate_map(Map* dungeon, Player* player)
{
    int x, y, down_x, down_y, up_x;
    int candidates = 0;
    int obstacles;
    int width = random_between(MAP_MIN_SIZE, MAP_MAX_SIZE);
    int height = random_between(MAP_MIN_SIZE, MAP_MAX_SIZE);
    if (!allocate_map(dungeon, width, height)) return 0;
    player->x = random_between(1, width - 2);
    player->y = random_between(1, height - 2);
    up_x = player->x == width - 2 ? player->x - 1 : player->x + 1;
    do {
        down_x = random_between(1, width - 2);
        down_y = random_between(1, height - 2);
    } while ((down_x == player->x && down_y == player->y) ||
        (down_x == up_x && down_y == player->y));

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            dungeon->cells[y][x] = (x == 0 || y == 0 || x == width - 1 || y == height - 1)
                ? TILE_WALL : TILE_FLOOR;
        }
    }
    dungeon->cells[player->y][up_x] = TILE_UP;
    dungeon->cells[down_y][down_x] = TILE_DOWN;
    for (y = 1; y < height - 1; y++) {
        for (x = 1; x < width - 1; x++) {
            if (dungeon->cells[y][x] == TILE_FLOOR &&
                !is_stair_path(x, y, player->x, player->y, down_x, down_y)) candidates++;
        }
    }
    obstacles = ((width - 2) * (height - 2) * OBSTACLE_PERCENT + PERCENT_TOTAL - 1) / PERCENT_TOTAL;
    /* Выбор без повторений: число препятствий не зависит от удачи в цикле. */
    for (y = 1; y < height - 1; y++) {
        for (x = 1; x < width - 1; x++) {
            if (dungeon->cells[y][x] != TILE_FLOOR ||
                is_stair_path(x, y, player->x, player->y, down_x, down_y)) continue;
            if (random_between(1, candidates) <= obstacles) {
                dungeon->cells[y][x] = random_between(0, 1) ? TILE_TREE : TILE_ROCK;
                obstacles--;
            }
            candidates--;
        }
    }
    return 1;
}

/* Туман скрывает только изображение, сами клетки карты не меняются. */
char visible_tile(const Map* dungeon, const Player* player, int x, int y)
{
    int dx = x - player->x;
    int dy = y - player->y;
    if (dx * dx + dy * dy > VISIBILITY_RADIUS * VISIBILITY_RADIUS) return TILE_UNKNOWN;
    if (dx == 0 && dy == 0) return TILE_HERO;
    return dungeon->cells[y][x];
}

void print_map(const Map* dungeon, const Player* player)
{
    int x, y;
    printf("\nГлубина: %d | Размер: %dx%d\n", dungeon->depth, dungeon->width, dungeon->height);
    for (y = 0; y < dungeon->height; y++) {
        for (x = 0; x < dungeon->width; x++) {
            putchar(visible_tile(dungeon, player, x, y));
        }
        putchar('\n');
    }
}

int can_walk(const Map* dungeon, int x, int y)
{
    char tile;
    if (x < 0 || y < 0 || x >= dungeon->width || y >= dungeon->height) return 0;
    tile = dungeon->cells[y][x];
    return tile == TILE_FLOOR || tile == TILE_UP || tile == TILE_DOWN;
}

#define STEP_COST 1
#define FLOOR_COST 5
#define FLOOR_REWARD 10
#define GOLD_LOSS_DIVISOR 2

static void spend_stamina(Player* player, int cost)
{
    player->stamina = player->stamina > cost ? player->stamina - cost : 0;
}

static int evacuate_if_needed(Player* player, const GameClock* clock)
{
    if (player->hp > 0 && player->stamina > 0 && clock->hour != BEDTIME_HOUR) return 0;
    int fee = player->gold / GOLD_LOSS_DIVISOR;
    player->gold -= fee;
    printf("Герой потерял сознание. Спасатели доставили его домой за %d золотых.\n", fee);
    return 1;
}

static int next_floor(Map** map, Player* player, GameClock* clock)
{
    if ((*map)->depth == INT_MAX) return 0;
    Map* next = calloc(1, sizeof(*next));
    if (!next) {
        puts("Не удалось выделить память для нового этажа.");
        return 0;
    }
    next->depth = (*map)->depth + 1;
    if (!generate_map(next, player) || !advance_clock(clock, 1)) {
        destroy_map(next);
        return 0;
    }

    destroy_map(*map);
    *map = next;
    spend_stamina(player, FLOOR_COST);
    int available = INT_MAX - player->gold;
    int reward = next->depth > available / FLOOR_REWARD ? available : FLOOR_REWARD * next->depth;
    player->gold += reward;
    printf("Вы спустились на следующий этаж и получили %d золотых.\n", reward);
    return 1;
}

typedef enum Direction {
    UP = 'W', LEFT = 'A', DOWN = 'S', RIGHT = 'D', QUIT = 'Q'
} Direction;

static int read_direction(Direction* direction)
{
    char text[INPUT_SIZE];
    for (;;) {
        if (!read_text("W/A/S/D — движение, Q — домой с лестницы U: ", text, sizeof(text))) return 0;
        if (text[1] == '\0') {
            int command = toupper((unsigned char)text[0]);
            if (command == UP || command == LEFT || command == DOWN || command == RIGHT || command == QUIT) {
                *direction = (Direction)command;
                return 1;
            }
        }
        puts("Введите одну команду W, A, S, D или Q и нажмите Enter.");
    }
}

static int move_hero(Map** map, Player* player, GameClock* clock, Direction direction)
{
    int x = player->x + (direction == RIGHT) - (direction == LEFT);
    int y = player->y + (direction == DOWN) - (direction == UP);
    if (!can_walk(*map, x, y)) {
        puts("Проход закрыт стеной или препятствием.");
        return 1;
    }

    player->x = x;
    player->y = y;
    spend_stamina(player, apply_step_effect(player, STEP_COST));
    if (evacuate_if_needed(player, clock)) return 0;
    if ((*map)->cells[y][x] != TILE_DOWN) return 1;
    return next_floor(map, player, clock) && !evacuate_if_needed(player, clock);
}

void adventure(Player* player, GameClock* clock)
{
    if (player->hp == 0 || player->stamina == 0 || clock->hour == BEDTIME_HOUR) {
        puts("Перед приключением нужно отдохнуть.");
        return;
    }
    Map* map = calloc(1, sizeof(*map));
    if (!map) {
        puts("Не удалось выделить память для карты.");
        return;
    }
    map->depth = 1;
    if (!generate_map(map, player)) {
        puts("Не удалось выделить память для клеток карты.");
        destroy_map(map);
        return;
    }
    puts("Подземелье: @ — герой, # — стена, T/R — препятствия, U — домой, D — вниз.");
    for (;;) {
        Direction direction;
        printf("HP: %d/%d | Выносливость: %d/%d | Золото: %d | Этаж: %d | Время: %02d:00\n",
            player->hp, player->max_hp, player->stamina, player->max_stamina,
            player->gold, map->depth, clock->hour);
        print_status(player);
        print_map(map, player);
        if (!read_direction(&direction)) break;
        if (direction == QUIT) {
            if (map->cells[player->y][player->x] == TILE_UP) {
                puts("Вы вернулись домой.");
                break;
            }
            puts("Вернуться домой можно только с лестницы U.");
        } else if (!move_hero(&map, player, clock, direction)) {
            break;
        }
    }
    destroy_map(map);
    player->x = player->y = 0;
}
