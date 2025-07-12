#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "des_proc.h"
#include "ext_des_proc.h"
#include "ext_proc.h"
#include "mymalloc.h"
#include "obs_check.h"
#include "tct_io.h"

// Operations added on part_node

void insertelem_part(INT_S row, INT_S col, INT_T elem, INT_S *s_par,
                     part_node **par) {
    INT_S i, j;

    if (row < 0 || col < 0)
        return;
    else if (row > *s_par - 1) {
        *par = (part_node *)REALLOC(*par, (row + 1) * sizeof(part_node));
        if (*par == NULL) {
            mem_result = 1;
            return;
        }
        for (i = *s_par; i < row + 1; i++) {
            (*par)[i].next = NULL;
            (*par)[i].numelts = 0;
        }
        *s_par = row + 1;
    }
    if ((*par)[row].numelts == 0 && (*par)[row].next != NULL) {
        free((*par)[row].next);
        (*par)[row].next = NULL;
    }
    if (col > (*par)[row].numelts - 1) {
        (*par)[row].next =
            (INT_S *)REALLOC((*par)[row].next, (col + 1) * sizeof(INT_S));
        if ((*par)[row].next == NULL) {
            mem_result = 1;
            return;
        }
        for (j = (*par)[row].numelts; j < col + 1; j++) {
            (*par)[row].next[j] = -1;
        }

        (*par)[row].numelts = col + 1;
    }

    /*Fill new elment in (row, col) of par*/
    (*par)[row].next[col] = elem;
}

void ehsync_blockevents(state_node *t1, INT_S s1, state_node *t2, INT_S s2,
                        state_node *t3, INT_S s3, INT_T **be, INT_T *s_be) {
    INT_T *e1 = NULL;
    INT_T s_e1 = 0;
    INT_T *e3 = NULL;
    INT_T s_e3 = 0;
    INT_S i;
    INT_T j;
    INT_B ok;

    /* Collect all events of t1 = L1 */
    for (i = 0; i < s1; i++)
        for (j = 0; j < t1[i].numelts; j++) {
            addordlist(t1[i].next[j].data1, &e1, s_e1, &ok);
            if (ok)
                s_e1++;
        }

    /* Collect all events for t2+t1 = L1+L2 = L12 */
    for (i = 0; i < s2; i++)
        for (j = 0; j < t2[i].numelts; j++) {
            addordlist(t2[i].next[j].data1, &e1, s_e1, &ok);
            if (ok)
                s_e1++;
        }

    /* Collect all events for t3 = L3 */
    for (i = 0; i < s3; i++)
        for (j = 0; j < t3[i].numelts; j++) {
            addordlist(t3[i].next[j].data1, &e3, s_e3, &ok);
            if (ok)
                s_e3++;
        }

    /* D = L12-L3 */
    for (j = 0; j < s_e1; j++)
        if (!inlist(e1[j], e3, s_e3)) {
            addordlist(e1[j], be, *s_be, &ok);
            if (ok)
                (*s_be)++;
        }

    free(e1);
    free(e3);
}

