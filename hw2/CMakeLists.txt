cmake_minimum_required(VERSION 3.5)
project(hw2)

add_library(core
  cfg.cpp
  )

add_executable(cnf
  cnf.cpp
  )
target_link_libraries(cnf PRIVATE core)

add_executable(cyk
  cyk.cpp
  )
target_link_libraries(cyk PRIVATE core)
