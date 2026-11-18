/**
 * \author Mr.Nobody
 * \file Test_Syscfg.c
 * \ingroup Syscfg
 * \brief Unit tests of the Syscfg module (Ethernet PHY interface select: SYSCFG PMCR.EPIS_SEL,
 *        SBS PMCR.ETH_PHYSEL on STM32H7R / STM32H7S).
 *
 * Syscfg.c is compiled unchanged. The SYSCFG / SBS register is emulated by RegMem (plain host
 * memory, the PHY interface field is a plain read-write field, no self-clearing bit involved). Rcc
 * is mocked by CMock: \ref Syscfg_Set_EthPhyInterface enables the peripheral clock of the block
 * through Rcc before writing PMCR. STM32H7R / STM32H7S (Ral family STM32H7RS, built with the same
 * Mcal branch) have no SYSCFG peripheral: the field is SBS PMCR.ETH_PHYSEL there (AB#2204), the
 * test cases use the register, the field and the Rcc item of the line (UT_SYSCFG_PMCR,
 * UT_SYSCFG_PHY_SEL, UT_SYSCFG_PERIPH). The lines without the field (STM32H7A3 / H7B0 / H7B3, no
 * Ethernet MAC) are tested by the dedicated test case at the end of the file; the test cases that
 * need the field are ignored there (`TEST_IGNORE_MESSAGE`).
 *
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "RegMem.h"                         /* Register memory emulation      */
#include "Syscfg_Port.h"                    /* Module under test              */
#include "MockRcc_Port.h"                   /* RCC module mock                 */
#include "Stm32.h"                          /* SYSCFG / SBS registers         */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

#if defined(SYSCFG)
/** \brief PMCR register of the line (SYSCFG, classic STM32H7) */
#define UT_SYSCFG_PMCR                      ( SYSCFG->PMCR )

/** \brief PMCR bits outside the PHY interface field, preset to a non-zero pattern to prove they are preserved */
#define UT_SYSCFG_PMCR_OTHER_BITS           ( SYSCFG_PMCR_I2C1_FMP )
#elif defined(SBS)
/** \brief PMCR register of the line (SBS, STM32H7R / STM32H7S) */
#define UT_SYSCFG_PMCR                      ( SBS->PMCR )

/** \brief PMCR bits outside the PHY interface field, preset to a non-zero pattern to prove they are preserved */
#define UT_SYSCFG_PMCR_OTHER_BITS           ( SBS_PMCR_BOOSTEN )
#else
#error "Device line without SYSCFG and SBS"
#endif /* SYSCFG / SBS */

#if defined(SYSCFG_PMCR_EPIS_SEL)
/** \brief The device line has the Ethernet PHY interface field (Ethernet MAC present) */
#define UT_SYSCFG_HAS_PHY_SEL

/** \brief Ethernet PHY interface field (PMCR.EPIS_SEL[2:0]) */
#define UT_SYSCFG_PHY_SEL                   ( SYSCFG_PMCR_EPIS_SEL )

/** \brief Field value of the RMII interface (bit 2 only) */
#define UT_SYSCFG_PHY_SEL_RMII              ( SYSCFG_PMCR_EPIS_SEL_2 )

/** \brief Rcc item of the register interface clock */
#define UT_SYSCFG_PERIPH                    ( RCC_PERIPH_SYSCFG )
#elif defined(SBS_PMCR_ETH_PHYSEL)
/** \brief The device line has the Ethernet PHY interface field (Ethernet MAC present) */
#define UT_SYSCFG_HAS_PHY_SEL

/** \brief Ethernet PHY interface field (PMCR.ETH_PHYSEL[2:0]) */
#define UT_SYSCFG_PHY_SEL                   ( SBS_PMCR_ETH_PHYSEL )

/** \brief Field value of the RMII interface (bit 2 only) */
#define UT_SYSCFG_PHY_SEL_RMII              ( SBS_PMCR_ETH_PHYSEL_2 )

/** \brief Rcc item of the register interface clock */
#define UT_SYSCFG_PERIPH                    ( RCC_PERIPH_SBS )
#endif /* SYSCFG_PMCR_EPIS_SEL / SBS_PMCR_ETH_PHYSEL */

/* ========================== LOCAL VARIABLES =============================== */

/* ============================================================================ */

void setUp( void )
{
    TEST_ASSERT_EQUAL( REGMEM_REQUEST_OK, RegMem_Reset() );

    UT_SYSCFG_PMCR = UT_SYSCFG_PMCR_OTHER_BITS;
}


void tearDown( void )
{
    /* No action required */
}


