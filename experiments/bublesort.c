#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

void sort (int* mlist, int size)
{
  for (int i = 0; i < (size - 1); ++i)
    for (int j = 0; j < size - 1 - i; ++j) {
      if (mlist[j] > mlist[j+1]) {
        int tmp = mlist[j];
        mlist[j] = mlist[j+1];
        mlist[j+1] = tmp;
      }
    }
  return;
}

int main(int argc, char* argv[])
{
  /* unsigned char read_list[5] = {2, 5, 3, 6, 1}; */
  /* unsigned char read_list[6] = {7, 2, 5, 3, 6, 1}; */
  /* unsigned char read_list[5] = {2, 5, 7, 6, 1}; */
  /* unsigned char read_list[5] = {2, 5, 8, 6, 1}; */
  /* unsigned char read_list[5] = {7, 5, 8, 6, 1}; */
  /* unsigned char read_list[6] = {7, 5, 8, 6, 1, 10}; */
  /* unsigned char read_list[7] = {7, 5, 8, 6, 1, 10, 2}; */

  int size_of_list = atoi(argv[1]);

  int* list = (int*)malloc(size_of_list * sizeof(int));
  for (int i = 0; i < size_of_list; ++i) {
    list[i] = atoi(argv[2 + i]);
  }

  sort(list, size_of_list);

  for (int i = 0; i < size_of_list; ++i) {
    printf("%d", list[i]);
  }
  printf("\n");

  /* int fd = open("data.dat", O_RDONLY); */
  /* if (fd >= 0) { */
  /*   unsigned char read_list[5] = {2, 5, 3, 6, 1};  */
  /*   read(fd, read_list, 5); */
  /*   sort(read_list, 5); */
  /*   close(fd); */
  /* } */
  return 0;
}
