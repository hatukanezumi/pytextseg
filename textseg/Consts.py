# -*- python -*-
# -*- coding: utf-8 -*-
"""\
Constants for textseg package.

.. data:: eawNa
.. data:: eawN
.. data:: eawA
.. data:: eawW
.. data:: eawH
.. data:: eawF
.. data:: eawZ

   Index values to specify six :term:`East_Asian_Width` properties
   defined by [UAX #11], and eawZ to specify nonspacing.

   .. note::
      Property value ``Z`` is non-standard.

.. data:: lbcBK
.. data:: lbcCR
.. data:: lbcLF
.. data:: lbcNL
.. data:: lbcSP
.. data:: lbcOP
.. data:: lbcCL
.. data:: lbcCP
.. data:: lbcQU
.. data:: lbcGL
.. data:: lbcNS
.. data:: lbcEX
.. data:: lbcSY
.. data:: lbcIS
.. data:: lbcPR
.. data:: lbcPO
.. data:: lbcNU
.. data:: lbcAL
.. data:: lbcHL
.. data:: lbcID
.. data:: lbcIN
.. data:: lbcHY
.. data:: lbcBA
.. data:: lbcBB
.. data:: lbcB2
.. data:: lbcCB
.. data:: lbcZW
.. data:: lbcCM
.. data:: lbcWJ
.. data:: lbcH2
.. data:: lbcH3
.. data:: lbcJL
.. data:: lbcJV
.. data:: lbcJT
.. data:: lbcSG
.. data:: lbcAI
.. data:: lbcCJ
.. data:: lbcSA
.. data:: lbcXX

   Index values to specify 39 line breaking properties (classes)
   defined by [UAX #14].

   .. note::
      Property value ``CP`` was introduced by Unicode 5.2.0.
      Property value ``HL`` and ``CJ`` were introduced by Unicode 6.1.0.

.. data:: sea_support

   Flag to determin if word segmentation for South East Asian writing
   systems is enabled.  If this feature was enabled, a non-empty string
   is set.  Otherwise, ``None`` is set.

   .. note::
      Current release supports Thai script of modern Thai language only.

.. data:: unicode_version

   A string to specify version of Unicode Standard this module refers.

"""
# Copyright (C) 2012 by Hatuka*nezumi - IKEDA Soji.
#
# This file is part of the pytextseg package.  This program is free
# software; you can redistribute it and/or modify it under the terms of
# either the GNU General Public License or the Artistic License, as
# specified in the README file.

from _textseg_Consts import *

AMBIGUOUS_CYRILLIC = (0x0401, ) + tuple(range(0x0410, 0x044F+1)) + \
                     (0x0451, )
AMBIGUOUS_GREEK = tuple(range(0x0391, 0x03A9+1)) + \
                  tuple(range(0x03B1, 0x03C1+1)) + \
                  tuple(range(0x03C3, 0x03C9+1))
AMBIGUOUS_LATIN = (0x00C6, 0x00D0, 0x00D8, 0x00DE, 0x00DF, 0x00E0, 0x00E1,
                   0x00E6, 0x00E8, 0x00E9, 0x00EA, 0x00EC, 0x00ED, 0x00F0,
                   0x00F2, 0x00F3, 0x00F8, 0x00F9, 0x00FA, 0x00FC, 0x00FE,
                   0x0101, 0x0111, 0x0113, 0x011B, 0x0126, 0x0127, 0x012B,
                   0x0131, 0x0132, 0x0133, 0x0138, 0x013F, 0x0140, 0x0141,
                   0x0142, 0x0144, 0x0148, 0x0149, 0x014A, 0x014B, 0x014D,
                   0x0152, 0x0153, 0x0166, 0x0167, 0x016B, 0x01CE, 0x01D0,
                   0x01D2, 0x01D4, 0x01D6, 0x01D8, 0x01DA, 0x01DC, 0x0251,
                   0x0261, )
BACKWORD_GUILLEMETS = (0x00AB, 0x2039, )
BACKWORD_QUOTES = (0x2018, 0x201C, )
FORWARD_GUILLEMETS = (0x00BB, 0x203A, )
FORWARD_QUOTES = (0x2019, 0x201D, )
IDEOGRAPHIC_ITERATION_MARKS = (0x3005, 0x303B, 0x309D, 0x309E,
                               0x30FD, 0x30FE, )
KANA_PROLONGED_SOUND_MARKS = (0x30FC, 0xFF70, )
KANA_SMALL_LETTERS = (0x3041, 0x3043, 0x3045, 0x3047, 0x3049, 0x3063,
                      0x3083, 0x3085, 0x3087, 0x308E,
                      0x3095, 0x3096,
                      0x30A1, 0x30A3, 0x30A5, 0x30A7, 0x30A9, 0x30C3,
                      0x30E3, 0x30E5, 0x30E7, 0x30EE,
                      0x30F5, 0x30F6, ) + \
                     tuple(range(0x31F0, 0x31FF+1)) + \
                     tuple(range(0xFF67, 0xFF6F+1))
MASU_MARK = (0x303C, )
QUESTIONABLE_NARROW_SIGNS = (0x00A2, 0x00A3, 0x00A5, 0x00A6, 0x00AC,
                             0x00AF, )

AMBIGUOUS_ALPHABETICS = AMBIGUOUS_CYRILLIC + AMBIGUOUS_GREEK + \
                        AMBIGUOUS_LATIN
KANA_NONSTARTERS = IDEOGRAPHIC_ITERATION_MARKS + \
                   KANA_PROLONGED_SOUND_MARKS + KANA_SMALL_LETTERS + \
                   MASU_MARK

