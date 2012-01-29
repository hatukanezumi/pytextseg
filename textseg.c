#include <sombok.h>
#include <Python.h>
#include "structmember.h"

#if PY_MAJOR_VERSION == 2
#    if PY_MINOR_VERSION <= 4
	typedef int Py_ssize_t;
#	define ARG_FORMAT_SSIZE_T "i"
#	define PyInt_FromSsize_t(i) PyInt_FromLong(i)
#    else /* PY_MINOR_VERSION */
#	define ARG_FORMAT_SSIZE_T "n"
#    endif /* PY_MINOR_VERSION */
#endif /* PY_MAJOR_VERSION */

#ifndef Py_TYPE
#    define Py_TYPE(o) ((o)->ob_type)
#endif

/***
 *** Objects
 ***/

typedef struct LineBreakObject {
    PyObject_HEAD
    linebreak_t *obj;
} LineBreakObject;

typedef struct GCStrObject {
    PyObject_HEAD
    gcstring_t *obj;
} GCStrObject;

static PyTypeObject LineBreakType;
static PyTypeObject GCStrType;

#define LineBreak_Check(op) PyObject_TypeCheck(op, &LineBreakType)
#define GCStr_Check(op) PyObject_TypeCheck(op, &GCStrType)

/***
 *** Data conversion.
 ***/

/*
 * Convert PyUnicodeObject to Unicode string.
 * If error occurred, exception will be raised and NULL will be returned.
 * @note buffer of Py_UNICODE will be copied.
 */
static unistr_t *
unicode_ToCstruct(unistr_t *unistr, PyObject *pyobj)
{
    PyObject *pystr;
    Py_UNICODE *str;
    Py_ssize_t len;
#ifndef Py_UNICODE_WIDE
    Py_ssize_t j;
#endif /* Py_UNICODE_WIDE */

    if (pyobj == NULL)
	return NULL;
    else if (PyUnicode_Check(pyobj))
	pystr = pyobj;
    else if ((pystr = PyObject_Unicode(pyobj)) == NULL)
	return NULL;

    str = PyUnicode_AS_UNICODE(pystr);
    len = PyUnicode_GET_SIZE(pystr);

    if (len == 0) {
	if (! PyUnicode_Check(pyobj)) {
	   Py_DECREF(pystr);
	}
	unistr->str = NULL;
	unistr->len = 0;
	return unistr;
    }

    if ((unistr->str = malloc(sizeof(unichar_t) * len)) == NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);

        if (! PyUnicode_Check(pyobj)) {
           Py_DECREF(pystr);
	}
        return NULL;
    }

#ifndef Py_UNICODE_WIDE
    for (i = 0, j = 0; i < len; i++, j++)
	if (0xD800 <= str[i] && str[i] <= 0xDBFF && i + 1 < len &&
	    0xDC00 <= str[i + 1] && str[i + 1] <= 0xDFFF) {
	    unistr->str[j] = (str[i] & 0x3FF) << 10 + (str[i+1] & 0x3FF) +
	    0x10000;
	    i++;
	} else
	    unistr->str[j] = str[i];
    unistr->len = j;
#else /* Py_UNICODE_WIDE */
    /* sizeof(unichar_t) is sizeof(Py_UNICODE). */
    Py_UNICODE_COPY(unistr->str, str, len);
    unistr->len = len;
#endif /* Py_UNICODE_WIDE */

    if (! PyUnicode_Check(pyobj)) {
	Py_DECREF(pystr);
    }
    return unistr;
}

/*
 * Convert Unicode string to PyUnicodeObject
 * If error occurred, exception will be raised and NULL will be returned.
 * @note buffer of Unicode characters will be copied.
 */
static PyObject *
unicode_FromCstruct(unistr_t *unistr)
{
    Py_UNICODE *str;
    PyObject *ret;
#ifndef Py_UNICODE_WIDE
    size_t i, j;
    Py_UNICODE hi, lo;
#endif /* Py_UNICODE_WIDE */

    if (unistr->str == NULL || unistr->len == 0) {
	Py_UNICODE buf[1] = { (Py_UNICODE)0 };
	return PyUnicode_FromUnicode(buf, 0);
    }

#ifndef Py_UNICODE_WIDE
    for (i = 0, j = unistr->len; i < unistr->len; i++)
	if (0x10000 <= unistr->str[i])
	    j++;
    if ((str = malloc(sizeof(Py_UNICODE) * j)) == NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);
        return NULL;
    }
    for (i = 0, j = 0; i < unistr->len; i++, j++)
	if (0x10000 <= unistr->str[i]) {
	    hi = ((unistr->str[i] - 0x10000) >> 10) & 0x3FF + 0xD800;
	    lo = unistr->str[i] & 0x3FF + 0xDC00;
	    str[j] = hi;
	    str[j + 1] = lo;
            j++;
        } else
            str[j] = (Py_UNICODE)unistr->str[i];
    ret = PyUnicode_FromUnicode(str, j);
    free(str);
#else /* Py_UNICODE_WIDE */
    if ((str = malloc(sizeof(Py_UNICODE) * (unistr->len + 1))) == NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);
	return NULL;
    }
    /* sizeof(unistr_t) is sizeof(Py_UNICODE). */
    Py_UNICODE_COPY(str, unistr->str, unistr->len);
    ret = PyUnicode_FromUnicode(str, unistr->len);
    free(str);
#endif /* Py_UNICODE_WIDE */

    return ret;
}

/**
 * Convert LineBreakObject to linebreak object.
 */
#define LineBreak_AS_CSTRUCT(pyobj) \
    (((LineBreakObject *)(pyobj))->obj)

static linebreak_t *
LineBreak_AsCstruct(PyObject *pyobj)
{
    if (LineBreak_Check(pyobj))
	return LineBreak_AS_CSTRUCT(pyobj);
    PyErr_Format(PyExc_TypeError,
		 "expected LineBreak object, %200s found",
		 Py_TYPE(pyobj)->tp_name);
    return NULL;
}

/**
 * Convert linebreak object to LineBreakObject.
 */
static PyObject *
LineBreak_FromCstruct(linebreak_t *obj)
{
    LineBreakObject *self;

    if ((self = (LineBreakObject *)(LineBreakType.tp_alloc(&LineBreakType, 0)))
	== NULL)
	return NULL;

    self->obj = obj;
    return (PyObject *)self;
}

/**
 * Convert GCStrObject to gcstring object.
 */
#define GCStr_AS_CSTRUCT(pyobj) \
    (((GCStrObject *)(pyobj))->obj)

static gcstring_t *
GCStr_AsCstruct(PyObject *pyobj)
{
    if (GCStr_Check(pyobj))
	return GCStr_AS_CSTRUCT(pyobj);
    PyErr_Format(PyExc_TypeError,
		 "expected GCStr object, %200s found",
		 Py_TYPE(pyobj)->tp_name);
    return NULL;
}

/**
 * Convert grapheme cluster string to GCStrObject.
 */
static PyObject *
GCStr_FromCstruct(gcstring_t *obj)
{
    GCStrObject *self;

    if ((self = (GCStrObject *)(GCStrType.tp_alloc(&GCStrType, 0))) == NULL)
	return NULL;

    self->obj = obj;
    return (PyObject *)self;
}

/**
 * Convert Python object, Unicode string or GCStrObject to
 * grapheme cluster string.
 * If error occurred, exception will be raised and NULL will be returned.
 * @note if pyobj was GCStrObject, returned object will be original
 * object (not a copy of it).  Otherwise, _new_ object will be returned.
 */

static gcstring_t *
genericstr_ToCstruct(PyObject * pyobj, linebreak_t *lbobj)
{
    unistr_t unistr = { NULL, 0 };
    gcstring_t *gcstr;

    if (pyobj == NULL)
	return NULL;
    if (GCStr_Check(pyobj))
        return GCStr_AS_CSTRUCT(pyobj);
    if (unicode_ToCstruct(&unistr, pyobj) == NULL)
	return NULL;
    if ((gcstr = gcstring_new(&unistr, lbobj)) == NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);

	free(unistr.str);
	return NULL;
    }
    return gcstr;
}

/***
 *** Other utilities
 ***/

/*
 * Do regex match once then returns offset and length.
 */
