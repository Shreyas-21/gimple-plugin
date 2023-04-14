#include <stdio.h>

int abc(int y, char a) {
  y = 5;
  printf("Hello, World %d %c\n", y, a);
  return 1;
}

int main() {
  int x = abc(0, 'a');
  return 0;
}
