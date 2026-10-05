/**
 * \author Mr.Nobody
 * \file Test_Dwt.c
 * \ingroup Dwt
 * \brief Unit tests of Data Watchpoint and Trace Unit (DWT) module.
 *
 * Dwt.c is compiled unchanged with real CMSIS headers. DWT, CoreDebug and ITM
 * registers (private peripheral bus) are emulated by RegMem. Counters do not
 * count on host, tests check register values written by the module.
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "RegMem.h"                         /* Register memory emulation      */
#include "Dwt_Port.h"                       /* Module under test              */
#include "Stm32.h"                          /* DWT / CoreDebug / ITM registers*/
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

static dwt_Config_t     Ut_Dwt_Get_Config   ( dwt_FunctionState_t state );

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Mask of all counter enable bits in DWT_CTRL */
#define UT_DWT_CTRL_ALL_COUNTERS            ( DWT_CTRL_CYCCNTENA_Msk   | \
                                              DWT_CTRL_CPIEVTENA_Msk   | \
                                              DWT_CTRL_EXCEVTENA_Msk   | \
                                              DWT_CTRL_SLEEPEVTENA_Msk | \
                                              DWT_CTRL_LSUEVTENA_Msk   | \
                                              DWT_CTRL_FOLDEVTENA_Msk    )

/** Value preset to counters to check reset */
#define UT_DWT_COUNTER_PRESET               ( 0xA5u )

/** Value preset to cycle counter to check reset */
#define UT_DWT_CYCCNT_PRESET                ( 0x12345678u )

/* ========================== LOCAL VARIABLES =============================== */

/* ============================ TEST FIXTURE ================================ */

void setUp( void )
{
    TEST_ASSERT_EQUAL( REGMEM_REQUEST_OK, RegMem_Reset() );
}


void tearDown( void )
{
}

/* ========================== MODULE VERSION ================================ */

/**
 * \brief   Dwt_Get_ModuleVersion() returns version of the module.
 *
 * \par Expected results
 * - Version is 1.0.0.
 */
void Ut_Dwt_Get_ModuleVersion_ReturnsVersion( void )
{
    dwt_ModuleVersion_t version = Dwt_Get_ModuleVersion();

    TEST_ASSERT_EQUAL_UINT8( 1u, version.Major );
    TEST_ASSERT_EQUAL_UINT8( 0u, version.Minor );
    TEST_ASSERT_EQUAL_UINT8( 0u, version.Patch );
}

/* ================================ TASK ==================================== */

/**
 * \brief   Dwt_Task() has no periodic processing.
 *
 * \details DWT and DEMCR registers preset, Dwt_Task() called.
 *
 * \par Expected results
 * - DWT CTRL / CYCCNT and DEMCR not changed.
 */
void Ut_Dwt_Task_NoRegisterAccess( void )
{
    DWT->CTRL            = DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT          = 0x1234u;
    CoreDebug->DEMCR     = CoreDebug_DEMCR_TRCENA_Msk;

    Dwt_Task();

    TEST_ASSERT_EQUAL_HEX32( DWT_CTRL_CYCCNTENA_Msk, DWT->CTRL );
    TEST_ASSERT_EQUAL_HEX32( 0x1234u, DWT->CYCCNT );
    TEST_ASSERT_EQUAL_HEX32( CoreDebug_DEMCR_TRCENA_Msk, CoreDebug->DEMCR );
}

/* =========================== DEFAULT CONFIG =============================== */

/**
 * \brief   Dwt_Get_DefaultConfig() fills default configuration.
 *
 * \par Expected results
 * - DWT_REQUEST_OK, only cycle counter active.
 * - NULL pointer: DWT_REQUEST_ERROR.
 */
