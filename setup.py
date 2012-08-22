#! /usr/bin/env python

import sys
import os

cflags = os.environ.get("CFLAGS", "")
os.environ["CFLAGS"] = cflags + " -fno-strict-aliasing"

from distutils.core import setup, Extension

setup(name="pysubnettree",
    version="0.19", # Filled in automatically.
    author_email="info@bro-ids.org",
    license="BSD",
    py_modules=['SubnetTree'],
    ext_modules = [
        Extension("_SubnetTree", ["SubnetTree.cc", "patricia.c", "SubnetTree_wrap.cc"]),
        ]
)

