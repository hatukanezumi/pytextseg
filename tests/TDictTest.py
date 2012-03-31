#! /usr/bin/env python
# -*- coding: utf-8 -*-
'''
Copyright (C) 2012 by Hatuka*nezumi - IKEDA Soji.

This file is part of the pytextseg package.  This program is free
software; you can redistribute it and/or modify it under the terms of
either the GNU General Public License or the Artistic License, as
specified in the README file.
'''
import unittest
from random import randrange
from textseg import LineBreak

class TDictTest(unittest.TestCase):

    def test_setitem(self):
        lb = LineBreak(lbc=None, eaw=None)
        lbc = {}
        eaw = {}

        count = 0
        while count < 1000:
            beg = randrange(0, 512)
            end = randrange(0, 512)
            if beg > end:
                beg, end = end, beg
            prop = randrange(0, 39)
            idx = randrange(0, 2)
            for c in range(beg, end + 1):
                if idx:
                    lbc[c] = prop
                else:
                    eaw[c] = prop
            if idx:
                lb.lbc[tuple(range(beg, end + 1))] = prop
            else:
                lb.eaw[tuple(range(beg, end + 1))] = prop

            #print("(%5X-%5X) = %d: %d" % (beg, end, idx, prop))
            #if idx:
            #    lb.lbc._dump()
            #else:
            #    lb.eaw._dump()

            for c in range(0, 512):
                if c not in lbc:
                    try:
                        lb.lbc[c]
                    except KeyError:
                        pass
                    else:
                        raise
                else:
                    self.assertEqual(lbc[c], lb.lbc[c])
                if c not in eaw:
                    try:
                        lb.eaw[c]
                    except KeyError:
                        pass
                    else:
                        raise
                else:
                    self.assertEqual(eaw[c], lb.eaw[c])

            count = count + 1

    def test_merge(self):
        lb = LineBreak(lbc=None, eaw=None)
        lb2 = LineBreak(lbc=None, eaw=None)
        lbc = {}
        eaw = {}

        count = 0
        while count < 1000:
            if not (randrange(0, 100)):
                lb.lbc.clear()
                lb.eaw.clear()
                lbc.clear()
                eaw.clear()
                lb2 = LineBreak(lbc=None, eaw=None)

            beg = randrange(0, 512)
            end = randrange(0, 512)
            if beg > end:
                beg, end = end, beg
            prop = randrange(0, 39)
            idx = randrange(0, 2)
            for c in range(beg, end + 1):
                if idx:
                    lbc[c] = prop
                else:
                    eaw[c] = prop
            if idx:
                lb2.lbc[tuple(range(beg, end + 1))] = prop
                lb.lbc.update(lb2.lbc)
            else:
                lb2.eaw[tuple(range(beg, end + 1))] = prop
                lb.eaw.update(lb2.eaw)

            #print("(%5X-%5X) = %d: %d" % (beg, end, idx, prop))
            #if idx:
            #    lb.lbc._dump()
            #else:
            #    lb.eaw._dump()

            for c in range(0, 512):
                if c not in lbc:
                    try:
                        lb.lbc[c]
                    except KeyError:
                        pass
                    else:
                        raise
                else:
                    self.assertEqual(lbc[c], lb.lbc[c])
                if c not in eaw:
                    try:
                        lb.eaw[c]
                    except KeyError:
                        pass
                    else:
                        raise
                else:
                    self.assertEqual(eaw[c], lb.eaw[c])

            count = count + 1

def suite():
    return unittest.makeSuite(TDictTest)

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())

