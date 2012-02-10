# -*- python -*-
# -*- coding: utf-8 -*-
"""Constants for textseg package

eawNa, eawN, eawA, eawW, eawH, eawF, eawZ

Index values to specify 6 East_Asian_Width properties
defined by [UAX #11], and EA_Z to specify nonspacing.

lbcBK, lbcCR, lbcLF, lbcNL, lbcSP, lbcOP, lbcCL, lbcCP, lbcQU, lbcGL, lbcNS, lbcEX, lbcSY, lbcIS, lbcPR, lbcPO, lbcNU, lbcAL, lbcHL, lbcID, lbcIN, lbcHY, lbcBA, lbcBB, lbcB2, lbcCB, lbcZW, lbcCM, lbcWJ, lbcH2, lbcH3, lbcJL, lbcJV, lbcJT, lbcSG, lbcAI, lbcCJ, lbcSA, lbcXX

Index values to specify 39 line breaking properties (classes)
defined by [UAX #14].

*Note*: Property value CP was introduced by Unicode 5.2.0.
Property value HL and CJ may be introduced by Unicode 6.1.0.

sea_support

Flag to determin if word segmentation for South East Asian writing systems 
is enabled.  If this feature was enabled, a non-empty string is set.  
Otherwise, None is set.

*N.B.*: Current release supports Thai script of modern Thai language only.

unicode_version

A string to specify version of Unicode standard this module refers.
"""

from _textseg_Consts import *

