#Made for nvim, so i can run ":make" and build my project
.PHONY: run build clean
runcmd = ../fbmanga ../dat/png/x*
run: build
	$(runcmd)

build:
	ninja -C ..

clean:
	ninja -C .. clean
grind: build
	valgrind $(runcmd)

debug: build
	gdbtui --args $(runcmd)
