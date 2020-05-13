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

project(jsonToBlobConverter)

set(SOURCES jsonToBlobConverter.c)

add_executable(jsonToBlobConverter ${SOURCES})

target_link_libraries (jsonToBlobConverter
    ${CMAKE_THREAD_LIBS_INIT}
    -lmsgpackc
    -ltrower-base64
    -lm
    -lcjson
    -lpthread
    -lrt
    )
install (TARGETS jsonToBlobConverter DESTINATION bin)
