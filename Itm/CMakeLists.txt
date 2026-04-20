# Template version: 1.0.0
# Sets the version of CMakeLists.txt file.
# Never, under any circumstances, change this line. It is used by
# the module generation script to identify the template version.

#======================== Library configuration ===============================#
# Publicly included directories to provide headers include
set( Itm_PublicIncludeDirs
    
)


# Privately included directories to provide headers include
set( Itm_PrivateIncludeDirs
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# Source files list for current library generation
set( Itm_SourceFiles
    Itm.c 
)


# List of public header files provided by the current library
set( Itm_PublicHeaders 
    Itm_Types.h
    Itm_Port.h 
)


# List of dependent libraries that are publicly used by the current library
set( Itm_PublicDependentLibs
     
)


# List of dependent libraries that are privately used by the current library
set( Itm_PrivateDependentLibs
    Ral_Lib
    Rcc_Lib
)


#========================== Library generation ================================#
# Create library
add_library(Itm_Lib STATIC ${Itm_SourceFiles})


# Include necessary libraries to the target
target_link_libraries( Itm_Lib
    PUBLIC 
        ${Itm_PublicDependentLibs} 
    PRIVATE 
        ${Itm_PrivateDependentLibs}
)


# Set public headers directory path in build folder
set(PUBLIC_HEADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/PublicHeaders)

# Generates the public headers directory in build folder
file(MAKE_DIRECTORY ${PUBLIC_HEADERS_DIR})

# Process all public headers
foreach(PublicHeader IN LISTS Itm_PublicHeaders)
    
    # Copy (or configure) only the public header there
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${PublicHeader} 
        ${PUBLIC_HEADERS_DIR}/${PublicHeader}
        COPYONLY
    )
    
endforeach()


# Set include directories for the created target
target_include_directories( Itm_Lib
    PUBLIC
        $<BUILD_INTERFACE:${PUBLIC_HEADERS_DIR}>
        $<INSTALL_INTERFACE:include>
        Itm_PublicIncludeDirs
    PRIVATE
        ${Itm_IncludeDirs}
        Itm_PrivateIncludeDirs
)


# Print status message
message(STATUS "Itm_Lib library created")

#========================= Documentation configuration ========================#
if(DOXYGEN_AVAILABLE STREQUAL "ON")
    Doxygen_AddPath("${CMAKE_CURRENT_SOURCE_DIR}/")
endif()
