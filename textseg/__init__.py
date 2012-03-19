#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""
The pytextseg package provides functions to wrap plain texts:
:func:`fill` and :func:`wrap` are Unicode-aware alternatives for those
of :mod:`textwrap` standard module; :func:`fold` and :func:`unfold` are
functions mainly focus on plain text messages such as e-mail.

It also provides lower level interfaces for text segmentation:
:class:`LineBreak` class for line breaking; :class:`GCStr` class for
grapheme cluster segmentation.
"""
# Copyright (C) 2012 by Hatuka*nezumi - IKEDA Soji.
#
# This file is part of the pytextseg package.  This program is free
# software; you can redistribute it and/or modify it under the terms of
# either the GNU General Public License or the Artistic License, as
# specified in the README file.

__all__ = ['Consts', 'GCStr', 'LineBreak', 'LineBreakException',
           'fill', 'fold', 'unfold', 'wrap']

import re
import _textseg
from textseg.Consts import lbcBK, lbcCR, lbcLF, lbcNL, lbcSP, eawN
try:
    from email.charset import Charset
except ImportError:
    from email.Charset import Charset

try:
    unicode, unichr
except NameError:
    unicode = str
    unichr = chr

###
### Function wrap()
###

def wrap(text,
         width = 70,
         initial_indent = "",
         subsequent_indent = "",
         expand_tabs = True,
         replace_whitespace = True,
         fix_sentence_endings = False,
         break_long_words = True,
         break_on_hyphens = True,
         drop_whitespace = True,
         **kwds):
    '''\
wrap(text[, options...]) -> [unicode]

Wrap paragraphs of a text then return a list of wrapped lines.

Reformat each paragraph in *text* so that it fits in lines of no more than 
*width* :term:`columns<number of columns>` if possible, and return a list 
of wrapped lines.
By default, tabs in *text* are expanded and all other whitespace characters 
(including newline) are converted to space.

See :mod:`textwrap` about options.

.. note::
   Some options take no effects on this module:
   *fix_sentence_endings*, *break_on_hyphens*, *drop_whitespace*.

For other named arguments see instance attributes of :class:`LineBreak`
class.
'''

    def format(self, action, s):
        if action.startswith('eo'):
            return self.newline
        if action in ('sot', 'sop'):
            return s * 0 + initial_indent + s
        if action == 'sol':
            return s * 0 + subsequent_indent + s
        if action == '':
            if s == initial_indent or s == subsequent_indent:
                return ''
        return None

    if expand_tabs:
        text = GCStr(text).expandtabs()
    if replace_whitespace:
        table = {}
        for c in unicode('\t\n\x0b\x0c\r '):
            table[c] = unicode(' ')
        if isinstance(text, GCStr):
            text = text * 0 + unicode(text).translate(table)
        else:
            text = text.translate(table)

    for k, v in list({ 'charmax': 0,
                       'format': format,
                       'newline': '',
                       'urgent': (break_long_words and "FORCE" or None),
                       'width': width,
                     }.items()):
        kwds.setdefault(k, v)
    lb = LineBreak(**kwds)
    return [unicode(s) for s in lb.wrap(text)]

###
### Function fill()
###

def fill(text, **kwds):
    '''\
fill(text[, options...]) -> unicode

Reformat the single paragraph in *text* to fit in lines of no more than 
*width* :term:`columns<number of columns>`, and return a new string 
containing the entire wrapped paragraph.
Optional named arguments will be passed to :func:`wrap<textseg.wrap>` 
function.'''

    return unicode("\n").join(wrap(text, **kwds))

###
### Function fold()
###

def _fold_FIXED(self, action, s):
    if action in ('sot', 'sop'):
        self['_'] = {}
        self['_']['width'] = self.width
        if s.startswith('>'):
            self.width = 0
    elif action == '':
        self['_']['line'] = s
    elif action == 'eol':
        return self.newline
    elif action.startswith('eo'):
        if len(self['_']['line']) and self.width:
            s = self.newline + self.newline
        else:
            s = self.newline
        self.width = self['_']['width']
        del self['_']
        return s
    return None

_prefixRe = re.compile(r'^>+')
def _fold_FLOWED(self, action, s):
    if action == 'sol':
        if len(self['_']['prefix']):
            return self['_']['prefix'] + ' ' + s
        elif s.startswith(' ') or s.startswith('From ') or s.startswith('>'):
            return ' ' + s
    elif action.startswith('so'):
        self['_'] = {}
        m = _prefixRe.match(unicode(s))
        if m:
            self['_']['prefix'] = m.group()
        else:
            self['_']['prefix'] = ''
            if s.startswith(' ') or s.startswith('From '):
                return ' ' + s
    elif action == '':
        self['_']['line'] = s
    elif action == 'eol':
        if len(s):
            s = ' '
        return s + ' ' + self.newline
    elif action.startswith('eo'):
        if len(self['_']['line']) and not len(self['_']['prefix']):
            s = ' ' + self.newline + self.newline
        else:
            s = self.newline
        del self['_']
        return s
    return None

def _fold_PLAIN(self, action, s):
    if action.startswith('eo'):
        return self.newline
    return None

_fold_funcs = {'flowed': _fold_FLOWED,
               'fixed': _fold_FIXED,
               'plain': _fold_PLAIN,
               }

_specialBreakRe = re.compile('([' + unichr(0xB) + unichr(0xC) +
                             unichr(0x85) + unichr(0x2028) + unichr(0x2029) +
                             '])', re.S)

def fold(string, method = 'plain', tabsize = 8,
         charset = None, language = None, **kwds):
    """\
