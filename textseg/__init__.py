#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Text segmentation module

Functions:
    fill()
    wrap()

Classes:
    GCStr
    LineBreak

Submodule:
    Consts: see documentation for textseg.Consts.
"""

# Copyright (C) 2012 Hatuka*nezumi - IKEDA Soji.

__date__ = '2012-01-29'
__author__ = 'Hatuka*nezumi - IKEDA Soji <hatuka@nezumi.nu>'
__version__ = "0.0.0"

__all__ = ['LineBreak', 'GCStr', 'wrap', 'fill']

import _textseg

try:
    unicode
except NameError:
    unicode = str

def wrap(text,
         expand_tabs=True,
         replace_whitespace=True,
         break_long_words=True,
         **kwds):
    '''\
wrap(text[, options...]) -> [unicode]

Wrap paragraphs of a text then return a list of wrapped lines.

Reformat each paragraph in 'text' so that it fits in lines of no more than 
'width' columns if possible, and return a list of wrapped lines.  By 
default, tabs in 'text' are expanded and all other whitespace characters 
(including newline) are converted to space.'''

    import re
    from textseg import LineBreak
    from textseg import GCStr

    if expand_tabs:
        text = GCStr(text).expandtabs()
    if replace_whitespace:
        table = {}
        for c in unicode('\t\n\x0b\x0c\r '):
            table[c] = unicode(' ')
        text = text.translate(table)

    for k, v in list({ 'charmax': 0,
                       'format': "NEWLINE",
                       'newline': None,
                       'urgent': (break_long_words and "FORCE" or None),
                     }.items()):
        if not k in kwds:
            kwds[k] = v
    lb = LineBreak(**kwds)
    return [unicode(s) for s in lb.wrap(text)]

def fill(text, **kwds):
    '''\
fill(text[, options...]) -> unicode

Reformat the single paragraph in 'text' to fit in lines of no more than 
'width' columns, and return a new string containing the entire wrapped 
paragraph.'''

    return unicode("\n").join(wrap(text, **kwds))

"""
Line breaking
"""

class LineBreak(_textseg.LineBreak):
    '''\
LineBreak class performs Line Breaking Algorithm described in Unicode 
Standards Annex #14 [UAX #14]. East_Asian_Width informative properties 
defined by Annex #11 [UAX #11] will be concerned to determine breaking 
positions.'''

    MANDATORY = 4
    DIRECT = 3
    INDIRECT = 2
    PROHIBITED = 1

    DEFAULTS = {
        'width': 70,
        'minwidth': 0,
        'charmax': 998,
        'eastasian_context': False,
        'eaw': None,
        'format': 'SIMPLE',
        'hangul_as_al': False,
        'lbc': None,
        'legacy_cm': True,
        'newline': "\n",
        'prep': None,
        'sizing': "UAX11",
        'urgent': None,
        'virama_as_joiner': True,
    }

    def __init__(self, **kwds):
        '''\
LineBreak([options...]) -> LineBreak

Create new LineBreak object.  Optional named arguments may specify 
initial property values.  See documentations of each properties.'''
        for k, v in list(self.DEFAULTS.items()):
            if k not in kwds:
                kwds[k] = v
        _textseg.LineBreak.__init__(self, **kwds)

"""
Grapheme cluster string
"""

class GCStr(_textseg.GCStr):
    '''\
GCStr class treats Unicode string as a sequence of 
_extended grapheme clusters_ defined by Unicode Standard Annex #29 
[UAX #29].

*Grapheme cluster* is a sequence of Unicode character(s) that consists 
of one *grapheme base* and optional *grapheme extender* and/or 
*"prepend" character*.  It is close in that people consider as 
_character_.'''

    PROHIBIT_BEFORE = 1
    ALLOW_BEFORE = 2

    def __init__(self, string, lb=None):
        '''\
GCStr(string[, lb]) -> GCStr

Create new grapheme cluster string (GCStr object) from Unicode string 
string.

Optional LineBreak object lb controls breaking features.  Following 
properties of LineBreak object affect new GCStr object:
* eastasian_context
* eaw
* lbc
* virama_as_joiner'''

        if lb is None:
            lb = LineBreak()
        _textseg.GCStr.__init__(self, string, lb)

    def expandtabs(self, tabsize=8):
        '''\
S.expandtabs([tabsize]) -> GCStr

Return a copy of S where all tab characters are expanded using spaces.
If tabsize is not given, a tab size of 8 characters is assumed.'''

        from textseg.Consts import lbcBK, lbcCR, lbcLF, lbcNL

        ret = self * 0
        j = 0
        for c in self:
            if c.lbc in (lbcBK, lbcCR, lbcLF, lbcNL):
                ret += c
                j = 0
            elif c == unicode('\t'):
                if 0 < tabsize:
                    incr = tabsize - (j % tabsize)
                    ret += unicode(' ') * incr
                    j += incr
            else:
                ret += c
                j += c.cols
        return ret

    def translate(self, table):
        '''\
        '''
        o = self * 0
        return o + unicode(self).translate(table)

