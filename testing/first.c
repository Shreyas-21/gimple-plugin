#include <stdio.h>
#include <string.h>

void abc(int y, char a) {
  y = 5;
  printf("%d %c", y, a);
}

int main() {
  char a;
  int y = 1;
  abc(y, a);
  return 0;
}
