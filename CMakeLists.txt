cmake_minimum_required(VERSION 3.1)

project(opengl_tutorial)
set(CMAKE_BUILD_TYPE "Debug")

include(CMakeParseArguments)

# copies files from src-tree to build-tree and adds them to a source group
function(_add_project_resource_files _NAME)
        set_source_files_properties(${ARGN} PROPERTIES HEADER_FILE_ONLY TRUE)
        # copy files to build-tree
        foreach(resource ${ARGN})
                configure_file(${resource} ${resource} COPYONLY)
        endforeach()
        # files in src-tree, not build-tree are referenced in project-file, so changes affect the src-tree
        # changed files will be copied into build-tree automatically
        source_group(${_NAME} FILES ${ARGN})
endfunction()

function(_add_project_files _RESULT)
        if(NOT "${ARGN}" STREQUAL "")
                file(GLOB_RECURSE ${_RESULT} RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${ARGN})
        endif()
        set(${_RESULT} ${${_RESULT}} PARENT_SCOPE)
endfunction()


# ensure that Cxx11 support is enabled:
set(CMAKE_CXX_STANDARD 11)


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules" /sw/config/cmake/Modules ${CMAKE_MODULE_PATH})

#########################################################
# FIND GLUT
#########################################################
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIR})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})
if(NOT GLUT_FOUND)
    message(ERROR " GLUT not found!")
endif(NOT GLUT_FOUND)

#########################################################
# FIND OPENGL
#########################################################
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)

#########################################################
# FIND GLM
#########################################################
find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

if(NOT GLM_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT GLM_FOUND)

#########################################################
# FIND GLEW
#########################################################
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})


if(NOT GLEW_FOUND)
    message(ERROR " GLEW not found!")
endif(NOT GLEW_FOUND)

#########################################################
# FIND FREEIMAGE
#########################################################
find_package(FreeImage REQUIRED)
include_directories(${FREEIMAGE_INCLUDE_DIRS})



#########################################################
# FIND ASSIMP
#########################################################


find_package(assimp REQUIRED)
include_directories(${ASSIMP_INCLUDE_DIRS})
#list(APPEND project_INCLUDES ${ASSIMP_INCLUDE_DIRS})
#list(APPEND project_LIBRARIES ${ASSIMP_LIBRARIES})
# because of bad cmake script for assimp:
link_directories(${ASSIMP_LIBRARY_DIRS})

MESSAGE("ASSIMP: ${ASSIMP_INCLUDE_DIRS} ")

SET( CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "/NODEFAULTLIBS:LIBCMT" )
SET( CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "/NODEFAULTLIBS:MSVCRT" )
SET( CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "/NODEFAULTLIBS:MSVCPRTD" )

SET(synch_DIR /sw/synch/latest/cmake)
find_package(synch REQUIRED)

#########################################################
# Include Files
#########################################################
set(HEADER_FILES
	src/mesh.h
	src/object.h
	src/ocTree.h
	)
set(SOURCE_FILES
	src/mesh.cpp
	src/object.cpp
	src/main.cpp
	src/ocTree.cpp
	)

SET(HEADER_SERVER
	src/mesh.h
	src/object.h
	src/ocTree.h
)
SET(SOURCE_SERVER
	src/mesh.cpp
	src/object.cpp
	src/ocTree.cpp
	src/main_server.cpp
)


include_directories(src)
SET(FIRST_SHADER_FILES shader/fragment.frag shader/fragment_tex.frag shader/vertex.vert shader/vertex_tex.vert)
SET(FIRST_SNIPPET_FILES snippets/snippets.cpp)
	
	_add_project_files(SHADER_FILES ${FIRST_SHADER_FILES})
	_add_project_resource_files("Shader Files" ${SHADER_FILES})
	
	_add_project_files(SNIPPET_FILES ${FIRST_SNIPPET_FILES})
	_add_project_resource_files("Snippet Files" ${SNIPPET_FILES})



add_executable(selection_app ${HEADER_FILES} ${SOURCE_FILES} ${SHADER_FILES} ${SNIPPET_FILES})
add_executable(selection_app_server ${HEADER_SERVER} ${SOURCE_SERVER} ${SHADER_FILES} ${SNIPPET_FILES})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
MESSAGE("Freeimage: ${FREEIMAGE_LIBRARIES}")

########################################################
# Linking & stuff
#########################################################
# create the program "test"
#set(FREEIMAGE_LIBRARIES2 /usr/lib/x86_64-linux-gnu/libfreeimage.so)
#MESSAGE("freeimage: ${FREEIMAGE_LIBRARIES}")
target_link_libraries(selection_app ${FREEIMAGE_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARY} ${OPENGL_LIBRARIES} ${ASSIMP_LIBRARIES} synch)
target_link_libraries(selection_app_server ${FREEIMAGE_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARY} ${OPENGL_LIBRARIES} ${ASSIMP_LIBRARIES} synch)



