#ifndef __IO_H__
#define __IO_H__
#include <stdio.h>

void get_window_size(int *width, int *height);

typedef struct mapped_file {
  FILE *fp;
  void *data;
  size_t length;
  size_t cursor;
} mapped_file;

void mf_map(mapped_file *mf);
void mf_unmap(mapped_file *mf, int close_file);
void mf_readline(mapped_file *mf, char **line, size_t *length);

FILE *copy_to_tmpfile(FILE *fd, size_t *totalbytes);

#endif
