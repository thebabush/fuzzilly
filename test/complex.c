#include <stdio.h>

void useless(int i) {
  printf("IMMA SLIGHTLY LESSA USELESSA %d\n", i + 1);
}


int main() {
  int a;

  if (a == 0) {
    puts("ITSAME!");
  } else {
    puts("ITSANOTTAME!");
  }

  for (int i=0; i<3; i++) {
    useless(i);
  }

  return 0;
}

