#include <sombok.h>
#include <Python.h>
#include "structmember.h"

/**
 ** Module definitions
 **/

static PyMethodDef module_methods[] = {
    { NULL } /* Sentinel */
};

/*
 * Initialize module
 */

PyDoc_STRVAR(module__doc__,
"\
Constants on module for text segmentation.");

#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef textseg_Consts_def = {
    PyModuleDef_HEAD_INIT,
    "_textseg_Consts",
    module__doc__,
    -1,
    module_methods,
    NULL, NULL, NULL, NULL
};
#define INITERROR return NULL
PyMODINIT_FUNC
PyInit__textseg_Consts(void)

#else /* PY_MAJOR_VERSION */

#define INITERROR return
void
init_textseg_Consts(void) 

#endif /* PY_MAJOR_VERSION */

{
    PyObject *m;
    size_t i;
    char name[8];

#if PY_MAJOR_VERSION >= 3
    m = PyModule_Create(&textseg_Consts_def);
#else /* PY_MAJOR_VERSION */
    m = Py_InitModule3("_textseg_Consts", module_methods, module__doc__);
#endif /* PY_MAJOR_VERSION */
    if (m == NULL)
        INITERROR;

    PyModule_AddStringConstant(m, "unicode_version",
                               (char *)linebreak_unicode_version);
    PyModule_AddStringConstant(m, "sombok_version", SOMBOK_VERSION);
    if (linebreak_southeastasian_supported == NULL) {
        Py_INCREF(Py_None);
        PyModule_AddObject(m, "sea_support", Py_None);
    } else
        PyModule_AddStringConstant(m, "sea_support",
                                   (char *)linebreak_southeastasian_supported);
    strcpy(name, "lbc");
    for (i = 0; linebreak_propvals_LB[i] != NULL; i++) {
        strcpy(name + 3, linebreak_propvals_LB[i]);
        PyModule_AddIntConstant(m, name, (long)i);
    }
    strcpy(name, "eaw");
    for (i = 0; linebreak_propvals_EA[i] != NULL; i++) {
        strcpy(name + 3, linebreak_propvals_EA[i]);
        PyModule_AddIntConstant(m, name, (long)i);
    }
    PyModule_AddStringConstant(m, "unicode_version",
                               (char *)linebreak_unicode_version);
    PyModule_AddStringConstant(m, "sombok_version", SOMBOK_VERSION);
    if (linebreak_southeastasian_supported == NULL) {
        Py_INCREF(Py_None);
        PyModule_AddObject(m, "sea_support", Py_None);
    } else
        PyModule_AddStringConstant(m, "sea_support",
                                   (char *)linebreak_southeastasian_supported);
    strcpy(name, "lbc");
    for (i = 0; linebreak_propvals_LB[i] != NULL; i++) {
        strcpy(name + 3, linebreak_propvals_LB[i]);
        PyModule_AddIntConstant(m, name, (long)i);
    }
    strcpy(name, "eaw");
    for (i = 0; linebreak_propvals_EA[i] != NULL; i++) {
        strcpy(name + 3, linebreak_propvals_EA[i]);
        PyModule_AddIntConstant(m, name, (long)i);
    }

#if PY_MAJOR_VERSION >= 3
    return m;
#endif /* PY_MAJOR_VERSION */
}

