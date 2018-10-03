CXX=g++
CXX_FLAGS=-std=c++14

%.o: %.cpp
	${CXX} ${CXX_FLAGS} -c -o $@ $<

prob1: prob1.o nfa.o dfa.o
	${CXX} -o $@ $^

prob2: prob2.o nfa.o dfa.o
	${CXX} -o $@ $^

run1: prob1
	./prob1

run2: prob2
	./prob2

all: prob1 prob2

clean:
	rm -f *.o prob1 prob2