void Ut_Dwt_Get_DefaultConfig_CycleCounterOnly( void )
{
    dwt_Config_t config = Ut_Dwt_Get_Config( DWT_FUNCTION_INACTIVE );

    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_DefaultConfig( &config ) );

    TEST_ASSERT_EQUAL( DWT_FUNCTION_ACTIVE,   config.CycleCounterState );
    TEST_ASSERT_EQUAL( DWT_FUNCTION_INACTIVE, config.CpiCounterState );
    TEST_ASSERT_EQUAL( DWT_FUNCTION_INACTIVE, config.ExceptionCounterState );
    TEST_ASSERT_EQUAL( DWT_FUNCTION_INACTIVE, config.SleepCounterState );
    TEST_ASSERT_EQUAL( DWT_FUNCTION_INACTIVE, config.LoadStoreCounterState );
    TEST_ASSERT_EQUAL( DWT_FUNCTION_INACTIVE, config.FoldCounterState );

    TEST_ASSERT_EQUAL( DWT_REQUEST_ERROR, Dwt_Get_DefaultConfig( NULL ) );
}

/* ============================ INITIALIZATION ============================== */

/**
 * \brief   Dwt_Init() with NULL configuration fails.
 *
 * \par Expected results
 * - DWT_REQUEST_ERROR, no register is changed.
 */
void Ut_Dwt_Init_NullConfig_ReturnsError( void )
{
    TEST_ASSERT_EQUAL( DWT_REQUEST_ERROR, Dwt_Init( NULL ) );

    TEST_ASSERT_EQUAL_HEX32( 0u, CoreDebug->DEMCR );
    TEST_ASSERT_EQUAL_HEX32( 0u, DWT->CTRL );
}


/**
 * \brief   Dwt_Init() with all counters active enables trace and all counters.
 *
 * \details Counters are preset to non-zero values.
 *
 * \par Expected results
 * - DWT_REQUEST_OK, DEMCR.TRCENA set.
 * - All counter enable bits set in CTRL.
 * - All counters reset to 0.
 */
void Ut_Dwt_Init_AllActive_EnablesAllCounters( void )
{
    dwt_Config_t config = Ut_Dwt_Get_Config( DWT_FUNCTION_ACTIVE );

    DWT->CYCCNT   = UT_DWT_CYCCNT_PRESET;
    DWT->CPICNT   = UT_DWT_COUNTER_PRESET;
    DWT->EXCCNT   = UT_DWT_COUNTER_PRESET;
    DWT->SLEEPCNT = UT_DWT_COUNTER_PRESET;
    DWT->LSUCNT   = UT_DWT_COUNTER_PRESET;
    DWT->FOLDCNT  = UT_DWT_COUNTER_PRESET;

    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );

    TEST_ASSERT_BITS_HIGH( CoreDebug_DEMCR_TRCENA_Msk, CoreDebug->DEMCR );
    TEST_ASSERT_EQUAL_HEX32( UT_DWT_CTRL_ALL_COUNTERS, DWT->CTRL );

    TEST_ASSERT_EQUAL_UINT32( 0u, DWT->CYCCNT );
    TEST_ASSERT_EQUAL_UINT32( 0u, DWT->CPICNT );
    TEST_ASSERT_EQUAL_UINT32( 0u, DWT->EXCCNT );
    TEST_ASSERT_EQUAL_UINT32( 0u, DWT->SLEEPCNT );
    TEST_ASSERT_EQUAL_UINT32( 0u, DWT->LSUCNT );
    TEST_ASSERT_EQUAL_UINT32( 0u, DWT->FOLDCNT );
}


/**
 * \brief   Dwt_Init() with all counters inactive enables only trace.
 *
 * \par Expected results
 * - DWT_REQUEST_OK, DEMCR.TRCENA set, CTRL = 0.
 */
void Ut_Dwt_Init_AllInactive_NoCounterEnabled( void )
{
    dwt_Config_t config = Ut_Dwt_Get_Config( DWT_FUNCTION_INACTIVE );

    DWT->CTRL = UT_DWT_CTRL_ALL_COUNTERS;

    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );

    TEST_ASSERT_BITS_HIGH( CoreDebug_DEMCR_TRCENA_Msk, CoreDebug->DEMCR );
    TEST_ASSERT_EQUAL_HEX32( 0u, DWT->CTRL );
}


