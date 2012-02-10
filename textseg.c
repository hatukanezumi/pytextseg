/*
 * textseg.c - Implementation of _textseg module for Python.
 *
 * Copyright (C) 2012 by Hatuka*nezumi - IKEDA Soji.
 *
 * This file is part of the pytextseg package.  This program is free
 * software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version.  This program is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * COPYING file for more details.
 */

#include <sombok.h>
#include <Python.h>
#include "structmember.h"
#include "python_compat.h"

/***
 *** Objects
 ***/

typedef struct LineBreakObject {
    PyObject_HEAD
    linebreak_t * obj;
} LineBreakObject;

typedef struct GCStrObject {
    PyObject_HEAD
    gcstring_t * obj;
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
 * @note Unicode buffer will be copied.
 */
static unistr_t *
unicode_ToCstruct(unistr_t * unistr, PyObject * pyobj)
{
    size_t i;
    unichar_t *uni;
    PyObject *pystr;
    Py_ssize_t len;
    void *ucs;
#ifdef OLDAPI_Py_UNICODE_NARROW
    unichar_t uc1, uc2;
    size_t unilen;
    int pass;
#else				/* OLDAPI_Py_UNICODE_NARROW */
    int kind;
#endif				/* OLDAPI_Py_UNICODE_NARROW */

    if (pyobj == NULL)
	return NULL;
    else if (PyUnicode_Check(pyobj))
	pystr = pyobj;
    else if ((pystr = PyObject_Unicode(pyobj)) == NULL)
	return NULL;

    if (PyUnicode_READY(pystr) != 0) {
	if (!PyUnicode_Check(pyobj)) {
	    Py_DECREF(pystr);
	}
	return NULL;
    }
    if ((len = PyUnicode_GET_LENGTH(pystr)) == 0) {
	if (!PyUnicode_Check(pyobj)) {
	    Py_DECREF(pystr);
	}
	unistr->str = NULL;
	unistr->len = 0;
	return unistr;
    }
    ucs = PyUnicode_DATA(pystr);

#ifdef OLDAPI_Py_UNICODE_NARROW
    for (pass = 1; pass <= 2; pass++) {
	for (i = 0, unilen = 0; i < len; i++, unilen++) {
	    uc1 = (unichar_t) ((Py_UCS2 *) ucs)[i];
	    if (Py_UNICODE_IS_HIGH_SURROGATE(uc1) &&
		i + 1 < len &&
		(uc2 = (unichar_t) ((Py_UCS2 *) ucs)[i + 1]) &&
		Py_UNICODE_IS_LOW_SURROGATE(uc2)) {
		if (pass == 2)
		    uni[unilen] = Py_UNICODE_JOIN_SURROGATES(uc1, uc2);
		i++;
	    } else if (pass == 2)
		uni[unilen] = uc1;
	}

	if (pass == 1) {
	    if ((uni = malloc(sizeof(unichar_t) * (unilen + 1))) == NULL) {
		PyErr_SetFromErrno(PyExc_RuntimeError);

		if (!PyUnicode_Check(pyobj)) {
		    Py_DECREF(pystr);
		}
		return NULL;
	    }
	    uni[unilen] = 0;
	}
    }

    unistr->str = uni;
    unistr->len = unilen;
#else				/* OLDAPI_Py_UNICODE_NARROW */
    kind = PyUnicode_KIND(pystr);

    if ((uni = malloc(sizeof(unichar_t) * (len + 1))) == NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);

	if (!PyUnicode_Check(pyobj)) {
	    Py_DECREF(pystr);
	}
	return NULL;
    }
    uni[len] = 0;

    if ((kind == PyUnicode_1BYTE_KIND &&
	 sizeof(Py_UCS1) == sizeof(unichar_t)) ||
	(kind == PyUnicode_2BYTE_KIND &&
	 sizeof(Py_UCS2) == sizeof(unichar_t)) ||
	(kind == PyUnicode_4BYTE_KIND &&
	 sizeof(Py_UCS4) == sizeof(unichar_t)))
	memcpy(uni, ucs, sizeof(unichar_t) * len);
    else if (kind == PyUnicode_1BYTE_KIND)
	for (i = 0; i < len; i++)
	    uni[i] = (unichar_t) ((Py_UCS1 *) ucs)[i];
    else if (kind == PyUnicode_2BYTE_KIND)
	for (i = 0; i < len; i++)
	    uni[i] = (unichar_t) ((Py_UCS2 *) ucs)[i];
    else if (kind == PyUnicode_4BYTE_KIND)
	for (i = 0; i < len; i++)
	    uni[i] = (unichar_t) ((Py_UCS4 *) ucs)[i];
    else {
	PyErr_SetString(PyExc_SystemError, "invalid kind.");

	free(uni);
	return NULL;
    }

    unistr->str = uni;
    unistr->len = len;
#endif				/* OLDAPI_Py_UNICODE_NARROW */

    if (!PyUnicode_Check(pyobj)) {
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
unicode_FromCstruct(unistr_t * unistr)
{
    size_t i, unilen;
    void *ucs;
    PyObject *ret;
#ifdef OLDAPI_Py_UNICODE_NARROW
    size_t len;
    unichar_t unichar;
    int pass;
#else				/* OLDAPI_Py_UNICODE_NARROW */
    int kind;
#endif				/* OLDAPI_Py_UNICODE_NARROW */

    if (unistr->str == NULL || unistr->len == 0) {
	Py_UCS2 buf[1] = { 0 };
	return PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, buf, 0);
    }

    unilen = unistr->len;

#ifdef OLDAPI_Py_UNICODE_NARROW
    for (pass = 1; pass <= 2; pass++) {
	for (i = 0, len = 0; i < unilen; i++, len++) {
	    unichar = unistr->str[i];

	    if ((unichar_t) 0x10000 <= unichar) {
		if (pass == 2) {
		    ((Py_UCS2 *) ucs)[len] =
			(Py_UCS2) Py_UNICODE_HIGH_SURROGATE(unichar);
		    ((Py_UCS2 *) ucs)[len + 1] =
			(Py_UCS2) Py_UNICODE_LOW_SURROGATE(unichar);
		}
		len++;
	    } else if (pass == 2)
		((Py_UCS2 *) ucs)[len] = (Py_UCS2) unichar;
	}

	if (pass == 1) {
	    if ((ucs = PyMem_Malloc(sizeof(Py_UCS2) * len)) == NULL)
		return NULL;
	}
    }
    ret = PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, ucs, len);
    PyMem_Free(ucs);
#else				/* OLDAPI_Py_UNICODE_NARROW */
    unistr_KIND(kind, unistr->str, unichar_t, unilen);

    if ((kind == PyUnicode_1BYTE_KIND &&
	 sizeof(Py_UCS1) == sizeof(unichar_t)) ||
	(kind == PyUnicode_2BYTE_KIND &&
	 sizeof(Py_UCS2) == sizeof(unichar_t)) ||
	(kind == PyUnicode_4BYTE_KIND &&
	 sizeof(Py_UCS4) == sizeof(unichar_t)))
	ret = PyUnicode_FromKindAndData(kind, unistr->str, unilen);
    else {
	if (kind == PyUnicode_1BYTE_KIND) {
	    if ((ucs = PyMem_Malloc(sizeof(Py_UCS1) * unilen)) == NULL)
		return NULL;
	    for (i = 0; i < unilen; i++)
		((Py_UCS1 *) ucs)[i] = (Py_UCS1) unistr->str[i];
	} else if (kind == PyUnicode_2BYTE_KIND) {
	    if ((ucs = PyMem_Malloc(sizeof(Py_UCS2) * unilen)) == NULL)
		return NULL;
	    for (i = 0; i < unilen; i++)
		((Py_UCS2 *) ucs)[i] = (Py_UCS2) unistr->str[i];
	} else if (kind == PyUnicode_4BYTE_KIND) {
	    if ((ucs = PyMem_Malloc(sizeof(Py_UCS4) * unilen)) == NULL)
		return NULL;
	    for (i = 0; i < unilen; i++)
		((Py_UCS4 *) ucs)[i] = (Py_UCS4) unistr->str[i];
	} else {
	    PyErr_SetString(PyExc_SystemError, "invalid kind.");
	    return NULL;
	}
	ret = PyUnicode_FromKindAndData(kind, ucs, unilen);
	PyMem_Free(ucs);
    }
#endif				/* OLDAPI_Py_UNICODE_NARROW */

    return ret;
}

