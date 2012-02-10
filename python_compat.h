/*
 * pysombok - Unicode Text Segmentation Package for Python.
 *
 * Copyright (C) 2012 by Hatuka*nezumi - IKEDA Soji.
 *
 * This file is part of the pysombok Package.  This program is free
 * software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version.  This program is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * COPYING file for more details.
 */

#if PY_MAJOR_VERSION == 2
#   if PY_MINOR_VERSION <= 3
#       define Py_RETURN_NONE return Py_INCREF(Py_None), Py_None
#   endif
#   if PY_MINOR_VERSION <= 4
typedef int Py_ssize_t;
#       define ARG_FORMAT_SSIZE_T "i"
#       define PyInt_FromSsize_t(i) PyInt_FromLong(i)
#       define PyInt_AsSsize_t(o) ((Py_ssize_t)PyInt_AsLong(o))
#       define PyLong_AsSsize_t(o) ((Py_ssize_t)PyLong_AsLong(o))
#   else			/* PY_MINOR_VERSION */
#       define ARG_FORMAT_SSIZE_T "n"
#   endif			/* PY_MINOR_VERSION */
#   if PY_MINOR_VERSION <= 5
#       define Py_TYPE(o) ((o)->ob_type)
#       define PyBytes_Check(o) PyString_Check(o)
#       define PyBytes_AsString(o) PyString_AsString(o)
#   endif
#elif PY_MAJOR_VERSION >= 3
#   define ARG_FORMAT_SSIZE_T "n"
#   define PyInt_Check(o) PyLong_Check(o)
#   define PyInt_FromSsize_t(o) PyLong_FromSsize_t(o)
#   define PyInt_FromLong(o) PyLong_FromLong(o)
#   define PyInt_AsLong(o) PyLong_AsLong(o)
#   define PyObject_Unicode(o) PyObject_Str(o)
#   define PyString_Check(s) PyUnicode_Check(s)
#   define PyString_FromString(s) PyUnicode_FromString(s)
#endif

#undef OLDAPI_Py_UNICODE_NARROW
#undef OLDAPI_Py_UNICODE_WIDE

#if PY_MAJOR_VERSION == 2 || (PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION <= 2)
typedef unsigned char Py_UCS1;
typedef Py_UNICODE Py_UCS2;
#   define PyUnicode_1BYTE_KIND (1)
#   define PyUnicode_2BYTE_KIND (2)
#   define PyUnicode_4BYTE_KIND (4)
/*
 * By "wide" mode of old API, Py_UNICODE is not always identical to Py_UCS4.
 * So fake it to be 2-byte kind.
 */
#   define PyUnicode_KIND(o) PyUnicode_2BYTE_KIND
#   ifndef Py_UNICODE_WIDE
#       define OLDAPI_Py_UNICODE_NARROW
#   else			/* Py_UNICODE_WIDE */
#       define OLDAPI_Py_UNICODE_WIDE
#   endif			/* Py_UNICODE_WIDE */
#   define unistr_KIND(kind, buf, type, len) kind = PyUnicode_2BYTE_KIND

#   define PyUnicode_READY(o) ((PyUnicode_AsUnicode(o) == NULL) ? -1 : 0)
#   define PyUnicode_GET_LENGTH(o) PyUnicode_GET_SIZE(o)
#   define PyUnicode_DATA(o) ((void *)PyUnicode_AS_DATA(o))
#   define PyUnicode_FromKindAndData(kind, buf, size) \
        PyUnicode_FromUnicode((Py_UNICODE *)(buf), size)

#   define Py_UNICODE_IS_SURROGATE(ch) (0xD800 <= ch && ch <= 0xDFFF)
#   define Py_UNICODE_IS_HIGH_SURROGATE(ch) (0xD800 <= ch && ch <= 0xDBFF)
#   define Py_UNICODE_IS_LOW_SURROGATE(ch) (0xDC00 <= ch && ch <= 0xDFFF)
#   define Py_UNICODE_JOIN_SURROGATES(high, low) \
        (((((Py_UCS4)(high) & 0x03FF) << 10) | \
        ((Py_UCS4)(low) & 0x03FF)) + 0x10000)
#   define Py_UNICODE_HIGH_SURROGATE(ch) (0xD800 | (((ch) - 0x10000) >> 10))
#   define Py_UNICODE_LOW_SURROGATE(ch) (0xDC00 | (((ch) - 0x10000) & 0x3FF))

#else				/* PY_MAJOR_VERSION == 2 ... */

#   define unistr_KIND(kind, buf, type, len) \
    do { \
        size_t i; \
        kind = PyUnicode_1BYTE_KIND; \
        for (i = 0; i < len; i++) \
            if (0x10000 <= ((type *)(buf))[i]) { \
                kind = PyUnicode_4BYTE_KIND; \
                break; \
            } else if (0x100 <= ((type *)(buf))[i]) \
                kind = PyUnicode_2BYTE_KIND; \
    } while (0)
#endif				/* PY_MAJOR_VERSION == 2 ... */

