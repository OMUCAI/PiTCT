#include "setup.h"
#include "des_data.h"

#ifdef __cplusplus
extern "C" {
#endif

INT_OS autotest = 0; /* False by default */

INT_OS ring_active = 1; /* All turned on */
INT_OS debug_mode = 0;
INT_OS timing_mode = 0;

INT_OS cmdline = 0; /* False by default */

INT_OS minflag = 1; /* set to use the new one for now */

char path[256];
char prefix[256];
char ctct_ini_file[256];
char info_file[256];

char *argv0;

#ifdef __cplusplus
}
#endif