/**
 * Convert LineBreakObject to linebreak object.
 */
#define LineBreak_AS_CSTRUCT(pyobj) \
    (((LineBreakObject *)(pyobj))->obj)

linebreak_t *
LineBreak_AsCstruct(PyObject * pyobj)
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
LineBreak_FromCstruct(linebreak_t * obj)
{
    LineBreakObject *self;

    if ((self =
	 (LineBreakObject *) (LineBreakType.tp_alloc(&LineBreakType, 0)))
	== NULL)
	return NULL;

    self->obj = obj;
    return (PyObject *) self;
}

/**
 * Convert GCStrObject to gcstring object.
 */
#define GCStr_AS_CSTRUCT(pyobj) \
    (((GCStrObject *)(pyobj))->obj)

gcstring_t *
GCStr_AsCstruct(PyObject * pyobj)
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
GCStr_FromCstruct(gcstring_t * obj)
{
    GCStrObject *self;

    if ((self =
	 (GCStrObject *) (GCStrType.tp_alloc(&GCStrType, 0))) == NULL)
	return NULL;

    self->obj = obj;
    return (PyObject *) self;
}

/**
 * Convert Python object, Unicode string or GCStrObject to
 * grapheme cluster string.
 * If error occurred, exception will be raised and NULL will be returned.
 * @note if pyobj was GCStrObject, returned object will be original
 * object (not a copy of it).  Otherwise, _new_ object will be returned.
 */

static gcstring_t *
genericstr_ToCstruct(PyObject * pyobj, linebreak_t * lbobj)
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

/**
 * Convert Python object, Byte string, Unicode string or GCStrObject to 
 * NULL-termineted string.
 * If error occurred, exception will be raised and NULL will be returned.
 * @note string buffer must be free()'ed by caller.
 */

static char *
genericstr_ToString(PyObject * pyobj)
{
    PyObject *pystr;
    char *ret;

    if (pyobj == NULL)
	return NULL;
    else if (PyBytes_Check(pyobj)) {
	if ((ret = strdup(PyBytes_AsString(pyobj))) == NULL) {
	    PyErr_SetFromErrno(PyExc_RuntimeError);
	    return NULL;
	}
    } else if (PyUnicode_Check(pyobj)) {
	if ((pystr = PyUnicode_AsEncodedString(pyobj, NULL, NULL)) == NULL)
	    return NULL;
	if ((ret = strdup(PyBytes_AsString(pystr))) == NULL) {
	    PyErr_SetFromErrno(PyExc_RuntimeError);

	    Py_DECREF(pystr);
	    return NULL;
	}
	Py_DECREF(pystr);
    } else {
	if ((pyobj = PyObject_Unicode(pyobj)) == NULL)
	    return NULL;
	if ((pystr = PyUnicode_AsEncodedString(pyobj, NULL, NULL)) == NULL) {
	    Py_DECREF(pyobj);
	    return NULL;
	}
	Py_DECREF(pyobj);
	if ((ret = strdup(PyBytes_AsString(pystr))) == NULL) {
	    PyErr_SetFromErrno(PyExc_RuntimeError);

	    Py_DECREF(pystr);
	    return NULL;
	}
	Py_DECREF(pystr);
    }
    return ret;
}

/***
 *** Other utilities
 ***/

/*
 * Do regex match once then returns offset and length.
 */
static void
do_re_search_once(PyObject * rx, unistr_t * str, unistr_t * text)
{
    PyObject *strobj, *matchobj, *func_search, *args, *pyobj;
    Py_ssize_t pos, endpos, start, end;
#ifdef OLDAPI_Py_UNICODE_NARROW
    size_t i;
#endif				/* OLDAPI_Py_UNICODE_NARROW */

#ifdef OLDAPI_Py_UNICODE_NARROW
    start = str->str - text->str;
    end = start + str->len;
    for (i = 0, pos = 0; i < start; i++, pos++)
	if (0x10000 <= text->str[i])
	    pos++;
    for (endpos = pos; i < end; i++, endpos++)
	if (0x10000 <= text->str[i])
	    endpos++;
#else				/* OLDAPI_Py_UNICODE_NARROW */
    pos = str->str - text->str;
    endpos = pos + str->len;
#endif				/* OLDAPI_Py_UNICODE_NARROW */

    if ((strobj = unicode_FromCstruct(text)) == NULL) {
	str->str = NULL;
	return;
    }
    if ((func_search = PyObject_GetAttrString(rx, "search")) == NULL) {
	Py_DECREF(strobj);
	str->str = NULL;
	return;
    }
    if (!PyCallable_Check(func_search)) {
	PyErr_SetString(PyExc_ValueError, "object is not callable");

	Py_DECREF(strobj);
	Py_DECREF(func_search);
	str->str = NULL;
	return;
    }
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
	    Py_DECREF(matchobj);
	    str->str = NULL;
	    return;
	}
	start = PyLong_AsLong(pyobj);
	Py_DECREF(pyobj);

	if ((pyobj = PyObject_CallMethod(matchobj, "end", NULL)) == NULL) {
	    Py_DECREF(matchobj);
	    str->str = NULL;
	    return;
	}
	end = PyLong_AsLong(pyobj);
	Py_DECREF(pyobj);

#ifdef OLDAPI_Py_UNICODE_NARROW
	for (i = 0, pos = 0; i < start; i++, pos++)
	    if (0x10000 <= text->str[pos])
		i++;
	for (endpos = pos; i < end; i++, endpos++)
	    if (0x10000 <= text->str[endpos])
		i++;
	str->str = text->str + pos;
	str->len = endpos - pos;
#else				/* OLDAPI_Py_UNICODE_NARROW */
	str->str = text->str + start;
	str->len = end - start;
