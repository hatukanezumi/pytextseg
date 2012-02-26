#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (C) 2012 Hatuka*nezumi - IKEDA Soji
#
# This file is part of pytextseg.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.

has_setuptools = False
try:
    from setuptools import setup, Extension
    has_setuptools = True
except ImportError:
    from distutils.core import setup, Extension

###############################################################################
# CONSTANTS
###############################################################################

INSTALL_CONFIG_DIR = '/etc/pytextseg'
INSTALL_DOC_DIR = 'share/doc/pytextseg'

###############################################################################
# MAIN
###############################################################################
if __name__ == '__main__':
    setup(
        name="pytextseg",
        version='0.0.3',
        license="GNU General Public License (GPL)",
        description='Python module for text segmentation.',
        long_description=open('README', 'r').read(),
        author='Hatuka*nezumi - IKEDA Soji',
        author_email='hatuka@nezumi.nu',
        url="",
        download_url="",
        platforms=["any"],
        classifiers=[
            "Development Status :: 3 - Alpha",
	    "Environment :: Other Environment",
	    "Intended Audience :: Developers",
	    "License :: OSI Approved :: GNU General Public License (GPL)",
	    "Operating System :: OS Independent",
	    "Programming Language :: Python",
	    "Programming Language :: Python :: 2",
	    "Programming Language :: Python :: 3",
	    "Programming Language :: C",
	    "Topic :: Software Development :: Internationalization",
	    "Topic :: Software Development :: Libraries :: Python Modules",
	    "Topic :: Text Processing :: General",
	    "Topic :: Text Processing :: Linguistic"
	],
        scripts=[],
	packages=['textseg'],
	ext_modules=[
	    Extension(
		'_textseg',
		['textseg.c'],
		libraries=['sombok', 'thai'],
		include_dirs=['.', 'sombok/include'],
		library_dirs=['sombok/.libs'],
	    ),
	    Extension(
		'_textseg_Consts',
		['textseg_Consts.c'],
		libraries=['sombok'],
		include_dirs=['sombok/include'],
		library_dirs=['sombok/.libs'],
	    ),
	],
	test_suite='tests.alltests.suite'
    )

