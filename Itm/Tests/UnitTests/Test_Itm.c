/**
 * \author Mr.Nobody
 * \file Test_Itm.c
 * \ingroup Itm
 * \brief Unit tests of Instrumentation Trace Macrocell (ITM) module.
 *
 * Itm.c is compiled unchanged with real CMSIS headers and LL drivers. ITM, DWT,
 * CoreDebug (private peripheral bus), DBGMCU, SWO and SWTF registers are emulated by
 * RegMem, RCC module (trace clock) is mocked by CMock.
 *
 * \note Stimulus port read value (FIFO ready) is set by HW. Tests preset it
 *       where the module waits for it. Emulated port keeps the last written
 *       data.
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "RegMem.h"                         /* Register memory emulation      */
#include "Itm_Port.h"                       /* Module under test              */
#include "MockRcc_Port.h"                   /* RCC module mock                */
#include "Stm32.h"                          /* ITM / DWT / CoreDebug          */
#include "Stm32_system.h"                   /* DBGMCU registers definition    */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

static void             Ut_Itm_Expect_TraceClk  ( rcc_FreqHz_t traceClk );
static itm_Config_t     Ut_Itm_Get_Config       ( itm_Port_t portId, itm_FreqHz_t swoClk );
static void             Ut_Itm_Set_PortReady    ( itm_Port_t portId );

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Trace clock (TRACECLKIN) used by tests */
#define UT_ITM_TRACE_CLK_HZ                 ( 200000000u )

/** SWO clock used by tests */
#define UT_ITM_SWO_CLK_HZ                   ( 2000000u )

/** SWO prescaler of UT_ITM_SWO_CLK_HZ with UT_ITM_TRACE_CLK_HZ (200 MHz / 2 MHz - 1) */
#define UT_ITM_SWO_PRESCALER                ( 99u )

/** Maximal SWO prescaler (CODR register width) */
#define UT_ITM_SWO_PRESCALER_MAX            ( 0x1FFFu )

/** SWO current output divisor register (STM32H7 SWO unit) */
#define UT_ITM_SWO_CODR                     ( *(volatile uint32_t *)0x5C003010u )

/** SWO selected pin protocol register */
#define UT_ITM_SWO_SPPR                     ( *(volatile uint32_t *)0x5C0030F0u )

/** SWO lock access register */
#define UT_ITM_SWO_LAR                      ( *(volatile uint32_t *)0x5C003FB0u )

/** SWO trace funnel control register */
#define UT_ITM_SWTF_CTRL                    ( *(volatile uint32_t *)0x5C004000u )

/** SWO trace funnel lock access register */
#define UT_ITM_SWTF_LAR                     ( *(volatile uint32_t *)0x5C004FB0u )

/** SWTF slave port 0 enable (Cortex-M7) */
#define UT_ITM_SWTF_ENS0                    ( 0x00000001u )

/** SWTF minimum hold time field preset by tests (kept by the module) */
#define UT_ITM_SWTF_HT                      ( 0x00000300u )

#if defined(STM32H7RS)
/** DBGMCU clocks enabled by the module: trace port clock, debug clock */
#define UT_ITM_DBGMCU_CLOCKS                ( DBGMCU_CR_TRACECLKEN | DBGMCU_CR_DBGCKEN )

/** DBGMCU debug clocks kept by Itm_Deinit() */
#define UT_ITM_DBGMCU_DEBUG_CLOCKS          ( DBGMCU_CR_DBGCKEN )

/** DBGMCU bit not used by the module (kept by Itm_Deinit()) */
#define UT_ITM_DBGMCU_OTHER                 ( DBGMCU_CR_DBG_STOP )
#else
/** DBGMCU clocks enabled by the module: trace port clock, D1 / D3 debug clocks */
#define UT_ITM_DBGMCU_CLOCKS                ( DBGMCU_CR_DBG_TRACECKEN | DBGMCU_CR_DBG_CKD1EN | DBGMCU_CR_DBG_CKD3EN )

