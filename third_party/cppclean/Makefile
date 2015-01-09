check:
	pylint \
		--reports=no \
		--rcfile=/dev/null \
		--dummy-variables-rgx='^_+$$' \
		--disable=bad-continuation \
		--disable=duplicate-code \
		--disable=fixme \
		--disable=missing-docstring \
		--disable=too-many-arguments \
		--disable=invalid-name \
		--disable=too-many-locals \
		--disable=too-many-return-statements \
		--disable=too-many-instance-attributes \
		--disable=too-many-public-methods \
		--disable=too-many-branches \
		--disable=too-many-lines \
		--disable=too-many-statements \
		--disable=no-self-use \
		--disable=unused-argument \
		--disable=too-few-public-methods \
		cpp cppclean setup.py
	pep8 cpp $(wildcard *.py)
	check-manifest
	python setup.py --long-description | rstcheck -

coverage:
	@coverage erase
	@PYTHON='coverage run --parallel-mode' ./test.bash
	@coverage combine
	@coverage report

open_coverage: coverage
	@coverage html
	@python -m webbrowser -n "file://${PWD}/htmlcov/index.html"

readme:
	@restview --long-description --strict
