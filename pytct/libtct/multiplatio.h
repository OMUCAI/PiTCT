#ifndef _MULTI_PLAT_IO_H
#define _MULTI_PLAT_IO_H


#ifdef __cplusplus
extern "C" {
#endif

int mlt_access(const char *path, int mode);
int mlt_mkdir(const char* path);
// int mlt_findfirst(const char *path, fileinfo);
// int mlt_findnext();
// int mlt_findclose();
int mlt_rmdir(const char *path);
#ifdef __cplusplus
}
#endif

#endif /* _MULTI_PLAT_IO_H */

