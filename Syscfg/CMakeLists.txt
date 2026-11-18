# Template version: 1.0.0
# Sets the version of CMakeLists.txt file.
# Never, under any circumstances, change this line. It is used by
# the module generation script to identify the template version.

#======================== Library configuration ===============================#
# Publicly included directories to provide headers include
set( Syscfg_PublicIncludeDirs

)


# Privately included directories to provide headers include
set( Syscfg_PrivateIncludeDirs
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# Source files list for current library generation
set( Syscfg_SourceFiles
    Syscfg.c
)


# List of public header files provided by the current library
set( Syscfg_PublicHeaders
    Syscfg_Types.h
    Syscfg_Port.h
)


# List of dependent libraries that are publicly used by the current library
set( Syscfg_PublicDependentLibs

)


# List of dependent libraries that are privately used by the current library
set( Syscfg_PrivateDependentLibs
    Ral_Lib
    Rcc_Lib
)


#========================== Library generation ================================#
# Create library
add_library(Syscfg_Lib STATIC ${Syscfg_SourceFiles})


# Include necessary libraries to the target
target_link_libraries( Syscfg_Lib
    PUBLIC
        ${Syscfg_PublicDependentLibs}
    PRIVATE
        ${Syscfg_PrivateDependentLibs}
)


# Set public headers directory path in build folder
set(PUBLIC_HEADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/PublicHeaders)

# Generates the public headers directory in build folder
file(MAKE_DIRECTORY ${PUBLIC_HEADERS_DIR})

# Process all public headers
foreach(PublicHeader IN LISTS Syscfg_PublicHeaders)

    # Copy (or configure) only the public header there
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${PublicHeader}
        ${PUBLIC_HEADERS_DIR}/${PublicHeader}
        COPYONLY
    )

endforeach()


# Set include directories for the created target
target_include_directories( Syscfg_Lib
    PUBLIC
        $<BUILD_INTERFACE:${PUBLIC_HEADERS_DIR}>
        $<INSTALL_INTERFACE:include>
        ${Syscfg_PublicIncludeDirs}
    PRIVATE
        ${Syscfg_PrivateIncludeDirs}
)


# Print status message
message(STATUS "Syscfg_Lib library created")

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
