
#include <stdio.h>
int main() {
    int negX = ~4 + 1;
  // 

    int sign = (negX | 4) >> 31;
    printf ("%d\n",sign);
}