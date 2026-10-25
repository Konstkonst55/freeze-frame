cmake_minimum_required(VERSION 3.16)
project(freeze_frame CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PostgreSQL REQUIRED)

find_path(FLTK_INCLUDE_DIR FL/Fl.H)
find_library(FLTK_LIBRARY NAMES fltk)

find_path(ODB_INCLUDE_DIR odb/core.hxx)
find_library(ODB_LIBRARY NAMES odb)
find_library(ODB_PGSQL_LIBRARY NAMES odb-pgsql)
find_program(ODB_EXECUTABLE odb)

if(NOT FLTK_LIBRARY OR NOT ODB_LIBRARY OR NOT ODB_PGSQL_LIBRARY OR NOT ODB_EXECUTABLE)
    message(FATAL_ERROR "Missing deps: FLTK / ODB / odb-pgsql / odb compiler")
endif()

set(ODB_GEN_DIR ${CMAKE_BINARY_DIR}/odb)
file(MAKE_DIRECTORY ${ODB_GEN_DIR})

set(ODB_HEADERS
    include/domain/employee.hxx
    include/domain/asset.hxx
    include/domain/assignment.hxx)

set(ODB_GENERATED)
foreach(hxx ${ODB_HEADERS})
    get_filename_component(name ${hxx} NAME_WE)
    set(gen ${ODB_GEN_DIR}/${name}-odb.cxx)
    add_custom_command(
        OUTPUT ${gen} ${ODB_GEN_DIR}/${name}-odb.hxx ${ODB_GEN_DIR}/${name}-odb.ixx
        COMMAND ${ODB_EXECUTABLE} -d pgsql --std c++11
                --generate-query --generate-schema
                --output-dir ${ODB_GEN_DIR}
                -I${CMAKE_SOURCE_DIR}/include
                -I${CMAKE_SOURCE_DIR}/include/domain
                ${CMAKE_SOURCE_DIR}/${hxx}
        DEPENDS ${CMAKE_SOURCE_DIR}/${hxx}
        COMMENT "odb ${name}"
    )
    list(APPEND ODB_GENERATED ${gen})
endforeach()

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(freezeframe ${APP_SOURCES} ${ODB_GENERATED})

set_source_files_properties(${ODB_GENERATED} PROPERTIES
    COMPILE_FLAGS "-std=c++11")

target_include_directories(freezeframe PRIVATE
    include
    include/domain
    ${ODB_GEN_DIR}
    ${FLTK_INCLUDE_DIR}
    ${ODB_INCLUDE_DIR}
    ${PostgreSQL_INCLUDE_DIRS})

target_link_libraries(freezeframe PRIVATE
    ${FLTK_LIBRARY} ${ODB_LIBRARY} ${ODB_PGSQL_LIBRARY}
    ${PostgreSQL_LIBRARIES} pthread ${CMAKE_DL_LIBS})
