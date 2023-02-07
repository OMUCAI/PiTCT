#ifndef _DES_DATA_H
#define _DES_DATA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Common definitions */
#ifndef __cplusplus
#define false 0
#define true 1
#endif

/* Some constants */

#define MAX_STATES 2000000

#define MAX_TRANSITIONS 999
#define MAX_VOCAL_OUTPUT 999

#define MAX_DESS 20
#define MAX_FILENAME 256
#define MAX_LONG_FILENAME 1024

#define EEE 1000

#define NUM_CMD 30
#define MAX_CMDLINE 1024
#define MAX_OPERA 1024

/* Define Identifiers used to generate the standard command line*/
#define MARK_PROC1 "{" // start of a procedure
#define CMARK_PROC1                                                            \
  '{' // the prefiex 'C' is used to identify the character from the string
#define MARK_PROC2 "}" // end of a procedure
#define CMARK_PROC2 '}'
#define MARK_CMD ";" // closing mark of a command line
#define CMARK_CMD ';'
#define MARK_OPER1 ":" // start of an operation
#define CMARK_OPER1 ':'
#define MARK_OPER2 "," // separator in an operation
#define CMARK_OPER2 ','
#define MARK_OPER3 "*" // separator in an operation
#define CMARK_OPER3 '*'

#define ID_LEV1 ":"
#define ID_LEV2 "\\"
#define ID_LEV3 "/"
#define ID_LEV4 "*"

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

/* Some macros */
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

// C Language Max Macro.
// max macro cannot use linux gcc compiler. 
#define CMAX(a, b) (((a) > (b)) ? (a) : (b))

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

typedef int64_t INT_P;
typedef int64_t INT_S; /* State type        */

typedef int INT_OS;
typedef unsigned char INT_B;
typedef unsigned short INT_T; /* Event label type  */
typedef short INT_V;          /* Vocal output type */
typedef unsigned int DWORD;

typedef char filename1[MAX_FILENAME];

extern INT_OS mem_result;

/* Try to pack in as much as possible */
typedef struct tran_node {
  INT_T data1; /* data1 = event label, usually    */
  INT_S data2; /* data2 = entrance state, usually */
} tran_node;
/* Try to pack in as much as possible */
typedef struct recode_node {
  INT_S recode;
  INT_B reached : 1;
} recode_node;

/* State map */
typedef struct state_map {
  INT_B marked : 1;
  INT_S state;
  INT_S numelts;
  INT_S *next;
} state_map;

/* The fields are aligned on those important fields */
typedef struct state_node {
  INT_B marked : 1;              /* 1-bit  */
  INT_B reached : 1;             /* 1-bit  */
  INT_B coreach : 1;             /* 1-bit  */
  INT_V vocal : 13; /* 13-bit */ /* 0..999 */
  INT_T numelts;                 /* 16-bit */
  tran_node *next;               /* 32-bit */
} state_node;                    /* 8 bytes total */

/* A structure to hold a list of states */
typedef struct part_node {
  INT_S numelts;
  INT_S *next;
} part_node;

// A structure to hold a list of events
typedef struct t_part_node {
  INT_T numelts;
  INT_T *next;
} t_part_node;

// A structure to hold a list of flags
typedef struct bt_part_node {
  INT_T numelts;
  INT_B *next;
} bt_part_node;

/* State pair */
typedef struct state_pair {
  INT_S data1, data2;
} state_pair;

/* Triple */
typedef struct triple {
  INT_S i;
  INT_T e;
  INT_S j;
} triple;

/* Quad set */
typedef struct quad__t {
  INT_S a;
  INT_T b;
  INT_S c;
  INT_V d;
} quad__t;

typedef struct check_mark {
  INT_B flag;
  INT_S index;
  INT_S minindex;
  INT_S numelts;
  INT_S *dynindex;
} check_mark;
typedef struct cc_check_table {
  INT_S numelts;
  check_mark *next;
} cc_check_table;

typedef short color_t;

#define green_color 0
#define red_color 1
#define amber_color 2

extern state_node *newdes(INT_S);
extern void resize_des(state_node **, INT_S, INT_S);
extern void freedes(INT_S, state_node **);
extern void free_map(INT_S, state_map **);
extern void free_part(INT_S, part_node **);
extern void free_t_part(INT_S, t_part_node **);
extern void free_bt_part(INT_S, bt_part_node **);
extern void free_cc_check_table(INT_S, cc_check_table **);
extern INT_OS filedes(char *, INT_S, INT_S, state_node *);
extern INT_OS file_extdes(char *, INT_S, INT_S, state_node *);
extern INT_B getdes(char *, INT_S *, INT_S *, state_node **);
extern void export_copy_des(INT_S *, state_node **, INT_S, state_node *);
extern void reverse_des(INT_S *, state_node **, INT_S, state_node *);

extern void addordlist1(INT_T, INT_S, tran_node **, INT_T, INT_B *);
extern INT_B inordlist1(INT_T, tran_node *, INT_T);
extern INT_B inordlist2(INT_T, INT_S, tran_node *, INT_T);
extern void addordlist(INT_T, INT_T **, INT_T, INT_B *);

extern void addstatelist(INT_S, INT_S **, INT_S, INT_B *);
extern INT_B instatelist(INT_S, INT_S *, INT_S);
extern void addstatemap(INT_S, INT_S, state_map **, INT_S, INT_B *);
extern void addstatepair(INT_S, INT_S, state_pair **, INT_S, INT_B *);
extern INT_B instatepair(INT_S, INT_S, state_pair **, INT_S);

extern void insertlist4(INT_S, INT_T, INT_S, INT_S *, state_node **);
extern void addtriple(INT_S, INT_T, INT_S, triple **, INT_S, INT_B *);
extern void delete_ordlist1(INT_T, INT_S, tran_node **, INT_T, INT_B *);
extern INT_B inlist(INT_T, INT_T *, INT_T);
extern void add_quad(INT_S, INT_T, INT_S, INT_V, quad__t **, INT_S, INT_B *);
extern INT_B check_des_sanity(state_node *, INT_S);
extern INT_S num_mark_states(state_node *, INT_S);
extern void mark_state_list(state_node *, INT_S, INT_S *);
extern INT_S num_vocal_output(state_node *, INT_S);

extern void recode_min(INT_S, state_node *, INT_S, state_node *, INT_S *);

// Print the contents into output.txt file
extern void zprints(char *);
extern void zprintn(INT_S);
extern void zprintsn(char *, INT_S);
extern void zprint_list(INT_S, INT_S *);
extern void zprint_list1(INT_T, INT_T *);
extern void zprint_par(INT_S, part_node *);
extern void zprint_tpar(INT_S, t_part_node *);
extern void zprint_btpar(INT_S, bt_part_node *);
extern void zprint_map(INT_S, state_map *);
extern void zprint_pair(INT_S, state_pair *);
extern void zprint_triple(INT_S, triple *);
extern void zprint_check_table(INT_S, cc_check_table *);
#ifdef __cplusplus
}
#endif

#endif
