cmake_minimum_required(VERSION 2.8)
project(brisquequality)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug build in brisque.")
  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
  message("Release build in brisque.")
  #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
ELSE()
  message("Some other build type.")
ENDIF()

find_package( OpenCV 2.4.5 REQUIRED )
include_directories ( ${OpenCV_INCLUDE_DIRS} )
link_directories ("/usr/lib")
add_definitions(-DGCC_COMPILER=1)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_CXX_FLAGS -fpermissive)

add_library( brisquequality SHARED trainModel.cpp  computescore.cpp brisque.cpp libsvm/svm.cpp)

target_link_libraries( brisquequality  ${OpenCV_LIBS} opencv_videostab)

