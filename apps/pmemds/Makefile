CXX = g++
CXXFLAGS = -O3 -fPIC -Wall
CPPFLAGS = -I../../include/pmemds

libpmemds.a: src/pmemds.o 
	ar rcs $@ $^

pmemds.o: pmemds.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(CPPFLAGS) -c $< -o $@  $(LDFLAGS)




.PHONY: clean install

clean:
	rm -f libpmemds.a
