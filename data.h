#ifndef __DATA_H_
#define __DATA_H_

#include "io.h"

typedef struct dataframe {
  int nrows;
  int ncols;
  double *data;
} dataframe;

double *getrow(dataframe *df, int row);
double *vecalloc(int ncols);
void free_dataframe(dataframe *df);

void fputrow(FILE *fp, const char *label, dataframe *df, int row);
void fputvec(FILE *fp, const char *label, double *vec, int nels);
dataframe *parse_file(mapped_file *mf, int ncols);

#endif
