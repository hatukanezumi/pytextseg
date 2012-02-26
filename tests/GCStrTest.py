#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import unittest
from textseg import GCStr, LineBreak
from textseg.Consts import lbcAL, lbcGL

try:
    unicode, unichr
except NameError:
    unicode = str
    unichr = chr

def unistr(list):
    return ''.join([unichr(c) for c in list])

class GCStrTest(unittest.TestCase):

    def test_00GraphemeBreakTest(self):
        commentRe = re.compile(r'\s*#\s*')
        opRe = re.compile(r'\s*(?:' + unichr(0xF7) + '|' + unichr(0xD7) +
                          r')\s*')

        try:
            fp = open(os.path.join('test-data', 'GraphemeBreakTest.txt'), 'rb')
        except IOError:
            return

        print('')
        print(fp.readline().strip())
        print(fp.readline().strip())

        errs = 0
        tests = 0
        for l in fp.readlines():
            l = unicode(l, 'utf-8').rstrip()
            a = commentRe.split(l, 1)
            if len(a) > 1:
                desc = a[1]
            else:
                desc = ''
            l = a[0].strip()
            if not len(l):
                continue

            if l.startswith(unichr(0xF7)):      # ÷
                l = l[1:].lstrip()
            if l.endswith(unichr(0xF7)):        # ×
                l = l[:-1].rstrip()

            s = ''.join([unichr(int(c, 16))
                         for c in opRe.split(l)
                         if len(c) > 0])
            b = unistr([0x20, 0xF7, 0x20]).join([unistr([0x20, 0xD7, 0x20]).join(['%04X' % ord(c) for c in unicode(x)])
                                                 for x in GCStr(s)])
            try:
                self.assertEqual(b, l)
            except AssertionError:
                import sys
                import traceback
                print('Failed: %s' % desc)
                traceback.print_exc(0)
                errs = errs + 1
            tests = tests + 1
        fp.close()

        if errs > 0:
            raise AssertionError("%d of %d subtests are failed." % (errs, tests))

    def test_10gcstring01(self):
        s = unistr([0x300, 0, 0x0D, 0x41, 0x300, 0x301, 0x3042, 0xD, 0xA,
                    0xAC00, 0x11A8])
        r = unistr([0xAC00, 0x11A8, 0xD, 0xA, 0x3042, 0x41, 0x300, 0x301,
                    0xD, 0, 0x300])
        string = GCStr(s)
        self.assertEqual(len(string), 7)
        self.assertEqual(string.cols, 5)
        self.assertEqual(string.chars, 11)
        l = [unicode(c) for c in string]
        l.reverse()
        self.assertEqual(r, ''.join(l))

    def test_10gcstring02(self):
        string = GCStr(unistr([0x1112, 0x1161,
                               0x11AB, 0x1100, 0x1173, 0x11AF]))
        self.assertEqual(len(string), 2)
        self.assertEqual(string.cols, 4)
        self.assertEqual(string.chars, 6)
        self.assertEqual(string, string.__copy__())

    def test_10gcstring03(self):
        string = GCStr(unistr([0x1112, 0x1161,
                               0x11AB, 0x1100, 0x1173, 0x11AF]))
        s1 = unistr([0x1112, 0x1161])
        s2 = unistr([0x11AB, 0x1100, 0x1173, 0x11AF])
        g1 = GCStr(s1)
        g2 = GCStr(s2)
        self.assertEqual(g1 + g2, string)
        self.assertEqual(len(g1 + g2), 2)
        self.assertEqual((g1 + g2).cols, 4)
        self.assertEqual(string.chars, 6)
        self.assertEqual(g1 + s2, string)
        self.assertEqual(len(g1 + s2), 2)
        self.assertEqual((g1 + s2).cols, 4)
        self.assertEqual(string.chars, 6)
        self.assertEqual(s1 + g2, string)
        self.assertEqual(len(s1 + g2), 2)
        self.assertEqual((s1 + g2).cols, 4)
        self.assertEqual(string.chars, 6)
        s1 += g2
        self.assertEqual(s1, string)
        g1 += s2
        self.assertEqual(g1, string)

    def test_10gcstring04(self):
        string = GCStr(unistr([0x1112, 0x1161,
                               0x11AB, 0x1100, 0x1173, 0x11AF]))
        self.assertEqual(unicode(string[1:]),
                         unistr([0x1100, 0x1173, 0x11AF]))
        self.assertEqual(unicode(string[-1:]),
                         unistr([0x1100, 0x1173, 0x11AF]))
        self.assertEqual(string[0:-1],
                         unistr([0x1112, 0x1161, 0x11AB]))
        string[-1:] = "A"
        self.assertEqual(unicode(string),
                         unistr([0x1112, 0x1161, 0x11AB, 0x41]))
        string[2:] = "B"
        self.assertEqual(string,
                         unistr([0x1112, 0x1161, 0x11AB, 0x41, 0x42]))
        string[0:0] = "C"
        self.assertEqual(unicode(string),
                         unistr([0x43, 0x1112, 0x1161, 0x11AB, 0x41, 0x42]))

    def test_10gcstring05(self):
        s = [unistr([0x0300]), unistr([0x00]), unistr([0x0D]),
             unistr([0x41, 0x0300, 0x0301]), unistr([0x3042]),
             unistr([0x0D, 0x0A]), unistr([0xAC00, 0x11A8])]
        string = GCStr(''.join(s))
        self.assertEqual([unicode(c) for c in string], s)

    def test_17prop(self):
        lb = LineBreak(eastasian_context = True)

        for s in [unichr(0xA0)]:
            g = GCStr(s, lb)
            self.assertEqual(len(g), 1)
            self.assertEqual(g.lbc, lbcGL)

        g = GCStr(unistr([0xC2, 0xA0]), lb)
        self.assertEqual(len(g), 2)
        self.assertEqual(g.lbc, lbcAL)

        for s in [unichr(0xD7)]:
            g = GCStr(s, lb)
            self.assertEqual(len(g), 1)
            self.assertEqual(g.cols, 2)

        g = GCStr(unistr([0xC3, 0x97]), lb)
        self.assertEqual(len(g), 2)
        self.assertEqual(g.cols, 1)


def suite():
    return unittest.makeSuite(GCStrTest)

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())

