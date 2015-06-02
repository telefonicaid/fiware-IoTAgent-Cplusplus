
tests: 02-enum.test

02-enum.test: 02-enum.yaml
	python -c "import json; import yaml; import sys; json.dump(yaml.load(sys.stdin), sys.stdout);" < $< > $@
