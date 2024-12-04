#! /usr/bin/env python3

import os

cflags = os.environ.get("CFLAGS", "")
os.environ["CFLAGS"] = cflags + " -fno-strict-aliasing"

from distutils.core import Extension

from setuptools import setup

setup(
    version="0.37.dev5",  # Filled in automatically.
    py_modules=["SubnetTree"],
    url="https://github.com/zeek/pysubnettree",
    ext_modules=[
        Extension(
            "_SubnetTree",
            sources=["SubnetTree.cc", "patricia.c", "SubnetTree_wrap.cc"],
            depends=["include/SubnetTree.h", "include/patricia.h"],
            include_dirs=["include/"],
        )
    ],
)
