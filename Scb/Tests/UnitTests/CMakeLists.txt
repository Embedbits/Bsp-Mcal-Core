#======================== Unit tests configuration ============================#
# Unit tests of Scb module. Registered in module CMakeLists.txt by
# UnitTesting_AddPath() and created by UnitTesting_Generate() in "UnitTest"
# build (see EmBi_Platform/CMake/UnitTesting/README.md).

get_filename_component(SCB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

UnitTesting_Add_Test(
    NAME            Scb
    TEST_SOURCE     Test_Scb.c
    UUT_SOURCES     ${SCB_DIR}/Scb.c
    INCLUDE_DIRS    ${SCB_DIR}
    LINK_LIBS       ut_RegMem                           # Registers in host memory
                    ut_CmsisHost                        # CMSIS core intrinsics on host
    FORCE_INCLUDES  ${UNIT_TESTING_CMSIS_HOST_HEADER}   # Replaces cmsis_gcc.h
    HOST_LIBS       Ral_Lib                             # ST LL compiled for host
    COVERAGE_EXCLUDE ".*/Ral/.*"
)