void
do_re_search_once(PyObject *rx, unistr_t *str, unistr_t *text)
{
    PyObject *strobj, *matchobj, *func_search, *args, *pyobj;
    Py_ssize_t pos, endpos, start, end;

#ifndef Py_UNICODE_WIDE
    unichar_t *p;
    pos = str->str - text->str;
    for (p = text->str; p < str->str; p++)
	if ((unichar_t)0x10000UL <= *p)
	    pos++;
    endpos = pos + str->len;
    for ( ; p < str->str + str->len; p++)
	if ((unichar_t)0x10000UL <= *p)
	    endpos++;
#else /* Py_UNICODE_WIDE */
    pos = str->str - text->str;
    endpos = pos + str->len;
#endif /* Py_UNICODE_WIDE */

    if ((strobj = unicode_FromCstruct(text)) == NULL) {
	str->str = NULL;
	return;
    }
    func_search = PyObject_GetAttrString(rx, "search");
    args = PyTuple_New(3);
    PyTuple_SetItem(args, 0, strobj);
    PyTuple_SetItem(args, 1, PyInt_FromSsize_t(pos));
    PyTuple_SetItem(args, 2, PyInt_FromSsize_t(endpos));
    matchobj = PyObject_CallObject(func_search, args);
    Py_DECREF(args);
    Py_DECREF(func_search);
    if (matchobj == NULL) {
	str->str = NULL;
	return;
    }

    if (matchobj != Py_None) {
	if ((pyobj = PyObject_CallMethod(matchobj, "start", NULL)) == NULL) {
	    str->str = NULL;
	    return;
	}
	start = PyLong_AsLong(pyobj);
	Py_DECREF(pyobj);

	if ((pyobj = PyObject_CallMethod(matchobj, "end", NULL)) == NULL) {
	    str->str = NULL;
	    return;
	}
	end = PyLong_AsLong(pyobj);
	Py_DECREF(pyobj); 

#ifndef Py_UNICODE_WIDE
	#error "Not yet implemented"
#else /* Py_UNICODE_WIDE */
	str->str = text->str + start;
	str->len = end - start;
#endif
    } else
	str->str = NULL;
    Py_DECREF(matchobj);
}

/***
 *** Callbacks for linebreak library.  For more details see Sombok
 *** library documentations.
 ***/

/*
 * Increment/decrement reference count
 */
static void
ref_func(PyObject *obj, int datatype, int d)
{
    if (0 < d) {
        Py_INCREF(obj);
    } else if (d < 0) {
        Py_DECREF(obj);
    }
}

/*
 * Call preprocessing function
 * @note Python callback may return list of broken text or single text.
 */
static gcstring_t *
prep_func(linebreak_t *lbobj, void *data, unistr_t *str, unistr_t *text)
{
    PyObject *rx = NULL, *func = NULL, *pyret, *pyobj, *args;
    int count, i, j;
    gcstring_t *gcstr, *ret;

    /* Pass I */

    if (text != NULL) {
	rx = PyTuple_GetItem(data, 0);
        if (rx == NULL)
            return (lbobj->errnum = EINVAL), NULL;

        do_re_search_once(rx, str, text);
        return NULL;
    }

    /* Pass II */

    func = PyTuple_GetItem(data, 1);
    if (func == NULL) {
        if ((ret = gcstring_newcopy(str, lbobj)) == NULL)
            return (lbobj->errnum = errno ? errno : ENOMEM), NULL;
	return ret;
    }

    args = PyTuple_New(2);
    linebreak_incref(lbobj); /* prevent destruction */
    PyTuple_SetItem(args, 0, LineBreak_FromCstruct(lbobj));
    PyTuple_SetItem(args, 1, unicode_FromCstruct(str)); /* FIXME: err */
    pyret = PyObject_CallObject(func, args);
    Py_DECREF(args);
    if (PyErr_Occurred()) {
	if (! lbobj->errnum)
	    lbobj->errnum = LINEBREAK_EEXTN;
	return NULL;
    } 

    if (pyret == NULL)
	return NULL;

    if (PyList_Check(pyret)) {
        if ((ret = gcstring_new(NULL, lbobj)) == NULL)
            return (lbobj->errnum = errno ? errno : ENOMEM), NULL;

	count = PyList_Size(pyret);
        for (i = 0; i < count; i++) {
            pyobj = PyList_GetItem(pyret, i); /* borrowed ref. */
            if (pyobj == Py_None)
                continue;
	    else if (GCStr_Check(pyobj))
		gcstr = gcstring_copy(GCStr_AS_CSTRUCT(pyobj));
	    else
		gcstr = genericstr_ToCstruct(pyobj, lbobj);
	    if (gcstr == NULL) {
		if (! lbobj->errnum)
		    lbobj->errnum = errno ? errno : LINEBREAK_EEXTN;

		Py_DECREF(pyret);
		return NULL;
	    }

            for (j = 0; j < gcstr->gclen; j++) {
                if (gcstr->gcstr[j].flag &
                    (LINEBREAK_FLAG_ALLOW_BEFORE |
                     LINEBREAK_FLAG_PROHIBIT_BEFORE))
                    continue;
                if (i < count - 1 && j == 0)
                    gcstr->gcstr[j].flag |= LINEBREAK_FLAG_ALLOW_BEFORE;
                else if (0 < j)
                    gcstr->gcstr[j].flag |= LINEBREAK_FLAG_PROHIBIT_BEFORE;
            }

	    gcstring_append(ret, gcstr);
	    gcstring_destroy(gcstr);
	}
	Py_DECREF(pyret);
	return ret;
    }

    if (GCStr_Check(pyret))
	ret = gcstring_copy(GCStr_AS_CSTRUCT(pyret));
    else
	ret = genericstr_ToCstruct(pyret, lbobj);
    if (ret == NULL) {
	if (! lbobj->errnum)
	    lbobj->errnum = LINEBREAK_EEXTN;

	Py_DECREF(pyret);
	return NULL;
    }
    Py_DECREF(pyret);

    for (j = 0; j < ret->gclen; j++) {
	if (ret->gcstr[j].flag &
	    (LINEBREAK_FLAG_ALLOW_BEFORE | LINEBREAK_FLAG_PROHIBIT_BEFORE))
	    continue;
	if (0 < j)
	    ret->gcstr[j].flag |= LINEBREAK_FLAG_PROHIBIT_BEFORE;
    }

    return ret;
}

/*
 * Call format function
 */
static char *linebreak_states[] = {
    NULL, "sot", "sop", "sol", "", "eol", "eop", "eot", NULL
};
static gcstring_t *
format_func(linebreak_t *lbobj, linebreak_state_t action, gcstring_t *str)
{
    PyObject *func, *args, *pyobj;
    char *actionstr;
    gcstring_t *gcstr;

    func = (PyObject *)lbobj->format_data;
    if (func == NULL)
	return NULL;

    if (action <= LINEBREAK_STATE_NONE || LINEBREAK_STATE_MAX <= action)
	return NULL;
    actionstr = linebreak_states[(size_t)action];

    args = PyTuple_New(3);
    linebreak_incref(lbobj); /* prevent destruction */
    PyTuple_SetItem(args, 0, LineBreak_FromCstruct(lbobj));
    PyTuple_SetItem(args, 1, PyString_FromString(actionstr));
    PyTuple_SetItem(args, 2, GCStr_FromCstruct(gcstring_copy(str)));
    pyobj = PyObject_CallObject(func, args);
    Py_DECREF(args);

    if (PyErr_Occurred()) {
	if (! lbobj->errnum)
	    lbobj->errnum = LINEBREAK_EEXTN;
	if (pyobj != NULL) {
	    Py_DECREF(pyobj);
	}
	return NULL;
    }
    if (pyobj == NULL)
	return NULL;
    if (pyobj == Py_None)
	gcstr = NULL;
    else if ((gcstr = genericstr_ToCstruct(pyobj, lbobj)) == NULL) {
	if (! lbobj->errnum)
	    lbobj->errnum = errno ? errno : ENOMEM;
	PyErr_SetFromErrno(PyExc_RuntimeError);
	Py_DECREF(pyobj);
	return NULL;
    }
    if (GCStr_Check(pyobj))
	GCStr_AS_CSTRUCT(pyobj) = NULL; /* prevent destruction */
    Py_DECREF(pyobj);
    return gcstr;
}

/*
 * Call sizing function
 */
