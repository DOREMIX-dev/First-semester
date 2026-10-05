#include <stdio.h>

int main()
{
    int current_day = 1;
    int current_hour = 8;
    int inventory[10] = { 0 }; // Сколько находится?
    char* inventoryv[10] = {
        "", "Дерево", "Камень", "Семена", "Железо",
        "Золото", "Серебро", "Алмазы", "Обсидиан", "Безумруды"
    }; // Библиотека существующих вещей.
    int inventoryx[10] = { 0 }; // Что именно находится?
    int hour;
    int x = 7;
    int sch;
    int per;

    while (x != 0) {
        printf("Введите номер действия:\n");
        scanf("%d", &x);

        if (x == 1) {
            printf("Текущее время: %d, %d:00 \n", current_day, current_hour);
        }
        else if (x == 2) {
            printf("Сколько хотите потратить часов на работу? ");
            scanf("%d", &hour);
            if (current_hour + hour < 24) {
                current_hour = current_hour + hour;
            }
            else {
                current_day = current_day + 1;
                current_hour = current_hour + hour - 24;
            }
        }
        else if (x == 3) {
            sch = 0;
            while (sch != 10) {
                if (inventoryv[inventoryx[sch]] == "") {
                    printf("Слот %d: [0]\n", sch);
                }
                else {
                    printf("Слот %d: [%d] (%s)\n", sch, inventory[sch], inventoryv[inventoryx[sch]]);
                }
                sch = sch + 1;
            }
        }
        else if (x == 4) {
            printf("Введите индекс слота: \n");
            scanf("%d", &per);
            printf("Введите ID предмета: \n");
            scanf("%d", &sch);
            if ((per >= 0) && (per <= 9) && (sch >= 0) && (sch <= 9)) {
                inventory[per] = inventory[per] + 1;
                inventoryx[per] = sch;
            }
            else {
                printf("Введен неверный индекс или ID");
            }
        }
        else if (x == 5) {
            printf("Введите индекс слота с которого хотите выбросить предметы\n");
            scanf("%d", &per);
            if (per >= 0 && per <= 9) {
                inventory[per] = 0;
                inventoryx[per] = 0;
            }
            else {
                printf("Введен неверный индекс слота!");
            }
        }
        else {
            printf("Команда пока не реализована\n");
        }
    }

    return 0;
}