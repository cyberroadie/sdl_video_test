cmake_minimum_required(VERSION 3.3)
project(SDLVideotutorial)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lavutil \
                                    -lavformat \
                                    -lavcodec \
                                    -lz \
                                    -lavutil \
                                    -lm \
                                    -lswscale \
                                    -framework CoreVideo \
                                    -framework CoreFoundation \
                                    -framework AudioToolbox \
                                    -framework AudioUnit \
                                    -framework Cocoa \
                                    -framework Carbon \
                                    -framework CoreAudio \
                                    -framework IOKit \
                                    -framework ForceFeedback")

include_directories(./include)
include_directories(./lib)

set(HEADER_FILES include/SDL.h)
set(SOURCE_FILES video.c)

add_executable(SDLVideoTutorial ${SOURCE_FILES} ${HEADER_FILES})

add_library(sdl2 STATIC IMPORTED)
set_target_properties(sdl2 PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libSDL2.a)

target_link_libraries(SDLVideoTutorial sdl2 C)