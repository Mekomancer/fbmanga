sources = main.cpp framebuffer.cpp img.cpp
objs = $(sources:%.cpp=build/debug/%.o)
deps = $(sources:%.cpp=build/debug/%.d)
cppflags = --std=c++23 -Wextra -Wall -Winvalid-pch 
.PHONY: clean
fbmanga-debug: $(objs)|build/debug
	g++ $(cppflags) $^ -o fbmanga-debug

include $(sources:%.cpp=build/debug/%.d)

pch.h.gch: pch.h
	g++ $(cppflags) pch.h -o pch.h.gch

$(objs): build/debug/%.o: %.cpp pch.h.gch
	g++ $(cppflags) -c $< -o $@

$(deps): build/debug/%.d: %.cpp | build/debug
	@set -e; rm -f $@; \
	 g++ -M -MG $(cppflags) $< > $@.$$$$; \
	 sed 's,\(/build/debug/$*\)\.o[ :]*,\1.o $@ : pch ,g' < $@.$$$$ > $@; \
 	 rm -f $@.$$$$

build/debug: 
	mkdir build
	mkdir build/debug
clean:
	rm -r build
