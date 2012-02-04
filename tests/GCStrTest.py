#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import unittest
from textseg import GCStr

try:
    unicode, unichr
except NameError:
    unicode = str
    unichr = chr

def unistr(list):
    return ''.join([unichr(c) for c in list])

class GCStrTest(unittest.TestCase):

    def test_01(self):
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

    def test_02(self):
        string = GCStr(unistr([0x1112, 0x1161,
                               0x11AB, 0x1100, 0x1173, 0x11AF]))
        self.assertEqual(len(string), 2)
        self.assertEqual(string.cols, 4)
        self.assertEqual(string.chars, 6)
        self.assertEqual(string, string.__copy__())

    def test_03(self):
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

    def test_04(self):
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

    def test_05(self):
        s = [unistr([0x0300]), unistr([0x00]), unistr([0x0D]),
             unistr([0x41, 0x0300, 0x0301]), unistr([0x3042]),
             unistr([0x0D, 0x0A]), unistr([0xAC00, 0x11A8])]
        string = GCStr(''.join(s))
        self.assertEqual([unicode(c) for c in string], s)

def suite():
    return unittest.makeSuite(GCStrTest)

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())