#endif				/* OLDAPI_Py_UNICODE_NARROW */
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
ref_func(PyObject * obj, int datatype, int d)
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
prep_func(linebreak_t * lbobj, void *data, unistr_t * str, unistr_t * text)
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
    linebreak_incref(lbobj);	/* prevent destruction */
    PyTuple_SetItem(args, 0, LineBreak_FromCstruct(lbobj));
    PyTuple_SetItem(args, 1, unicode_FromCstruct(str));	/* FIXME: err */
    pyret = PyObject_CallObject(func, args);
    Py_DECREF(args);
    if (PyErr_Occurred()) {
	if (!lbobj->errnum)
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
	    pyobj = PyList_GetItem(pyret, i);	/* borrowed ref. */
	    if (pyobj == Py_None)
		continue;
	    else if (GCStr_Check(pyobj))
		gcstr = gcstring_copy(GCStr_AS_CSTRUCT(pyobj));
	    else
		gcstr = genericstr_ToCstruct(pyobj, lbobj);
	    if (gcstr == NULL) {
		if (!lbobj->errnum)
		    lbobj->errnum = errno ? errno : LINEBREAK_EEXTN;

		Py_DECREF(pyret);
		return NULL;
	    }

	    for (j = 0; j < gcstr->gclen; j++) {
		if (gcstr->gcstr[j].flag &
		    (LINEBREAK_FLAG_ALLOW_BEFORE |
		     LINEBREAK_FLAG_PROHIBIT_BEFORE))
		    continue;
		if (0 < i && j == 0)
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

    if (pyret == Py_None) {
	Py_DECREF(pyret);
	return NULL;
    }

    if (GCStr_Check(pyret))
	ret = gcstring_copy(GCStr_AS_CSTRUCT(pyret));
    else
	ret = genericstr_ToCstruct(pyret, lbobj);
    if (ret == NULL) {
	if (!lbobj->errnum)
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
format_func(linebreak_t * lbobj, linebreak_state_t action, gcstring_t * str)
{
    PyObject *func, *args, *pyret;
    char *actionstr;
    gcstring_t *gcstr;

    func = (PyObject *) lbobj->format_data;
    if (func == NULL)
	return NULL;

    if (action <= LINEBREAK_STATE_NONE || LINEBREAK_STATE_MAX <= action)
	return NULL;
    actionstr = linebreak_states[(size_t) action];

    args = PyTuple_New(3);
    linebreak_incref(lbobj);	/* prevent destruction */
    PyTuple_SetItem(args, 0, LineBreak_FromCstruct(lbobj));
    PyTuple_SetItem(args, 1, PyString_FromString(actionstr));
    PyTuple_SetItem(args, 2, GCStr_FromCstruct(gcstring_copy(str)));
    pyret = PyObject_CallObject(func, args);
    Py_DECREF(args);

    if (PyErr_Occurred()) {
	if (!lbobj->errnum)
	    lbobj->errnum = LINEBREAK_EEXTN;
	if (pyret != NULL) {
	    Py_DECREF(pyret);
	}
	return NULL;
    }
    if (pyret == NULL)
	return NULL;

    if (pyret == Py_None) {
	Py_DECREF(pyret);
	return NULL;
    }

    if (GCStr_Check(pyret))
	gcstr = gcstring_copy(GCStr_AS_CSTRUCT(pyret));
    else if ((gcstr = genericstr_ToCstruct(pyret, lbobj)) == NULL) {
	if (!lbobj->errnum)
	    lbobj->errnum = errno ? errno : ENOMEM;
	PyErr_SetFromErrno(PyExc_RuntimeError);
	Py_DECREF(pyret);
	return NULL;
    }
    Py_DECREF(pyret);
    return gcstr;
}

/*
 * Call sizing function
 */
static double
sizing_func(linebreak_t * lbobj, double len,
	    gcstring_t * pre, gcstring_t * spc, gcstring_t * str)
{
    PyObject *func, *args, *pyret;
    double ret;

    func = (PyObject *) lbobj->sizing_data;
    if (func == NULL)
	return -1.0;

    args = PyTuple_New(5);
    linebreak_incref(lbobj);	/* prevent destruction. */
    PyTuple_SetItem(args, 0, LineBreak_FromCstruct(lbobj));
    PyTuple_SetItem(args, 1, PyInt_FromSsize_t(len));
    PyTuple_SetItem(args, 2, GCStr_FromCstruct(gcstring_copy(pre)));
    PyTuple_SetItem(args, 3, GCStr_FromCstruct(gcstring_copy(spc)));
    PyTuple_SetItem(args, 4, GCStr_FromCstruct(gcstring_copy(str)));
    pyret = PyObject_CallObject(func, args);
    Py_DECREF(args);

    if (PyErr_Occurred()) {
	if (!lbobj->errnum)
	    lbobj->errnum = LINEBREAK_EEXTN;
	if (pyret != NULL) {
	    Py_DECREF(pyret);
	}
	return -1.0;
    }

    ret = PyFloat_AsDouble(pyret);
    if (PyErr_Occurred()) {
	if (!lbobj->errnum)
	    lbobj->errnum = LINEBREAK_EEXTN;
	Py_DECREF(pyret);
	return -1.0;
    }
    Py_DECREF(pyret);

    return ret;
}

/*
 * Call urgent breaking function
 */
static gcstring_t *
urgent_func(linebreak_t * lbobj, gcstring_t * str)
{
    PyObject *func, *args, *pyret, *pyobj;
    size_t count, i;
    gcstring_t *gcstr, *ret;

    func = (PyObject *) lbobj->urgent_data;
    if (func == NULL)
	return NULL;

    args = PyTuple_New(2);
    linebreak_incref(lbobj);	/* prevent destruction. */
    PyTuple_SetItem(args, 0, LineBreak_FromCstruct(lbobj));
    PyTuple_SetItem(args, 1, GCStr_FromCstruct(gcstring_copy(str)));
    pyret = PyObject_CallObject(func, args);
    Py_DECREF(args);

    if (PyErr_Occurred()) {
	if (!lbobj->errnum)
	    lbobj->errnum = LINEBREAK_EEXTN;
	if (pyret != NULL) {
	    Py_DECREF(pyret);
	}
	return NULL;
    }
    if (pyret == NULL)
	return NULL;

    if (pyret == Py_None) {
	Py_DECREF(pyret);
	return NULL;
    }

    if (!PyList_Check(pyret)) {
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
	pyobj = PyList_GetItem(pyret, i);	/* borrowed ref. */
	if (pyobj == Py_None)
	    continue;
	else if (GCStr_Check(pyobj))
	    gcstr = gcstring_copy(GCStr_AS_CSTRUCT(pyobj));
	else
	    gcstr = genericstr_ToCstruct(pyobj, lbobj);
	if (gcstr == NULL) {
	    if (!lbobj->errnum)
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
LineBreak_dealloc(LineBreakObject * self)
{
    linebreak_destroy(self->obj);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
LineBreak_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
    LineBreakObject *self;
    PyObject *stash;

    if ((self = (LineBreakObject *) type->tp_alloc(type, 0)) == NULL)
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
    Py_DECREF(stash);		/* fixup */

    return (PyObject *) self;
}

static PyGetSetDef LineBreak_getseters[];

static int
LineBreak_init(LineBreakObject * self, PyObject * args, PyObject * kwds)
{
    PyObject *key, *value;
    Py_ssize_t pos;
    PyGetSetDef *getset;
    char *keystr;

    if (kwds == NULL)
	return 0;

    pos = 0;
    while (PyDict_Next(kwds, &pos, &key, &value)) {
	if ((keystr = genericstr_ToString(key)) == NULL)
	    return -1;
	for (getset = LineBreak_getseters; getset->name != NULL; getset++) {
	    if (getset->set == NULL)
		continue;
	    if (strcmp(getset->name, keystr) != 0)
		continue;
	    if ((getset->set) ((PyObject *) self, value, NULL) != 0) {
		free(keystr);
		return -1;
	    }
	    break;
	}
	free(keystr);
	if (getset->name == NULL) {
	    PyErr_SetString(PyExc_ValueError, "invalid argument");
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
LineBreak_get_charmax(PyObject * self)
{
    return PyInt_FromSsize_t(LineBreak_AS_CSTRUCT(self)->charmax);
}

static PyObject *
LineBreak_get_width(PyObject * self)
{
    return PyFloat_FromDouble(LineBreak_AS_CSTRUCT(self)->colmax);
}

static PyObject *
LineBreak_get_minwidth(PyObject * self)
{
    return PyFloat_FromDouble(LineBreak_AS_CSTRUCT(self)->colmin);
}

_get_Boolean(complex_breaking, LINEBREAK_OPTION_COMPLEX_BREAKING)

_get_Boolean(eastasian_context, LINEBREAK_OPTION_EASTASIAN_CONTEXT)

static PyObject *
LineBreak_get_eaw(PyObject * self)
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
		p = PyInt_FromLong((signed long) lbobj->map[i].eaw);
		for (c = lbobj->map[i].beg; c <= lbobj->map[i].end; c++)
		    PyDict_SetItem(val, PyInt_FromLong(c), p);
	    }
    }
    return val;
}

static PyObject *
LineBreak_get_format(PyObject * self)
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
	val = (PyObject *) lbobj->format_data;
	Py_INCREF(val);
    } else {
	PyErr_Format(PyExc_RuntimeError,
		     "Internal error.  Ask developer.");
	return NULL;
    }
    return val;
}

_get_Boolean(hangul_as_al, LINEBREAK_OPTION_HANGUL_AS_AL)

static PyObject *
LineBreak_get_lbc(PyObject * self)
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
		p = PyInt_FromLong((signed long) lbobj->map[i].lbc);
		for (c = lbobj->map[i].beg; c <= lbobj->map[i].end; c++)
		    PyDict_SetItem(val, PyInt_FromLong(c), p);
	    }
    }
    return val;
}

_get_Boolean(legacy_cm, LINEBREAK_OPTION_LEGACY_CM)

static PyObject *
LineBreak_get_newline(PyObject * self)
{
    return unicode_FromCstruct(&(LineBreak_AS_CSTRUCT(self)->newline));
}

static PyObject *
LineBreak_get_prep(PyObject * self)
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
LineBreak_get_sizing(PyObject * self)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);
    PyObject *val;

    if (lbobj->sizing_func == NULL)
	val = Py_None;
    else if (lbobj->sizing_func == linebreak_sizing_UAX11 ||
	     lbobj->sizing_func == sizing_func)
	val = (PyObject *) lbobj->sizing_data;
    else {
	PyErr_Format(PyExc_RuntimeError, "XXX");
	return NULL;
    }
    Py_INCREF(val);

    return val;
}

