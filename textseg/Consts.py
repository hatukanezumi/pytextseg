# -*- python -*-
# -*- coding: utf-8 -*-
"""Constants for textseg package

_`eawNa`, _`eawN`, _`eawA`, _`eawW`, _`eawH`, _`eawF`, _`eawZ`
    Index values to specify 6 East_Asian_Width properties
    defined by [UAX #11], and EA_Z to specify nonspacing.

_`lbcBK`, _`lbcCR`, _`lbcLF`, _`lbcNL`, _`lbcSP`, _`lbcOP`, _`lbcCL`, _`lbcCP`, _`lbcQU`, _`lbcGL`, _`lbcNS`, _`lbcEX`, _`lbcSY`, _`lbcIS`, _`lbcPR`, _`lbcPO`, _`lbcNU`, _`lbcAL`, _`lbcHL`, _`lbcID`, _`lbcIN`, _`lbcHY`, _`lbcBA`, _`lbcBB`, _`lbcB2`, _`lbcCB`, _`lbcZW`, _`lbcCM`, _`lbcWJ`, _`lbcH2`, _`lbcH3`, _`lbcJL`, _`lbcJV`, _`lbcJT`, _`lbcSG`, _`lbcAI`, _`lbcCJ`, _`lbcSA`, _`lbcXX`
    Index values to specify 39 line breaking properties (classes)
    defined by [UAX #14].

*Note*:
    Property value ``CP`` was introduced by Unicode 5.2.0.
    Property value ``HL`` and ``CJ`` may be introduced by Unicode 6.1.0.

_`sea_support`
    Flag to determin if word segmentation for South East Asian writing
    systems is enabled.  If this feature was enabled, a non-empty string
    is set.  Otherwise, ``None`` is set.

*N.B.*:
    Current release supports Thai script of modern Thai language only.

_`unicode_version`
    A string to specify version of Unicode standard this module refers.
"""

from _textseg_Consts import *

