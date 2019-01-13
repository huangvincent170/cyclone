CPPFLAGS=-I../pmemkv/src
LDFLAGS= ../pmemkv/libpmemkv.a
CXXFLAGS=

#PLOC=/home/pfernando/pmdk/src/nondebug
PLOC=/home/pfernando/pmdknlog/src/nondebug

LIBS= $(PLOC)/libpmemobj.a $(PLOC)/libpmem.a -lpthread -ldl

all:
	g++ pmemkv_app.cc -o pmemkv_app $(CPPFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm pmemkv_app