static PyObject *
LineBreak_get_urgent(PyObject * self)
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
	val = (PyObject *) lbobj->urgent_data;
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
_update_maps(linebreak_t * lbobj, PyObject * dict, int maptype)
{
    Py_ssize_t pos, i;
    PyObject *key, *value, *item;
    propval_t p;
    unichar_t c;
    int kind;

    if (dict == NULL) {
	PyErr_Format(PyExc_NotImplementedError,
		     "Can not delete attribute");
	return -1;
    }
    if (!PyDict_Check(dict)) {
	PyErr_Format(PyExc_TypeError,
		     "attribute must be dictionary, not %200s",
		     Py_TYPE(dict)->tp_name);
	return -1;
    }

    pos = 0;
    while (PyDict_Next(dict, &pos, &key, &value)) {
	if (PyInt_Check(value))
	    p = (propval_t) PyInt_AsLong(value);
	else if (PyLong_Check(value))
	    p = (propval_t) PyLong_AsLong(value);
	else {
	    PyErr_Format(PyExc_ValueError,
			 "value of map must be integer, not %200s",
			 Py_TYPE(value)->tp_name);
	    return -1;
	}

	if (PySequence_Check(key)) {
	    for (i = 0; (item = PySequence_GetItem(key, i)) != NULL; i++) {
		if (PyUnicode_Check(item)) {
		    if (PyUnicode_READY(item) != 0)
			return -1;
		    if (PyUnicode_GET_LENGTH(item) == 0)
			continue;

		    kind = PyUnicode_KIND(item);
		    if (kind == PyUnicode_1BYTE_KIND)
			c = (unichar_t)
			    *((Py_UCS1 *) PyUnicode_DATA(item));
		    else if (kind == PyUnicode_2BYTE_KIND)
			c = (unichar_t)
			    *((Py_UCS2 *) PyUnicode_DATA(item));
		    else if (kind == PyUnicode_4BYTE_KIND)
			c = (unichar_t)
			    *((Py_UCS4 *) PyUnicode_DATA(item));
		    else {
			PyErr_SetString(PyExc_SystemError, "invalid kind.");
			return -1;
		    }
#ifdef OLDAPI_Py_UNICODE_NARROW
		    /* FIXME: Add surrogate pair support. */
#endif				/* OLDAPI_Py_UNICODE_NARROW */
		} else if (PyInt_Check(item))
		    c = (unichar_t) PyInt_AsLong(item);
		else if (PyLong_Check(item))
		    c = (unichar_t) PyLong_AsLong(item);
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
	    continue;		/* while (PyDict_Next( ... */
	} else if (PyInt_Check(key))
	    c = (unichar_t) PyInt_AsLong(key);
	else if (PyLong_Check(key))
	    c = (unichar_t) PyLong_AsLong(key);
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
LineBreak_set_charmax(PyObject * self, PyObject * value, void *closure)
{
    long ival;

    if (value == NULL) {
	PyErr_Format(PyExc_NotImplementedError,
		     "Can not delete attribute");
	return -1;
    }
    if (PyInt_Check(value))
	ival = (long) PyInt_AsLong(value);
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
LineBreak_set_width(PyObject * self, PyObject * value, void *closure)
{
    double dval;

    if (value == NULL) {
	PyErr_Format(PyExc_NotImplementedError,
		     "Can not delete attribute");
	return -1;
    }
    if (PyInt_Check(value))
	dval = (double) PyInt_AsLong(value);
    else if (PyLong_Check(value))
	dval = (double) PyInt_AsLong(value);
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
LineBreak_set_minwidth(PyObject * self, PyObject * value, void *closure)
{
    double dval;

    if (value == NULL) {
	PyErr_Format(PyExc_NotImplementedError,
		     "Can not delete attribute");
	return -1;
    }
    if (PyInt_Check(value))
	dval = (double) PyInt_AsLong(value);
    else if (PyLong_Check(value))
	dval = (double) PyInt_AsLong(value);
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
LineBreak_set_eaw(PyObject * self, PyObject * value, void *closure)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);

    if (value == Py_None)
	linebreak_clear_eawidth(lbobj);
    else
	return _update_maps(lbobj, value, 1);
    return 0;
}

static int
LineBreak_set_format(PyObject * self, PyObject * value, void *closure)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);

    if (value == NULL)
	linebreak_set_format(lbobj, NULL, NULL);
    else if (value == Py_None)
	linebreak_set_format(lbobj, NULL, NULL);
    else if (PyString_Check(value) || PyUnicode_Check(value)) {
	char *str;
	if ((str = genericstr_ToString(value)) == NULL)
	    return -1;

	if (strcasecmp(str, "SIMPLE") == 0)
	    linebreak_set_format(lbobj, linebreak_format_SIMPLE, NULL);
	else if (strcasecmp(str, "NEWLINE") == 0)
	    linebreak_set_format(lbobj, linebreak_format_NEWLINE, NULL);
	else if (strcasecmp(str, "TRIM") == 0)
	    linebreak_set_format(lbobj, linebreak_format_TRIM, NULL);
	else {
	    PyErr_Format(PyExc_ValueError,
			 "unknown attribute value, %200s", str);

	    free(str);
	    return -1;
	}
	free(str);
    } else if (PyFunction_Check(value))
	linebreak_set_format(lbobj, format_func, (void *) value);
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
LineBreak_set_lbc(PyObject * self, PyObject * value, void *closure)
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
LineBreak_set_newline(PyObject * self, PyObject * value, void *closure)
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
LineBreak_set_prep(PyObject * self, PyObject * value, void *closure)
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
		if ((str = genericstr_ToString(item)) == NULL)
		    break;
		else if (strcasecmp(str, "BREAKURI") == 0)
		    linebreak_add_prep(lbobj, linebreak_prep_URIBREAK,
				       Py_True);
		else if (strcasecmp(str, "NONBREAKURI") == 0)
		    linebreak_add_prep(lbobj, linebreak_prep_URIBREAK,
				       NULL);
		else {
		    PyErr_Format(PyExc_ValueError,
				 "unknown attribute value %200s", str);

		    free(str);
		    return -1;
		}
		free(str);
	    } else if (PyTuple_Check(item)) {
		PyObject *re_module, *patt, *func_compile, *args;

		if (PyTuple_Size(item) != 2) {
		    PyErr_Format(PyExc_ValueError,
				 "argument size mismatch");
		    return -1;
		}

		patt = PyTuple_GetItem(item, 0);
		if (PyString_Check(patt) || PyUnicode_Check(patt)) {
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
LineBreak_set_sizing(PyObject * self, PyObject * value, void *closure)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);

    if (value == NULL)
	linebreak_set_sizing(lbobj, NULL, NULL);
    if (value == Py_None)
	linebreak_set_sizing(lbobj, NULL, NULL);
    else if (PyString_Check(value) || PyUnicode_Check(value)) {
	char *str;
	if ((str = genericstr_ToString(value)) == NULL)
	    return -1;

	if (strcasecmp(str, "UAX11") == 0)
	    linebreak_set_sizing(lbobj, linebreak_sizing_UAX11, NULL);
	else {
	    PyErr_Format(PyExc_ValueError,
			 "unknown attribute value %200s", str);

	    free(str);
	    return -1;
	}
	free(str);
    } else if (PyFunction_Check(value))
	linebreak_set_sizing(lbobj, sizing_func, (void *) value);
    else {
	PyErr_Format(PyExc_ValueError,
		     "attribute must be function, not %200s",
		     Py_TYPE(value)->tp_name);
	return -1;
    }
    return 0;
}

static int
LineBreak_set_urgent(PyObject * self, PyObject * value, void *closure)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);

    if (value == NULL)
	linebreak_set_urgent(lbobj, NULL, NULL);
    else if (value == Py_None)
	linebreak_set_urgent(lbobj, NULL, NULL);
    else if (PyString_Check(value) || PyUnicode_Check(value)) {
	char *str;

	if ((str = genericstr_ToString(value)) == NULL)
	    return -1;
	else if (strcasecmp(str, "FORCE") == 0)
	    linebreak_set_urgent(lbobj, linebreak_urgent_FORCE, NULL);
	else if (strcasecmp(str, "RAISE") == 0)
	    linebreak_set_urgent(lbobj, linebreak_urgent_ABORT, NULL);
	else {
	    PyErr_Format(PyExc_ValueError,
			 "unknown attribute value %200s", str);

	    free(str);
	    return -1;
	}
	free(str);
    } else if (PyFunction_Check(value))
	linebreak_set_urgent(lbobj, urgent_func, (void *) value);
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
    {"break_indent",
     (getter) LineBreak_get_break_indent,
     (setter) LineBreak_set_break_indent,
     PyDoc_STR("\
Always allows break after SPACEs at beginning of line, \
a.k.a. indent.  [UAX #14] does not take account of such usage of SPACE.")},
    {"charmax",
     (getter) LineBreak_get_charmax,
     (setter) LineBreak_set_charmax,
     PyDoc_STR("\
Possible maximum number of characters in one line, \
not counting trailing SPACEs and newline sequence.  \
Note that number of characters generally doesn't represent length of line.  \
0 means unlimited.")},
    {"width",
     (getter) LineBreak_get_width,
     (setter) LineBreak_set_width,
     PyDoc_STR("\
Maximum number of columns line may include not counting \
trailing spaces and newline sequence.  In other words, recommended maximum \
length of line.")},
    {"minwidth",
     (getter) LineBreak_get_minwidth,
     (setter) LineBreak_set_minwidth,
     PyDoc_STR("\
Minimum number of columns which line broken arbitrarily may \
include, not counting trailing spaces and newline sequences.")},
    {"complex_breaking",
     (getter) LineBreak_get_complex_breaking,
     (setter) LineBreak_set_complex_breaking,
     PyDoc_STR("\
Performs heuristic breaking on South East Asian complex \
context.  If word segmentation for South East Asian writing systems is not \
enabled, this does not have any effect.")},
    {"eastasian_context",
     (getter) LineBreak_get_eastasian_context,
     (setter) LineBreak_set_eastasian_context,
     PyDoc_STR("\
Enable East Asian language/region context.")},
    {"eaw",
     (getter) LineBreak_get_eaw,
     (setter) LineBreak_set_eaw,
     PyDoc_STR("\
Tailor classification of East_Asian_Width property.  \
Value may be a dictionary with its keys are Unicode string or \
UCS scalar and with its values are any of East_Asian_Width properties \
(See class properties).  \
If None is specified, all tailoring assigned before will be canceled.\n\
By default, no tailorings are available.")},
    {"format",
     (getter) LineBreak_get_format,
     (setter) LineBreak_set_format,
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
    {"hangul_as_al",
     (getter) LineBreak_get_hangul_as_al,
     (setter) LineBreak_set_hangul_as_al,
     PyDoc_STR("\
Treat hangul syllables and conjoining jamo as alphabetic \
characters (AL).")},
    {"lbc",
     (getter) LineBreak_get_lbc,
     (setter) LineBreak_set_lbc,
     PyDoc_STR("\
Tailor classification of line breaking property.  \
Value may be a dictionary with its keys are Unicode string or \
UCS scalar and its values with any of Line Breaking Classes \
(See class properties).  \
If None is specified, all tailoring assigned before will be canceled.\n\
By default, no tailorings are available.")},
    {"legacy_cm",
     (getter) LineBreak_get_legacy_cm,
     (setter) LineBreak_set_legacy_cm,
     PyDoc_STR("\
Treat combining characters lead by a SPACE as an isolated \
combining character (ID).  As of Unicode 5.0, such use of SPACE is not \
recommended.")},
    {"newline",
     (getter) LineBreak_get_newline,
     (setter) LineBreak_set_newline,
     PyDoc_STR("\
Unicode string to be used for newline sequence.  \
It may be None.")},
    {"prep",
     (getter) LineBreak_get_prep,
     (setter) LineBreak_set_prep,
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
    {"sizing",
     (getter) LineBreak_get_sizing,
     (setter) LineBreak_set_sizing,
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
    {"urgent",
     (getter) LineBreak_get_urgent,
     (setter) LineBreak_set_urgent,
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
    {"virama_as_joiner",
     (getter) LineBreak_get_virama_as_joiner,
     (setter) LineBreak_set_virama_as_joiner,
     PyDoc_STR("\
Virama sign (\"halant\" in Hindi, \"coeng\" in Khmer) and its succeeding \
letter are not broken.\n\
\"Default\" grapheme cluster defined by [UAX #29] does not contain this \
feature.")},
    {NULL}			/* Sentinel */
};

/*
 * Mapping methods
 */

static Py_ssize_t
LineBreak_length(PyObject * self)
{
    return PyMapping_Length(LineBreak_AS_CSTRUCT(self)->stash);
}

static PyObject *
LineBreak_subscript(PyObject * self, PyObject * key)
{
    return PyObject_GetItem(LineBreak_AS_CSTRUCT(self)->stash, key);
}

static int
LineBreak_ass_subscript(PyObject * self, PyObject * key, PyObject * value)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);

    if (value == NULL)
	return PyObject_DelItem(lbobj->stash, key);
    else
	return PyObject_SetItem(lbobj->stash, key, value);
}

static PyMappingMethods LineBreak_as_mapping = {
    LineBreak_length,		/* mp_length */
    LineBreak_subscript,	/* mp_subscript */
    LineBreak_ass_subscript	/* mp_ass_subscript */
};

/*
 * Class-specific methods
 */

PyDoc_STRVAR(LineBreak_copy__doc__, "\
Copy LineBreak object.");

static PyObject *
LineBreak_copy(PyObject * self, PyObject * args)
{
    return
	LineBreak_FromCstruct(linebreak_copy(LineBreak_AS_CSTRUCT(self)));
}

PyDoc_STRVAR(LineBreak_wrap__doc__, "\
S.wrap(text) -> [GCStr]\n\
\n\
Break a Unicode string text and returns list of lines contained in the \
result.  Each item of list is grapheme cluster string (GCStr object).");

static PyObject *
LineBreak_wrap(PyObject * self, PyObject * args)
{
    linebreak_t *lbobj = LineBreak_AS_CSTRUCT(self);
    PyObject *str, *ret;
    unistr_t unistr = { NULL, 0 };
    gcstring_t **broken;
    size_t i;

    if (!PyArg_ParseTuple(args, "O", &str))
	return NULL;
    if (unicode_ToCstruct(&unistr, str) == NULL)
	return NULL;

    linebreak_reset(lbobj);
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
	    PyErr_SetString(LineBreakException,
			    "Excessive line was found");
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
    {"__copy__",
     (PyCFunction) LineBreak_copy, METH_NOARGS,
     LineBreak_copy__doc__},
    {"wrap",
     (PyCFunction) LineBreak_wrap, METH_VARARGS,
     LineBreak_wrap__doc__},
    {NULL}			/* Sentinel */
};


static PyTypeObject LineBreakType = {
#if PY_MAJOR_VERSION >= 3
    PyVarObject_HEAD_INIT(NULL, 0)
#else				/* PY_MAJOR_VERSION */
    PyObject_HEAD_INIT(NULL)
    0,				/*ob_size */
#endif				/* PY_MAJOR_VERSION */
    "_textseg.LineBreak",	/*tp_name */
    sizeof(LineBreakObject),	/*tp_basicsize */
    0,				/*tp_itemsize */
    (destructor) LineBreak_dealloc,	/*tp_dealloc */
    0,				/*tp_print */
    0,				/*tp_getattr */
    0,				/*tp_setattr */
    0,				/*tp_compare */
    0,				/*tp_repr */
    0,				/*tp_as_number */
    0,				/*tp_as_sequence */
    &LineBreak_as_mapping,	/*tp_as_mapping */
    0,				/*tp_hash */
    0,				/*tp_call */
    0,				/*tp_str */
    0,				/*tp_getattro */
    0,				/*tp_setattro */
    0,				/*tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/*tp_flags */
    "LineBreak objects",	/* tp_doc */
    0,				/* tp_traverse */
    0,				/* tp_clear */
    0,				/* tp_richcompare */
    0,				/* tp_weaklistoffset */
    0,				/* tp_iter */
    0,				/* tp_iternext */
    LineBreak_methods,		/* tp_methods */
    0,				/* tp_members */
    LineBreak_getseters,	/* tp_getset */
    0,				/* tp_base */
    0,				/* tp_dict: See LineBreak_dict_init() */
    0,				/* tp_descr_get */
    0,				/* tp_descr_set */
    0,				/* tp_dictoffset */
    (initproc) LineBreak_init,	/* tp_init */
    0,				/* tp_alloc */
    LineBreak_new,		/* tp_new */
};


/**
 ** GCStr class
 **/

/*
 * Constructor & Destructor
 */

static void
GCStr_dealloc(GCStrObject * self)
{
    gcstring_destroy(self->obj);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
GCStr_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
    GCStrObject *self;
    if ((self = (GCStrObject *) type->tp_alloc(type, 0)) == NULL)
	return NULL;
    return (PyObject *) self;
}

static int
GCStr_init(GCStrObject * self, PyObject * args, PyObject * kwds)
{
    unistr_t unistr = { NULL, 0 };
    PyObject *pystr = NULL, *pyobj = NULL;
    linebreak_t *lbobj;

    if (!PyArg_ParseTuple(args, "O|O!", &pystr, &LineBreakType, &pyobj))
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
GCStr_get_chars(PyObject * self)
{
#ifdef OLDAPI_Py_UNICODE_NARROW
    gcstring_t *gcstr = GCStr_AS_CSTRUCT(self);
    size_t i, chars;

    if (gcstr->str == NULL)
	return PyInt_FromSsize_t(0);
    for (i = 0, chars = 0; i < gcstr->len; i++, chars++)
	if (0x10000 <= gcstr->str[i])
	    chars++;
    return PyInt_FromSsize_t(chars);
#else				/* OLDAPI_Py_UNICODE_NARROW */
    return PyInt_FromSsize_t(GCStr_AS_CSTRUCT(self)->len);
#endif				/* OLDAPI_Py_UNICODE_NARROW */
}

static PyObject *
GCStr_get_cols(PyObject * self)
{
    return PyInt_FromSsize_t(gcstring_columns(GCStr_AS_CSTRUCT(self)));
}

static PyObject *
GCStr_get_lbc(PyObject * self)
{
    gcstring_t *gcstr = GCStr_AS_CSTRUCT(self);

    if (gcstr->gclen == 0) {
	Py_RETURN_NONE;
    }
    return PyInt_FromLong((long) gcstr->gcstr[0].lbc);
}

static PyObject *
GCStr_get_lbcext(PyObject * self)
{
    gcstring_t *gcstr = GCStr_AS_CSTRUCT(self);

    if (gcstr->gclen == 0) {
	Py_RETURN_NONE;
    }
    return PyInt_FromLong((long) gcstr->gcstr[gcstr->gclen - 1].elbc);

}

static PyGetSetDef GCStr_getseters[] = {
    {"chars",
     (getter) GCStr_get_chars, NULL,
     "Number of Unicode characters grapheme cluster string includes, "
     "i.e. length as Unicode string.",
     NULL},
    {"cols",
     (getter) GCStr_get_cols, NULL,
     "Total number of columns of grapheme clusters "
     "defined by built-in character database.  "
     "For more details see documentations of LineBreak class.",
     NULL},
    {"lbc",
     (getter) GCStr_get_lbc, NULL,
     "Line Breaking Class (See LineBreak class) of the first "
     "character of first grapheme cluster.",
     NULL},
    {"lbcext",
     (getter) GCStr_get_lbcext, NULL,
     "Line Breaking Class (See LineBreak class) of last "
     "grapheme extender of last grapheme cluster.  If there are no "
     "grapheme extenders or its class is CM, value of last grapheme base "
     "will be returned.",
     NULL},
    {NULL}			/* Sentinel */
};

/*
 * String methods
 */

#if PY_MAJOR_VERSION >= 3
static PyObject *
GCStr_Str(PyObject * self)
{
    return unicode_FromCstruct((unistr_t *) (GCStr_AS_CSTRUCT(self)));
}
#endif

static PyObject *
GCStr_compare(PyObject * a, PyObject * b, int op)
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

    if ((astr = genericstr_ToCstruct(a, lbobj)) == NULL ||
	(bstr = genericstr_ToCstruct(b, lbobj)) == NULL) {
	if (a != NULL && !GCStr_Check(a))
	    gcstring_destroy(astr);
	return NULL;
    }
    cmp = gcstring_cmp(astr, bstr);
    if (!GCStr_Check(a))
	gcstring_destroy(astr);
    if (!GCStr_Check(b))
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
GCStr_length(PyObject * self)
{
    return (Py_ssize_t) GCStr_AS_CSTRUCT(self)->gclen;
}

static PyObject *
GCStr_concat(PyObject * o1, PyObject * o2)
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
	if (!GCStr_Check(o1))
	    gcstring_destroy(gcstr1);
	return NULL;
    }
    if ((gcstr = gcstring_concat(gcstr1, gcstr2)) == NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);

	if (!GCStr_Check(o1))
	    gcstring_destroy(gcstr1);
	if (!GCStr_Check(o2))
	    gcstring_destroy(gcstr2);
	return NULL;
    }

    if (!GCStr_Check(o1))
	gcstring_destroy(gcstr1);
    if (!GCStr_Check(o2))
	gcstring_destroy(gcstr2);
    return GCStr_FromCstruct(gcstr);
}