static double
sizing_func(linebreak_t *lbobj, double len,
	    gcstring_t *pre, gcstring_t *spc, gcstring_t *str)
{
    PyObject *func, *args, *pyobj;
    double ret;

    func = (PyObject *)lbobj->sizing_data;
    if (func == NULL)
	return -1.0;

    args = PyTuple_New(5);
    linebreak_incref(lbobj); /* prevent destruction. */
    PyTuple_SetItem(args, 0, LineBreak_FromCstruct(lbobj));
    PyTuple_SetItem(args, 1, PyInt_FromSsize_t(len));
    PyTuple_SetItem(args, 2, GCStr_FromCstruct(gcstring_copy(pre)));
    PyTuple_SetItem(args, 3, GCStr_FromCstruct(gcstring_copy(spc)));
    PyTuple_SetItem(args, 4, GCStr_FromCstruct(gcstring_copy(str)));
    pyobj = PyObject_CallObject(func, args);
    Py_DECREF(args);

    if (PyErr_Occurred()) {
	if (! lbobj->errnum)
	    lbobj->errnum = LINEBREAK_EEXTN;
	if (pyobj != NULL) {
	    Py_DECREF(pyobj);
	}
	return -1.0;
    }

    ret = PyFloat_AsDouble(pyobj);
    if (PyErr_Occurred()) {
	if (! lbobj->errnum)
	    lbobj->errnum = LINEBREAK_EEXTN;
	Py_DECREF(pyobj);
	return -1.0;
    }
    Py_DECREF(pyobj);

    return ret;
}

/*
 * Call urgent breaking function
 */
static gcstring_t *
urgent_func(linebreak_t *lbobj, gcstring_t *str)
{
    PyObject *func, *args, *pyret, *pyobj;
    size_t count, i;
    gcstring_t *gcstr, *ret;

    func = (PyObject *)lbobj->urgent_data;
    if (func == NULL)
	return NULL;

    args = PyTuple_New(2);
    linebreak_incref(lbobj); /* prevent destruction. */
    PyTuple_SetItem(args, 0, LineBreak_FromCstruct(lbobj));
    PyTuple_SetItem(args, 1, GCStr_FromCstruct(gcstring_copy(str)));
    pyret = PyObject_CallObject(func, args);
    Py_DECREF(args);

    if (PyErr_Occurred()) {
	if (! lbobj->errnum)
	    lbobj->errnum = LINEBREAK_EEXTN;
	if (pyret != NULL) {
	    Py_DECREF(pyret);
	}
	return NULL;
    }

    if (pyret == Py_None) {
	Py_DECREF(pyret);
	return NULL;
    }
    if (! PyList_Check(pyret)) {
	if (GCStr_Check(pyret))
	    ret = gcstring_copy(GCStr_AS_CSTRUCT(pyret));
	else
	    ret = genericstr_ToCstruct(pyret, lbobj);
	Py_DECREF(pyret);
	return ret;
    }

    ret = gcstring_new(NULL, lbobj);
    count = PyList_Size(pyret);
    for (i = 0; i < count; i++) {
	pyobj = PyList_GetItem(pyret, i); /* borrowed ref. */
	if (pyobj == Py_None)
	    continue;
	else if (GCStr_Check(pyobj))
	    gcstr = gcstring_copy(GCStr_AS_CSTRUCT(pyobj));
	else
	    gcstr = genericstr_ToCstruct(pyobj, lbobj);
	if (gcstr == NULL) {
	    if (! lbobj->errnum)
		lbobj->errnum = errno ? errno : LINEBREAK_EEXTN;

	    Py_DECREF(pyret);
	    return NULL;
       }

	if (gcstr->gclen)
	    gcstr->gcstr[0].flag = LINEBREAK_FLAG_ALLOW_BEFORE;
	gcstring_append(ret, gcstr);
	gcstring_destroy(gcstr);
    }

    Py_DECREF(pyret);
    return ret;
}

/*** 
 *** Python module definitions
 ***/

/**
 ** Exceptions
 **/

static PyObject *LineBreakException;

/**
 ** LineBreak class
 **/

/*
 * Constructor & Destructor
 */

static void
LineBreak_dealloc(LineBreakObject *self)
{
    linebreak_destroy(self->obj);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
LineBreak_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    LineBreakObject *self;
    PyObject *stash;

    if ((self = (LineBreakObject *)type->tp_alloc(type, 0)) == NULL)
	return NULL;

    if ((self->obj = linebreak_new()) == NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);
	Py_DECREF(self);
	return NULL;
    }

    /* set reference count handler */
    self->obj->ref_func = (void *) ref_func;
    /* set stash, dictionary: See Mapping methods */
    if ((stash = PyDict_New()) == NULL) {
	Py_DECREF(self);
	return NULL;
    }
    linebreak_set_stash(self->obj, stash);
    Py_DECREF(stash); /* fixup */

    return (PyObject *)self;
}

static PyGetSetDef LineBreak_getseters[];

static int
LineBreak_init(LineBreakObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *key, *value;
    Py_ssize_t pos;
    PyGetSetDef *getset;
    char * keystr;

    if (kwds == NULL)
	return 0;

    pos = 0;
    while (PyDict_Next(kwds, &pos, &key, &value)) {
	keystr = PyString_AsString(key);
	for (getset = LineBreak_getseters; getset->name != NULL; getset++) {
	    if (getset->set == NULL)
		continue;
	    if (strcmp(getset->name, keystr) != 0)
		continue;
	    if ((getset->set)((PyObject *)self, value, NULL) != 0)
		return -1;
	    break;
	}
	if (getset->name == NULL) {
	    PyErr_SetString(PyExc_ValueError,
			    "invalid argument");
	    return -1;
	}
    }
    return 0;
}

/*
 * Attribute methods
 */

#define _get_Boolean(name, bit) \
    static PyObject * \
    LineBreak_get_##name(PyObject *self) \
    { \
        PyObject *val; \
    \
        val = (LineBreak_AS_CSTRUCT(self)->options & bit) ? \
              Py_True : Py_False; \
        Py_INCREF(val); \
        return val; \
    }

_get_Boolean(break_indent, LINEBREAK_OPTION_BREAK_INDENT)

static PyObject *
LineBreak_get_charmax(PyObject *self)
{
    return PyInt_FromSsize_t(LineBreak_AS_CSTRUCT(self)->charmax);
}

static PyObject *
LineBreak_get_width(PyObject *self)
{
    return PyFloat_FromDouble(LineBreak_AS_CSTRUCT(self)->colmax);
}

static PyObject *
LineBreak_get_minwidth(PyObject *self)
{
    return PyFloat_FromDouble(LineBreak_AS_CSTRUCT(self)->colmin);
}

_get_Boolean(complex_breaking, LINEBREAK_OPTION_COMPLEX_BREAKING)

_get_Boolean(eastasian_context, LINEBREAK_OPTION_EASTASIAN_CONTEXT)

static PyObject *
LineBreak_get_eaw(PyObject *self)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);
    PyObject *val;

    val = PyDict_New();
    if (lbobj->map != NULL && lbobj->mapsiz != 0) {
	unichar_t c;
	PyObject *p;
	size_t i;
	for (i = 0; i < lbobj->mapsiz; i++)
	    if (lbobj->map[i].eaw != PROP_UNKNOWN) {
		p = PyInt_FromLong((signed long)lbobj->map[i].eaw);
		for (c = lbobj->map[i].beg; c <= lbobj->map[i].end; c++)
		    PyDict_SetItem(val, PyInt_FromLong(c), p);
	    }
    }
    return val;
}

static PyObject *
LineBreak_get_format(PyObject *self)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);
    PyObject *val;

    if (lbobj->format_func == NULL) {
	val = Py_None;
	Py_INCREF(val);
    } else if (lbobj->format_func == linebreak_format_NEWLINE)
	val = PyString_FromString("NEWLINE");
    else if (lbobj->format_func == linebreak_format_SIMPLE)
	val = PyString_FromString("SIMPLE");
    else if (lbobj->format_func == linebreak_format_TRIM)
	val = PyString_FromString("TRIM");
    else if (lbobj->format_func == format_func) {
	val = (PyObject *)lbobj->format_data;
	Py_INCREF(val);
    } else {
	PyErr_Format(PyExc_RuntimeError, "Internal error.  Ask developer.");
	return NULL;
    }
    return val;
}

_get_Boolean(hangul_as_al, LINEBREAK_OPTION_HANGUL_AS_AL)

static PyObject *
LineBreak_get_lbc(PyObject *self)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);
    PyObject *val;

    val = PyDict_New();
    if (lbobj->map != NULL && lbobj->mapsiz != 0) {
        unichar_t c;
        PyObject *p;
        size_t i;
        for (i = 0; i < lbobj->mapsiz; i++)
            if (lbobj->map[i].lbc != PROP_UNKNOWN) {
                p = PyInt_FromLong((signed long)lbobj->map[i].lbc);
                for (c = lbobj->map[i].beg; c <= lbobj->map[i].end; c++)
                    PyDict_SetItem(val, PyInt_FromLong(c), p);
            }
    }
    return val;
}

_get_Boolean(legacy_cm, LINEBREAK_OPTION_LEGACY_CM)

