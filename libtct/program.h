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
#define EXT_RST ".RST"

#define LOC_TEMP_NAME "###"
#define OBS_TEMP_NAME "$$$"

#define RESULT_OK 0          // no problem
#define ERR_FILE_OPEN -1     // fopen error
#define ERR_MEM -2           // out of memory
#define ERR_PRM_FILE -3      // prm content error
#define ERR_SUPREDUCE -4     // supreduce internal error
#define ERR_UNKNOWN -5

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
int printdat_program(const char *filename);
int getdes_parameter_program(const char *filename);
int supconrobs_program(const char *filename);
int project_program(const char *filename);
int localize_program(const char *filename);
int minstate_program(const char *filename);
int force_program(const char *filename);
int convert_program(const char *filename);
int supnorm_program(const char *filename);
int supscop_program(const char *filename);
int canQC_program(const char *filename);
int obs_program(const char *filename);
int natobs_program(const char *filename);
int supobs_program(const char *filename);
int bfs_recode_program(const char *filename);
int ext_suprobs_program(const char* filename);
int export_ext_des_program(const char* filename);
int eh_sync_program(const char* filename);

#endif