static PyObject *
GCStr_repeat(PyObject * self, Py_ssize_t count)
{
    gcstring_t *gcstr;
    Py_ssize_t i;		/* need signed comparison */

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
GCStr_item(PyObject * self, Py_ssize_t i)
{
    gcstring_t *gcstr;

    if (GCStr_AS_CSTRUCT(self)->gclen == 0) {
	PyErr_SetString(PyExc_IndexError, "GCStr index out of range");
	return NULL;
    }
    if ((gcstr = gcstring_substr(GCStr_AS_CSTRUCT(self), i, 1))
	== NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);
	return NULL;
    }
    if (gcstr->gclen == 0) {
	PyErr_SetString(PyExc_IndexError, "GCStr index out of range");
	gcstring_destroy(gcstr);
	return NULL;
    }
    return GCStr_FromCstruct(gcstr);
}

static PyObject *
GCStr_slice(PyObject * self, Py_ssize_t start, Py_ssize_t end)
{
    gcstring_t *gcstr;

    /* standard clamping */
    if (start < 0)
	start = 0;
    if (end < 0)
	end = 0;
    if (GCStr_AS_CSTRUCT(self)->gclen < end)
	end = GCStr_AS_CSTRUCT(self)->gclen;
    if (end < start)
	start = end;
    /* copy slice */
    if ((gcstr =
	 gcstring_substr(GCStr_AS_CSTRUCT(self), start, end - start))
	== NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);
	return NULL;
    }
    return GCStr_FromCstruct(gcstr);
}

