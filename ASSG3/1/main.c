#include <stdio.h>
#include "utils.h"
int main() {
int a, b;
printf("Enter two integers: ");
scanf("%d %d", &a, &b);
// Use functions from utils.c
print_sum(a, b);
print_product(a, b);
return 0;
}
