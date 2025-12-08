#Made for (n)vim, so i can run ":make" and build my project
.PHONY : run build clean grind debug tidy check analyze

runcmd = ../fbmanga -f log 
nin = ninja -j1 -C ..
b : build

r : run

run : build
	$(runcmd) -v

build :
	$(nin)

clean :
	$(nin) clean

grind : build
	valgrind -v -v -v $(runcmd) 2> log.grind

debug : build
	gdbtui --args $(runcmd)

fmt :
	clang-format-20 -i *.cpp *.cxx

analyze:
	clang --analyze --analyzer-output text *.h *.cpp

check :
	cppcheck --enable=all --report-progress ./



