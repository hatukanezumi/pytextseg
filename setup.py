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

import os
import re
from glob import glob

try:
    from distutils2.core import setup, Extension
    from distutils2.command.build_clib import build_clib
except ImportError:
    try:
        from setuptools import setup, Extension
        from setuptools.command.build_clib import build_clib
    except ImportError:
        from distutils.core import setup, Extension
        from distutils.command.build_clib import build_clib

###############################################################################
# CONSTANTS
###############################################################################

SOMBOK_VERSION = '2.1.1'
UNICODE_VERSION = '6.1.0'
LIBTHAI_VERSION = '0.1.9'
PKG_CONFIG = os.environ.get('PKG_CONFIG', 'pkg-config')

###############################################################################
# UTILS
###############################################################################
try:
    from subprocess import getstatusoutput
except ImportError:
    from commands import getstatusoutput

def pkg_config(*packages, **kwds):
    flag_map = {'-I': 'include_dirs', '-L': 'library_dirs', '-l': 'libraries'}

    pkgs = ' '.join(["'%s'" % p for p in packages])
    stat, out = getstatusoutput("%s --libs --cflags %s" % (PKG_CONFIG, pkgs))
    if stat >> 8:
        raise RuntimeError(out)
    for token in out.split():
        kwds.setdefault(flag_map.get(token[:2]), []).append(token[2:])
    return kwds

###############################################################################
# CUSTOMIZE
###############################################################################
class my_build_clib(build_clib):
    def run(self):
        include_dir = os.path.join('sombok', 'include')

        h = open(os.path.join(include_dir, 'sombok.h.in'), 'rt').read()
        h = h.replace('#ifdef HAVE_CONFIG_H', '#if 1')
        try:
            pkg_config('libthai >= %s' % LIBTHAI_VERSION)
            print("# libthai support is enabled.")
            h = h.replace('"config.h"', '''\
<Python.h>
#    define USE_LIBTHAI "libthai-unknown"''')
        except RuntimeError:
            print("# libthai support is disabled.  You might want to install it. and to try again.")
            h = h.replace('"config.h"', '''\
<Python.h>
#    undef USE_LIBTHAI''')
        h = h.replace('@SOMBOK_UNICHAR_T@', 'Py_UCS4')
        h = h.replace('@PACKAGE_VERSION@', 'bundled')
        h = h.replace('@SOMBOK_UNICHAR_T_IS_WCHAR_T@', '')
        h = h.replace('@SOMBOK_UNICHAR_T_IS_UNSIGNED_INT@', '')
        h = h.replace('@SOMBOK_UNICHAR_T_IS_UNSIGNED_LONG@', '')
        sombok_h = open(os.path.join(include_dir, 'sombok.h'), 'wt')
        sombok_h.write(h)
        sombok_h.close()

        build_clib.run(self)

###############################################################################
# MAIN
###############################################################################
if __name__ == '__main__':

    try:
        ext_config = pkg_config('libthai >= %s' % LIBTHAI_VERSION,
                                include_dirs = ['.'])
    except RuntimeError:
        ext_config = {'include_dirs': ['.'], }
    libraries = []
    try:
        ext_config = pkg_config('sombok >= %s' % SOMBOK_VERSION, **ext_config)
    except RuntimeError:
        from distutils.sysconfig import get_python_inc

        sources = [s for s in glob(os.path.join('sombok', 'lib', '*.c'))
                     if not re.search(r'[0-9][.0-9]+\.c', s)]
        sources.append(os.path.join('sombok', 'lib', '%s.c' % UNICODE_VERSION))
        include_dir = os.path.join('sombok', 'include')

        libraries.append(('bundled_sombok',
                          {'sources': sources,
                           'include_dirs': [include_dir,
                                            get_python_inc(plat_specific=1)],
                          },
                         ))
        ext_config['include_dirs'].append(include_dir)

    setup(
        name="pytextseg",
        version='0.0.4',
        license="GNU General Public License (GPL)",
        description='Python module for text segmentation.',
        long_description=open('README', 'r').read(),
        author='Hatuka*nezumi - IKEDA Soji',
        author_email='hatuka@nezumi.nu',
        url="http://pypi.python.org/pypi/pytextseg/",
        download_url = '',
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
		**ext_config
	    ),
	    Extension(
		'_textseg_Consts',
		['textseg_Consts.c'],
		**ext_config
	    ),
	],
        libraries=libraries,
        cmdclass={'build_clib': my_build_clib, },
    )

