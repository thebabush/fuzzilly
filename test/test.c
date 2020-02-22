#include <stdio.h>

int child(int arg) {
  return arg + 1;
}

int main() {
  int a = getc(stdin);
  int b = 0;

  if (a == 0) {
    puts("ITSAME!");
  } else {
    puts("ITSANOTTAME!");
  }

  for (int i=0; i<3; i++) {
    a *= 2;
  }

  if (a > 1) {
    if (a > 2) {
      a = 2;
    } else {
      b = 3;
    }
  } else {
    a = 1;
  }

  a = child(a);

  return a + b;
}