/**
 * \brief \ref Syscfg_Get_ModuleVersion returns the version structure (major/minor/patch).
 *
 * \par Expected results
 * Version 0.1.0.
 */
void Ut_Syscfg_GetModuleVersion( void )
{
    const syscfg_ModuleVersion_t version = Syscfg_Get_ModuleVersion();

    TEST_ASSERT_EQUAL_UINT8( 0u, version.Major );
    TEST_ASSERT_EQUAL_UINT8( 1u, version.Minor );
    TEST_ASSERT_EQUAL_UINT8( 0u, version.Patch );
}


/**
 * \brief Selecting \ref SYSCFG_ETH_PHY_MII clears the PHY interface field and preserves the other bits.
 *
 * \par Expected results
 * SYSCFG_REQUEST_OK, the SYSCFG / SBS clock is enabled, field = 0, the other bits unchanged.
 */
void Ut_Syscfg_SetEthPhyInterface_Mii( void )
{
#if defined(UT_SYSCFG_HAS_PHY_SEL)
    UT_SYSCFG_PMCR = ( UT_SYSCFG_PMCR_OTHER_BITS | UT_SYSCFG_PHY_SEL );

    Rcc_Set_PeriphActive_ExpectAndReturn( UT_SYSCFG_PERIPH, RCC_REQUEST_OK );
    const syscfg_RequestState_t retState = Syscfg_Set_EthPhyInterface( SYSCFG_ETH_PHY_MII );

    TEST_ASSERT_EQUAL( SYSCFG_REQUEST_OK, retState );
    TEST_ASSERT_EQUAL_HEX32( UT_SYSCFG_PMCR_OTHER_BITS,
                             ( UT_SYSCFG_PMCR & ( UT_SYSCFG_PHY_SEL | UT_SYSCFG_PMCR_OTHER_BITS ) ) );
#else
    TEST_IGNORE_MESSAGE( "MCU without the Ethernet PHY interface field" );
#endif
}


/**
 * \brief Selecting \ref SYSCFG_ETH_PHY_RMII sets bit 2 of the PHY interface field only and preserves
 *        the other bits.
 *
 * \par Expected results
 * SYSCFG_REQUEST_OK, the SYSCFG / SBS clock is enabled, field = 0b100, the other bits unchanged.
 */
void Ut_Syscfg_SetEthPhyInterface_Rmii( void )
{
#if defined(UT_SYSCFG_HAS_PHY_SEL)
    Rcc_Set_PeriphActive_ExpectAndReturn( UT_SYSCFG_PERIPH, RCC_REQUEST_OK );
    const syscfg_RequestState_t retState = Syscfg_Set_EthPhyInterface( SYSCFG_ETH_PHY_RMII );

    TEST_ASSERT_EQUAL( SYSCFG_REQUEST_OK, retState );
    TEST_ASSERT_EQUAL_HEX32( UT_SYSCFG_PHY_SEL_RMII, ( UT_SYSCFG_PMCR & UT_SYSCFG_PHY_SEL ) );
    TEST_ASSERT_EQUAL_HEX32( UT_SYSCFG_PMCR_OTHER_BITS, ( UT_SYSCFG_PMCR & UT_SYSCFG_PMCR_OTHER_BITS ) );
#else
    TEST_IGNORE_MESSAGE( "MCU without the Ethernet PHY interface field" );
#endif
}


/**
 * \brief An out-of-range interface value is rejected before the RCC clock is enabled.
 *
 * \par Expected results
 * SYSCFG_REQUEST_ERROR, no Rcc call, PMCR unchanged (on every line).
 */
void Ut_Syscfg_SetEthPhyInterface_Invalid( void )
{
    const uint32_t beforeValue = UT_SYSCFG_PMCR;
    const syscfg_RequestState_t retState = Syscfg_Set_EthPhyInterface( SYSCFG_ETH_PHY_INTERFACE_CNT );

    TEST_ASSERT_EQUAL( SYSCFG_REQUEST_ERROR, retState );
    TEST_ASSERT_EQUAL_HEX32( beforeValue, UT_SYSCFG_PMCR );
}


/**
 * \brief \ref Syscfg_Set_EthPhyInterface fails when the SYSCFG / SBS peripheral clock cannot be enabled.
 *
 * \par Expected results
 * SYSCFG_REQUEST_ERROR, PMCR unchanged.
 */
