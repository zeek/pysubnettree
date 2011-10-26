#! /usr/bin/env python

import sys

from distutils.core import setup, Extension

setup(name="pysubnettree",
    version="0.14", # Filled in automatically.
    author_email="info@bro-ids.org",
    license="BSD",
    py_modules=['SubnetTree'],
    ext_modules = [
        Extension("_SubnetTree", ["SubnetTree.cc", "patricia.c", "SubnetTree_wrap.cc"]),
        ]
)

