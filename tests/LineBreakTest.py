#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import unittest
from textseg import LineBreak

try:
    unicode, unichr
except NameError:
    unicode = str
    unichr = chr

def unistr(list):
    return ''.join([unichr(c) for c in list])

from textseg.Consts import eawZ, eawN, lbcID

LineBreak.DEFAULTS = {
    'charmax': 998,
    'eastasian_context': False,
    'eaw': { unistr([0x302E, 0x302F]): eawZ },
    'format': 'SIMPLE',
    'hangul_as_al': False,
    'lbc': None,
    'legacy_cm': True,
    'minwidth': 0,
    'newline': "\n",
    'prep': None,
    'sizing': "UAX11",
    'urgent': None,
    'virama_as_joiner': True,
    'width': 76,
}

SMALL_KANA = unistr([0x3041, 0x3043, 0x3045, 0x3047, 0x3049, 0x3063, 0x3083, 0x3085, 0x3087, 0x308E, 0x3095, 0x3096, 0x30A1, 0x30A3, 0x30A5, 0x30A7, 0x30A9, 0x30C3, 0x30E3, 0x30E5, 0x30E7, 0x30EE, 0x30F5, 0x30F6])

AMBIG_LATIN = unistr([0x00C6, 0x00D0, 0x00D8, 0x00DE, 0x00DF, 0x00E0, 0x00E1, 0x00E6, 0x00E8, 0x00E9, 0x00EA, 0x00EC, 0x00ED, 0x00F0, 0x00F2, 0x00F3, 0x00F8, 0x00F9, 0x00FA, 0x00FC, 0x00FE, 0x0101, 0x0111, 0x0113, 0x011B, 0x0126, 0x0127, 0x012B, 0x0131, 0x0132, 0x0133, 0x0138, 0x013F, 0x0140, 0x0141, 0x0142, 0x0144, 0x0148, 0x0149, 0x014A, 0x014B, 0x014D, 0x0152, 0x0153, 0x0166, 0x0167, 0x016B, 0x01CE, 0x01D0, 0x01D2, 0x01D4, 0x01D6, 0x01D8, 0x01DA, 0x01DC, 0x0251, 0x0261])

