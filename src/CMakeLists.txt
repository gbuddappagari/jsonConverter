#   Copyright 2020
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

project(jsonConverter)

set(HEADERS jsonConverter.h)
set(SOURCES jsonConverter.c)
add_library(jsonConverter STATIC ${HEADERS} ${SOURCES})
add_library(jsonConverter.shared SHARED ${HEADERS} ${SOURCES})
set_target_properties(jsonConverter.shared PROPERTIES OUTPUT_NAME jsonConverter)
set_property(TARGET jsonConverter PROPERTY C_STANDARD 99)
set_property(TARGET jsonConverter.shared PROPERTY C_STANDARD 99)
install (TARGETS jsonConverter DESTINATION ${LIBRARY_DIR})
install (TARGETS jsonConverter.shared DESTINATION ${LIBRARY_DIR})
install (FILES ${HEADERS} DESTINATION include/jsonConverter)

#------------------------------------------#
#------- jsonConverter cli ----------------#
#------------------------------------------#
set(BIN_SOURCES ${SOURCES} main.c)
add_executable(jsonConverterCli ${BIN_SOURCES} ${HEADERS} )

target_link_libraries (jsonConverterCli
    ${CMAKE_THREAD_LIBS_INIT}
    -lmsgpackc
    -ltrower-base64
    -lm
    -lcjson
    -lpthread
    -lrt
    )
install (TARGETS jsonConverterCli DESTINATION bin)

add_subdirectory(multipart)
