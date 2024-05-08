#! /usr/bin/env python3

import sys
import os

cflags = os.environ.get("CFLAGS", "")
os.environ["CFLAGS"] = cflags + " -fno-strict-aliasing"

from setuptools import setup
from distutils.core import Extension

setup(
    version="0.37", # Filled in automatically.
    py_modules=['SubnetTree'],
    url="https://github.com/zeek/pysubnettree",
    ext_modules = [
        Extension("_SubnetTree",
            sources=["SubnetTree.cc", "patricia.c", "SubnetTree_wrap.cc"],
            depends=["include/SubnetTree.h", "include/patricia.h"],
            include_dirs=["include/"])
        ],
)