class LineBreakTest(unittest.TestCase):

    def doTest(self, pairs, **kwds):
        lb = LineBreak(**kwds)
        for infn, outfn in pairs:
            instring = unicode(open(os.path.join('test-data', infn + '.in'), 'rb').read(), 'utf-8')
            b = lb.wrap(instring)
            broken = ''.join([unicode(x) for x in b])
            outstring = unicode(open(os.path.join('test-data', outfn + '.out'), 'rb').read(), 'utf-8')
            self.assertEqual(broken, outstring)

    def test_01break(self):
        langs = ['ar', 'el', 'fr', 'he', 'ja', 'ja-a', 'ko', 'ru',
                 'vi', 'vi-decomp', 'zh']
        self.doTest([(x, x) for x in langs])

    def test_02hangul(self):
        self.doTest([('ko', 'ko.al')], hangul_as_al=True);
        self.doTest([('amitagyong', 'amitagyong')]);

    def test_03ns(self):
        self.doTest([('ja-k', 'ja-k')], width=72)
        self.doTest([('ja-k', 'ja-k.ns')],
                    lbc={ SMALL_KANA: lbcID }, width=72)

    def test_05urgent(self):
        self.doTest([('ecclesiazusae', 'ecclesiazusae')]);
        self.doTest([('ecclesiazusae', 'ecclesiazusae.ColumnsMax')],
                    urgent='FORCE')
        self.doTest([('ecclesiazusae', 'ecclesiazusae.CharactersMax')],
                    charmax=79)
        self.doTest([('ecclesiazusae', 'ecclesiazusae.ColumnsMin')],
                    minwidth=7, width=66, urgent='FORCE')
        try:
            self.doTest([('ecclesiazusae', 'ecclesiazusae')], urgent='RAISE')
        except:
            self.assertEqual(True, True)
        else:
            self.assertEqual(True, False)

    def test_06context(self):
        self.doTest([('fr', 'fr.ea')], eastasian_context=True)
        self.doTest([('fr', 'fr')], eastasian_context=True,
                    eaw={ AMBIG_LATIN: eawN })

    def test_07sea(self):
        from textseg.Consts import sea_support

        if sea_support is not None:
            print(('\nSA word segmentation supported: %s' % sea_support))
        else:
            print('\nSA word segmentation not supported.')
            return
        self.doTest([('th', 'th')], complex_breaking=True)

    def test_09uri(self):
        self.doTest([('uri', 'uri.break')], width=1, prep=['BREAKURI'])
        self.doTest([('uri', 'uri.nonbreak')], width=1, prep=['NONBREAKURI'])

    def test_11format(self):
        def format_func(self, state, s):
            if state.startswith('so'):
                return '    ' + state + '>' + unicode(s)
            if state.startswith('eo'):
                return '<' + state + "\n"
            return None
        self.doTest([(x, x + '.format') for x in ['fr', 'ja']],
                    format=format_func)
        self.doTest([(x, x + '.newline') for x in ['fr', 'ko']],
                    format='NEWLINE')
        self.doTest([(x, x + '.newline') for x in ['fr', 'ko']],
                    format='TRIM')

    def test_14sea_al(self):
        self.doTest([('th', 'th.al')], complex_breaking=False)

    def doTestArray(self, pairs, **kwds):
        for infn, outfn in pairs:
            instring = unicode(open(os.path.join('test-data',
                                                 infn + '.in'), 'rb').read(),
                               'utf-8')
            lb = LineBreak(**kwds)
            broken = [unicode(x) for x in lb.wrap(instring)]
            outstring = [unicode(x, 'utf-8') for x in \
                         open(os.path.join('test-data',
                                           outfn + '.out'), 'rb').readlines()]
            self.assertEqual(broken, outstring);

    def test_15array(self):
        # break
        self.doTestArray([(x, x) for x in ['ar', 'el', 'fr', 'ja', 'ja-a',
                                           'ko', 'ru', 'zh']])
        # urgent
        self.doTestArray([('ecclesiazusae', 'ecclesiazusae')])
        self.doTestArray([('ecclesiazusae', 'ecclesiazusae.ColumnsMax')],
                         urgent='FORCE')
        self.doTestArray([('ecclesiazusae', 'ecclesiazusae.CharactersMax')],
                         charmax=79)
        self.doTestArray([('ecclesiazusae', 'ecclesiazusae.ColumnsMin')],
                         minwidth=7, width=66, urgent='FORCE')
        try:
            self.doTestArray([('ecclesiazusae', 'ecclesiazusae')],
                             urgent='RAISE')
        except:
            # unittest module of Python 2.3.x does not have assertTrue()
            self.assertEqual(True, True)
        else:
            self.assertEqual(True, False)

        # format
        def fmt(self, stat, string):
            if stat.startswith('so'):
                return '    %s>%s' % (stat, unicode(string))
            if stat.startswith('eo'):
                return '<%s\n' % (stat,)
            return None

        self.doTestArray([(x, x + '.format') for x in ['fr', 'ja']],
                         format=fmt)
        self.doTestArray([(x, x + '.newline') for x in ['fr', 'ko']],
                         format="NEWLINE")
        self.doTestArray([(x, x + '.newline') for x in ['fr', 'ko']],
                         format="TRIM")

    def test_16regex(self):
        import re

        # Regex matching most of URL-like strings.
        urire = re.compile(r'''(?iux)\b(?:url:)?
        (?:[a-z][-0-9a-z+.]+://|news:|mailto:)
        [\x21-\x7E]+''')
        punctre = re.compile(r'\A[\".:;,>]+\Z', re.U)

        # Breaking URIs according to some CMoS rules.
        def breakURI(self, s):
            # 17.11 1.1: [/] ÷ [^/]
            # 17.11 2:   [-] ×
            # 6.17 2:   [.] ×
            # 17.11 1.2: ÷ [-~.,_?#%]
            # 17.11 1.3: ÷ [=&]
            # 17.11 1.3: [=&] ÷
            # Default:  ALL × ALL
            r = ''
            ret = []
            b = ''
            for c in s:
                if b == '':
                    r = c
                elif r.lower().endswith('url:'):
                    ret += [r]
                    r = c
                elif b in '/' and not c in '/' or \
                    not b in '-.' and c in '-~.,_?\#%=&' or \
                    b in '=&' or c in '=&':
                     if r != '':
                         ret += [r]
                     r = c
                else:
                    r += c
                b = c
            if r != '':
                ret += [r]

            # Won't break punctuations at end of matches.
            while 2 <= len(ret) and re.search(punctre, ret[-1]):
                c = ret[-1]
                ret = ret[0:-1]
                ret[-1] += c;
            return ret

        def nonBreak(self, s):
            return s

        # (REGEX, FUNC) pair
        self.doTest([('uri', 'uri.break')], width=1,
                    prep=[(urire, breakURI)])
        self.doTest([('uri', 'uri.nonbreak')], width=1,
                    prep=[(urire, nonBreak)])
        # [STRING, FUNC] pair
        self.doTest([('uri', 'uri.nonbreak')], width=1,
                     prep=[(urire.pattern, nonBreak)])
        # multiple patterns
        self.doTest([('uri', 'uri.break')], width=1,
                    prep=[(urire, breakURI),
                          (r'ftp://[\x21-\x7e]+', nonBreak)])
        self.doTest([('uri', 'uri.break.http')], width=1,
                    prep=[(r'ftp://[\x21-\x7e]+', nonBreak),
                          (urire, breakURI)])
        self.doTest([('uri', 'uri.nonbreak')], width=1,
                    prep=[(r'ftp://[\x21-\x7e]+', nonBreak),
                          (r'http://[\x21-\x7e]+', nonBreak),
                          (urire, breakURI)])


def suite():
    return unittest.makeSuite(LineBreakTest)

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())