static PyObject *
LineBreak_get_newline(PyObject *self)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);
    unistr_t unistr;

    unistr.str = lbobj->newline.str;
    unistr.len = lbobj->newline.len;
    return unicode_FromCstruct(&unistr);
}

static PyObject *
LineBreak_get_prep(PyObject *self)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);
    PyObject *val;

    if (lbobj->prep_func == NULL || lbobj->prep_func[0] == NULL) {
	val = Py_None;
	Py_INCREF(val);
    } else {
	size_t i;
	PyObject *v;
	val = PyList_New(0);
	for (i = 0; lbobj->prep_func[i] != NULL; i++) {
	    if (lbobj->prep_func[i] == linebreak_prep_URIBREAK) {
		if (lbobj->prep_data == NULL ||
		    lbobj->prep_data[i] == NULL)
		    v = PyString_FromString("NONBREAKURI");
		else
		    v = PyString_FromString("BREAKURI");
	    } else if (lbobj->prep_data == NULL ||
		       lbobj->prep_data[i] == NULL) {
		v = Py_None;
		Py_INCREF(v);
	    } else {
		v = lbobj->prep_data[i];
		Py_INCREF(v);
	    }
	    PyList_Append(val, v);
	}
    }
    return val;
}

static PyObject *
LineBreak_get_sizing(PyObject *self)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);
    PyObject *val;

    if (lbobj->sizing_func == NULL)
	val = Py_None;
    else if (lbobj->sizing_func == linebreak_sizing_UAX11 ||
	     lbobj->sizing_func == sizing_func)
	val = (PyObject *)lbobj->sizing_data;
    else {
	PyErr_Format(PyExc_RuntimeError, "XXX");
	return NULL;
    }
    Py_INCREF(val);

    return val;
}

static PyObject *
LineBreak_get_urgent(PyObject *self)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);
    PyObject *val;

    if (lbobj->urgent_func == NULL) {
        val = Py_None;
	Py_INCREF(val);
    } else if (lbobj->urgent_func == linebreak_urgent_ABORT)
        val = PyString_FromString("RAISE");
    else if (lbobj->urgent_func == linebreak_urgent_FORCE)
        val = PyString_FromString("FORCE");
    else if (lbobj->urgent_func == urgent_func) {
        val = (PyObject *)lbobj->urgent_data;
	Py_INCREF(val);
    } else {
        PyErr_Format(PyExc_RuntimeError, "XXX");
	return NULL;
    }
    return val;
}

_get_Boolean(virama_as_joiner, LINEBREAK_OPTION_VIRAMA_AS_JOINER)


#define _set_Boolean(name, bit) \
    static int \
    LineBreak_set_##name(PyObject *self, PyObject *arg, void *closure) \
    { \
        linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self); \
        long ival; \
    \
        if (arg == NULL) { \
	    PyErr_Format(PyExc_NotImplementedError, \
		         "Can not delete attribute"); \
	    return -1; \
        } \
    \
        ival = PyInt_AsLong(arg); \
        if (PyErr_Occurred()) \
	    return -1; \
        if (ival) \
	    lbobj->options |= bit; \
        else \
	    lbobj->options &= ~bit; \
	return 0; \
    }

static int
_update_maps(linebreak_t *lbobj, PyObject *dict, int maptype)
{
    Py_ssize_t pos, i;
    PyObject *key, *value, *item;
    propval_t p;
    unichar_t c;

    if (dict == NULL) {
	PyErr_Format(PyExc_NotImplementedError,
		     "Can not delete attribute");
	return -1;
    }
    if (! PyDict_Check(dict)) {
	PyErr_Format(PyExc_TypeError,
		     "attribute must be dictionary, not %200s",
                     Py_TYPE(dict)->tp_name);
	return -1;
    }

    pos = 0;
    while (PyDict_Next(dict, &pos, &key, &value)) {
	if (PyInt_Check(value))
	    p = (propval_t)PyInt_AsLong(value);
	else if (PyLong_Check(value))
	    p = (propval_t)PyLong_AsLong(value);
	else {
	    PyErr_Format(PyExc_ValueError,
			 "value of map must be integer, not %200s",
			 Py_TYPE(value)->tp_name);
	    return -1;
	}

	if (PySequence_Check(key)) {
	    for (i = 0; (item = PySequence_GetItem(key, i)) != NULL; i++) {
		if (PyUnicode_Check(item) && 0 < PyUnicode_GET_SIZE(item)) {
#ifndef Py_UNICODE_WIDE
		    #error "Not yet implemented"
#else /* Py_UNICODE_WIDE */
		    c = *PyUnicode_AS_UNICODE(item);
#endif /* Py_UNICODE_WIDE */
		} else if (PyInt_Check(item))
		    c = (unichar_t)PyInt_AsLong(item);
		else if (PyLong_Check(item))
		    c = (unichar_t)PyLong_AsLong(item);
		else {
		    PyErr_Format(PyExc_ValueError,
				 "key of map must be integer or character, "
				 "not %200s", Py_TYPE(value)->tp_name);
		    return -1;
		}
		if (maptype == 0)
		    linebreak_update_lbclass(lbobj, c, p);
		else
		    linebreak_update_eawidth(lbobj, c, p);
	    }
	    PyErr_Clear();
	    continue; /* while (PyDict_Next( ... */
	} else if (PyInt_Check(key))
	    c = (unichar_t)PyInt_AsLong(key);
	else if (PyLong_Check(key))
	    c = (unichar_t)PyLong_AsLong(key);
	else {
	    PyErr_Format(PyExc_ValueError,
			 "key of map must be integer or character, not %200s",
			 Py_TYPE(value)->tp_name);
	    return -1;
	}
	if (maptype == 0)
	    linebreak_update_lbclass(lbobj, c, p);
	else
	    linebreak_update_eawidth(lbobj, c, p);
    }

    return 0;
}


_set_Boolean(break_indent, LINEBREAK_OPTION_BREAK_INDENT)

static int
LineBreak_set_charmax(PyObject *self, PyObject *value, void *closure)
{
    long ival;

    if (value == NULL) {
	PyErr_Format(PyExc_NotImplementedError,
		     "Can not delete attribute");
	return -1;
    }
    if (PyInt_Check(value))
	ival = (long)PyInt_AsLong(value);
    else if (PyLong_Check(value))
	ival = PyInt_AsLong(value);
    else {
	PyErr_Format(PyExc_TypeError,
		     "attribute must be non-negative integer, not %200s",
		     Py_TYPE(value)->tp_name);
	return -1;
    }
    if (ival < 0) {
	PyErr_Format(PyExc_ValueError,
		     "attribute must be non-negative integer, not %ld",
		     ival);
	return -1;
    }
    LineBreak_AS_CSTRUCT(self)->charmax = ival;
    return 0;
}

static int
LineBreak_set_width(PyObject *self, PyObject *value, void *closure)
{
    double dval;

    if (value == NULL) {
        PyErr_Format(PyExc_NotImplementedError,
                     "Can not delete attribute");
        return -1;
    }
    if (PyInt_Check(value))
        dval = (double)PyInt_AsLong(value);
    else if (PyLong_Check(value))
        dval = (double)PyInt_AsLong(value);
    else if (PyFloat_Check(value))
        dval = PyFloat_AsDouble(value);
    else {
	PyErr_Format(PyExc_TypeError,
		     "attribute must be non-negative real number, not %200s",
		     Py_TYPE(value)->tp_name);
	return -1;
    }
    if (dval < 0.0) {
	PyErr_Format(PyExc_ValueError,
		     "attribute must be non-negative real number, not %f",
		     dval);
	return -1;
    }
    LineBreak_AS_CSTRUCT(self)->colmax = dval;
    return 0;
}

static int
LineBreak_set_minwidth(PyObject *self, PyObject *value, void *closure)
{
    double dval;

    if (value == NULL) {
        PyErr_Format(PyExc_NotImplementedError,
                     "Can not delete attribute");
        return -1;
    }
    if (PyInt_Check(value))
        dval = (double)PyInt_AsLong(value);
    else if (PyLong_Check(value))
        dval = (double)PyInt_AsLong(value);
    else if (PyFloat_Check(value))
        dval = PyFloat_AsDouble(value);
    else {
        PyErr_Format(PyExc_TypeError,
                     "attribute must be non-negative real number, not %200s",
                     Py_TYPE(value)->tp_name);
        return -1;
    }
    if (dval < 0.0) {
        PyErr_Format(PyExc_ValueError,
                     "attribute must be non-negative real number, not %f",
                     dval);
        return -1;
    }
    LineBreak_AS_CSTRUCT(self)->colmin = dval;
    return 0;
}