fold(string[, method, options...]) -> unicode

Fold lines of string *string* to fit in lines of no more than *width*
columns, and return it.

Following options may be specified for *method* argument.

``"fixed"``
    Lines preceded by ">" won't be folded.
    Paragraphs are separated by empty line.
``"flowed"``
    "Format=Flowed; DelSp=Yes" formatting defined by :rfc:`3676`.
``"plain"``
    Default method.  All lines are folded.

Surplus SPACEs and horizontal tabs at end of line are removed,
newline sequences are replaced by that specified by optional *newline*
argument and newline is appended at end of text if it does not exist.
Horizontal tabs are treated as tab stops according to *tabsize* argument.

*charset* or *language* is used to determine language/region context:
East Asian or not.

For other named arguments see instance attributes of :class:`LineBreak`
class.
"""
    def sizing(self, cols, pre, spc, s):
        spcstr = spc + s
        i = 0
        for c in spcstr:
            if c.lbc != lbcSP:
                cols += spcstr[i:].cols
                break
            if c == "\t":
                if 0 < tabsize:
                    cols += tabsize - (cols % tabsize)
            else:
                cols += c.cols
            i = i + 1
        return cols

    if string is None or not len(string):
        return ''
    if not isinstance(string, unicode):
        string = unicode(string)

    if charset is not None:
        charset = Charset(charset).get_output_charset() 
        eastasian_context = not not re.match(
            r'''big5|cp9\d\d|euc-|gb18030|gb2312|gbk|hz|
                iso-2022-|ks_c_5601|shift_jis''', charset, re.I + re.X)
    elif language is not None:
        eastasian_context = not not re.match(
            r'ain|ja\b|jpn|ko\b|kor|zh\b|chi', language, re.I)
    else:
        eastasian_context = None

    kwds.update({'format': _fold_funcs.get(method.lower(), _fold_PLAIN),
                 'sizing': sizing,
                 })
    if eastasian_context is not None:
        kwds['eastasian_context'] = eastasian_context

    lb = LineBreak(**kwds)
    lbc = lb.lbc
    lbc[ord("\t")] = lbcSP
    lb.lbc = lbc

    result = ''
    for s in _specialBreakRe.split(string):
        if not len(s):
            continue
        elif _specialBreakRe.match(s):
            result += s
        else:
            result += ''.join([unicode(l) for l in lb.wrap(s)])
    return result

###
### function unfold()
###

def unfold(string, method = 'fixed', newline = "\n", **kwds):
    '''\
unfold(text[, method]) -> unicode

Conjunct folded paragraphs of string STRING and returns it.
Following options may be specified for *method* argument.

``"fixed"``
    Default method.
    Lines preceded by ``">"`` won't be conjuncted.
    Treat empty line as paragraph separator.
``"flowed"``
    Unfold "Format=Flowed; DelSp=Yes" formatting defined by :rfc:`3676`.
``"flowedsp"``
    Unfold "Format=Flowed; DelSp=No" formatting defined by :rfc:`3676`.
'''
    def reMatch(regexp, s, pos):
        # keep result of matching as an attribute of function itself.
        reMatch.m = re.compile(regexp).match(s, pos)
        return reMatch.m

    if not isinstance(string, unicode):
        string = unicode(string)
    if not len(string):
        return string
    string = re.sub(r'\r\n|\r', "\n", string)    

    method = method.lower()
    if method not in ('fixed', 'flowed', 'flowedsp'):
        method = 'fixed'
    delsp = (method == 'flowed')

    ## Do unfolding.
    result = '';
    for s in _specialBreakRe.split(string):
        if s == '':
            continue
        elif _specialBreakRe.match(s):
            result += s
            continue
        elif method == 'fixed':
            lb = LineBreak(**kwds)
            pos = 0
            while pos < len(s):
                if reMatch(r'\n', s, pos):
                    result += newline
                elif reMatch(r'(.+)\n\n', s, pos):
                    result += reMatch.m.group(1) + newline
                elif reMatch(r'(>.*)\n', s, pos):
                    result += reMatch.m.group(1) + newline
                elif reMatch(r'(.+)\n(?=>)', s, pos):
                    result += reMatch.m.group(1) + newline
                elif reMatch(r'(.+?)( *)\n(?=(.+))', s, pos):
                    sl, ss, sn = reMatch.m.group(1, 2, 3)
                    result += sl
                    if sn.startswith(' '):
                        result += newline
                    elif len(ss):
                        result += ss
                    elif len(sl):
                        if lb.breakingRule(sl, sn) == LineBreak.INDIRECT:
                            result += ' '
                elif reMatch(r'(.+)\n', s, pos):
                    result += reMatch.m.group(1) + newline
                elif reMatch(r'(.+)', s, pos):
                    result += reMatch.m.group(1) + newline
                    break
                pos += len(reMatch.m.group(0))
        else:
            prefix = None
            pos = 0
            while pos < len(s):
                if reMatch(r'(>+) ?(.*?)( ?)\n', s, pos):
                    sp, sl, ss = reMatch.m.group(1, 2, 3)
                    if prefix is None:
                        result += sp + ' ' + sl
                    elif sp != prefix:
                        result += newline + sp + ' ' + sl
                    else:
                        result += sl
                    if not len(ss):
                        result += newline
                        prefix = None
                    else:
                        prefix = sp
                        if not delsp:
                            result += ss
                elif reMatch(r' ?(.*?)( ?)\n', s, pos):
                    sl, ss = reMatch.m.group(1, 2)
                    if prefix is None:
                        result += sl
                    elif prefix != '':
                        result += newline + sl
                    else:
                        result += sl
                    if not len(ss):
                        result += newline
                        prefix = None
                    else:
                        if not delsp:
                            result += ss
                        prefix = ''
                elif reMatch(r' ?(.*)', s, pos):
                    result += reMatch.m.group(1) + newline
                    break
                pos += len(reMatch.m.group(0))
    return result

###
### Exception
###
# On Python 2.4 or earlier, exception is classical class.  So derive the
# base object class to make sure that this exception is new style class.

class LineBreakException(_textseg.LineBreakException, object):
    '''\
See :attr:`urgent<textseg.LineBreak.urgent>` attribute of
:class:`LineBreak` class.
    '''
    pass

###
### Class for Line breaking
###

class LineBreak(_textseg.LineBreak):
    '''\
LineBreak class performs Line Breaking Algorithm described in Unicode 
Standard Annex #14 ([UAX14]_). :term:`East_Asian_Width` informative properties 
defined by Annex #11 ([UAX11]_) will be concerned to determine breaking 
positions.
'''

    MANDATORY = 4
    DIRECT = 3
    INDIRECT = 2
    PROHIBITED = 1

    #: Dictionary containing default values of instance attributes.
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
initial attribute values.  See documentations of instance attributes.
Initial defaults are:

    *break_indent=False*,
    *charmax=998*,
    *eastasian_context=False*,
    *eaw=None*,
    *format="SIMPLE"*,
    *hangul_as_al=False*,
    *lbc=None*,
    *legacy_cm=True*,
    *minwidth=0*,
    *newline="\\\\n"*,
    *prep=[None]*,
    *sizing="UAX11"*,
    *urgent=None*,
    *virama_as_joiner=True*,
    *width=70*
'''
        for k, v in list(self.DEFAULTS.items()):
            kwds.setdefault(k, v)
        kwds.setdefault('linebreakType', type(self))
        kwds.setdefault('gcstrType', GCStr)
        kwds.setdefault('exceptionType', LineBreakException)
        _textseg.LineBreak.__init__(self, **kwds)

###
### Class for Grapheme cluster string
###

class GCStr(_textseg.GCStr):
    '''\
GCStr class treats Unicode string as a sequence of 
:term:`extended grapheme clusters<grapheme cluster>` defined by 
Unicode Standard Annex #29 ([UAX29]_).'''

    PROHIBIT_BEFORE = 1
    ALLOW_BEFORE = 2

    def __new__(cls, string, lb = None):
        '''\
GCStr(string[, lb]) -> GCStr

Create new grapheme cluster string (GCStr object) from Unicode string
*string*.

Optional LineBreak object *lb* controls breaking features.  Following
attributes of LineBreak object affect new GCStr object.

- :attr:`eastasian_context<LineBreak.eastasian_context>`
- :attr:`eaw<LineBreak.eaw>`
- :attr:`lbc<LineBreak.lbc>`
- :attr:`legacy_cm<LineBreak.legacy_cm>`
- :attr:`virama_as_joiner<LineBreak.virama_as_joiner>`
'''

        if lb is None:
            lb = LineBreak()
        return _textseg.GCStr.__new__(cls, string, lb=lb)

    def center(self, width, fillchar=' '):
        """\
