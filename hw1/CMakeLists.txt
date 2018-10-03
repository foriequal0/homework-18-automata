cmake_minimum_required(VERSION 3.1)

project(automata_hw1)

add_executable(prob1
  prob1.cpp
  nfa.cpp
  dfa.cpp
  )

set_property(TARGET prob1
  PROPERTY CXX_STANDARD 14
  )

add_executable(prob2
  prob1.cpp
  nfa.cpp
  dfa.cpp
  )

set_property(TARGET prob2
  PROPERTY CXX_STANDARD 14
  )
