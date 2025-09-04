sources = main.cpp mangadex.cpp view.cpp
objs = $(sources:%.cpp=build/debug/%.o)
deps = $(sources:%.cpp=build/debug/%.d)
optflags = --std=c++23
warnings = -Wextra -Wall -Winvalid-pch 
libcurl != curlpp-config --libs
libpng != libpng-config --libs
libflags = $(libcurl) $(libpng)
cppflags = $(optflags) $(warnings) 
.PHONY: clean
fbmanga-debug: $(objs)|build/debug
	g++ $(cppflags) $^ -o fbmanga-debug $(libflags)

include $(sources:%.cpp=build/debug/%.d)

pch.h.gch: pch.h
	g++ $(warnings) $(optflags) pch.h -o pch.h.gch

$(objs): build/debug/%.o: %.cpp
	g++ $(cppflags) -c $< -o $@ $(libflags)

$(deps): build/debug/%.d: %.cpp | build/debug
	@set -e; rm -f $@; \
	 cc -M $(cppflags) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,build/debug/\1.o $@ : pch.h.gch ,g' < $@.$$$$ > $@; \
 	 rm -f $@.$$$$

build/debug: 
	mkdir build
	mkdir build/debug
clean:
	rm -r build
	rm pch.h.gch
