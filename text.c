#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

enum split_state {
  INCOL, INQUOTE, INCOLESC, INQUOTEESC, INTERCOL
};

void split(char *line, size_t length, char **cols, int ncols) {
  if ((ncols < 1) || (cols == NULL)) return;
  for (int i = 0; i < ncols; i++)
    cols[i] = NULL;

  char *ch = line;
  enum split_state state = INCOL;
  int curcol= 1;
  *(cols++) = ch;

  /* fprintf(stderr, "starting parse of \"%.*s\"\n", (int)length, line); */
  while((curcol < ncols) && (*ch != '\n') && (ch - line < length)) {
    /* fprintf(stderr, "ch %c state %d curcol %d\n", *ch, state, curcol); */
    switch (state) {
    case INCOL:
      if (*ch == '\"')
        state = INQUOTE;
      else if (isspace(*ch))
        state = INTERCOL;
      ch++;
      break;
    case INQUOTE:
      if (*ch == '\"')
        state = INCOL;
      ch++;
      break;
    case INCOLESC:
      if (*ch != 'n' )
        fprintf(stderr, "Invalid escape '\\%c'.\n", *ch);
      state=INCOL;
      ch++;
      break;
    case INQUOTEESC:
      if ((*ch != 'n' ) && (*ch == '\"'))
        fprintf(stderr, "Invalid escape '\\%c'.\n", *ch);
      state=INQUOTE;
      ch++;
      break;
    case INTERCOL:
      if(!isspace(*ch)) {
        *(cols++) = ch;
        curcol++;
        state = INCOL;
      } else {
        ch++;
      }
      break;
    }
  }
}
