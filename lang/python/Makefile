default: all

all: epython

epython:
	python setup.py build

clean:
	@(echo "cleaning eclipse-python object files...")
	python setup.py clean

veryclean:
	@(echo "cleaning eclipse-python object files and libraries...")
	python setup.py clean --all

install:
	python setup.py install
