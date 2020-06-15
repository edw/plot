#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "io.h"

void get_window_size(int *width, int *height) {
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  *height = w.ws_row;
  *width = w.ws_col;
}

void mf_map(mapped_file *mf) {
  mf->data = mmap(NULL, mf->length, PROT_READ, MAP_PRIVATE, fileno(mf->fp), 0);
  mf->cursor = 0;
}

void mf_unmap(mapped_file *mf, int close_file) {
  int err = munmap(mf->data, mf->length);
  if (close_file) fclose(mf->fp);
}

void mf_readline(mapped_file *mf, char **line, size_t *length) {
  *length = 0;

  if (mf->cursor == mf->length) {
    *line = NULL;
    return;
  }

  *line = &mf->data[mf->cursor];
  char *ch = *line;
  while (1) {
    if (mf->cursor >= mf->length) {
      break;
    }
    mf->cursor++;
    if (*ch == '\n') {
      break;
    }
    ch++;
    (*length)++;
  }
}

FILE *copy_to_tmpfile(FILE *fd, size_t *totalbytes) {
  const size_t bufsiz = 1024 * 1024;
  static char buf[bufsiz];
  FILE *t = tmpfile();
  if (totalbytes) *totalbytes = 0;
  if (t == NULL)
    return NULL;
  size_t nbytes;
  int final_char_was_nl = 0;
  while(!feof(fd)) {
    nbytes =  fread(buf, 1, bufsiz, fd);
    if ((nbytes != bufsiz) && ferror(fd)) {
      fclose(t);
      return NULL;
    }

    if (nbytes > 0) {
      if (buf[nbytes-1] == '\n')
        final_char_was_nl = 1;
      else
        final_char_was_nl = 0;
    }

    if (nbytes != fwrite(buf, 1, nbytes, t)) {
      fclose(t);
      return NULL;
    }

    if (totalbytes) *totalbytes += nbytes;
  }

  if (!final_char_was_nl) {
    buf[0] = '\n';
    if(fwrite(buf, 1, 1, t) != 1) {
      fclose(t);
      return NULL;
    }
  }
  fseek(t, 0, SEEK_SET);
  return t;
}
