#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc == 0) return 0; else argv++; argc--;
  int n = atoi(*argv);

  if (argc == 0) return 0; else argv++; argc--;
  if (strcmp(*argv, "sin") == 0) {
    if (n == 0) {
      fprintf(stdout, "0.0 0.0\n");
      return 0;
    }

    for (int i=0; i<n; i++) {
      double x = M_PI * 2 / (n - 1) * i;
      double y = sin(x);
      fprintf(stdout, "%f %f\n", x, y);
    }
    return 0;
  }
}
