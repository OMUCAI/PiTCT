#ifndef _CL_TCT_H
#define _CL_TCT_H

#include "des_data.h"
#include "des_proc.h"
#include "setup.h"

#define EXT_DES ".DES"
#define EXT_DAT ".DAT"
#define EXT_TXT ".TXT"
#define EXT_PDS ".PDS"
#define EXT_PDT ".PDT"
#define EXT_PSS ".PSS"
#define EXT_PST ".PST"
#define EXT_ADS ".ADS"
#define EXT_GIF ".GIF"
#define EXT_SPL ".SPL"
#define EXT_CSV ".CSV"

#define LOC_TEMP_NAME "###"
#define OBS_TEMP_NAME "$$$"

int create_program(const char *filename);
int selfloop_program(const char *filename);
int trim_program(const char *filename);
int printdes_program(const char *filename);
int sync_program(const char *filename);
int meet_program(const char *filename);
int supcon_program(const char *filename);
int allevents_program(const char *filename);
int mutex_program(const char *filename);
int complement_program(const char *filename);
int nonconflict_program(const char *filename);
int condat_program(const char *filename);
int supreduce_program(const char *filename);
int isomorph_program(const char *filename);

#endif
