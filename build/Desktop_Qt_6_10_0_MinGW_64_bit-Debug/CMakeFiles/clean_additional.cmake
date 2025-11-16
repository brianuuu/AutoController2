# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "AutoController2_autogen"
  "CMakeFiles\\AutoController2_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\AutoController2_autogen.dir\\ParseCache.txt"
  )
endif()
