CXX=g++
CXX_FLAGS=-std=c++14 -Wall -fsanitize=undefined -g -frtti -fexceptions

all: tm

clean:
	rm -f *.o tm tr

tm: tm.o
	${CXX} ${CXX_FLAGS} -o $@ $^

tr: transform.o
	${CXX} ${CXX_FLAGS} -o $@ $^

%.o: %.cpp Makefile
	${CXX} ${CXX_FLAGS} -c -o $@ $<