/**
 * \brief   Dwt_Init() enables every counter by its own configuration item.
 *
 * \details Initializes the module six times, each time with one counter active.
 *
 * \par Expected results
 * - CTRL contains only the enable bit of the active counter.
 */
void Ut_Dwt_Init_SingleCounter_EnablesOnlyThisCounter( void )
{
    dwt_Config_t config;

    config = Ut_Dwt_Get_Config( DWT_FUNCTION_INACTIVE );
    config.CycleCounterState = DWT_FUNCTION_ACTIVE;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );
    TEST_ASSERT_EQUAL_HEX32( DWT_CTRL_CYCCNTENA_Msk, DWT->CTRL );

    config = Ut_Dwt_Get_Config( DWT_FUNCTION_INACTIVE );
    config.CpiCounterState = DWT_FUNCTION_ACTIVE;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );
    TEST_ASSERT_EQUAL_HEX32( DWT_CTRL_CPIEVTENA_Msk, DWT->CTRL );

    config = Ut_Dwt_Get_Config( DWT_FUNCTION_INACTIVE );
    config.ExceptionCounterState = DWT_FUNCTION_ACTIVE;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );
    TEST_ASSERT_EQUAL_HEX32( DWT_CTRL_EXCEVTENA_Msk, DWT->CTRL );

    config = Ut_Dwt_Get_Config( DWT_FUNCTION_INACTIVE );
    config.SleepCounterState = DWT_FUNCTION_ACTIVE;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );
    TEST_ASSERT_EQUAL_HEX32( DWT_CTRL_SLEEPEVTENA_Msk, DWT->CTRL );

    config = Ut_Dwt_Get_Config( DWT_FUNCTION_INACTIVE );
    config.LoadStoreCounterState = DWT_FUNCTION_ACTIVE;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );
    TEST_ASSERT_EQUAL_HEX32( DWT_CTRL_LSUEVTENA_Msk, DWT->CTRL );

    config = Ut_Dwt_Get_Config( DWT_FUNCTION_INACTIVE );
    config.FoldCounterState = DWT_FUNCTION_ACTIVE;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );
    TEST_ASSERT_EQUAL_HEX32( DWT_CTRL_FOLDEVTENA_Msk, DWT->CTRL );
}

/* =========================== DEINITIALIZATION ============================= */

/**
 * \brief   Dwt_Deinit() disables counters and trace when ITM is not enabled.
 *
 * \par Expected results
 * - CTRL = 0, CYCCNT = 0, DEMCR.TRCENA cleared.
 */
void Ut_Dwt_Deinit_ItmDisabled_DisablesTrace( void )
{
    CoreDebug->DEMCR  = CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL   = UT_DWT_CTRL_ALL_COUNTERS;
    DWT->CYCCNT = UT_DWT_CYCCNT_PRESET;

    Dwt_Deinit();

    TEST_ASSERT_EQUAL_HEX32( 0u, DWT->CTRL );
    TEST_ASSERT_EQUAL_UINT32( 0u, DWT->CYCCNT );
    TEST_ASSERT_BITS_LOW( CoreDebug_DEMCR_TRCENA_Msk, CoreDebug->DEMCR );
}


/**
 * \brief   Dwt_Deinit() keeps trace enabled while ITM is enabled.
 *
 * \par Expected results
 * - CTRL = 0, DEMCR.TRCENA kept set.
 */
void Ut_Dwt_Deinit_ItmEnabled_KeepsTrace( void )
{
    CoreDebug->DEMCR = CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL  = UT_DWT_CTRL_ALL_COUNTERS;
    ITM->TCR   = ITM_TCR_ITMENA_Msk;

    Dwt_Deinit();

    TEST_ASSERT_EQUAL_HEX32( 0u, DWT->CTRL );
    TEST_ASSERT_BITS_HIGH( CoreDebug_DEMCR_TRCENA_Msk, CoreDebug->DEMCR );
}

