# Makefile not needed to build module. Use "python3 setup.py install" instead.
#
# This Makefile generates the SWIG wrappers and the documentation.

CLEAN=build SubnetTree_wrap.cc SubnetTree.py README.html *.pyc

VERSION=`test -e VERSION && cat VERSION || cat ../VERSION`
BUILD=build
TGZ=pysubnettree-$(VERSION)

all: SubnetTree_wrap.cc

SubnetTree_wrap.cc SubnetTree.py: SubnetTree.i include/SubnetTree.h
	swig -c++ -python -Iinclude -o SubnetTree_wrap.cc SubnetTree.i

clean:
	rm -rf $(CLEAN)

.PHONY: dist
dist:
	@python3 setup.py sdist
	@printf "Package: "; echo dist/*.tar.gz

distclean:
	rm -rf pysubnettree.egg-info
	rm -rf dist
	rm -rf build

.PHONY: upload
upload: twine-check dist
	twine upload -u zeek dist/pysubnettree-$(VERSION).tar.gz

.PHONY: twine-check
twine-check:
	@type twine > /dev/null 2>&1 || \
		{ \
		echo "Uploading to PyPi requires 'twine' and it's not found in PATH."; \
		echo "Install it and/or make sure it is in PATH."; \
		echo "E.g. you could use the following command to install it:"; \
		echo "\tpip3 install twine"; \
		echo ; \
		exit 1; \
		}

.PHONY : test
test:
	@make -C testing
