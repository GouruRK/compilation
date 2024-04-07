#include <inttypes.h>
#include <stdio.h>
void my_putchar(char);
char my_getchar();
int my_getint();
void my_putint(int);

int main(){
    my_putint(-565549);
    my_putchar('\n');
    my_putchar('p');
    my_putchar('\n');
    printf("test getint : %d\n", my_getint());
    my_putchar(my_getchar());
    my_putchar('\n');
}