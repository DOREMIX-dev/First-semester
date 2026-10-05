#include <stdio.h>

int main()
{

    int current_day = 1;
    int current_hour = 8;
    int x = 7;

    while (x != 0) {
        printf("Введите номер действия:\n");
        scanf("%d", &x);

        if (x == 1) {
            printf("Текущее время: %d, %d:00 \n", current_day, current_hour);
        }
        else {
            printf("Команда пока не доделана\n");
        }
    }

    return 0;
}