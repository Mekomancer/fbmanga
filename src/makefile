#Made for (n)vim, so i can run ":make" and build my project
.PHONY : run build clean grind debug tidy check

runcmd = ../fbmanga ../dat/png/*
nin = ninja -j2 -C ..
run : build
	$(runcmd)

build :
	$(nin)

clean :
	$(nin) clean

grind : build
	valgrind -v -v -v $(runcmd) 2> log.grind

debug : build
	gdbtui --args $(runcmd)

tidy :
	clang-format -i *.h *.cpp

check :
	cppcheck --enable=all --report-progress ./