_set_Boolean(complex_breaking, LINEBREAK_OPTION_COMPLEX_BREAKING)

_set_Boolean(eastasian_context, LINEBREAK_OPTION_EASTASIAN_CONTEXT)

static int
LineBreak_set_eaw(PyObject *self, PyObject *value, void *closure)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);

    if (value == Py_None)
	linebreak_clear_eawidth(lbobj);
    else
	return _update_maps(lbobj, value, 1);
    return 0;
}

static int
LineBreak_set_format(PyObject *self, PyObject *value, void *closure)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);

    if (value == NULL)
	linebreak_set_format(lbobj, NULL, NULL);
    else if (value == Py_None)
	linebreak_set_format(lbobj, NULL, NULL);
    else if (PyString_Check(value) || PyUnicode_Check(value)) {
	char *str;
	str = PyString_AsString(value);

	if (strcmp(str, "SIMPLE") == 0)
	    linebreak_set_format(lbobj, linebreak_format_SIMPLE, NULL);
	else if (strcmp(str, "NEWLINE") == 0)
	    linebreak_set_format(lbobj, linebreak_format_NEWLINE, NULL);
	else if (strcmp(str, "TRIM") == 0)
	    linebreak_set_format(lbobj, linebreak_format_TRIM, NULL);
	else {
	    PyErr_Format(PyExc_ValueError,
			 "unknown attribute value, %200s", str);
	    return -1;
	}
    } else if (PyFunction_Check(value))
	linebreak_set_format(lbobj, format_func, (void *)value);
    else {
	PyErr_Format(PyExc_ValueError,
		     "attribute must be list, not %200s",
		     Py_TYPE(value)->tp_name);
	return -1;
    }
    return 0;
}

_set_Boolean(hangul_as_al, LINEBREAK_OPTION_HANGUL_AS_AL)

static int
LineBreak_set_lbc(PyObject *self, PyObject *value, void *closure)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);

    if (value == Py_None)
	linebreak_clear_lbclass(lbobj);
    else
	return _update_maps(lbobj, value, 0);
    return 0;
}

_set_Boolean(legacy_cm, LINEBREAK_OPTION_LEGACY_CM)

static int
LineBreak_set_newline(PyObject *self, PyObject *value, void *closure)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);
    unistr_t unistr = { NULL, 0 };

    if (value == NULL)
	linebreak_set_newline(lbobj, &unistr);
    else if (value == Py_None)
	linebreak_set_newline(lbobj, &unistr);
    else {
	if (unicode_ToCstruct(&unistr, value) == NULL)
	    return -1;
	linebreak_set_newline(lbobj, &unistr);
	free(unistr.str);
    }
    return 0;
}

static int
LineBreak_set_prep(PyObject *self, PyObject *value, void *closure)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);
    Py_ssize_t i, len;
    PyObject *item;
    char *str;

    if (value == NULL)
	linebreak_add_prep(lbobj, NULL, NULL);
    else if (value == Py_None)
	linebreak_add_prep(lbobj, NULL, NULL);
    else if (PyList_Check(value)) {
	linebreak_add_prep(lbobj, NULL, NULL);
	len = PyList_Size(value);
	for (i = 0; i < len; i++) {
	    item = PyList_GetItem(value, i);
	    if (PyString_Check(item) || PyUnicode_Check(item)) {
		str = PyString_AsString(item);
		if (PyErr_Occurred())
		    ;
		else if (strcasecmp(str, "BREAKURI") == 0)
		    linebreak_add_prep(lbobj, linebreak_prep_URIBREAK,
				       Py_True);
		else if (strcasecmp(str, "NONBREAKURI") == 0)
		    linebreak_add_prep(lbobj, linebreak_prep_URIBREAK,
				       NULL);
		else {
		    PyErr_Format(PyExc_ValueError,
				 "unknown attribute value %200s", str);
		    return -1;
		}
	    } else if (PyTuple_Check(item)) {
		PyObject *re_module, *patt, *func_compile, *args;

	        if (PyTuple_Size(item) != 2) {
		    PyErr_Format(PyExc_ValueError,
				 "argument size mismatch");
		    return -1;
		}

		patt = PyTuple_GetItem(item, 0);
		if (PyString_Check(patt)) {
		    re_module = PyImport_ImportModule("re");
		    func_compile = PyObject_GetAttrString(re_module,
							  "compile");
		    args = PyTuple_New(1); 
		    PyTuple_SetItem(args, 0, patt);
		    patt = PyObject_CallObject(func_compile, args);
		    Py_DECREF(args);

		    PyTuple_SetItem(item, 0, patt);
		}
		linebreak_add_prep(lbobj, prep_func, item);
	    } else {
	        PyErr_Format(PyExc_TypeError,
			     "prep argument must be list of tuples, not %200s",
			     Py_TYPE(item)->tp_name);
		return -1;
	    }
	}
    } else {
	PyErr_Format(PyExc_TypeError,
		     "prep argument must be list of tuples, not %200s",
		     Py_TYPE(value)->tp_name);
	return -1;
    }
    return 0;
}

static int
LineBreak_set_sizing(PyObject *self, PyObject *value, void *closure)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);

    if (value == NULL)
	linebreak_set_sizing(lbobj, NULL, NULL);
    if (value == Py_None)
	linebreak_set_sizing(lbobj, NULL, NULL);
    else if (PyString_Check(value) || PyUnicode_Check(value)) {
        char *str;
	str = PyString_AsString(value);

	if (strcmp(str, "UAX11") == 0)
	    linebreak_set_sizing(lbobj, linebreak_sizing_UAX11, NULL);
	else {
	    PyErr_Format(PyExc_ValueError,
			 "unknown attribute value %200s",
			 str);
	    return -1;
	}
    } else if (PyFunction_Check(value))
        linebreak_set_sizing(lbobj, sizing_func, (void *)value);
    else {
        PyErr_Format(PyExc_ValueError,
		     "attribute must be function, not %200s",
		     Py_TYPE(value)->tp_name);
	return -1;
    }
    return 0;
}

static int
LineBreak_set_urgent(PyObject *self, PyObject *value, void *closure)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);

    if (value == NULL)
        linebreak_set_urgent(lbobj, NULL, NULL);
    else if (value == Py_None)
        linebreak_set_urgent(lbobj, NULL, NULL);
    else if (PyString_Check(value) || PyUnicode_Check(value)) {
        char *str;

	str = PyString_AsString(value);
	if (strcmp(str, "FORCE") == 0)
	    linebreak_set_urgent(lbobj, linebreak_urgent_FORCE, NULL);
	else if (strcmp(str, "RAISE") == 0)
	    linebreak_set_urgent(lbobj, linebreak_urgent_ABORT, NULL);
	else {
	    PyErr_Format(PyExc_ValueError,
			 "unknown attribute value %200s",
			 str);
	    return -1;
	}
    } else if (PyFunction_Check(value))
        linebreak_set_urgent(lbobj, urgent_func, (void *)value);
    else {
        PyErr_Format(PyExc_ValueError,
		     "attribute must be list, not %200s",
		     Py_TYPE(value)->tp_name);
	return -1;
    }
    return 0;
}

_set_Boolean(virama_as_joiner, LINEBREAK_OPTION_VIRAMA_AS_JOINER)