static int
GCStr_ass_item(PyObject * self, Py_ssize_t i, PyObject * v)
{
    gcstring_t *gcstr, *repl;

    if (GCStr_AS_CSTRUCT(self)->gclen == 0) {
	PyErr_SetString(PyExc_IndexError, "GCStr index out of range");
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

	if (!GCStr_Check(v))
	    gcstring_destroy(repl);
	return -1;
    }

    if (!GCStr_Check(v))
	gcstring_destroy(gcstr);
    return 0;
}

static int
GCStr_ass_slice(PyObject * self, Py_ssize_t start, Py_ssize_t end,
		PyObject * v)
{
    gcstring_t *gcstr, *repl;
    linebreak_t *lbobj = GCStr_AS_CSTRUCT(self)->lbobj;

    if (v == NULL)
	repl = gcstring_new(NULL, lbobj);
    else if ((repl = genericstr_ToCstruct(v, lbobj)) == NULL)
	return -1;

    /* standard clamping */
    if (start < 0)
	start = 0;
    if (end < 0)
	end = 0;
    if (GCStr_AS_CSTRUCT(self)->gclen < end)
	end = GCStr_AS_CSTRUCT(self)->gclen;
    if (end < start)
	start = end;

    if ((gcstr =
	 gcstring_replace(GCStr_AS_CSTRUCT(self), start, end - start,
			  repl)) == NULL) {
	PyErr_SetFromErrno(PyExc_RuntimeError);

	if (v == NULL || !GCStr_Check(v))
	    gcstring_destroy(repl);
	return -1;
    }

    if (v == NULL || !GCStr_Check(v))
	gcstring_destroy(repl);
    return 0;
}

