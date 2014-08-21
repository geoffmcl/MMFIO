###################################################################
# 20140816 - Hand crafed CMakeLists.txt for MMFIO project - geoff #
###################################################################
cmake_minimum_required (VERSION 2.8.5)

project(MMFIO)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(IS_64BIT_BUILD 1)
    add_definitions( -DIS_64BIT_BUILD )
endif ()

#########################################################
### some general utility functions
if (IS_64BIT_BUILD)
    set(name genutils)
else ()
    set(name genutils32)
endif ()    
set(dir src/utils)
include_directories( ${dir} )
add_library(${name}
    ${dir}/utils.cxx ${dir}/utils.hxx 
    )
list(APPEND add_LIBS ${name})
# simple static library - NO INSTALL required

##########################################################
# separate to a library so can be used by other apps
if (IS_64BIT_BUILD)
    set(name MMFIODef)
else ()
    set(name MMFIODef32)
endif ()    
set(dir src)
add_library( ${name} 
    ${dir}/MMFIODef.cpp ${dir}/MMFIODef.h 
    )
list(APPEND add_LIBS ${name})

##########################################################
### example usage of MMFIODef library
if (IS_64BIT_BUILD)
    set(name MMFIO)
else ()
    set(name MMFIO32)
endif ()    
set(dir src)
include_directories( ${dir} )
add_executable( ${name} 
    ${dir}/MMFIO.cpp ${dir}/MMFIO.h 
    )
target_link_libraries( ${name} ${add_LIBS} )
set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
# only an EXAMPLE usage app so NO INSTALL required

##########################################################
### example usage of MMFIODef library as a backstore
if (IS_64BIT_BUILD)
    set(name MMBS)
else ()
    set(name MMBS32)
endif ()    
set(dir src)
include_directories( ${dir} )
add_executable( ${name} 
    ${dir}/MMBS.cxx ${dir}/MMBS.hxx 
    )
target_link_libraries( ${name} ${add_LIBS} )
set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
# only an EXAMPLE usage app so NO INSTALL required

###########################################################
# eof
