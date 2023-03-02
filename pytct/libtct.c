#include "../libtct/program.h"
#include <Python.h>

int (*programs[32])(const char *filename) = {
    create_program, selfloop_program, trim_program,   printdes_program,
    sync_program,   meet_program,     supcon_program, allevents_program,
    mutex_program, complement_program, nonconflict_program, condat_program,
    supreduce_program, isomorph_program, printdat_program, getdes_parameter_program,
    supconrobs_program, project_program, localize_program, minstate_program,
    force_program, convert_program, supnorm_program, supscop_program, 
    canQC_program, obs_program, natobs_program, supobs_program, bfs_recode_program,
    ext_suprobs_program, export_ext_des_program, eh_sync_program};

static PyObject *call_program(PyObject *self, PyObject *args) {
  const char *prm_filename;
  int program;

  if (!PyArg_ParseTuple(args, "is", &program, &prm_filename))
    return NULL;
  return PyLong_FromLong((long)programs[program](prm_filename));
}

static PyMethodDef LibTCTMethods[] = {{"call_program",
                                       (PyCFunction)call_program, METH_VARARGS,
                                       "Call LibTCT programs."},
                                      {NULL, NULL, 0, NULL}};

static struct PyModuleDef libtctmodule = {PyModuleDef_HEAD_INIT, "libtct", NULL,
                                          -1, LibTCTMethods};

PyMODINIT_FUNC
PyInit_libtct(void) {
  return PyModule_Create(&libtctmodule);
}
