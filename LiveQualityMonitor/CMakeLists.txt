cmake_minimum_required(VERSION 2.8)
project(LiveQualityMonitor )

find_package( OpenCV 2.4.5 REQUIRED )
include_directories ( ${OpenCV_INCLUDE_DIRS} )
link_directories ("/usr/lib")
add_definitions(-DGCC_COMPILER=1)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#SET(CMAKE_CXX_FLAGS -fpermissive)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug build.")
  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
  message("Release build.")
  #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
ELSE()
  message("Some other build type.")
ENDIF()

#ffmpeg settings
set(FFMPEG_INCLUDE_DIR /usr/local/include)
set(FFMPEG_LIBRARY_DIR /usr/local/lib)
include_directories(${FFMPEG_INCLUDE_DIR})
link_directories(${FFMPEG_LIBRARY_DIR})
SET (FFMPEG_LIBRARIES
  avcodec
  avdevice
  avfilter
  avformat
  avutil
  postproc
  swresample
  swscale
)

include_directories(/home/water/projects/LiveQualityMonitor/brisque_revised)
add_subdirectory(./brisque_revised)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS")

add_executable(LiveQualityMonitor main.cpp SaveFlv.cpp FlvReader.cpp Inspector.cpp)

target_link_libraries(LiveQualityMonitor brisquequality ${OpenCV_LIBS} ${FFMPEG_LIBRARIES} opencv_videostab rtmp)