S.center(width[, fillchar]) -> GCStr

Return S centered in a string of *width* :term:`columns<number of columns>`.
Padding is done using the specified fill character (default is a space)"""

        fillchar = self * 0 + fillchar
        if width < self.cols + fillchar.cols:
            return self

        marg = (width - self.cols) // fillchar.cols
        right = marg // 2;
        return fillchar * (marg - right) + self + fillchar * right

    def endswith(self, suffix, start = 0, end = None):
        '''\
S.endswith(suffix[, start[, end]]) -> bool

Return True if S ends with the specified suffix, False otherwise.
With optional start, test S beginning at that position.
With optional end, stop comparing S at that position.
suffix can also be a tuple of strings to try.'''
        if isinstance(suffix, tuple):
            pass
        elif not isinstance(suffix, unicode):
            prefix = unicode(suffix)

        if end is None:
            return unicode(self[start:]).endswith(suffix)
        else:
            return unicode(self[start:end]).endswith(suffix)

    def expandtabs(self, tabsize=8):
        '''\
S.expandtabs([tabsize]) -> GCStr

Return a copy of S where all tab characters are expanded using spaces.
If *tabsize* is not given, a tab size of 8 columns is assumed.'''

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

    def join(self, iterable):
        """\
S.join(iterable) -> GCStr

