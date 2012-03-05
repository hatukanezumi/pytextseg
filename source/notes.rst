.. pytextseg documentation implementation notes

====================
Implementation Notes
====================

Character properties this module is based on are defined by
Unicode Standards version 6.1.0.

UAX #14 and UAX #11
===================

* Character(s) assigned to CB are not resolved.

* Characters assigned to CJ are always resolved to NS.
  More flexible tailoring mechanism is provided.

* When word segmentation for South East Asian writing systems is not supported,
  characters assigned to SA are resolved to AL,
  except that characters that have Grapheme_Cluster_Break property value
  Extend or SpacingMark be resolved to CM.

* Characters assigned to SG or XX are resolved to AL.

* Code points of following UCS ranges are given fixed property values even
  if they have not been assigned any characers.

  +--------------------+------------+------------+----------------+
  | Ranges             | UAX #14    | UAX #11    | Description    |
  +====================+============+============+================+
  | U+3400..U+4DBF     | ID         | W          | CJK ideographs |
  +--------------------+------------+------------+----------------+
  | U+4E00..U+9FFF     | ID         | W          | CJK ideographs |
  +--------------------+------------+------------+----------------+
  | U+D800..U+DFFF     | AL (SG)    | N          | Surrogates     |
  +--------------------+------------+------------+----------------+
  | U+E000..U+F8FF     | AL (XX)    | F or N (A) | Private use    |
  +--------------------+------------+------------+----------------+
  | U+F900..U+FAFF     | ID         | W          | CJK ideographs |
  +--------------------+------------+------------+----------------+
  | U+20000..U+2FFFD   | ID         | W          | CJK ideographs |
  +--------------------+------------+------------+----------------+
  | U+30000..U+3FFFD   | ID         | W          | Old hanzi      |
  +--------------------+------------+------------+----------------+
  | U+F0000..U+FFFFD   | AL (XX)    | F or N (A) | Private use    |
  +--------------------+------------+------------+----------------+
  | U+100000..U+10FFFD | AL (XX)    | F or N (A) | Private use    |
  +--------------------+------------+------------+----------------+
  | Other unassigned   | AL (XX)    | N          | Unassigned,    |
  |                    |            |            | reserved or    |
  |                    |            |            | noncharacters  |
  +--------------------+------------+------------+----------------+

* Characters belonging to General Category Mn, Me, Cc, Cf, Zl or Zp have the
  property value Z (nonspacing) defined by this module, regardless of
  East_Asian_Width property values assigned by [UAX11]_.

UAX #29
=======

* This module implements *default* algorithm for determining grapheme cluster
  boundaries.  Tailoring mechanism has not been supported yet.

