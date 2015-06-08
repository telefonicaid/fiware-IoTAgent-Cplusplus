
TARGETS := $(patsubst %.json,%.test,$(wildcard *.json))

all: $(TARGETS)

%.test: %.json
	python convert.py < $< > $@
