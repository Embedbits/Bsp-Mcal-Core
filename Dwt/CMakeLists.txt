# Template version: 1.0.0
# Sets the version of CMakeLists.txt file.
# Never, under any circumstances, change this line. It is used by
# the module generation script to identify the template version.

#======================== Library configuration ===============================#
# Publicly included directories to provide headers include
set( Dwt_PublicIncludeDirs
    
)


# Privately included directories to provide headers include
set( Dwt_PrivateIncludeDirs
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# Source files list for current library generation
set( Dwt_SourceFiles
    Dwt.c 
)


# List of public header files provided by the current library
set( Dwt_PublicHeaders 
    Dwt_Types.h
    Dwt_Port.h 
)


# List of dependent libraries that are publicly used by the current library
set( Dwt_PublicDependentLibs
     
)


# List of dependent libraries that are privately used by the current library
set( Dwt_PrivateDependentLibs
    Ral_Lib
)


#========================== Library generation ================================#
# Create library
add_library(Dwt_Lib STATIC ${Dwt_SourceFiles})


# Include necessary libraries to the target
target_link_libraries( Dwt_Lib
    PUBLIC 
        ${Dwt_PublicDependentLibs} 
    PRIVATE 
        ${Dwt_PrivateDependentLibs}
)


# Set public headers directory path in build folder
set(PUBLIC_HEADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/PublicHeaders)

# Generates the public headers directory in build folder
file(MAKE_DIRECTORY ${PUBLIC_HEADERS_DIR})

# Process all public headers
foreach(PublicHeader IN LISTS Dwt_PublicHeaders)
    
    # Copy (or configure) only the public header there
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${PublicHeader} 
        ${PUBLIC_HEADERS_DIR}/${PublicHeader}
        COPYONLY
    )
    
endforeach()


# Set include directories for the created target
target_include_directories( Dwt_Lib
    PUBLIC
        $<BUILD_INTERFACE:${PUBLIC_HEADERS_DIR}>
        $<INSTALL_INTERFACE:include>
        Dwt_PublicIncludeDirs
    PRIVATE
        ${Dwt_IncludeDirs}
        Dwt_PrivateIncludeDirs
)


# Print status message
message(STATUS "Dwt_Lib library created")

#========================= Documentation configuration ========================#
if(DOXYGEN_AVAILABLE STREQUAL "ON")
    Doxygen_AddPath("${CMAKE_CURRENT_SOURCE_DIR}/")
endif()
