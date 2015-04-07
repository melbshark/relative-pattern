#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

void sort (unsigned char* mlist, unsigned int size)
{
  for (unsigned int i = 0; i < (size - 1); ++i)
    for (unsigned int j = 0; j < size - 1 - i; ++j) {
      if (mlist[j] > mlist[j+1]) {
        unsigned char tmp = mlist[j];
        mlist[j] = mlist[j+1];
        mlist[j+1] = tmp;
      }
    }
  return;
}

int main()
{
  unsigned char read_list[5] = {2, 5, 3, 6, 1};
  /* unsigned char read_list[5] = {2, 5, 7, 6, 1}; */
  /* unsigned char read_list[5] = {2, 5, 8, 6, 1}; */
  /* unsigned char read_list[5] = {7, 5, 8, 6, 1}; */
  /* unsigned char read_list[6] = {7, 5, 8, 6, 1, 10}; */
  /* unsigned char read_list[7] = {7, 5, 8, 6, 1, 10, 2}; */
  sort(read_list, 5);

  for (unsigned int i = 0; i < 5; ++i) {
    printf("%d",read_list[i]);
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