/* =============================== COUNTERS ================================= */

/**
 * \brief   Getters return values of counter registers.
 *
 * \details Presets every counter to a different value.
 *
 * \par Expected results
 * - Every getter returns its own register value.
 */
void Ut_Dwt_Get_Counters_ReturnRegisterValues( void )
{
    DWT->CYCCNT   = UT_DWT_CYCCNT_PRESET;
    DWT->CPICNT   = 0x11u;
    DWT->EXCCNT   = 0x22u;
    DWT->SLEEPCNT = 0x33u;
    DWT->LSUCNT   = 0x44u;
    DWT->FOLDCNT  = 0x55u;

    TEST_ASSERT_EQUAL_HEX32( UT_DWT_CYCCNT_PRESET, Dwt_Get_CycleCounter() );
    TEST_ASSERT_EQUAL_HEX32( 0x11u, Dwt_Get_CpiCounter() );
    TEST_ASSERT_EQUAL_HEX32( 0x22u, Dwt_Get_ExceptionCounter() );
    TEST_ASSERT_EQUAL_HEX32( 0x33u, Dwt_Get_SleepCounter() );
    TEST_ASSERT_EQUAL_HEX32( 0x44u, Dwt_Get_LoadStoreCounter() );
    TEST_ASSERT_EQUAL_HEX32( 0x55u, Dwt_Get_FoldCounter() );
}


/**
 * \brief   Reset functions clear only their own counter.
 *
 * \details Presets all counters, then resets them one by one.
 *
 * \par Expected results
 * - After each reset the reset counter is 0, the following counters keep value.
 */
void Ut_Dwt_Reset_Counters_ClearOwnCounterOnly( void )
{
    DWT->CYCCNT   = UT_DWT_CYCCNT_PRESET;
    DWT->CPICNT   = UT_DWT_COUNTER_PRESET;
    DWT->EXCCNT   = UT_DWT_COUNTER_PRESET;
    DWT->SLEEPCNT = UT_DWT_COUNTER_PRESET;
    DWT->LSUCNT   = UT_DWT_COUNTER_PRESET;
    DWT->FOLDCNT  = UT_DWT_COUNTER_PRESET;

    Dwt_Reset_CycleCounter();
    TEST_ASSERT_EQUAL_UINT32( 0u, DWT->CYCCNT );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_COUNTER_PRESET, DWT->CPICNT );

    Dwt_Reset_CpiCounter();
    TEST_ASSERT_EQUAL_UINT32( 0u, DWT->CPICNT );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_COUNTER_PRESET, DWT->EXCCNT );

    Dwt_Reset_ExceptionCounter();
    TEST_ASSERT_EQUAL_UINT32( 0u, DWT->EXCCNT );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_COUNTER_PRESET, DWT->SLEEPCNT );

    Dwt_Reset_SleepCounter();
    TEST_ASSERT_EQUAL_UINT32( 0u, DWT->SLEEPCNT );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_COUNTER_PRESET, DWT->LSUCNT );

    Dwt_Reset_LoadStoreCounter();
    TEST_ASSERT_EQUAL_UINT32( 0u, DWT->LSUCNT );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_COUNTER_PRESET, DWT->FOLDCNT );

    Dwt_Reset_FoldCounter();
    TEST_ASSERT_EQUAL_UINT32( 0u, DWT->FOLDCNT );
}

/* ============================ HELPER FUNCTIONS ============================ */

/** Returns configuration with all counters in required state */
static dwt_Config_t Ut_Dwt_Get_Config( dwt_FunctionState_t state )
{
    dwt_Config_t config;

    config.CycleCounterState     = state;
    config.CpiCounterState       = state;
    config.ExceptionCounterState = state;
    config.SleepCounterState     = state;
    config.LoadStoreCounterState = state;
    config.FoldCounterState      = state;

    return ( config );
}
