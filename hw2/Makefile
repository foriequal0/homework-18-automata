CXX=g++
CXX_FLAGS=-std=c++14 -Wall -fsanitize=undefined -g -frtti -fexceptions

all: cnf cyk

clean:
	rm -f *.o cnf cyk

cnf: cnf.o cfg.o to_cnf.o
	${CXX} ${CXX_FLAGS} -o $@ $^

cyk: cyk.o cfg.o to_cnf.o
	${CXX} ${CXX_FLAGS} -o $@ $^

%.o: %.cpp
	${CXX} ${CXX_FLAGS} -c -o $@ $<
