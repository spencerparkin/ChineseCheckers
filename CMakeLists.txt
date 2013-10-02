cmake_minimum_required (VERSION 2.6)
project (ChineseChechers)

include_directories (${ChineseChechers_SOURCE_DIR}/Code)
aux_source_directory (${ChineseChechers_SOURCE_DIR}/Code SRC_FILES)
aux_source_directory (${ChineseChechers_SOURCE_DIR}/Code/c3ga SRC_FILES)

if (WX_INSTALL_DIR)
  set (CMAKE_WX_CONFIG "${WX_INSTALL_DIR}/wx-config")
else (WX_INSTALL_DIR)
  find_program (CMAKE_WX_CONFIG wx-config)
endif (WX_INSTALL_DIR)
message (STATUS "Find wx-config at ${CMAKE_WX_CONFIG}")
execute_process (COMMAND ${CMAKE_WX_CONFIG} --cxxflags
  OUTPUT_VARIABLE CMAKE_WX_CXX_FLAGS)
execute_process (COMMAND ${CMAKE_WX_CONFIG} --libs
  OUTPUT_VARIABLE WXWINDOWS_LIBRARIES)
execute_process (COMMAND ${CMAKE_WX_CONFIG} --gl-libs
  OUTPUT_VARIABLE WXWINDOWSGL_LIBRARIES)
add_definitions ("${CMAKE_WX_CXX_FLAGS} -std=c++11")
set (CMAKE_EXE_LINKER_FLAGS "${WXWINDOWS_LIBRARIES} ${WXWINDOWSGL_LIBRARIES}")
string (REPLACE "\n" " "  CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
add_executable (ChineseChecker ${SRC_FILES})
