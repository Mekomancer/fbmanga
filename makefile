.PHONY: run build clean
run: build
	./fbmanga dat/x1*

build:
	ninja

clean:
	ninja clean