static PyGetSetDef LineBreak_getseters[] = {
    { "break_indent",
      (getter)LineBreak_get_break_indent,
      (setter)LineBreak_set_break_indent,
      PyDoc_STR("\
Always allows break after SPACEs at beginning of line, \
a.k.a. indent.  [UAX #14] does not take account of such usage of SPACE.")},
    { "charmax",
      (getter)LineBreak_get_charmax,
      (setter)LineBreak_set_charmax,
      PyDoc_STR("\
Possible maximum number of characters in one line, \
not counting trailing SPACEs and newline sequence.  \
Note that number of characters generally doesn't represent length of line.  \
0 means unlimited.")},
    { "width",
      (getter)LineBreak_get_width,
      (setter)LineBreak_set_width,
      PyDoc_STR("\
Maximum number of columns line may include not counting \
trailing spaces and newline sequence.  In other words, recommended maximum \
length of line.")},
    { "minwidth",
      (getter)LineBreak_get_minwidth,
      (setter)LineBreak_set_minwidth,
      PyDoc_STR("\
Minimum number of columns which line broken arbitrarily may \
include, not counting trailing spaces and newline sequences.")},
    { "complex_breaking",
      (getter)LineBreak_get_complex_breaking,
      (setter)LineBreak_set_complex_breaking,
      PyDoc_STR("\
Performs heuristic breaking on South East Asian complex \
context.  If word segmentation for South East Asian writing systems is not \
enabled, this does not have any effect.")},
    { "eastasian_context",
      (getter)LineBreak_get_eastasian_context,
      (setter)LineBreak_set_eastasian_context,
      PyDoc_STR("\
Enable East Asian language/region context.")},
    { "eaw",
      (getter)LineBreak_get_eaw,
      (setter)LineBreak_set_eaw,
      PyDoc_STR("\
Tailor classification of East_Asian_Width property.  \
Value may be a dictionary with its keys are Unicode string or \
UCS scalar and with its values are any of East_Asian_Width properties \
(See class properties).  \
If None is specified, all tailoring assigned before will be canceled.\n\
By default, no tailorings are available.")},
    { "format",
      (getter)LineBreak_get_format,
      (setter)LineBreak_set_format,
      PyDoc_STR("\
Specify the method to format broken lines.\n\
: \"SIMPLE\" : \
Just only insert newline at arbitrary breaking positions.\n\
: \"NEWLINE\" : \
Insert or replace newline sequences with that specified by newline option, \
remove SPACEs leading newline sequences or end-of-text.  Then append newline \
at end of text if it does not exist.\n\
: \"TRIM\" : \
Insert newline at arbitrary breaking positions.  Remove SPACEs leading \
newline sequences.\n\
: None : \
Do nothing, even inserting any newlines.\n\
: callable object : \
See \"Formatting Lines\".")},
    { "hangul_as_al",
      (getter)LineBreak_get_hangul_as_al,
      (setter)LineBreak_set_hangul_as_al,
      PyDoc_STR("\
Treat hangul syllables and conjoining jamo as alphabetic \
characters (AL).")},
    { "lbc",
      (getter)LineBreak_get_lbc,
      (setter)LineBreak_set_lbc,
      PyDoc_STR("\
Tailor classification of line breaking property.  \
Value may be a dictionary with its keys are Unicode string or \
UCS scalar and its values with any of Line Breaking Classes \
(See class properties).  \
If None is specified, all tailoring assigned before will be canceled.\n\
By default, no tailorings are available.")},
    { "legacy_cm",
      (getter)LineBreak_get_legacy_cm,
      (setter)LineBreak_set_legacy_cm,
      PyDoc_STR("\
Treat combining characters lead by a SPACE as an isolated \
combining character (ID).  As of Unicode 5.0, such use of SPACE is not \
recommended.")},
    { "newline",
      (getter)LineBreak_get_newline,
      (setter)LineBreak_set_newline,
      PyDoc_STR("\
Unicode string to be used for newline sequence.  \
It may be None.")},
    { "prep",
      (getter)LineBreak_get_prep,
      (setter)LineBreak_set_prep,
      PyDoc_STR("\
Add user-defined line breaking behavior(s).  \
Value shall be list of items described below.\n\
: \"NONBREAKURI\" : \
Won't break URIs.\n\
: \"BREAKURI\" : \
Break URIs according to a rule suitable for printed materials.  \
For more details see [CMOS], sections 6.17 and 17.11.\n\
: (regex object, callable object) : \
The sequences matching regex object will be broken by callable object.  \
For more details see \"User-Defined Breaking Behaviors\".\n\
: None : \
Cancel all methods assigned before.")},
    { "sizing",
      (getter)LineBreak_get_sizing,
      (setter)LineBreak_set_sizing,
      PyDoc_STR("\
Specify method to calculate size of string.  \
Following options are available.\n\
: \"UAX11\" : \
Sizes are computed by columns of each characters.\n\
: None : \
Number of grapheme clusters (See documentation of GCStr class) contained \
in the string.\n\
: callable object : \
See \"Calculating String Size\".\n\
\n\
See also eaw property.")},
    { "urgent",
      (getter)LineBreak_get_urgent,
      (setter)LineBreak_set_urgent,
      PyDoc_STR("\
Specify method to handle excessing lines.  \
Following options are available.\n\
: \"RAISE\" : \
Raise an exception.\n\
: \"FORCE\" : \
Force breaking excessing fragment.\n\
: None : \
Won't break excessing fragment.\n\
: callable object : \
See \"User-Defined Breaking Behaviors\".")},
    { "virama_as_joiner",
      (getter)LineBreak_get_virama_as_joiner,
      (setter)LineBreak_set_virama_as_joiner,
      PyDoc_STR("\
Virama sign (\"halant\" in Hindi, \"coeng\" in Khmer) and its succeeding \
letter are not broken.\n\
\"Default\" grapheme cluster defined by [UAX #29] does not contain this \
feature.")},
    { NULL } /* Sentinel */
};

/*
 * Mapping methods
 */

static Py_ssize_t
LineBreak_length(PyObject *self)
{
    return PyMapping_Length(LineBreak_AS_CSTRUCT(self)->stash);
}

static PyObject *
LineBreak_subscript(PyObject *self, PyObject *key)
{
    return PyObject_GetItem(LineBreak_AS_CSTRUCT(self)->stash, key);
}

static int
LineBreak_ass_subscript(PyObject *self, PyObject *key, PyObject *value)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);

    if (value == NULL)
	return PyObject_DelItem(lbobj->stash, key);
    else
	return PyObject_SetItem(lbobj->stash, key, value);
}

static PyMappingMethods LineBreak_as_mapping = {
    LineBreak_length,          /* mp_length */
    LineBreak_subscript,       /* mp_subscript */
    LineBreak_ass_subscript    /* mp_ass_subscript */
};

/*
 * Class-specific methods
 */

PyDoc_STRVAR(LineBreak_copy__doc__,
"\
Copy LineBreak object.");

static PyObject *
LineBreak_copy(PyObject *self, PyObject *args)
{
    return LineBreak_FromCstruct(linebreak_copy(LineBreak_AS_CSTRUCT(self)));
}

PyDoc_STRVAR(LineBreak_wrap__doc__,
"\
S.wrap(text) -> [GCStr]\n\
\n\
Break a Unicode string text and returns list of lines contained in the \
result.  Each item of list is grapheme cluster string (GCStr object).");

static PyObject *
LineBreak_wrap(PyObject *self, PyObject *args)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);
    PyObject *str, *ret;
    unistr_t unistr = {NULL, 0};
    gcstring_t **broken;
    size_t i;

    if (! PyArg_ParseTuple(args, "O", &str))
	return NULL;
    if (unicode_ToCstruct(&unistr, str) == NULL)
	return NULL;

    broken = linebreak_break(lbobj, &unistr);
    free(unistr.str);
    if (PyErr_Occurred()) {
	if (broken != NULL) {
	    size_t i;
	    for (i = 0; broken[i] != NULL; i++)
		gcstring_destroy(broken[i]);
	}
	free(broken);
        return NULL;
    } else if (broken == NULL) {
	if (lbobj->errnum == LINEBREAK_ELONG)
	    PyErr_SetString(LineBreakException, "Excessive line was found");
	else if (lbobj->errnum) {
	    errno = lbobj->errnum;
	    PyErr_SetFromErrno(PyExc_RuntimeError);
	} else
	    PyErr_SetString(PyExc_RuntimeError, "unknown error");
	return NULL;
    }

    ret = PyList_New(0);
    for (i = 0; broken[i] != NULL; i++) {
	PyObject *v;
	if ((v = GCStr_FromCstruct(broken[i])) == NULL) {
	    Py_DECREF(ret);
	    for (; broken[i] != NULL; i++)
		gcstring_destroy(broken[i]);
	    free(broken);
	    return NULL;
	}
	PyList_Append(ret, v);
    }
    free(broken);

    Py_INCREF(ret);
    return ret;
}

static PyMethodDef LineBreak_methods[] = {
    { "__copy__",
      (PyCFunction)LineBreak_copy, METH_NOARGS,
      LineBreak_copy__doc__},
    { "wrap",
      (PyCFunction)LineBreak_wrap, METH_VARARGS,
      LineBreak_wrap__doc__},
    { NULL } /* Sentinel */
};


static PyTypeObject LineBreakType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "_textseg.LineBreak",      /*tp_name*/
    sizeof(LineBreakObject),   /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)LineBreak_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    &LineBreak_as_mapping,     /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "LineBreak objects",       /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    LineBreak_methods,         /* tp_methods */
    0,                         /* tp_members */
    LineBreak_getseters,       /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict: See LineBreak_dict_init() */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)LineBreak_init,  /* tp_init */
    0,                         /* tp_alloc */
    LineBreak_new,             /* tp_new */
};


/**
 ** GCStr class
 **/

/*
 * Constructor & Destructor
 */

