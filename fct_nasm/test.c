#include <inttypes.h>
#include <stdio.h>
void my_putchar(char);
char my_getchar();

int main(){
    my_putchar('p');
    my_putchar('\n');
    my_putchar(my_getchar());
    my_putchar('\n');
}