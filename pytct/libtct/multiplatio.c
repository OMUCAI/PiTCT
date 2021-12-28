#include "multiplatio.h"
#include "setup.h"

#if defined(PLATFORM_WIN)
#include <io.h>
#elif defined(PLATFOM_LINUX) || defined(PLATFORM_MAC)
#include 
#endif


int mlt_access(const char *path, int mode) {

}
