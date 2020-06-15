#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "bfb.h"
#include "data.h"
#include "io.h"

char *progname;

inline static double scale(
  double x,
  double in_min, double in_max,
  double out_min, double out_max)
{
  double in_range = in_max - in_min, out_range = out_max - out_min;
  return out_min + (x - in_min) * out_range / in_range;
}

void histo_file(
  mapped_file *mf, int nbins, bfb *fb, char **caption,
  int left, int top, int width, int height)
{
  dataframe *df = parse_file(mf, 1);
  double min = DBL_MAX, max = DBL_MIN;

  for (int i=0; i<df->nrows; i++) {
    min = min < df->data[i] ? min : df->data[i];
    max = max > df->data[i] ? max : df->data[i];
  }

  double bin_w = (max - min)/(nbins + 1.0);
  double *bins = vecalloc(nbins);
  for (int i=0; i<nbins; i++) bins[i] = 0.0;
  for (int i=0; i<df->nrows; i++)
    bins[(int)round((df->data[i]-min)/bin_w)] += 1.0;

  double binmax = DBL_MIN;
  for(int i=0; i<nbins; i++) binmax = binmax > bins[i] ? binmax : bins[i];

  for (int i=0; i<nbins; i++) {
    int x1 = left + (int)round(i*width/nbins);
    int x2 = x1 + (int)round(width/nbins) + ((i == nbins-1) ? 0 : -1);
    int y1 = (int)round(scale(bins[i], 0, binmax, top + height - 1, top));
    int y2 = top + height;
    for (int x = x1; x<x2; x++)
      for (int y = y1; y<y2; y++)
        bfb_plot(fb, x, y, 1);
  }

  asprintf(caption, "n bins: %d\nwidth: %f\nmin: %f\nmax: %f\n",
           nbins, bin_w, min, max);

  free_dataframe(df);
}

void xyplot_file(
  mapped_file *mf, bfb *fb, char **caption,
  int left, int top, int width, int height)
{
  const int ncols = 2;
  double *min = vecalloc(ncols), *max = vecalloc(ncols);
  dataframe *df = parse_file(mf, ncols);
  double pxmin = left, pymin = top,
    pxmax = left + width - 1 ,
    pymax = top + height - 1;

  for (int i=0; i<ncols; i++) {
    min[i] = DBL_MAX;
    max[i] = DBL_MIN;
  }

  for (int i=0; i < df->nrows; i++) {
    double *r = getrow(df, i);

    for (int j=0; j<ncols; j++) {
      min[j] = min[j] < r[j] ? min[j] : r[j];
      max[j] = max[j] > r[j] ? max[j] : r[j];
    }
  }

  for (int i=0; i<df->nrows; i++) {
    double *r = getrow(df, i);
    double x = round(scale(r[0], min[0], max[0], pxmin ,pxmax));
    double y = round(scale(r[1], min[1], max[1], pymax, pymin));
    bfb_plot(fb, (int)x, (int)y, 1);
  }

  asprintf(caption, "x axis: [%f, %f]\ny axis: [%f, %f]\nn: %d\n",
           min[0], max[0], min[1], max[1], df->nrows);

  free(min); free(max);
  free_dataframe(df);
}

void process_file(mapped_file *mf, int ncols) {
  double
    *min = vecalloc(ncols),
    *max = vecalloc(ncols),
    *sum = vecalloc(ncols),
    *range = vecalloc(ncols);
  dataframe *df = parse_file(mf, ncols);

  for (int i=0; i<ncols; i++) {
    min[i] = DBL_MAX;
    max[i] = DBL_MIN;
    sum[i] = 0.0;
  }

  for (int i=0; i < df->nrows; i++) {
    double *r = getrow(df, i);

    for (int j=0; j<ncols; j++) {
      min[j] = min[j] < r[j] ? min[j] : r[j];
      max[j] = max[j] > r[j] ? max[j] : r[j];
      sum[j] += r[j];
    }
    fputrow(stdout, "datum", df, i);
  }

  for (int i=0; i<ncols; i++)
    range[i] = max[i] - min[i];

  fputvec(stdout, "min  ", min, ncols);
  fputvec(stdout, "max  ", max, ncols);
  fputvec(stdout, "sum  ", sum, ncols);
  fputvec(stdout, "range", range, ncols);

  free(min); free(max); free(sum); free(range);
  free_dataframe(df);
}

enum plot_type {
  SCATTER2D,
  HISTO
};

int main(int argc, char **argv) {
  int plot_w = 150, plot_h = 60, nbins = 11;
  enum plot_type plot_type = SCATTER2D;
  progname = *argv;
  argv++;
  while (argc > 1) {
    if (strcmp("--width", *argv) == 0) {
      argv++;
      plot_w = atoi(*argv++) * 2;
      argc -= 2;
    } else if (strcmp("--height", *argv) == 0) {
      argv++;
      plot_h = atoi(*argv++) * 4;
      argc -= 2;
    } else if (strcmp("--bins", *argv) == 0) {
      argv++;
      nbins= atoi(*argv++);
      argc -= 2;
    } else if (strcmp("--type", *argv) == 0) {
      argv++;
      if (strcmp("scatter", *argv) == 0) {
        plot_type = SCATTER2D;
      } else if (strcmp("histogram", *argv) == 0) {
        plot_type = HISTO;
      } else {
        fprintf(stderr, "%s: Unknown plot type: %s\n", progname, *argv);
        exit(EXIT_FAILURE);
      }
      argv++;
      argc -= 2;
    } else {
      break;
    }
  }

  mapped_file mf;
  mf.fp = copy_to_tmpfile(stdin, &mf.length);
  mf_map(&mf);
  bfb plot;
  char *caption;

  init_bfb(&plot, plot_w, plot_h, 0);

  switch (plot_type) {
  case SCATTER2D:
    xyplot_file(&mf, &plot, &caption, 3, 2, plot_w-5, plot_h-5);
    break;

  case HISTO:
    histo_file(&mf, nbins, &plot, &caption, 3, 2, plot_w-5, plot_h-5);
    break;
  }

  for (int i=0; i<plot_h; i++) {
    bfb_plot(&plot, 0, (plot_h-1)-i, 1);
    bfb_plot(&plot, 1, (plot_h-1)-i, 1);
    bfb_plot(&plot, (plot_w-1), i, 1);
  }

  for (int i=0; i<plot_w; i++) {
    bfb_plot(&plot, i, (plot_h-1)-0, 1);
    bfb_plot(&plot, i, (plot_h-1)-1, 1);
    bfb_plot(&plot, i, 0, 1);
  }

  bfb_fput(&plot, stdout);
  fprintf(stdout, "%s", caption);
  mf_unmap(&mf, 1);

  return EXIT_SUCCESS;
}
