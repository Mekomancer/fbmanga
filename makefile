.PHONY: run build
run: build
	./fbmanga dat/x1*

build:
	ninja
