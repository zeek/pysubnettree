#! /usr/bin/env python

import sys
import os

cflags = os.environ.get("CFLAGS", "")
os.environ["CFLAGS"] = cflags + " -fno-strict-aliasing"

from setuptools import setup
from distutils.core import Extension

setup(name="pysubnettree",
    version="0.22", # Filled in automatically.
    author_email="info@bro.org",
    license="BSD",
    py_modules=['SubnetTree'],
    ext_modules = [
        Extension("_SubnetTree",
            sources=["SubnetTree.cc", "patricia.c", "SubnetTree_wrap.cc"],
            depends=["SubnetTree.h", "patricia.h"]),
        ]
)

