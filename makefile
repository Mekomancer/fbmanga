sources = main.cpp mangadex.cpp
objs = $(sources:%.cpp=build/debug/%.o)
deps = $(sources:%.cpp=build/debug/%.d)
optflags = --std=c++23
warnings = -Wextra -Wall -Winvalid-pch 
curl != curlpp-config --libs
cppflags = $(optflags) $(warnings) $(libflags) 
.PHONY: clean
fbmanga-debug: $(objs)|build/debug
	g++ $(cppflags) $^ -o fbmanga-debug $(curl)

include $(sources:%.cpp=build/debug/%.d)

pch.h.gch: pch.h
	g++ $(warnings) $(optflags) pch.h -o pch.h.gch

$(objs): build/debug/%.o: %.cpp pch.h.gch
	g++ $(cppflags) -c $< -o $@ $(curl)

$(deps): build/debug/%.d: %.cpp | build/debug
	@set -e; rm -f $@; \
	 cc -M $(cppflags) $< > $@.$$$$; \
	 sed 's,\(/build/debug/$*\)\.o[ :]*,\1.o $@ : pch ,g' < $@.$$$$ > $@; \
 	 rm -f $@.$$$$

build/debug: 
	mkdir build
	mkdir build/debug
clean:
	rm -r build
	rm pch.h.gch
