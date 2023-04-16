#include <stdio.h>

void abc(int y, char *a) {
  y = 5;
  /* a[1] = 'b'; */
  printf("%d %s", y, a);
}

int main() {
  char a[100] = "my string";
  abc(0, a);
  return 0;
}