/** DBGMCU debug clocks kept by Itm_Deinit() */
#define UT_ITM_DBGMCU_DEBUG_CLOCKS          ( DBGMCU_CR_DBG_CKD1EN | DBGMCU_CR_DBG_CKD3EN )

/** DBGMCU bit not used by the module (kept by Itm_Deinit()) */
#define UT_ITM_DBGMCU_OTHER                 ( DBGMCU_CR_DBG_STOPD1 )
#endif /* STM32H7RS */

/** Port used by tests */
#define UT_ITM_PORT                         ( ITM_PORT_5 )

/** Trace bus ID field value written by module */
#define UT_ITM_TCR_TRACEBUSID               ( 1uL << ITM_TCR_TraceBusID_Pos )

/** Selected pin protocol - asynchronous SWO, NRZ */
#define UT_ITM_SPPR_NRZ                     ( 2u )

/** Default SWO clock of Itm_Get_DefaultConfig */
#define UT_ITM_DEFAULT_SWO_CLK_HZ           ( 2000000u )

/** CoreSight software lock key written by module to ITM / SWO / SWTF LAR */
#define UT_ITM_LAR_UNLOCK_KEY               ( 0xC5ACCE55u )

/* ========================== LOCAL VARIABLES =============================== */

/* ============================ TEST FIXTURE ================================ */

void setUp( void )
{
    TEST_ASSERT_EQUAL( REGMEM_REQUEST_OK, RegMem_Reset() );
}


void tearDown( void )
{
    /* Mocks are verified by generated runner */
}

/* ========================== MODULE VERSION ================================ */

/**
 * \brief   Itm_Get_ModuleVersion() returns version of the module.
 *
 * \par Expected results
 * - Version is 1.0.0.
 */
void Ut_Itm_Get_ModuleVersion_ReturnsVersion( void )
{
    itm_ModuleVersion_t version = Itm_Get_ModuleVersion();

    TEST_ASSERT_EQUAL_UINT8( 1u, version.Major );
    TEST_ASSERT_EQUAL_UINT8( 0u, version.Minor );
    TEST_ASSERT_EQUAL_UINT8( 0u, version.Patch );
}

/* =========================== DEFAULT CONFIG =============================== */

/**
 * \brief   Itm_Get_DefaultConfig() fills default configuration.
 *
 * \par Expected results
 * - ITM_REQUEST_OK, port 0, SWO 2 MHz, timestamp and sync inactive.
 * - NULL pointer: ITM_REQUEST_ERROR.
 */
void Ut_Itm_Get_DefaultConfig_ReturnsDefaults( void )
{
    itm_Config_t config = Ut_Itm_Get_Config( ITM_PORT_31, 1u );

    TEST_ASSERT_EQUAL( ITM_REQUEST_OK, Itm_Get_DefaultConfig( &config ) );

    TEST_ASSERT_EQUAL( ITM_PORT_0,                         config.PortId );
    TEST_ASSERT_EQUAL( ITM_FUNCTION_INACTIVE,              config.TimestampState );
    TEST_ASSERT_EQUAL( ITM_FUNCTION_INACTIVE,              config.SyncState );
    TEST_ASSERT_EQUAL_UINT32( UT_ITM_DEFAULT_SWO_CLK_HZ,   config.SwoClk );

    TEST_ASSERT_EQUAL( ITM_REQUEST_ERROR, Itm_Get_DefaultConfig( NULL ) );
}

/* ============================ INITIALIZATION ============================== */

/**
 * \brief   Itm_Init() configures trace, SWO output and enables ITM.
 *
 * \details Initializes port 5 with SWO 2 MHz (trace clock 200 MHz from RCC mock),
 *          SWTF hold time field preset.
 *
 * \par Expected results
 * - ITM_REQUEST_OK, DEMCR.TRCENA set.
 * - DBGMCU: trace port clock, D1 and D3 debug clocks enabled.
 * - SWO: unlocked (LAR), CODR = 99, SPPR = NRZ.
 * - SWTF: unlocked (LAR), slave port 0 enabled, hold time kept.
 * - ITM: unlocked (LAR), TER = port 5 only, TPR = 0, TCR = trace bus ID 1 + ITMENA.
 */
