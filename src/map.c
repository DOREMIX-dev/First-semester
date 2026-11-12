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

typedef struct {
    char** cells;
    int width;
    int height;
    int hero_x;
    int hero_y;
    int depth;
} Dungeon;

void free_map(Dungeon* dungeon)
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

int allocate_map(Dungeon* dungeon, int width, int height)
{
    int y;
    if (dungeon->cells || width < MAP_MIN_SIZE || width > MAP_MAX_SIZE ||
        height < MAP_MIN_SIZE || height > MAP_MAX_SIZE) return 0;
    dungeon->width = width;
    dungeon->height = height;
    dungeon->cells = calloc((size_t)height, sizeof(*dungeon->cells));
    if (!dungeon->cells) {
        free_map(dungeon);
        return 0;
    }
    for (y = 0; y < height; y++) {
        dungeon->cells[y] = malloc((size_t)width * sizeof(*dungeon->cells[y]));
        if (!dungeon->cells[y]) {
            free_map(dungeon);
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

int generate_map(Dungeon* dungeon)
{
    int x, y, down_x, down_y, up_x;
    int candidates = 0;
    int obstacles;
    int width = random_between(MAP_MIN_SIZE, MAP_MAX_SIZE);
    int height = random_between(MAP_MIN_SIZE, MAP_MAX_SIZE);
    if (!allocate_map(dungeon, width, height)) return 0;
    dungeon->hero_x = random_between(1, width - 2);
    dungeon->hero_y = random_between(1, height - 2);
    up_x = dungeon->hero_x == width - 2 ? dungeon->hero_x - 1 : dungeon->hero_x + 1;
    do {
        down_x = random_between(1, width - 2);
        down_y = random_between(1, height - 2);
    } while ((down_x == dungeon->hero_x && down_y == dungeon->hero_y) ||
        (down_x == up_x && down_y == dungeon->hero_y));

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            dungeon->cells[y][x] = (x == 0 || y == 0 || x == width - 1 || y == height - 1)
                ? TILE_WALL : TILE_FLOOR;
        }
    }
    dungeon->cells[dungeon->hero_y][up_x] = TILE_UP;
    dungeon->cells[down_y][down_x] = TILE_DOWN;
    for (y = 1; y < height - 1; y++) {
        for (x = 1; x < width - 1; x++) {
            if (dungeon->cells[y][x] == TILE_FLOOR &&
                !is_stair_path(x, y, dungeon->hero_x, dungeon->hero_y, down_x, down_y)) candidates++;
        }
    }
    obstacles = ((width - 2) * (height - 2) * OBSTACLE_PERCENT + PERCENT_TOTAL - 1) / PERCENT_TOTAL;
    /* Выбор без повторений: число препятствий не зависит от удачи в цикле. */
    for (y = 1; y < height - 1; y++) {
        for (x = 1; x < width - 1; x++) {
            if (dungeon->cells[y][x] != TILE_FLOOR ||
                is_stair_path(x, y, dungeon->hero_x, dungeon->hero_y, down_x, down_y)) continue;
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
char visible_tile(const Dungeon* dungeon, int x, int y)
{
    int dx = x - dungeon->hero_x;
    int dy = y - dungeon->hero_y;
    if (dx * dx + dy * dy > VISIBILITY_RADIUS * VISIBILITY_RADIUS) return TILE_UNKNOWN;
    if (dx == 0 && dy == 0) return TILE_HERO;
    return dungeon->cells[y][x];
}

void print_map(const Dungeon* dungeon)
{
    int x, y;
    printf("\nГлубина: %d | Размер: %dx%d\n", dungeon->depth, dungeon->width, dungeon->height);
    for (y = 0; y < dungeon->height; y++) {
        for (x = 0; x < dungeon->width; x++) {
            putchar(visible_tile(dungeon, x, y));
        }
        putchar('\n');
    }
}

int can_walk(const Dungeon* dungeon, int x, int y)
{
    char tile;
    if (x < 0 || y < 0 || x >= dungeon->width || y >= dungeon->height) return 0;
    tile = dungeon->cells[y][x];
    return tile == TILE_FLOOR || tile == TILE_UP || tile == TILE_DOWN;
}

int next_floor(Dungeon* dungeon)
{
    if (dungeon->depth == INT_MAX) {
        printf("Достигнута максимальная глубина. Приключение завершено.\n");
        return 0;
    }
    free_map(dungeon);
    if (!generate_map(dungeon)) {
        printf("Не удалось выделить память для нового этажа.\n");
        return 0;
    }
    dungeon->depth++;
    printf("Вы спустились на следующий этаж.\n");
    return 1;
}

/* -1: приключение нужно завершить, 0: препятствие, 1: успешный ход. */
int move_hero(Dungeon* dungeon, int dx, int dy)
{
    int new_x = dungeon->hero_x + dx;
    int new_y = dungeon->hero_y + dy;
    if (!can_walk(dungeon, new_x, new_y)) {
        printf("Проход закрыт стеной или препятствием.\n");
        return 0;
    }
    dungeon->hero_x = new_x;
    dungeon->hero_y = new_y;
    if (dungeon->cells[new_y][new_x] == TILE_DOWN && !next_floor(dungeon)) return -1;
    return 1;
}

int read_direction(char* direction)
{
    char text[INPUT_SIZE];
    for (;;) {
        if (!read_text("W/A/S/D — движение, Q — домой с лестницы U: ", text, sizeof(text))) return 0;
        if (text[1] == '\0') {
            char command = (char)toupper((unsigned char)text[0]);
            if (command == 'W' || command == 'A' || command == 'S' ||
                command == 'D' || command == 'Q') {
                *direction = command;
                return 1;
            }
        }
        printf("Введите одну команду W, A, S, D или Q и нажмите Enter.\n");
    }
}

void adventure(void)
{
    Dungeon dungeon = {0};
    char direction;
    int playing = 1;
    dungeon.depth = 1;
    if (!generate_map(&dungeon)) {
        printf("Не удалось выделить память для карты.\n");
        return;
    }
    printf("Подземелье: @ — герой, # — стена, T/R — препятствия, U — домой, D — вниз.\n");
    printf("Туман войны: радиус обзора — %d клетки, ? — неизвестная область.\n", VISIBILITY_RADIUS);
    while (playing) {
        int dx = 0, dy = 0;
        print_map(&dungeon);
        if (!read_direction(&direction)) break;
        if (direction == 'Q') {
            if (dungeon.cells[dungeon.hero_y][dungeon.hero_x] == TILE_UP) {
                printf("Вы вернулись домой.\n");
                playing = 0;
            }
            else printf("Вернуться домой можно только с лестницы U.\n");
            continue;
        }
        switch (direction) {
        case 'W': dy = -1; break;
        case 'A': dx = -1; break;
        case 'S': dy = 1; break;
        case 'D': dx = 1; break;
        }
        if (move_hero(&dungeon, dx, dy) < 0) playing = 0;
    }
    free_map(&dungeon);
}
