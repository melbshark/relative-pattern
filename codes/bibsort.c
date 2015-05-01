#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


/* recursive buble sort */
void rbsort(int* mlist, int size)
{
  if (size > 1) {
    for (int i = 0; i < size - 1; ++i) {
      if (mlist[i] > mlist[i+1]) {
        int tmp = mlist[i];
        mlist[i] = mlist[i + 1];
        mlist[i + 1] = tmp;
      }
    }
    rbsort(mlist, size - 1);
  }
  return;
}

/* normal buble sort */
void bsort(int* mlist, int size)
{
  if (size > 1) {
    for (int i = 0; i < (size - 1); ++i) {
      for (int j = 0; j < size - 1 - i; ++j) {
        if (mlist[j] > mlist[j+1]) {
          int tmp = mlist[j];
          mlist[j] = mlist[j+1];
          mlist[j+1] = tmp;
        }
      }
    }
  }
  return;
}

/* normal insertion sort */
void isort(int* mlist, int size)
{
  if (size > 1) {
    for (int i = 1; i < size; ++i) {
      int ival, j;
      ival = mlist[i];
      for (j = i; (j > 0) && (mlist[j - 1] > ival); --j) {
        mlist[j] = mlist[j - 1];
      }
      mlist[j] = ival;
    }
  }
  return;
}


int main(int argc, char* argv[])
{
  int lsize = argc - 2;

  if (lsize > 0) {
    int* list = (int*)malloc(lsize * sizeof(int));

    /* get input list */
    for (int i = 0; i < lsize; ++i) {
      char *err_pos;
      list[i] = strtol(argv[2 + i], &err_pos, 10);
      /* list[i] = atoi(argv[2 + i]); */
    }

    /* type of sorting */
    if (strcasecmp(argv[1], "buble") == 0) {
      bsort(list, lsize);
    }
    else if (strcasecmp(argv[1], "insertion") == 0) {
      isort(list, lsize);
    }
    else return 1;

    /* print sorted list */
    for (int i = 0; i < lsize; ++i) {
      printf("%d ", list[i]);
    }
    printf("\n");
  }

  return 0;
}



