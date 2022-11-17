#include "tct_io.h"
#include "setup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(PLATFORM_MAC) || defined(PLATFORM_LINUX)
#include <unistd.h>
#endif

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

/* Determine if the file exist on the disk */
INT_OS exist(char* s)
{
#if defined(PLATFORM_WIN)
   return (_access(s, 0) == 0);
#elif defined(PLATFORM_MAC) || defined(PLATFORM_LINUX)
    return (access(s, 0) == 0);
#endif
}