void Ut_Syscfg_SetEthPhyInterface_RccClockFails( void )
{
#if defined(UT_SYSCFG_HAS_PHY_SEL)
    const uint32_t beforeValue = UT_SYSCFG_PMCR;

    Rcc_Set_PeriphActive_ExpectAndReturn( UT_SYSCFG_PERIPH, RCC_REQUEST_ERROR );
    const syscfg_RequestState_t retState = Syscfg_Set_EthPhyInterface( SYSCFG_ETH_PHY_RMII );

    TEST_ASSERT_EQUAL( SYSCFG_REQUEST_ERROR, retState );
    TEST_ASSERT_EQUAL_HEX32( beforeValue, UT_SYSCFG_PMCR );
#else
    TEST_IGNORE_MESSAGE( "MCU without the Ethernet PHY interface field" );
#endif
}


/**
 * \brief \ref Syscfg_Get_EthPhyInterface reads back \ref SYSCFG_ETH_PHY_RMII after it was selected.
 *
 * \par Expected results
 * SYSCFG_REQUEST_OK and SYSCFG_ETH_PHY_RMII.
 */
void Ut_Syscfg_GetEthPhyInterface_Rmii( void )
{
#if defined(UT_SYSCFG_HAS_PHY_SEL)
    syscfg_EthPhyInterface_t phyInterface = SYSCFG_ETH_PHY_MII;

    UT_SYSCFG_PMCR = ( UT_SYSCFG_PMCR_OTHER_BITS | UT_SYSCFG_PHY_SEL_RMII );

    const syscfg_RequestState_t retState = Syscfg_Get_EthPhyInterface( &phyInterface );

    TEST_ASSERT_EQUAL( SYSCFG_REQUEST_OK, retState );
    TEST_ASSERT_EQUAL( SYSCFG_ETH_PHY_RMII, phyInterface );
#else
    TEST_IGNORE_MESSAGE( "MCU without the Ethernet PHY interface field" );
#endif
}


/**
 * \brief \ref Syscfg_Get_EthPhyInterface reads back \ref SYSCFG_ETH_PHY_MII on reset (field = 0).
 *
 * \par Expected results
 * SYSCFG_REQUEST_OK and SYSCFG_ETH_PHY_MII.
 */
void Ut_Syscfg_GetEthPhyInterface_Mii( void )
{
#if defined(UT_SYSCFG_HAS_PHY_SEL)
    syscfg_EthPhyInterface_t phyInterface = SYSCFG_ETH_PHY_RMII;

    const syscfg_RequestState_t retState = Syscfg_Get_EthPhyInterface( &phyInterface );

    TEST_ASSERT_EQUAL( SYSCFG_REQUEST_OK, retState );
    TEST_ASSERT_EQUAL( SYSCFG_ETH_PHY_MII, phyInterface );
#else
    TEST_IGNORE_MESSAGE( "MCU without the Ethernet PHY interface field" );
#endif
}


/**
 * \brief A \ref SYSCFG_NULL_PTR argument is rejected by \ref Syscfg_Get_EthPhyInterface.
 *
 * \par Expected results
 * SYSCFG_REQUEST_ERROR (on every line).
 */
void Ut_Syscfg_GetEthPhyInterface_NullPtr( void )
{
    const syscfg_RequestState_t retState = Syscfg_Get_EthPhyInterface( SYSCFG_NULL_PTR );

    TEST_ASSERT_EQUAL( SYSCFG_REQUEST_ERROR, retState );
}


/**
 * \brief On a line without the Ethernet PHY interface field (STM32H7A3 / H7B0 / H7B3) both
 *        functions only return an error.
 *
 * \par Expected results
 * SYSCFG_REQUEST_ERROR for the setter and the getter, no Rcc call, PMCR and the output unchanged.
 */
void Ut_Syscfg_NoEpisSel_EveryFunctionReturnsError( void )
{
#if defined(UT_SYSCFG_HAS_PHY_SEL)
    TEST_IGNORE_MESSAGE( "MCU with the Ethernet PHY interface field, see the other test cases" );
#else
    const uint32_t           beforeValue  = UT_SYSCFG_PMCR;
    syscfg_EthPhyInterface_t phyInterface = SYSCFG_ETH_PHY_RMII;

    TEST_ASSERT_EQUAL( SYSCFG_REQUEST_ERROR, Syscfg_Set_EthPhyInterface( SYSCFG_ETH_PHY_RMII ) );
    TEST_ASSERT_EQUAL( SYSCFG_REQUEST_ERROR, Syscfg_Get_EthPhyInterface( &phyInterface ) );
    TEST_ASSERT_EQUAL_HEX32( beforeValue, UT_SYSCFG_PMCR );
    TEST_ASSERT_EQUAL( SYSCFG_ETH_PHY_RMII, phyInterface );
#endif
}
