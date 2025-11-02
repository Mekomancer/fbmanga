#Made for nvim, so i can run ":make" and build my project
.PHONY: run build clean
	
run: build
	../fbmanga `ls ../dat/png/x* -w 0`

build:
	ninja -C ..

clean:
	ninja -C .. clean

debug: build
	gdbtui --args ../fbmanga `ls ../dat/png/x* -w 0`
