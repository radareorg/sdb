all: sdbsrc
	python setup.py build_ext -j 8 -i -I../../src

sdbsrc:
	git clone ../.. sdbsrc-git
	mv sdbsrc-git/src sdbsrc
	rm -rf sdbsrc-git

test t:
	python test.py

install i:
	python setup.py install

clean:
	rm -rf *.out *.bin *.exe *.o *.a *.so test build

sdist d dist:
	python setup.py sdist
	python setup.py bdist

pub: sdist
	twine upload --repository-url https://upload.pypi.org/legacy/ --verbose dist/*
