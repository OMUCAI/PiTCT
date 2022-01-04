#include "multiplatio.h"
#include "setup.h"

#if defined(PLATFORM_WIN)
#include <io.h>
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MAC)
#include <unistd.h>
#include <sys/stat.h>
#endif


int mlt_access(const char *path, int mode) {
#if defined(PLATFORM_WIN)
    return _access(path, mode);
#elif defined(PLATFORM_MAC) || defined(PLATFORM_LINUX)
    return access(path, mode);
#endif
}

int mlt_mkdir(const char* path) {
#if defined(PLATFORM_WIN)
    return _mkdir(path);
#elif defined(PLATFORM_MAC) || defined(PLATFORM_LINUX)
    return mkdir(path, 0644);
#endif
}

int mlt_rmdir(const char *path) {
#if defined(PLATFORM_WIN)
    return _rmdir(path);
#elif defined(PLATFORM_MAC) || defined(PLATFORM_LINUX)
    return rmdir(path);
#endif
}
