ifneq ($(shell python3.12 --help),)
PYTHON?=python3.12
else
PYTHON?=python
endif

all: sdbsrc
	$(PYTHON) setup.py build_ext -j 8 -i -I../../include

sdbsrc:
	git clone ../.. sdbsrc-git
	mv sdbsrc-git/src sdbsrc
	rm -rf sdbsrc-git
	cp -f ../../include/sdb/version.h sdbsrc

test t:
	$(PYTHON) test.py

install i:
	$(PYTHON) setup.py install

clean:
	rm -rf dist
	rm -rf *.out *.bin *.exe *.o *.a *.so test build

sdist d dist: sdbsrc
	$(PYTHON) setup.py sdist
	# python setup.py bdist
	tar xzvf dist/r2sdb-*.tar.gz
	cd r2sdb-* && python setup.py build

pub: sdist
	twine upload --repository-url https://upload.pypi.org/legacy/ --verbose dist/*
