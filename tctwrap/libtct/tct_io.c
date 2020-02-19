#include "tct_io.h"
#include "setup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Construct the full filename base on the name and the extension */
void make_filename_ext(char *longfilename, char *name, char *ext) {
  INT_OS len;
  strcpy(longfilename, prefix);

  /* Trim excessive blank characters from the name */
  if (name[0] == ' ') {
    len = (INT_OS)strlen(name);
    memmove(name, &(name[1]), len - 1);
    name[len - 1] = '\0';
  }

  if (name[0] == ' ') {
    len = (INT_OS)strlen(name);
    memmove(name, &(name[1]), len - 1);
    name[len - 1] = '\0';
  }

  strcat(longfilename, name);
  strcat(longfilename, ext);
}
