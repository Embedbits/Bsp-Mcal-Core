# Template version: 1.0.0
# Sets the version of CMakeLists.txt file.
# Never, under any circumstances, change this line. It is used by
# the module generation script to identify the template version.

#======================== Library configuration ===============================#
# Publicly included directories to provide headers include
set( Scb_PublicIncludeDirs

)


# Privately included directories to provide headers include
set( Scb_PrivateIncludeDirs
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# Source files list for current library generation
set( Scb_SourceFiles
    Scb.c
)


# List of public header files provided by the current library
set( Scb_PublicHeaders
    Scb_Types.h
    Scb_Port.h
)


# List of dependent libraries that are publicly used by the current library
set( Scb_PublicDependentLibs

)


# List of dependent libraries that are privately used by the current library
set( Scb_PrivateDependentLibs
    Ral_Lib
)


#========================== Library generation ================================#
# Create library
add_library(Scb_Lib STATIC ${Scb_SourceFiles})


# Include necessary libraries to the target
target_link_libraries( Scb_Lib
    PUBLIC
        ${Scb_PublicDependentLibs}
    PRIVATE
        ${Scb_PrivateDependentLibs}
)


# Set public headers directory path in build folder
set(PUBLIC_HEADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/PublicHeaders)

# Generates the public headers directory in build folder
file(MAKE_DIRECTORY ${PUBLIC_HEADERS_DIR})

# Process all public headers
foreach(PublicHeader IN LISTS Scb_PublicHeaders)

    # Copy (or configure) only the public header there
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${PublicHeader}
        ${PUBLIC_HEADERS_DIR}/${PublicHeader}
        COPYONLY
    )

endforeach()


# Set include directories for the created target
target_include_directories( Scb_Lib
    PUBLIC
        $<BUILD_INTERFACE:${PUBLIC_HEADERS_DIR}>
        $<INSTALL_INTERFACE:include>
        ${Scb_PublicIncludeDirs}
    PRIVATE
        ${Scb_PrivateIncludeDirs}
)


# Print status message
message(STATUS "Scb_Lib library created")

#========================= Documentation configuration ========================#
if(DOXYGEN_AVAILABLE STREQUAL "ON")
    Doxygen_AddPath("${CMAKE_CURRENT_SOURCE_DIR}/")
endif()

#========================= Unit tests configuration ===========================#
if(UNIT_TESTING_AVAILABLE STREQUAL "ON")
    UnitTesting_AddPath("${CMAKE_CURRENT_SOURCE_DIR}/Tests/UnitTests")
endif()

#===================== Integration tests configuration ========================#
if(INTEGRATION_TESTING_AVAILABLE STREQUAL "ON")
    IntegrationTesting_AddPath("${CMAKE_CURRENT_SOURCE_DIR}/Tests/IntegrationTests")
endif()