static void
GCStr_dealloc(GCStrObject *self)
{
    gcstring_destroy(self->obj);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
GCStr_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    GCStrObject *self;
    if ((self = (GCStrObject *)type->tp_alloc(type, 0)) == NULL)
	return NULL;
    return (PyObject *)self;
}

static int
GCStr_init(GCStrObject *self, PyObject *args, PyObject *kwds)
{
    unistr_t unistr = { NULL, 0 };
    PyObject *pystr = NULL, *pyobj = NULL;
    linebreak_t *lbobj;

    if (! PyArg_ParseTuple(args, "O|O!", &pystr, &LineBreakType, &pyobj))
	return -1;

    if (pyobj == NULL)
	lbobj = NULL;
    else
	lbobj = LineBreak_AS_CSTRUCT(pyobj);

    if (unicode_ToCstruct(&unistr, pystr) == NULL)
	return -1;
    if ((self->obj = gcstring_new(&unistr, lbobj)) == NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);

	free(unistr.str);
	return -1;
    }

    return 0;
}

/*
 * Attribute methods
 */

static PyObject *
GCStr_get_chars(PyObject *self)
{
    return PyInt_FromSsize_t(GCStr_AS_CSTRUCT(self)->len);
}

static PyObject *
GCStr_get_cols(PyObject *self)
{
    return PyInt_FromSsize_t(gcstring_columns(GCStr_AS_CSTRUCT(self)));
}

static PyObject *
GCStr_get_lbc(PyObject *self)
{
    gcstring_t *gcstr = GCStr_AS_CSTRUCT(self);

    if (gcstr->gclen == 0) {
	Py_RETURN_NONE;
    }
    return PyInt_FromLong((long)gcstr->gcstr[0].lbc);
}

static PyObject *
GCStr_get_lbcext(PyObject *self)
{
    gcstring_t *gcstr = GCStr_AS_CSTRUCT(self);

    if (gcstr->gclen == 0) {
	Py_RETURN_NONE;
    }
    return PyInt_FromLong((long)gcstr->gcstr[gcstr->gclen - 1].elbc);

}

static PyGetSetDef GCStr_getseters[] = {
    { "chars",
      (getter)GCStr_get_chars, NULL,
      "Number of Unicode characters grapheme cluster string includes, "
      "i.e. length as Unicode string.",
      NULL },
    { "cols",
      (getter)GCStr_get_cols, NULL,
      "Total number of columns of grapheme clusters "
      "defined by built-in character database.  "
      "For more details see documentations of LineBreak class.",
      NULL },
    { "lbc",
      (getter)GCStr_get_lbc, NULL,
      "Line Breaking Class (See LineBreak class) of the first "
      "character of first grapheme cluster.",
      NULL },
    { "lbcext",
      (getter)GCStr_get_lbcext, NULL,
      "Line Breaking Class (See LineBreak class) of last "
      "grapheme extender of last grapheme cluster.  If there are no "
      "grapheme extenders or its class is CM, value of last grapheme base "
      "will be returned.",
      NULL },
    { NULL } /* Sentinel */
};

/*
 * String methods
 */

static PyObject *
GCStr_compare(PyObject *a, PyObject *b, int op)
{
    gcstring_t *astr, *bstr;
    linebreak_t *lbobj;
    int cmp;

    if (GCStr_Check(a))
	lbobj = GCStr_AS_CSTRUCT(a)->lbobj;
    else if (GCStr_Check(b))
	lbobj = GCStr_AS_CSTRUCT(b)->lbobj;
    else
	lbobj = NULL;

    if ((astr = genericstr_ToCstruct(a, NULL)) == NULL ||
	(bstr = genericstr_ToCstruct(b, NULL)) == NULL) {
	if (! GCStr_Check(a))
	    gcstring_destroy(astr);
	return NULL;
    }
    cmp = gcstring_cmp(astr, bstr);
    if (! GCStr_Check(a))
	gcstring_destroy(astr);
    if (! GCStr_Check(b))
	gcstring_destroy(bstr);

    switch (op) {
    case Py_LT:
	return PyBool_FromLong(cmp < 0);
    case Py_LE:
        return PyBool_FromLong(cmp <= 0);
    case Py_EQ:
        return PyBool_FromLong(cmp == 0);
    case Py_NE:
        return PyBool_FromLong(cmp != 0);
    case Py_GT:
        return PyBool_FromLong(cmp > 0);
    case Py_GE:
        return PyBool_FromLong(cmp >= 0);
    default:
	Py_INCREF(Py_NotImplemented);
	return Py_NotImplemented;
    }
}

/*
 * Sequence methods
 */

static Py_ssize_t
GCStr_length(PyObject *self)
{
    return (Py_ssize_t)GCStr_AS_CSTRUCT(self)->gclen;
}

static PyObject *
GCStr_concat(PyObject *o1, PyObject *o2)
{
    gcstring_t *gcstr1, *gcstr2, *gcstr;
    linebreak_t *lbobj;

    if (GCStr_Check(o1))
	lbobj = GCStr_AS_CSTRUCT(o1)->lbobj;
    else if (GCStr_Check(o2))
	lbobj = GCStr_AS_CSTRUCT(o2)->lbobj;
    else
	lbobj = NULL;

    if ((gcstr1 = genericstr_ToCstruct(o1, lbobj)) == NULL ||
	(gcstr2 = genericstr_ToCstruct(o2, lbobj)) == NULL) {
	if (! GCStr_Check(o1))
	    gcstring_destroy(gcstr1);
	return NULL;
    }
    if ((gcstr = gcstring_concat(gcstr1, gcstr2)) == NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);

        if (! GCStr_Check(o1))
            gcstring_destroy(gcstr1);
	if (! GCStr_Check(o2))
            gcstring_destroy(gcstr2);
	return NULL;
    }

    if (! GCStr_Check(o1))
	gcstring_destroy(gcstr1);
    if (! GCStr_Check(o2))
	gcstring_destroy(gcstr2);
    return GCStr_FromCstruct(gcstr);
}

static PyObject *
GCStr_repeat(PyObject *self, Py_ssize_t count)
{
    gcstring_t *gcstr;
    Py_ssize_t i; /* need signed comparison */

    if ((gcstr = gcstring_new(NULL, GCStr_AS_CSTRUCT(self)->lbobj))
	== NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);
	return NULL;
    }
    for (i = 0; i < count; i++)
      if (gcstring_append(gcstr, GCStr_AS_CSTRUCT(self)) == NULL) {
	    PyErr_SetFromErrno(PyExc_RuntimeError);
	    return NULL;
	}
    return GCStr_FromCstruct(gcstr);
}

static PyObject *
GCStr_item(PyObject *self, Py_ssize_t i)
{
    gcstring_t *gcstr;

    if (GCStr_AS_CSTRUCT(self)->gclen == 0) {
	PyErr_SetString(PyExc_IndexError,
			"GCStr index out of range");
	return NULL;
    }
    if ((gcstr = gcstring_substr(GCStr_AS_CSTRUCT(self), i, 1))
	== NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);
	return NULL;
    }
    if (gcstr->gclen == 0) {
	PyErr_SetString(PyExc_IndexError,
			"GCStr index out of range");
	gcstring_destroy(gcstr);
	return NULL;
    }
    return GCStr_FromCstruct(gcstr);
}

static PyObject *
GCStr_slice(PyObject *self, Py_ssize_t i, Py_ssize_t j)
{
    gcstring_t *gcstr;

    j -= i;
    if (j < 0)
	j = 0;
    if ((gcstr = gcstring_substr(GCStr_AS_CSTRUCT(self), i, j))
	== NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);
	return NULL;
    }
    return GCStr_FromCstruct(gcstr);
}

static int
GCStr_ass_item(PyObject *self, Py_ssize_t i, PyObject *v)
{
    gcstring_t *gcstr, *repl;

    if (GCStr_AS_CSTRUCT(self)->gclen == 0) {
        PyErr_SetString(PyExc_IndexError,
                        "GCStr index out of range");
        return -1;
    }
    if (v == NULL) {
	PyErr_SetString(PyExc_TypeError,
		        "object doesn't support item deletion");
	return -1;
    }
    if ((repl = genericstr_ToCstruct(v, GCStr_AS_CSTRUCT(self)->lbobj))
	== NULL)
	return -1;
    if ((gcstr = gcstring_replace(GCStr_AS_CSTRUCT(self), i, 1, repl))
	== NULL) {
        PyErr_SetFromErrno(PyExc_RuntimeError);

	if (! GCStr_Check(v))
	    gcstring_destroy(repl);
        return -1;
    }

    if (! GCStr_Check(v))
	gcstring_destroy(gcstr);
    return 0;
}

