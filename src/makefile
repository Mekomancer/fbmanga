#Made for (n)vim, so i can run ":make" and build my project
.PHONY : run build clean grind debug tidy

runcmd = ../fbmanga ../dat/png/*
nin = ninja -j1 -C .. 
run : build 
	$(runcmd)

build : 
	$(nin)

clean : 
	$(nin) clean

grind : build 
	valgrind $(runcmd)

debug : build 
	gdbtui --args $(runcmd)

tidy :
	clang-format -i *.h *.cpp
