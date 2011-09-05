# # Copyright 2011 Justin Bruce Van Horne <justinvh@gmail.com>
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

project(rotide)

set(project_name rotide)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses)
find_package(V8)

set(SOURCES
    src/rotide.cc
    src/curses.cc
    src/scripting.cc
    src/js/core.cc
    src/v8/type_conversion.cc
    )

include_directories(
    "include"
    ${CURSES_INCLUDE_DIR}
    ${V8_INCLUDE_DIR}
    )
add_executable(ro ${SOURCES})
target_link_libraries(ro -lncursesw ${V8_LIBRARY})