static PyObject *
GCStr_subscript(PyObject * self, PyObject * item)
{
    Py_ssize_t k;
    gcstring_t *gcstr = GCStr_AS_CSTRUCT(self);

#if PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION <= 4
    if (PyInt_Check(item))
	k = PyInt_AsSsize_t(item);
    else if (PyLong_Check(item))
	k = PyLong_AsSsize_t(item);
#else				/* PY_MAJOR_VERSION == 2 && ... */
    if (PyIndex_Check(item) || PyNumber_Check(item))
	k = PyNumber_AsSsize_t(item, PyExc_IndexError);
#endif				/* PY_MAJOR_VERSION == 2 && ... */
    else if (PySlice_Check(item)) {
	Py_ssize_t start, stop, step, len, cur, i;
	gcstring_t *result;

#if PY_MAJOR_VERSION == 2 || (PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION <= 1)
	if (PySlice_GetIndicesEx((PySliceObject *) item, gcstr->gclen,
				 &start, &stop, &step, &len) < 0)
	    return NULL;
#else				/* PY_MAJOR_VERSION ... */
	if (PySlice_GetIndicesEx((PyObject *) item, gcstr->gclen,
				 &start, &stop, &step, &len) < 0)
	    return NULL;
#endif				/* PY_MAJOR_VERSION ... */

	if (len <= 0)
	    return GCStr_FromCstruct(gcstring_new(NULL, gcstr->lbobj));
	else if (step == 1)
	    return GCStr_FromCstruct(gcstring_substr(gcstr, start, len));
	if ((result = gcstring_new(NULL, gcstr->lbobj)) == NULL) {
	    PyErr_SetFromErrno(PyExc_RuntimeError);
	    return NULL;
	}
	for (cur = start, i = 0; i < len; cur += step, i++)
	    if (gcstring_append(result, gcstring_substr(gcstr, cur, 1))
		== NULL) {
		PyErr_SetFromErrno(PyExc_RuntimeError);
		gcstring_destroy(result);
		return NULL;
	    }
	return GCStr_FromCstruct(result);
    } else {
	PyErr_SetString(PyExc_TypeError, "GCStr indices must be integers");
	return NULL;
    }

    if (k == -1 && PyErr_Occurred())
	return NULL;
    if (k < 0)
	k += gcstr->gclen;
    return GCStr_item(self, k);
}

static int
GCStr_ass_subscript(PyObject * self, PyObject * item, PyObject * v)
{
    Py_ssize_t k;
    gcstring_t *gcstr = GCStr_AS_CSTRUCT(self);

#if PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION <= 4
    if (PyInt_Check(item))
	k = PyInt_AsSsize_t(item);
    else if (PyLong_Check(item))
	k = PyLong_AsSsize_t(item);
#else				/* PY_MAJOR_VERSION == 2 && ... */
    if (PyIndex_Check(item) || PyNumber_Check(item))
	k = PyNumber_AsSsize_t(item, PyExc_IndexError);
#endif				/* PY_MAJOR_VERSION == 2 && ... */
    else if (PySlice_Check(item)) {
	Py_ssize_t start, stop, step, len;
	gcstring_t *repl;

#if PY_MAJOR_VERSION == 2 || (PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION <= 1)
	if (PySlice_GetIndicesEx((PySliceObject *) item, gcstr->gclen,
				 &start, &stop, &step, &len) < 0)
	    return -1;
#else				/* PY_MAJOR_VERSION ... */
	if (PySlice_GetIndicesEx((PyObject *) item, gcstr->gclen,
				 &start, &stop, &step, &len) < 0)
	    return -1;
#endif				/* PY_MAJOR_VERSION ... */

	if (step != 1) {
	    PyErr_SetNone(PyExc_NotImplementedError);
	    return -1;
	}
	if (len < 0)
	    return 0;

	if (v == NULL) {
	    if ((repl = gcstring_new(NULL, gcstr->lbobj)) == NULL) {
		PyErr_SetFromErrno(PyExc_RuntimeError);
		return -1;
	    }
	    if (gcstring_replace(gcstr, start, len, repl) == NULL) {
		PyErr_SetFromErrno(PyExc_RuntimeError);

		gcstring_destroy(repl);
		return -1;
	    }
	    gcstring_destroy(repl);
	    return 0;
	}

	if ((repl = genericstr_ToCstruct(v, gcstr->lbobj)) == NULL)
	    return -1;
	if (gcstring_replace(gcstr, start, len, repl) == NULL) {
	    PyErr_SetFromErrno(PyExc_RuntimeError);

	    if (!GCStr_Check(v))
		gcstring_destroy(repl);
	    return -1;
	}
	if (!GCStr_Check(v))
	    gcstring_destroy(repl);
	return 0;
    } else {
	PyErr_SetString(PyExc_TypeError, "GCStr indices must be integers");
	return -1;
    }

    if (k == -1 && PyErr_Occurred())
	return -1;
    if (k < 0)
	k += gcstr->gclen;
    return GCStr_ass_item(self, k, v);
}

