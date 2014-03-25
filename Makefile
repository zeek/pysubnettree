# Makefile not needed to build module. Use "python setup.py install" instead.
#
# This Makefile generates the SWIG wrappers and the documentation.

DISTFILES=COPYING Makefile README SubnetTree.cc SubnetTree.h \
	SubnetTree.i SubnetTree.py SubnetTree_wrap.cc patricia.c patricia.h setup.py test.py

CLEAN=build SubnetTree_wrap.cc SubnetTree.py README.html *.pyc 

VERSION=`test -e VERSION && cat VERSION || cat ../VERSION`
BUILD=build
TGZ=pysubnettree-$(VERSION)

all: SubnetTree_wrap.cpp

SubnetTree_wrap.cpp SubnetTree.py: SubnetTree.i SubnetTree.h
	swig -c++ -python -o SubnetTree_wrap.cc SubnetTree.i

clean:
	rm -rf $(CLEAN)

.PHONY: dist
dist:
	@python setup.py sdist

distclean:
	rm -rf pysubnettree.egg-info
	rm -rf dist
	rm -rf build

upload:
	python setup.py sdist upload --sign --identity F8CB8019
