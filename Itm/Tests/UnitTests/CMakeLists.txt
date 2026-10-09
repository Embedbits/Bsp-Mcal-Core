#======================== Unit tests configuration ============================#
# Unit tests of Itm module. Registered in module CMakeLists.txt by
# UnitTesting_AddPath() and created by UnitTesting_Generate() in "UnitTest"
# build (see EmBi_Platform/CMake/UnitTesting/README.md).

get_filename_component(ITM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
get_filename_component(MCAL_DIR "${ITM_DIR}/../.." ABSOLUTE)

UnitTesting_Add_Test(
    NAME            Itm
    TEST_SOURCE     Test_Itm.c
    UUT_SOURCES     ${ITM_DIR}/Itm.c
    MOCK_HEADERS    ${MCAL_DIR}/Rcc/Rcc_Port.h
    INCLUDE_DIRS    ${ITM_DIR}
                    ${MCAL_DIR}/Rcc
    LINK_LIBS       ut_RegMem                           # Registers in host memory
                    ut_CmsisHost                        # CMSIS core intrinsics on host
    FORCE_INCLUDES  ${UNIT_TESTING_CMSIS_HOST_HEADER}   # Replaces cmsis_gcc.h
    HOST_LIBS       Ral_Lib                             # ST LL compiled for host
    COVERAGE_EXCLUDE ".*/Ral/.*"
)