static int
GCStr_ass_slice(PyObject *self, Py_ssize_t i, Py_ssize_t j, PyObject *v)
{
    gcstring_t *gcstr, *repl;
    linebreak_t *lbobj = GCStr_AS_CSTRUCT(self)->lbobj;

    if (v == NULL)
	repl = gcstring_new(NULL, lbobj);
    else if ((repl = genericstr_ToCstruct(v, lbobj)) == NULL)
	return -1;

    j -= i;
    if (j < 0)
        j = 0;
    if ((gcstr = gcstring_replace(GCStr_AS_CSTRUCT(self), i, j, repl))
	== NULL) {
        PyErr_SetFromErrno(PyExc_RuntimeError);

	if (v == NULL || ! GCStr_Check(v))
	    gcstring_destroy(repl);
	return -1;
    }

    if (v == NULL || ! GCStr_Check(v))
	gcstring_destroy(repl);
    return 0;
}

static PyObject *
GCStr_subscript(PyObject *self, PyObject *key)
{
    Py_ssize_t k;

    if (PyInt_Check(key))
	k = PyInt_AsLong(key);
    else if (PyLong_Check(key))
        k = PyInt_AsLong(key);
    else {
	PyErr_SetString(PyExc_TypeError,
		     "GCStr indices must be integers");
	return NULL;
    }
    return GCStr_item(self, k);
}

static int
GCStr_ass_subscript(PyObject *self, PyObject *key, PyObject *v)
{
    Py_ssize_t k;

    if (PyInt_Check(key))
        k = PyInt_AsLong(key);
    else if (PyLong_Check(key))
        k = PyInt_AsLong(key);
    else {
        PyErr_SetString(PyExc_TypeError,
			"GCStr indices must be integers");
        return -1;
    }
    return GCStr_ass_item(self, k, v);
}

static PySequenceMethods GCStr_as_sequence = {
    GCStr_length,           /* sq_length */
    GCStr_concat,           /* sq_concat */
    GCStr_repeat,           /* sq_repeat */
    GCStr_item,             /* sq_item */
    GCStr_slice,            /* sq_slice: unused by 3.0 (?) */
    GCStr_ass_item,         /* sq_ass_item */
    GCStr_ass_slice,        /* sq_ass_slice: unused by 3.0 (?) */
    NULL,                      /* sq_contains */
    NULL,                      /* sq_inplace_concat */
    NULL                       /* sq_inplace_repeat */
};

static PyMappingMethods GCStr_as_mapping = {
    GCStr_length,           /* mp_length */
    GCStr_subscript,        /* mp_subscript */
    GCStr_ass_subscript     /* mp_ass_subscript */
};

/*
 * Class specific methods
 */

PyDoc_STRVAR(GCStr_copy__doc__,
"\
Create a copy of GCStr object.");

static PyObject *
GCStr_copy(PyObject *self, PyObject *args)
{
    return GCStr_FromCstruct(gcstring_copy(GCStr_AS_CSTRUCT(self)));
}

PyDoc_STRVAR(GCStr_flag__doc__,
"S.flag(offset [, value]) => int\n\
\n\
Get and optionally set flag value of offset-th grapheme cluster.  \
Flag value is an non-zero integer not greater than 255 and initially is 0.\n\
Predefined flag values are:\n\
: ALLOW_BEFORE : \
Allow line breaking just before this grapheme cluster.\n\
: PROHIBIT_BEFORE : \
Prohibit line breaking just before this grapheme cluster.");

static PyObject *
GCStr_flag(PyObject *self, PyObject *args)
{
    gcstring_t *gcstr = GCStr_AS_CSTRUCT(self);
    Py_ssize_t i;
    long v = -1L;
    PyObject *ret;

    if (! PyArg_ParseTuple(args, ARG_FORMAT_SSIZE_T "|i", &i, &v))
	return NULL;
    if (i < 0 || gcstr->gclen <= i) {
	Py_RETURN_NONE;
    }
    ret = PyInt_FromLong((unsigned long)gcstr->gcstr[i].flag);
    if (0 < v)
	gcstr->gcstr[i].flag = (propval_t)v;

    return ret;
}

/* FIXME: often unavailable */
static PyObject *
GCStr_radd(PyObject *self, PyObject *args)
{
    PyObject *pyobj;

    if (! PyArg_ParseTuple(args, "O", &pyobj))
        return NULL;
    return GCStr_concat(pyobj, self);
}

static PyObject *
GCStr_unicode(PyObject *self, PyObject *args)
{
    return unicode_FromCstruct((unistr_t *)(GCStr_AS_CSTRUCT(self)));
}

static PyMethodDef GCStr_methods[] = {
    { "__copy__",
      GCStr_copy, METH_NOARGS,
      GCStr_copy__doc__},
    { "flag",
      GCStr_flag, METH_VARARGS,
      GCStr_flag__doc__},
    { "__radd__",
      GCStr_radd, METH_VARARGS,
      "x.__radd__(y) <==> y+x" },
    { "__unicode__",
      GCStr_unicode, METH_NOARGS,
      "x.__unicode__() <==> unicode(x)\n\
      Convert grapheme cluster string to Unicode string." },
    { NULL } /* Sentinel */
};


static PyTypeObject GCStrType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "_textseg.GCStr",          /*tp_name*/
    sizeof(GCStrObject),       /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)GCStr_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    &GCStr_as_sequence,        /*tp_as_sequence*/
    &GCStr_as_mapping,         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "GCStr objects",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    GCStr_compare,             /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    GCStr_methods,             /* tp_methods */
    0,                         /* tp_members */
    GCStr_getseters,           /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)GCStr_init,      /* tp_init */
    0,                         /* tp_alloc */
    GCStr_new,                 /* tp_new */
};


/**
 ** Module definitions
 **/

static PyMethodDef module_methods[] = {
    { NULL } /* Sentinel */
};

/*
 * Initialize class properties
 */

static int
LineBreak_dict_init()
{
    PyObject *dict;
    size_t i;
    char name[8];

    if ((dict = PyDict_New()) == NULL)
	return -1;

    /* add "lbc??" */
    strcpy(name, "lbc");
    for (i = 0; linebreak_propvals_LB[i] != NULL; i++) {
	strcpy(name + 3, linebreak_propvals_LB[i]);
	if ((PyDict_SetItemString(dict, name, PyInt_FromLong((long)i))) != 0) {
	    Py_DECREF(dict);
	    return -1;
	}
    }
    /* add "eaw??" */
    strcpy(name, "eaw");
    for (i = 0; linebreak_propvals_EA[i] != NULL; i++) {
        strcpy(name + 3, linebreak_propvals_EA[i]);
        if ((PyDict_SetItemString(dict, name, PyInt_FromLong((long)i))) != 0) {
	    Py_DECREF(dict);
	    return -1;
	}
    }

    LineBreakType.tp_dict = dict;
    return 0;
}

/*
 * Initialize classes
 */

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
init_textseg(void) 
{
    PyObject *m, *c;
    size_t i;
    char name[8];

    if (LineBreak_dict_init() != 0)
	return;
    if (PyType_Ready(&LineBreakType) < 0)
        return;
    if (PyType_Ready(&GCStrType) < 0)
        return;

    if ((m = Py_InitModule3("_textseg", module_methods,
                            "Module for text segmentation.")) == NULL)
        return;

    LineBreakException = PyErr_NewException("_textseg.Exception", NULL, NULL);
    Py_INCREF(LineBreakException);

    Py_INCREF(&LineBreakType);
    PyModule_AddObject(m, "LineBreak", (PyObject *)&LineBreakType);
    Py_INCREF(&GCStrType);
    PyModule_AddObject(m, "GCStr", (PyObject *)&GCStrType);

    /* add module constants */

    if ((c = Py_InitModule("_textseg.Consts", module_methods)) == NULL)
	return;

    PyModule_AddStringConstant(c, "unicode_version",
			       (char *)linebreak_unicode_version);
    PyModule_AddStringConstant(c, "sombok_version", SOMBOK_VERSION);
    strcpy(name, "lbc");
    for (i = 0; linebreak_propvals_LB[i] != NULL; i++) {
        strcpy(name + 3, linebreak_propvals_LB[i]);
        PyModule_AddIntConstant(c, name, (long)i);
    }
    strcpy(name, "eaw");
    for (i = 0; linebreak_propvals_EA[i] != NULL; i++) {
        strcpy(name + 3, linebreak_propvals_EA[i]);
        PyModule_AddIntConstant(c, name, (long)i);
    }
    Py_INCREF(c);
    PyModule_AddObject(m, "Consts", c);
}

