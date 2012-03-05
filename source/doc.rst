.. pytextseg documentation module documentations

================
Package Contents
================

textseg module
==============

.. automodule:: textseg
   :members: fill, wrap, fold, Consts

   GCStr class
   -----------
   .. autoclass:: GCStr
      :members:
      :inherited-members:

      .. automethod:: __new__

      Most of operations for string object are available on GCStr object.

      +------------------+------------------------------------+----------+
      | Operation        | Result                             | Notes    |
      +==================+====================================+==========+
      | ``x in s``       | ``True`` if *s* contains a grapheme| \(1)     |
      |                  | cluster *x*, else ``False``        |          |
      +------------------+------------------------------------+----------+
      | ``x not in s``   | ``False`` if *s* contains a        | \(1)     |
      |                  | grapheme cluster *x*, else ``True``|          |
      +------------------+------------------------------------+----------+
      | ``s + t``        | the concatenation of *s* and *t*   | \(2) (3) |
      +------------------+------------------------------------+----------+
      | ``s * n, n * s`` | *n* copies of *s* concatenated     | \(3)     |
      +------------------+------------------------------------+----------+
      | ``s[i]``         | *i*\ th grapheme cluster of *s*,   |          |
      |                  | origin 0                           |          |
      +------------------+------------------------------------+----------+
      | ``s[i:j]``       | slice of *s* from *i* to *j*       |          |
      +------------------+------------------------------------+----------+
      | ``s[i:j:k]``     | slice of *s* from *i* to *j* with  |          |
      |                  | step *k*                           |          |
      +------------------+------------------------------------+----------+
      | ``len(s)``       | number of grapheme clusters *s*    | \(4)     |
      |                  | contains                           |          |
      +------------------+------------------------------------+----------+
      | ``min(s)``       | smallest grapheme cluster of *s*   |          |
      +------------------+------------------------------------+----------+
      | ``max(s)``       | largest grapheme cluster of *s*    |          |
      +------------------+------------------------------------+----------+
      | ``s < t``        | strictly less than                 | \(5)     |
      +------------------+------------------------------------+----------+
      | ``s <= t``       | less than or equal                 | \(5)     |
      +------------------+------------------------------------+----------+
      | ``s > t``        | strictly greater than              | \(5)     |
      +------------------+------------------------------------+----------+
      | ``s >= t``       | greater than or equal              | \(5)     |
      +------------------+------------------------------------+----------+
      | ``s == t``       | equal                              | \(5)     |
      +------------------+------------------------------------+----------+
      | ``s != t``       | not equal                          | \(5)     |
      +------------------+------------------------------------+----------+
      | ``str(s)``,      | string representation of object.   |          |
      | ``unicode(s)``   | unicode() is used by Python 2.x.   |          |
      +------------------+------------------------------------+----------+

      Notes:

      (1) *x* may be Unicode string.

      (2) One of operands may be Unicode string.

      (3) Note that number of columns (see cols) or grapheme clusters
          (see len()) of resulting grapheme cluster string is not always 
          equal to sum of both strings.

      (4) See also chars and cols properties.

      (5) Comparisons are performed by Unicode string value, not concerning
          grapheme cluster boundaries.

      |


   LineBreak class
   ---------------
   .. autoclass:: LineBreak
      :members:
      :inherited-members:

      .. automethod:: __init__

   Functions
   ---------

textseg.Consts module
=====================

.. automodule:: textseg.Consts
   :members:
   :inherited-members:

