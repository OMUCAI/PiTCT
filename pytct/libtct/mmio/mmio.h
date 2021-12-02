#ifndef _MMIO_H
#define _MMIO_H

#ifdef __cplusplus
extern "C" {
#endif

void CreateFileMapping(void);
void MapViewOfFile(void);
void UnmapViewOfFile(void);
void CloseHandle(void);
void WriteFile(void);
void CreateFile(void);
void _access(void);
void _mkdir(void);
void _findnext(void);
void _findfirst(void);
void _findclose(void);
void _rmdir(void);

#ifdef __cplusplus
}
#endif

#endif