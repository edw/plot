#include <stdlib.h>

#include "data.h"
#include "io.h"
#include "text.h"

double *getrow(dataframe *df, int row) {
  return &df->data[df->ncols * row];
}

double *vecalloc(int ncols) {
  return malloc(sizeof(double) * ncols);
}

void free_dataframe(dataframe *df) {
  if (df != NULL) free(df->data);
  free(df);
}

dataframe *parse_file(mapped_file *mf, int ncols) {
  char *line;
  size_t len;
  int rows_cap = 1;
  dataframe *df = malloc(sizeof(dataframe));
  double *r;

  df->nrows = 0;
  df->ncols = ncols;
  df->data = malloc(sizeof(double) * ncols * rows_cap);

  char **cols = malloc(sizeof(char *) * ncols);

  while (1) {
    mf_readline(mf, &line, &len);
    if(line == NULL) break;

    split(line, len, cols, ncols);

    if (len == 0) continue;

    if (df->nrows >= rows_cap) {
      rows_cap <<= 1;
      df->data = realloc(df->data, sizeof(double) * ncols * rows_cap);
      if (df->data == NULL) {
        fprintf(stderr, "Out of memory during realloc().");
        exit (EXIT_FAILURE);
      }
    }

    r = &df->data[ncols * df->nrows];

    for (int i=0; i<ncols; i++)
      r[i] = atof(cols[i]);

    df->nrows++;
  }

  free(cols);
  return df;
};

void fputrow(FILE *fp, const char *label, dataframe *df, int row) {
  fputvec(fp, label, getrow(df, row), df->ncols);
}

void fputvec(FILE *fp, const char *label, double *vec, int nels) {
  fprintf(fp, "%s: (", label);
  for (int i=0; i<nels; i++) {
    if (i+1 == nels)
      fprintf(fp, "%f)\n", vec[i]);
    else
      fprintf(fp, "%f, ", vec[i]);
  }
}
