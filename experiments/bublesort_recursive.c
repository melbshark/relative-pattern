#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

void sort(int* mlist, int size)
{
  if (size > 1) {
    for (int i = 0; i < size - 1; ++i) {
      if (mlist[i] > mlist[i+1]) {
        int tmp = mlist[i];
        mlist[i] = mlist[i + 1];
        mlist[i + 1] = tmp;
      }
    }
    sort(mlist, size - 1);
  }

  return;
}

int main(int argc, char* argv[])
{
  int lsize = atoi(argv[1]);
  int* list = (int*)malloc(lsize * sizeof(int));
  for (int i = 0; i < lsize; ++i) {
    list[i] = atoi(argv[2 + i]);
  }

  sort(list, lsize);
  for (int i = 0; i < lsize; ++i) {
    printf("%d", list[i]);
  }
  printf("\n");

  return 0;
}