/*Used to compute synchronous product of agents
whose number are more than 2*/
/* modification by pitct */
void ehsync1(INT_S num_of_des, filename1 out_name, filename1 names[MAX_DESS],
             state_node **t3, INT_S *s3, INT_T **be, INT_T *s_be, INT_S *s_pn,
             part_node **pn) {
    INT_S s1, s2, s4;
    state_node *t1, *t2, *t4;
    INT_S init;
    INT_T *tranlist, s_tranlist;
    INT_S i, j, k;
    INT_B ok;
    INT_OS result;
    INT_S s_macro_c, *macro_c, *macro_ab;
    INT_S s_tmpn;
    part_node *tmpn;
    INT_S state1, state2;
    filename1 tmp_name, out_long_name;

    s1 = s2 = s4 = 0;
    t1 = t2 = t4 = NULL;
    tranlist = NULL;
    s_tranlist = 0;
    s_macro_c = 0;
    macro_c = macro_ab = NULL;
    s_tmpn = 0;
    tmpn = NULL;

    result = 0;

    for (i = 0; i < num_of_des; i++) {
        init = 0L;
        getdes(names[i], &s1, &init, &t1);
        if (mem_result == 1) {
            goto FREE_DES;
        }

        for (j = 0; j < s1; j++) {
            for (k = 0; k < t1[j].numelts; k++) {
                addordlist(t1[j].next[k].data1, &tranlist, s_tranlist, &ok);
                if (ok) {
                    s_tranlist++;
                }
            }
        }
        freedes(s1, &t1);
        s1 = 0;
        t1 = NULL;
    }


    /* Pass to command line version of this program */
    for (i = 0; i < num_of_des - 1; i++) {
        if (i == 0) {
            strcpy(tmp_name, names[0]);
            init = 0L;
            getdes(tmp_name, &s1, &init, &t1);
            getdes(names[i + 1], &s2, &init, &t2);

            sync3(s1, t1, s2, t2, &s4, &t4, 0, s_tranlist, tranlist, &macro_ab,
                  &macro_c);
            s_macro_c = s4;

            filedes(out_name, s4, init, t4);

            // result = sync2_runProgram(name3, name1, names[i + 1], 0,
            // s_tranlist, tranlist, &s_macro_c, &macro_c);
            for (j = 0; j < s_macro_c; j++) {
                insertelem_part(j, 0, (INT_T)(macro_c[j] % s1), s_pn, pn);
                insertelem_part(j, 1, (INT_T)(macro_c[j] / s1), s_pn, pn);
            }

            freedes(s1, &t1);
            s1 = 0;
            t1 = NULL;
            freedes(s2, &t2);
            s2 = 0;
            t2 = NULL;
            freedes(s4, &t4);
            s4 = 0;
            t4 = NULL;
            if (mem_result == 1) {
                goto FREE_DES;
            }
        } else {
            init = 0L;
            getdes(out_name, &s1, &init, &t1);

            getdes(names[i + 1], &s2, &init, &t2);

            sync3(s1, t1, s2, t2, &s4, &t4, 1, s_tranlist, tranlist, &macro_ab,
                  &macro_c);
            s_macro_c = s4;

            filedes(out_name, s4, init, t4);
            // result = sync2_runProgram(out_name, out_name, names[i + 1], 1,
            // s_tranlist, tranlist,&s_macro_c, &macro_c);

            ext_copy_part(&s_tmpn, &tmpn, *s_pn, *pn);
            free_part(*s_pn, pn);
            *s_pn = 0;
            *pn = NULL;
            for (j = 0; j < s_macro_c; j++) {
                state1 = macro_c[j] % s1;
                state2 = macro_c[j] / s1;
                for (k = 0; k < tmpn[state1].numelts; k++) {
                    insertelem_part(j, k, (INT_T)tmpn[state1].next[k], s_pn,
                                    pn);
                }
                insertelem_part(j, i + 1, (INT_T)state2, s_pn, pn);
            }

            freedes(s1, &t1);
            s1 = 0;
            t1 = NULL;
            freedes(s2, &t2);
            s2 = 0;
            t2 = NULL;
            freedes(s4, &t4);
            s4 = 0;
            t4 = NULL;

            if (mem_result == 1) {
                goto FREE_DES;
            }
            free_part(s_tmpn, &tmpn);
            s_tmpn = 0;
            tmpn = NULL;
        }
        if (result == CR_OK) {
            strcpy(out_long_name, "");
            make_filename_ext(out_long_name, out_name, EXT_DES);
            if (exist(out_long_name)) {
                init = 0L;
                getdes(names[i + 1], &s2, &init, &t2);
                getdes(out_name, s3, &init, t3);
                if (mem_result != 1) {
                    ehsync_blockevents(t1, s1, t2, s2, *t3, *s3, be, s_be);
                }
                freedes(s1, &t1);
                freedes(s2, &t2);
                freedes(*s3, t3);
                s1 = s2 = *s3 = 0;
                t1 = t2 = *t3 = NULL;
            } else
                break;
        } else {
            break;
        }
        free(macro_ab);
        macro_ab = NULL;
        free(macro_c);
        s_macro_c = 0;
        macro_c = NULL;
    }


    if (result == CR_OUT_OF_MEMORY) {
        mem_result = 1;
    }

FREE_DES:
    free(tranlist);
    freedes(s1, &t1);
    freedes(s2, &t2);
    freedes(s4, &t4);
    free_part(s_tmpn, &tmpn);
    free(macro_c);
    free(macro_ab);
    return;
}