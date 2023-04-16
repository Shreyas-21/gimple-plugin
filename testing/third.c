#include <stdio.h>
#include <string.h>

void abc(int y, char *a) {
  y = 5;
  /* strcpy(a, "another string"); */
  char b[100];
  strcpy(b, a);
  printf("%d %s", y, a);
}

int main() {
  char a[100] = "my string";
  abc(0, a);
  return 0;
}
