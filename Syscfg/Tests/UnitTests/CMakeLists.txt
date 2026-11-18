#======================== Unit tests configuration ============================#
# Unit tests of Syscfg module. Registered in module CMakeLists.txt by
# UnitTesting_AddPath() and created by UnitTesting_Generate() in "UnitTest"
# build (see EmBi_Platform/CMake/UnitTesting/README.md).

get_filename_component(SYSCFG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
get_filename_component(CORE_DIR "${SYSCFG_DIR}/.." ABSOLUTE)
get_filename_component(MCAL_DIR "${CORE_DIR}/.." ABSOLUTE)

# Rcc_Port.h is mocked (AB#1866): Syscfg_Set_EthPhyInterface() enables the SYSCFG peripheral
# clock through Rcc before touching PMCR (finding 5 of the Eth module review - nothing used to
# enable it before), so the module now depends on Rcc at the register-access boundary.
UnitTesting_Add_Test(
    NAME            Syscfg
    TEST_SOURCE     Test_Syscfg.c
    UUT_SOURCES     ${SYSCFG_DIR}/Syscfg.c
    MOCK_HEADERS    ${MCAL_DIR}/Rcc/Rcc_Port.h
    INCLUDE_DIRS    ${SYSCFG_DIR}
                    ${MCAL_DIR}/Rcc
    LINK_LIBS       ut_RegMem                           # Registers in host memory
                    ut_CmsisHost                        # CMSIS core intrinsics on host
    FORCE_INCLUDES  ${UNIT_TESTING_CMSIS_HOST_HEADER}   # Replaces cmsis_gcc.h
    HOST_LIBS       Ral_Lib                             # ST LL compiled for host
    COVERAGE_EXCLUDE ".*/Ral/.*"
)