static PySequenceMethods GCStr_as_sequence = {
    GCStr_length,		/* sq_length */
    GCStr_concat,		/* sq_concat */
    GCStr_repeat,		/* sq_repeat */
    GCStr_item,			/* sq_item */
    GCStr_slice,		/* sq_slice: unused by 3.0 (?) */
    GCStr_ass_item,		/* sq_ass_item */
    GCStr_ass_slice,		/* sq_ass_slice: unused by 3.0 (?) */
    NULL,			/* sq_contains */
    NULL,			/* sq_inplace_concat */
    NULL			/* sq_inplace_repeat */
};

static PyMappingMethods GCStr_as_mapping = {
    GCStr_length,		/* mp_length */
    GCStr_subscript,		/* mp_subscript */
    GCStr_ass_subscript		/* mp_ass_subscript */
};

/*
 * Class specific methods
 */

PyDoc_STRVAR(GCStr_copy__doc__, "\
Create a copy of GCStr object.");

static PyObject *
GCStr_copy(PyObject * self, PyObject * args)
{
    return GCStr_FromCstruct(gcstring_copy(GCStr_AS_CSTRUCT(self)));
}

PyDoc_STRVAR(GCStr_flag__doc__, "S.flag(offset [, value]) => int\n\
\n\
Get and optionally set flag value of offset-th grapheme cluster.  \
Flag value is an non-zero integer not greater than 255 and initially is 0.\n\
Predefined flag values are:\n\
: ALLOW_BEFORE : \
Allow line breaking just before this grapheme cluster.\n\
: PROHIBIT_BEFORE : \
Prohibit line breaking just before this grapheme cluster.");

static PyObject *
GCStr_flag(PyObject * self, PyObject * args)
{
    gcstring_t *gcstr = GCStr_AS_CSTRUCT(self);
    Py_ssize_t i;
    long v = -1L;
    PyObject *ret;

    if (!PyArg_ParseTuple(args, ARG_FORMAT_SSIZE_T "|i", &i, &v))
	return NULL;
    if (i < 0 || gcstr->gclen <= i) {
	Py_RETURN_NONE;
    }
    ret = PyInt_FromLong((unsigned long) gcstr->gcstr[i].flag);
    if (0 < v)
	gcstr->gcstr[i].flag = (propval_t) v;

    return ret;
}

/* FIXME: often unavailable */
static PyObject *
GCStr_radd(PyObject * self, PyObject * args)
{
    PyObject *pyobj;

    if (!PyArg_ParseTuple(args, "O", &pyobj))
	return NULL;
    return GCStr_concat(pyobj, self);
}

static PyObject *
GCStr_unicode(PyObject * self, PyObject * args)
{
    return unicode_FromCstruct((unistr_t *) (GCStr_AS_CSTRUCT(self)));
}

static PyMethodDef GCStr_methods[] = {
    {"__copy__",
     GCStr_copy, METH_NOARGS,
     GCStr_copy__doc__},
    {"flag",
     GCStr_flag, METH_VARARGS,
     GCStr_flag__doc__},
    {"__radd__",
     GCStr_radd, METH_VARARGS,
     "x.__radd__(y) <==> y+x"},
    {"__unicode__",
     GCStr_unicode, METH_NOARGS,
     "x.__unicode__() <==> unicode(x)\n\
      Convert grapheme cluster string to Unicode string."},
    {NULL}			/* Sentinel */
};


static PyTypeObject GCStrType = {
#if PY_MAJOR_VERSION >= 3
    PyVarObject_HEAD_INIT(NULL, 0)
#else				/* PY_MAJOR_VERSION */
    PyObject_HEAD_INIT(NULL)
    0,				/*ob_size */
#endif				/* PY_MAJOR_VERSION */
    "_textseg.GCStr",		/*tp_name */
    sizeof(GCStrObject),	/*tp_basicsize */
    0,				/*tp_itemsize */
    (destructor) GCStr_dealloc,	/*tp_dealloc */
    0,				/*tp_print */
    0,				/*tp_getattr */
    0,				/*tp_setattr */
    0,				/*tp_compare */
    0,				/*tp_repr */
    0,				/*tp_as_number */
    &GCStr_as_sequence,		/*tp_as_sequence */
    &GCStr_as_mapping,		/*tp_as_mapping */
    0,				/*tp_hash */
    0,				/*tp_call */
#if PY_MAJOR_VERSION >= 3
    &GCStr_Str,			/*tp_str */
#else				/* PY_MAJOR_VERSION */
    0,				/*tp_str */
#endif				/* PY_MAJOR_VERSION */
    0,				/*tp_getattro */
    0,				/*tp_setattro */
    0,				/*tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/*tp_flags */
    "GCStr objects",		/* tp_doc */
    0,				/* tp_traverse */
    0,				/* tp_clear */
    GCStr_compare,		/* tp_richcompare */
    0,				/* tp_weaklistoffset */
    0,				/* tp_iter */
    0,				/* tp_iternext */
    GCStr_methods,		/* tp_methods */
    0,				/* tp_members */
    GCStr_getseters,		/* tp_getset */
    0,				/* tp_base */
    0,				/* tp_dict */
    0,				/* tp_descr_get */
    0,				/* tp_descr_set */
    0,				/* tp_dictoffset */
    (initproc) GCStr_init,	/* tp_init */
    0,				/* tp_alloc */
    GCStr_new,			/* tp_new */
};


/**
 * Initialize module
 */

static PyMethodDef module_methods[] = {
    {NULL}			/* Sentinel */
};

PyDoc_STRVAR(module__doc__, "\
Module for text segmentation.");

#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef textseg_def = {
    PyModuleDef_HEAD_INIT,
    "_textseg",
    module__doc__,
    -1,
    module_methods,
    NULL, NULL, NULL, NULL
};

#define INITERROR return NULL
PyMODINIT_FUNC
PyInit__textseg(void)
#else				/* PY_MAJOR_VERSION */

#define INITERROR return
void
init_textseg(void)
#endif				/* PY_MAJOR_VERSION */
{
    PyObject *m;

    if ((LineBreakException =
	 PyErr_NewException("_textseg.Error", NULL, NULL))
	== NULL)
	INITERROR;
    if (PyType_Ready(&LineBreakType) < 0) {
	Py_DECREF(LineBreakException);
	Py_DECREF(LineBreakType.tp_dict);
	INITERROR;
    }
    if (PyType_Ready(&GCStrType) < 0) {
	Py_DECREF(LineBreakException);
	Py_DECREF(LineBreakType.tp_dict);
	INITERROR;
    }
#if PY_MAJOR_VERSION >= 3
    m = PyModule_Create(&textseg_def);
#else				/* PY_MAJOR_VERSION */
    m = Py_InitModule3("_textseg", module_methods, module__doc__);
#endif				/* PY_MAJOR_VERSION */
    if (m == NULL) {
	Py_DECREF(LineBreakException);
	Py_DECREF(LineBreakType.tp_dict);
	INITERROR;
    }

    Py_INCREF(LineBreakException);
    PyModule_AddObject(m, "Error", LineBreakException);
    Py_INCREF(&LineBreakType);
    PyModule_AddObject(m, "LineBreak", (PyObject *) & LineBreakType);
    Py_INCREF(&GCStrType);
    PyModule_AddObject(m, "GCStr", (PyObject *) & GCStrType);

#if PY_MAJOR_VERSION >= 3
    return m;
#endif				/* PY_MAJOR_VERSION */
}
