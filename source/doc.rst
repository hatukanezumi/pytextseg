.. pytextseg documentation module documentations

================
Package Contents
================

textseg module
==============

.. automodule:: textseg

   If you are inpatient, see ":ref:`Functions`".

   .. _`Functions`:

   Functions
   ---------

   .. autofunction:: fold

   .. autofunction:: unfold

   textwrap Style Functions
   ^^^^^^^^^^^^^^^^^^^^^^^^

   .. autofunction:: fill

   .. autofunction:: wrap

   GCStr class
   -----------
   .. autoclass:: GCStr
      :members: chars,
                cols,
                lbc,
                lbcext

      .. automethod:: __new__

      .. automethod:: center

      .. automethod:: endswith

      .. automethod:: expandtabs

      .. .. automethod:: flag(offset[, value])

      .. automethod:: join

      .. automethod:: ljust

      .. automethod:: rjust

      .. automethod:: splitlines

      .. automethod:: startswith

      .. method:: translate(table)
      .. deprecated:: 0.1.0
         See ":ref:`Methods not Supported`".

      **String Operations**

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

      (3) Note that number of columns (see :attr:`cols`) or grapheme clusters
          (see len()) of resulting grapheme cluster string is not always 
          equal to sum of both strings.

      (4) See also :attr:`chars` and :attr:`cols` attributes.

      (5) Comparisons are performed by Unicode string value, not concerning
          grapheme cluster boundaries.

      GCStr object can not be operand of :mod:`re` regular expression
      operations.

      .. _`Methods not Supported`:

      **Methods not Supported**

      Some string methods are not supported since they break grapheme
      cluster boundaries.  Instead, use methods of stringified objects.
      For example::

          # For Python 3
          result = gcs * 0 + str(gcs).translate(table)
          # For Python 2
          result = gcs * 0 + unicode(gcs).translate(table)

      ``gcs * 0 + ...`` is a convenient way to recalculate grapheme
      clusters.

      **Instance Attributes**

      These attributes are read-only.


   LineBreak class
   ---------------
   .. autoclass:: LineBreak
      :members: break_indent,
                charmax,
                complex_breaking,
                eastasian_context,
                eaw,
                format,
                hangul_as_al,
                lbc,
                legacy_cm,
                minwidth,
                newline,
                prep,
                sizing,
                urgent,
                virama_as_joiner,
                width

      .. automethod:: __init__

      .. automethod:: breakingRule(before, after)

      .. automethod:: wrap(text)

      **Class Attributes**

      .. attribute:: DEFAULTS

         Dictionary containing default values of instance attributes.

      .. attribute:: MANDATORY
      .. attribute:: DIRECT
      .. attribute:: INDIRECT
      .. attribute:: PROHIBITED

         Four values to specify line breaking behaviors:
         Mandatory break; Both direct break and indirect break are allowed;
         Indirect break is allowed but direct break is prohibited;
         Prohibited break.

      **Instance Attributes**

      About default values of these attributes see :meth:`__init__`.

   Exception
   ---------
   .. autoexception:: LineBreakException


textseg.Consts module
=====================

.. automodule:: textseg.Consts
   :members:
   :inherited-members:

   See also ":ref:`Tailoring Character Properties`".

