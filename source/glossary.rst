.. pytextseg documentation glossary

Glossary
========
  
.. glossary::
   :sorted:

   mandatory break
      Obligatory line breaking behavior defined by core
      rules and performed regardless of surrounding characters.

   direct break
      A line break opportunity exists between two adjacent characters.
      See also :term:`indirect break`, :term:`mandatory break`.

   indirect break
      A line break opportunity exists between two characters only if 
      they are separated by one or more spaces.
      See also :term:`direct break`, :term:`mandatory break`.

   alphabetic character
      Characters that usually no line breaks are allowed
      between pairs of them, except that other characters provide break
      oppotunities
      (this term is inaccurate from the point of view by grammatology).
      [UAX14]_ classifies most of alphabetic characters to 
      :term:`line breaking class` AL.
      See also :term:`ideographic character`.

   ideographic character
      Characters that usually allow line breaks both before and after 
      themselves
      (this term is inaccurate from the point of view by grammatology).
      [UAX14]_ classifies most of ideographic characters to 
      :term:`line breaking class` ID.
      ee also :term:`alphabetic character`.

   complex breaking
      Heuristic line breaking based on dictionary for several scripts
      on which breaking positions are not obvious by each characters.
      [UAX14]_ classifys characters of several South East Asian scripts
      which need complex breaking to :term:`line breaking class` SA.

   number of columns
      Number of columns of a string is not always equal to the number of 
      characters it contains:
      Each of characters is either **wide**, **narrow** or nonspacing;
      they occupy 2, 1 or 0 columns, respectively.
      Several characters may be both wide and narrow by the contexts they 
      are used.
      Characters may have more various widths by customization.

   grapheme cluster
      A concept defined by Unicode Standard Annex #29 ([UAX29]_).
      Grapheme cluster is a sequence of Unicode character(s) that consists 
      of one **grapheme base** and optional **grapheme extender** and/or 
      **"prepend" character**.  It is close in that people consider as 
      *character*.

   line breaking class
      Classification of Unicode characters defined by Unicode Standard
      Annex #14 ([UAX14]_).

   East_Asian_Width
      Informative property of Unicode characters defined by Unicode Standard
      Annex #11 ([UAX11]_).
      It corresponds to the "width" (glyph spacing) of each characters on 
      implenentations for East Asian encodings.
      See also :term:`number of columns`.

   non-starter
      The character that cannot be placed at beginning of lines.
      [UAX14]_ classifies non-starters to :term:`line breaking class` 
      NS or CJ.
      It includes small hiragana/katakana and some punctuations.

   ambiguous quotation mark
      *To be written*