void Ut_Itm_Init_ValidConfig_ConfiguresSwoAndItm( void )
{
    itm_Config_t config = Ut_Itm_Get_Config( UT_ITM_PORT, UT_ITM_SWO_CLK_HZ );

    ITM->TER         = 0xFFFFFFFFu;
    ITM->TPR         = 0xFFFFFFFFu;
    UT_ITM_SWTF_CTRL = UT_ITM_SWTF_HT;

    Ut_Itm_Expect_TraceClk( UT_ITM_TRACE_CLK_HZ );

    TEST_ASSERT_EQUAL( ITM_REQUEST_OK, Itm_Init( &config ) );

    TEST_ASSERT_BITS_HIGH( CoreDebug_DEMCR_TRCENA_Msk, CoreDebug->DEMCR );

    TEST_ASSERT_EQUAL_HEX32( UT_ITM_DBGMCU_CLOCKS, DBGMCU->CR );

    TEST_ASSERT_EQUAL_HEX32( UT_ITM_LAR_UNLOCK_KEY, UT_ITM_SWO_LAR );
    TEST_ASSERT_EQUAL_UINT32( UT_ITM_SWO_PRESCALER, UT_ITM_SWO_CODR );
    TEST_ASSERT_EQUAL_HEX32( UT_ITM_SPPR_NRZ,       UT_ITM_SWO_SPPR );

    TEST_ASSERT_EQUAL_HEX32( UT_ITM_LAR_UNLOCK_KEY, UT_ITM_SWTF_LAR );
    TEST_ASSERT_EQUAL_HEX32( UT_ITM_SWTF_HT | UT_ITM_SWTF_ENS0, UT_ITM_SWTF_CTRL );

    TEST_ASSERT_EQUAL_HEX32( UT_ITM_LAR_UNLOCK_KEY,  ITM->LAR );
    TEST_ASSERT_EQUAL_HEX32( ( 1uL << UT_ITM_PORT ), ITM->TER );
    TEST_ASSERT_EQUAL_HEX32( 0u,                     ITM->TPR );
    TEST_ASSERT_EQUAL_HEX32( UT_ITM_TCR_TRACEBUSID | ITM_TCR_ITMENA_Msk, ITM->TCR );
}


/**
 * \brief   Itm_Init() enables timestamps and synchronization packets by configuration.
 *
 * \par Expected results
 * - TCR contains TSENA and SYNCENA beside trace bus ID and ITMENA.
 */
void Ut_Itm_Init_TimestampAndSync_SetsTcrBits( void )
{
    itm_Config_t config = Ut_Itm_Get_Config( ITM_PORT_0, UT_ITM_SWO_CLK_HZ );

    config.TimestampState = ITM_FUNCTION_ACTIVE;
    config.SyncState      = ITM_FUNCTION_ACTIVE;

    Ut_Itm_Expect_TraceClk( UT_ITM_TRACE_CLK_HZ );

    TEST_ASSERT_EQUAL( ITM_REQUEST_OK, Itm_Init( &config ) );

    TEST_ASSERT_EQUAL_HEX32( UT_ITM_TCR_TRACEBUSID | ITM_TCR_TSENA_Msk | ITM_TCR_SYNCENA_Msk | ITM_TCR_ITMENA_Msk,
                             ITM->TCR );
}


/**
 * \brief   Itm_Init() limits SWO prescaler.
 *
 * \details SWO clock higher than trace clock and SWO clock requiring prescaler
 *          above CODR register width.
 *
 * \par Expected results
 * - SWO clock above trace clock: CODR = 0 (SWO = trace clock).
 * - Very low SWO clock: CODR = maximal prescaler.
 */