Return a grapheme cluster string which is the concatenation of the strings 
in the *iterable*.  The separator between elements is S."""

        ret = self * 0
        first = True
        for s in iterable:
            if not first:
                ret += self + s
            else:
                ret += s
                first = False
        return ret

    def ljust(self, width, fillchar=' '):
        """\
S.ljust(width[, fillchar]) -> GCStr

Return S left-justified in a grapheme cluster string of *width* 
:term:`columns<number of columns>`.
Padding is done using the specified fill character (default is a space)."""

        fillchar = self * 0 + fillchar
        if width < self.cols + fillchar.cols:
            return self
        return self + fillchar * ((width - self.cols) // fillchar.cols)

    def rjust(self, width, fillchar=' '):
        """\
S.rjust(width[, fillchar]) -> GCStr

Return S right-justified in a string of *width* 
:term:`columns<number of columns>`.
Padding is done using the specified fill character (default is a space)."""

        fillchar = self * 0 + fillchar
        if width < self.cols + fillchar.cols:
            return self
        return fillchar * ((width - self.cols) // fillchar.cols) + self

    def splitlines(self, keepends = False):
        """\
S.splitlines([keepends]) -> [GCStr]

Return a list of the lines in S, breaking at line boundaries.
Line breaks are not included in the resulting list unless *keepends*
is given and true.

.. note::
   U+001C, U+001D and U+001E are not included in linebreak characters.
"""

        ret = []
        str_len = len(self)
        i = 0
        j = 0
        while i < str_len:
            while i < str_len and \
                  self[i].lbc not in (lbcBK, lbcCR, lbcLF, lbcNL):
                i += 1
            eol = i
            if i < str_len:
                i += 1
                if keepends:
                    eol = i
            ret.append(self[j:eol+1])
            j = i
            i += 1
        return ret

    def startswith(self, prefix, start = 0, end = None):
        '''\
S.startswith(prefix[, start[, end]]) -> bool

Return True if S starts with the specified prefix, False otherwise.
With optional start, test S beginning at that position.
With optional end, stop comparing S at that position.
prefix can also be a tuple of strings to try.'''
        if isinstance(prefix, tuple):
            pass
        elif not isinstance(prefix, unicode):
            prefix = unicode(prefix)

        if end is None:
            return unicode(self[start:]).startswith(prefix)
        else:
            return unicode(self[start:end]).startswith(prefix)

    """
    def translate(self, table):
        '''\
S.translate(table) -> GCStr

Return a copy of the GCStr object S, where all characters have been mapped
through the given translation table, which must be a mapping of
Unicode ordinals to Unicode ordinals, strings, or None.
Unmapped characters are left untouched. Characters mapped to None
are deleted.'''
        return self * 0 + unicode(self).translate(table)
    """

