.. pytextseg documentation master file.

=========================
pytextseg's documentation
=========================

The pytextseg package provides functions to wrap plain texts:
:func:`fill<textseg.fill>` and :func:`wrap<textseg.wrap>` are
Unicode-aware alternatives for those of :mod:`textwrap` standard module;
:func:`fold<textseg.fold>` and :func:`unfold<textseg.unfold>` are
functions mainly focus on plain text messages such as e-mail.

It also provides lower level interfaces for text segmentation:
:class:`LineBreak<textseg.LineBreak>` class for line breaking;
:class:`GCStr<textseg.GCStr>` class for grapheme cluster segmentation.

.. toctree::
   :maxdepth: 3

   install
   doc
   custom
   notes
   changes
   glossary
   back