void Ut_Itm_Init_SwoClkOutOfRange_LimitsPrescaler( void )
{
    itm_Config_t config = Ut_Itm_Get_Config( ITM_PORT_0, UT_ITM_TRACE_CLK_HZ * 2u );

    Ut_Itm_Expect_TraceClk( UT_ITM_TRACE_CLK_HZ );
    TEST_ASSERT_EQUAL( ITM_REQUEST_OK, Itm_Init( &config ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, UT_ITM_SWO_CODR );

    config.SwoClk = 1u;

    Ut_Itm_Expect_TraceClk( UT_ITM_TRACE_CLK_HZ );
    TEST_ASSERT_EQUAL( ITM_REQUEST_OK, Itm_Init( &config ) );
    TEST_ASSERT_EQUAL_UINT32( UT_ITM_SWO_PRESCALER_MAX, UT_ITM_SWO_CODR );
}


/**
 * \brief   Itm_Init() rejects invalid configuration.
 *
 * \details NULL configuration, invalid port, zero SWO clock.
 *
 * \par Expected results
 * - ITM_REQUEST_ERROR, RCC not called, ITM stays disabled.
 */
void Ut_Itm_Init_InvalidConfig_ReturnsError( void )
{
    itm_Config_t config = Ut_Itm_Get_Config( ITM_PORT_CNT, UT_ITM_SWO_CLK_HZ );

    TEST_ASSERT_EQUAL( ITM_REQUEST_ERROR, Itm_Init( NULL ) );
    TEST_ASSERT_EQUAL( ITM_REQUEST_ERROR, Itm_Init( &config ) );

    config = Ut_Itm_Get_Config( ITM_PORT_0, 0u );
    TEST_ASSERT_EQUAL( ITM_REQUEST_ERROR, Itm_Init( &config ) );

    TEST_ASSERT_EQUAL_HEX32( 0u, ITM->TCR );
    TEST_ASSERT_EQUAL_HEX32( 0u, CoreDebug->DEMCR );
}


/**
 * \brief   Itm_Init() fails when trace clock is not available.
 *
 * \par Expected results
 * - ITM_REQUEST_ERROR, ITM stays disabled, DBGMCU / SWO / SWTF not configured.
 */
void Ut_Itm_Init_RccError_ReturnsError( void )
{
    itm_Config_t config = Ut_Itm_Get_Config( ITM_PORT_0, UT_ITM_SWO_CLK_HZ );

    Rcc_Get_PeriphClk_ExpectAndReturn( RCC_PERIPH_TRACE, NULL, RCC_REQUEST_ERROR );
    Rcc_Get_PeriphClk_IgnoreArg_periphClk();

    TEST_ASSERT_EQUAL( ITM_REQUEST_ERROR, Itm_Init( &config ) );

    TEST_ASSERT_EQUAL_HEX32( 0u, ITM->TCR );
    TEST_ASSERT_EQUAL_HEX32( 0u, DBGMCU->CR );
    TEST_ASSERT_EQUAL_UINT32( 0u, UT_ITM_SWO_CODR );
    TEST_ASSERT_EQUAL_HEX32( 0u, UT_ITM_SWTF_CTRL );
}

/* =========================== DEINITIALIZATION ============================= */

/**
 * \brief   Itm_Deinit() disables ITM, SWO and trace when DWT is not used.
 *
 * \par Expected results
 * - TCR = 0, TER = 0, DBGMCU trace port clock cleared (debug clocks and other
 *   DBGMCU bits kept), DEMCR.TRCENA cleared.
 */
void Ut_Itm_Deinit_DwtDisabled_DisablesTrace( void )
{
    CoreDebug->DEMCR = CoreDebug_DEMCR_TRCENA_Msk;
    DBGMCU->CR = UT_ITM_DBGMCU_OTHER | UT_ITM_DBGMCU_CLOCKS;
    ITM->TCR   = ITM_TCR_ITMENA_Msk | ITM_TCR_TSENA_Msk;
    ITM->TER   = 0xFFFFFFFFu;

    Itm_Deinit();

    TEST_ASSERT_EQUAL_HEX32( 0u, ITM->TCR );
    TEST_ASSERT_EQUAL_HEX32( 0u, ITM->TER );
    TEST_ASSERT_EQUAL_HEX32( UT_ITM_DBGMCU_OTHER | UT_ITM_DBGMCU_DEBUG_CLOCKS, DBGMCU->CR );
    TEST_ASSERT_BITS_LOW( CoreDebug_DEMCR_TRCENA_Msk, CoreDebug->DEMCR );
}


/**
 * \brief   Itm_Deinit() keeps trace enabled while DWT counter is enabled.
 *
 * \par Expected results
 * - ITM disabled, DEMCR.TRCENA kept set.
 */
void Ut_Itm_Deinit_DwtEnabled_KeepsTrace( void )
{
    CoreDebug->DEMCR = CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL  = DWT_CTRL_CYCCNTENA_Msk;
    ITM->TCR   = ITM_TCR_ITMENA_Msk;

    Itm_Deinit();

    TEST_ASSERT_EQUAL_HEX32( 0u, ITM->TCR );
    TEST_ASSERT_BITS_HIGH( CoreDebug_DEMCR_TRCENA_Msk, CoreDebug->DEMCR );
}

/* ============================ PORT CONTROL ================================ */

/**
 * \brief   Itm_PortEnable() / Itm_PortDisable() change only bit of given port.
 *
 * \par Expected results
 * - Enable: port bit set, other bits kept.
 * - Disable: port bit cleared, other bits kept.
 * - Invalid port: ITM_REQUEST_ERROR, TER unchanged.
 */
void Ut_Itm_PortEnable_PortDisable_ChangeOwnBitOnly( void )
{
    ITM->TER = ( 1uL << ITM_PORT_0 );

    TEST_ASSERT_EQUAL( ITM_REQUEST_OK, Itm_PortEnable( ITM_PORT_31 ) );
    TEST_ASSERT_EQUAL_HEX32( ( 1uL << ITM_PORT_0 ) | ( 1uL << ITM_PORT_31 ), ITM->TER );

    TEST_ASSERT_EQUAL( ITM_REQUEST_OK, Itm_PortDisable( ITM_PORT_0 ) );
    TEST_ASSERT_EQUAL_HEX32( ( 1uL << ITM_PORT_31 ), ITM->TER );

    TEST_ASSERT_EQUAL( ITM_REQUEST_ERROR, Itm_PortEnable( ITM_PORT_CNT ) );
    TEST_ASSERT_EQUAL( ITM_REQUEST_ERROR, Itm_PortDisable( ITM_PORT_CNT ) );
    TEST_ASSERT_EQUAL_HEX32( ( 1uL << ITM_PORT_31 ), ITM->TER );
}


/**
 * \brief   Timestamp and sync functions change only their own TCR bit.
 *
 * \par Expected results
 * - Enable sets the bit, disable clears it, ITMENA kept.
 */
void Ut_Itm_Set_TimestampSync_ChangeOwnBitOnly( void )
{
    ITM->TCR = ITM_TCR_ITMENA_Msk;

    Itm_Set_TimestampEnable();
    TEST_ASSERT_EQUAL_HEX32( ITM_TCR_ITMENA_Msk | ITM_TCR_TSENA_Msk, ITM->TCR );

    Itm_Set_SyncEnable();
    TEST_ASSERT_EQUAL_HEX32( ITM_TCR_ITMENA_Msk | ITM_TCR_TSENA_Msk | ITM_TCR_SYNCENA_Msk, ITM->TCR );

    Itm_Set_TimestampDisable();
    TEST_ASSERT_EQUAL_HEX32( ITM_TCR_ITMENA_Msk | ITM_TCR_SYNCENA_Msk, ITM->TCR );

    Itm_Set_SyncDisable();
    TEST_ASSERT_EQUAL_HEX32( ITM_TCR_ITMENA_Msk, ITM->TCR );
}

/* ============================= TRANSMISSION =============================== */

/**
 * \brief   Transmit functions write data to ready enabled port.
 *
 * \details ITM and port 5 enabled, port FIFO ready preset before every write.
 *
 * \par Expected results
 * - 8, 16 and 32-bit data are written to stimulus port 5.
 */
void Ut_Itm_Transmit_PortReady_WritesData( void )
{
    ITM->TCR = ITM_TCR_ITMENA_Msk;
    ITM->TER = ( 1uL << UT_ITM_PORT );

    Ut_Itm_Set_PortReady( UT_ITM_PORT );
    Itm_Transmit_8b( UT_ITM_PORT, 0xA5u );
    TEST_ASSERT_EQUAL_HEX8( 0xA5u, ITM->PORT[ UT_ITM_PORT ].u8 );

    Ut_Itm_Set_PortReady( UT_ITM_PORT );
    Itm_Transmit_16b( UT_ITM_PORT, 0xBEEFu );
    TEST_ASSERT_EQUAL_HEX16( 0xBEEFu, ITM->PORT[ UT_ITM_PORT ].u16 );

    Ut_Itm_Set_PortReady( UT_ITM_PORT );
    Itm_Transmit_32b( UT_ITM_PORT, 0xDEADBEEFu );
    TEST_ASSERT_EQUAL_HEX32( 0xDEADBEEFu, ITM->PORT[ UT_ITM_PORT ].u32 );
}


/**
 * \brief   Transmit functions drop data when ITM or the port is disabled.
 *
 * \details Port FIFO ready preset, ITM disabled; then ITM enabled but port disabled.
 *
 * \par Expected results
 * - Stimulus port is not written.
 */
void Ut_Itm_Transmit_Disabled_DropsData( void )
{
    Ut_Itm_Set_PortReady( UT_ITM_PORT );
    ITM->TER = ( 1uL << UT_ITM_PORT );

    Itm_Transmit_32b( UT_ITM_PORT, 0xDEADBEEFu );
    TEST_ASSERT_EQUAL_HEX32( 1u, ITM->PORT[ UT_ITM_PORT ].u32 );

    ITM->TCR = ITM_TCR_ITMENA_Msk;
    ITM->TER = 0u;

    Itm_Transmit_8b( UT_ITM_PORT, 0xA5u );
    Itm_Transmit_16b( UT_ITM_PORT, 0xBEEFu );
    Itm_Transmit_32b( UT_ITM_PORT, 0xDEADBEEFu );
    TEST_ASSERT_EQUAL_HEX32( 1u, ITM->PORT[ UT_ITM_PORT ].u32 );
}


/**
 * \brief   Transmit function drops data when port FIFO is not ready in time.
 *
 * \details ITM and port enabled, FIFO ready never set (timeout branch).
 *
 * \par Expected results
 * - Stimulus port is not written.
 */
void Ut_Itm_Transmit_PortNotReady_DropsDataAfterTimeout( void )
{
    ITM->TCR = ITM_TCR_ITMENA_Msk;
    ITM->TER = ( 1uL << UT_ITM_PORT );

    Itm_Transmit_32b( UT_ITM_PORT, 0xDEADBEEEu );

    TEST_ASSERT_EQUAL_HEX32( 0u, ITM->PORT[ UT_ITM_PORT ].u32 );
}


/**
 * \brief   Transmit functions ignore invalid port.
 *
 * \par Expected results
 * - No write to emulated memory behind the last port (no crash), port 31 unchanged.
 */
void Ut_Itm_Transmit_InvalidPort_Ignored( void )
{
    ITM->TCR = ITM_TCR_ITMENA_Msk;
    ITM->TER = 0xFFFFFFFFu;
    Ut_Itm_Set_PortReady( ITM_PORT_31 );

    Itm_Transmit_8b( ITM_PORT_CNT, 0xA5u );
    Itm_Transmit_16b( ITM_PORT_CNT, 0xBEEFu );
    Itm_Transmit_32b( ITM_PORT_CNT, 0xDEADBEEFu );

    TEST_ASSERT_EQUAL_HEX32( 1u, ITM->PORT[ ITM_PORT_31 ].u32 );
}

/* ================================ STATUS ================================== */

/**
 * \brief   Itm_Get_TransmitState() reports ready enabled port.
 *
 * \par Expected results
 * - ITM_FLAG_ACTIVE only if ITM enabled, port enabled and FIFO ready.
 * - Invalid port: ITM_FLAG_INACTIVE.
 */
void Ut_Itm_Get_TransmitState_ReportsPortReady( void )
{
    TEST_ASSERT_EQUAL( ITM_FLAG_INACTIVE, Itm_Get_TransmitState( UT_ITM_PORT ) );

    Ut_Itm_Set_PortReady( UT_ITM_PORT );
    TEST_ASSERT_EQUAL( ITM_FLAG_INACTIVE, Itm_Get_TransmitState( UT_ITM_PORT ) );   /* ITM disabled */

    ITM->TCR = ITM_TCR_ITMENA_Msk;
    TEST_ASSERT_EQUAL( ITM_FLAG_INACTIVE, Itm_Get_TransmitState( UT_ITM_PORT ) );   /* Port disabled */

    ITM->TER = ( 1uL << UT_ITM_PORT );
    TEST_ASSERT_EQUAL( ITM_FLAG_ACTIVE, Itm_Get_TransmitState( UT_ITM_PORT ) );

    TEST_ASSERT_EQUAL( ITM_FLAG_INACTIVE, Itm_Get_TransmitState( ITM_PORT_CNT ) );
}


/**
 * \brief   Itm_Get_BusyState() reports TCR.BUSY flag.
 *
 * \par Expected results
 * - BUSY cleared: ITM_FLAG_INACTIVE, BUSY set: ITM_FLAG_ACTIVE.
 */
void Ut_Itm_Get_BusyState_ReportsBusyFlag( void )
{
    ITM->TCR = ITM_TCR_ITMENA_Msk;
    TEST_ASSERT_EQUAL( ITM_FLAG_INACTIVE, Itm_Get_BusyState() );

    ITM->TCR |= ITM_TCR_BUSY_Msk;
    TEST_ASSERT_EQUAL( ITM_FLAG_ACTIVE, Itm_Get_BusyState() );
}

/* ============================ HELPER FUNCTIONS ============================ */

/** Expects trace clock request to RCC module and returns required frequency */
static void Ut_Itm_Expect_TraceClk( rcc_FreqHz_t traceClk )
{
    static rcc_FreqHz_t traceClkRet;

    traceClkRet = traceClk;

    Rcc_Get_PeriphClk_ExpectAndReturn( RCC_PERIPH_TRACE, NULL, RCC_REQUEST_OK );
    Rcc_Get_PeriphClk_IgnoreArg_periphClk();
    Rcc_Get_PeriphClk_ReturnThruPtr_periphClk( &traceClkRet );
}


/** Returns configuration with given port and SWO clock, timestamp and sync inactive */
static itm_Config_t Ut_Itm_Get_Config( itm_Port_t portId, itm_FreqHz_t swoClk )
{
    itm_Config_t config;

    config.PortId         = portId;
    config.TimestampState = ITM_FUNCTION_INACTIVE;
    config.SyncState      = ITM_FUNCTION_INACTIVE;
    config.SwoClk         = swoClk;

    return ( config );
}


/** Presets stimulus port FIFO ready flag (set by HW) */
static void Ut_Itm_Set_PortReady( itm_Port_t portId )
{
    ITM->PORT[ portId ].u32 = 1u;
}
