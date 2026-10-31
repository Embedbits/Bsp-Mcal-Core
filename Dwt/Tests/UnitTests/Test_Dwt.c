/**
 * \author Mr.Nobody
 * \file Test_Dwt.c
 * \ingroup Dwt
 * \brief Unit tests of Data Watchpoint and Trace Unit (DWT) module.
 *
 * Dwt.c is compiled unchanged with real CMSIS headers. DWT, DCB and ITM
 * registers (private peripheral bus) are emulated by RegMem. Counters do not
 * count on host, tests check register values written by the module.
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "RegMem.h"                         /* Register memory emulation      */
#include "Dwt_Port.h"                       /* Module under test              */
#include "Stm32.h"                          /* DWT / DCB / ITM registers      */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

static dwt_Config_t     UtDwt_Get_Config   ( dwt_FunctionState_t state );

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

/** Values preset to profiling counters to check the getters */
#define UT_DWT_CPICNT_VALUE                 ( 0x11u )
#define UT_DWT_EXCCNT_VALUE                 ( 0x22u )
#define UT_DWT_SLEEPCNT_VALUE               ( 0x33u )
#define UT_DWT_LSUCNT_VALUE                 ( 0x44u )
#define UT_DWT_FOLDCNT_VALUE                ( 0x55u )

/** Preset of the output variables to check that they are not changed on error */
#define UT_DWT_SENTINEL                     ( 0x5Au )

/** Core clocks other than the default one */
#define UT_DWT_FREQ_64MHZ                   ( 64000000u )
#define UT_DWT_FREQ_12M5HZ                  ( 12500000u )
#define UT_DWT_FREQ_1KHZ                    ( 1000u )

/** Cycle counter values for the elapsed time tests (250 MHz = 250 cycles per microsecond) */
#define UT_DWT_START_CYCLE                  ( 1000u )
#define UT_DWT_CYCLES_1US_END               ( 1250u )
#define UT_DWT_CYCLES_BELOW_1US_END         ( 1249u )
#define UT_DWT_CYCLES_50US                  ( 12500u )
#define UT_DWT_CYCLES_12M5HZ_2US            ( 25u )
#define UT_DWT_WRAP_START                   ( 0xFFFFFF00u )
#define UT_DWT_WRAP_NOW                     ( 0x000000FFu )
#define UT_DWT_ALL_ONES                     ( 0xFFFFFFFFu )

/** Expected elapsed times in microseconds */
#define UT_DWT_TIME_2US                     ( 2u )
#define UT_DWT_TIME_50US                    ( 50u )
#define UT_DWT_TIME_FULL_US                 ( 17179869u )

/** Core clock used by the tests */
#define UT_DWT_CORE_CLOCK_HZ                ( 250000000u )

/* ========================== LOCAL VARIABLES =============================== */

/** CMSIS core clock, defined by Rcc in the firmware */
uint32_t SystemCoreClock = UT_DWT_CORE_CLOCK_HZ;

/* ============================ TEST FIXTURE ================================ */

void setUp( void )
{
    TEST_ASSERT_EQUAL( REGMEM_REQUEST_OK, RegMem_Reset() );
    SystemCoreClock = UT_DWT_CORE_CLOCK_HZ;
}


void tearDown( void )
{
}

/* ========================== MODULE VERSION ================================ */

/**
 * \brief   Dwt_Get_ModuleVersion() returns version of the module.
 *
 * \par Expected results
 * - Version is 1.1.0.
 */
void Ut_Dwt_Get_ModuleVersion_ReturnsVersion( void )
{
    dwt_ModuleVersion_t version = Dwt_Get_ModuleVersion();

    TEST_ASSERT_EQUAL_UINT8( 1u, version.Major );
    TEST_ASSERT_EQUAL_UINT8( 1u, version.Minor );
    TEST_ASSERT_EQUAL_UINT8( 0u, version.Patch );
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
    dwt_Config_t config = UtDwt_Get_Config( DWT_FUNCTION_INACTIVE );

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

    TEST_ASSERT_EQUAL_HEX32( 0u, DCB->DEMCR );
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
    dwt_Config_t config = UtDwt_Get_Config( DWT_FUNCTION_ACTIVE );

    DWT->CYCCNT   = UT_DWT_CYCCNT_PRESET;
    DWT->CPICNT   = UT_DWT_COUNTER_PRESET;
    DWT->EXCCNT   = UT_DWT_COUNTER_PRESET;
    DWT->SLEEPCNT = UT_DWT_COUNTER_PRESET;
    DWT->LSUCNT   = UT_DWT_COUNTER_PRESET;
    DWT->FOLDCNT  = UT_DWT_COUNTER_PRESET;

    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );

    TEST_ASSERT_BITS_HIGH( DCB_DEMCR_TRCENA_Msk, DCB->DEMCR );
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
    dwt_Config_t config = UtDwt_Get_Config( DWT_FUNCTION_INACTIVE );

    DWT->CTRL = UT_DWT_CTRL_ALL_COUNTERS;

    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );

    TEST_ASSERT_BITS_HIGH( DCB_DEMCR_TRCENA_Msk, DCB->DEMCR );
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

    config = UtDwt_Get_Config( DWT_FUNCTION_INACTIVE );
    config.CycleCounterState = DWT_FUNCTION_ACTIVE;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );
    TEST_ASSERT_EQUAL_HEX32( DWT_CTRL_CYCCNTENA_Msk, DWT->CTRL );

    config = UtDwt_Get_Config( DWT_FUNCTION_INACTIVE );
    config.CpiCounterState = DWT_FUNCTION_ACTIVE;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );
    TEST_ASSERT_EQUAL_HEX32( DWT_CTRL_CPIEVTENA_Msk, DWT->CTRL );

    config = UtDwt_Get_Config( DWT_FUNCTION_INACTIVE );
    config.ExceptionCounterState = DWT_FUNCTION_ACTIVE;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );
    TEST_ASSERT_EQUAL_HEX32( DWT_CTRL_EXCEVTENA_Msk, DWT->CTRL );

    config = UtDwt_Get_Config( DWT_FUNCTION_INACTIVE );
    config.SleepCounterState = DWT_FUNCTION_ACTIVE;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );
    TEST_ASSERT_EQUAL_HEX32( DWT_CTRL_SLEEPEVTENA_Msk, DWT->CTRL );

    config = UtDwt_Get_Config( DWT_FUNCTION_INACTIVE );
    config.LoadStoreCounterState = DWT_FUNCTION_ACTIVE;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );
    TEST_ASSERT_EQUAL_HEX32( DWT_CTRL_LSUEVTENA_Msk, DWT->CTRL );

    config = UtDwt_Get_Config( DWT_FUNCTION_INACTIVE );
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
    DCB->DEMCR  = DCB_DEMCR_TRCENA_Msk;
    DWT->CTRL   = UT_DWT_CTRL_ALL_COUNTERS;
    DWT->CYCCNT = UT_DWT_CYCCNT_PRESET;

    Dwt_Deinit();

    TEST_ASSERT_EQUAL_HEX32( 0u, DWT->CTRL );
    TEST_ASSERT_EQUAL_UINT32( 0u, DWT->CYCCNT );
    TEST_ASSERT_BITS_LOW( DCB_DEMCR_TRCENA_Msk, DCB->DEMCR );
}


/**
 * \brief   Dwt_Deinit() keeps trace enabled while ITM is enabled.
 *
 * \par Expected results
 * - CTRL = 0, DEMCR.TRCENA kept set.
 */
void Ut_Dwt_Deinit_ItmEnabled_KeepsTrace( void )
{
    DCB->DEMCR = DCB_DEMCR_TRCENA_Msk;
    DWT->CTRL  = UT_DWT_CTRL_ALL_COUNTERS;
    ITM->TCR   = ITM_TCR_ITMENA_Msk;

    Dwt_Deinit();

    TEST_ASSERT_EQUAL_HEX32( 0u, DWT->CTRL );
    TEST_ASSERT_BITS_HIGH( DCB_DEMCR_TRCENA_Msk, DCB->DEMCR );
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
    DWT->CPICNT   = UT_DWT_CPICNT_VALUE;
    DWT->EXCCNT   = UT_DWT_EXCCNT_VALUE;
    DWT->SLEEPCNT = UT_DWT_SLEEPCNT_VALUE;
    DWT->LSUCNT   = UT_DWT_LSUCNT_VALUE;
    DWT->FOLDCNT  = UT_DWT_FOLDCNT_VALUE;

    TEST_ASSERT_EQUAL_HEX32( UT_DWT_CYCCNT_PRESET, Dwt_Get_CycleCounter() );
    TEST_ASSERT_EQUAL_HEX32( UT_DWT_CPICNT_VALUE, Dwt_Get_CpiCounter() );
    TEST_ASSERT_EQUAL_HEX32( UT_DWT_EXCCNT_VALUE, Dwt_Get_ExceptionCounter() );
    TEST_ASSERT_EQUAL_HEX32( UT_DWT_SLEEPCNT_VALUE, Dwt_Get_SleepCounter() );
    TEST_ASSERT_EQUAL_HEX32( UT_DWT_LSUCNT_VALUE, Dwt_Get_LoadStoreCounter() );
    TEST_ASSERT_EQUAL_HEX32( UT_DWT_FOLDCNT_VALUE, Dwt_Get_FoldCounter() );
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

/* ======================= STATE, FREQUENCY, ELAPSED TIME =================== */

/** Starts the cycle counter by the default configuration */
static void UtDwt_Start_CycleCounter( void )
{
    dwt_Config_t config;

    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_DefaultConfig( &config ) );
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );
}


/**
 * \brief   Dwt_Get_State() reports whether the cycle counter runs.
 *
 * \par Expected results
 * - NULL pointer: DWT_REQUEST_ERROR.
 * - Before init, with TRCENA only, or with CYCCNTENA only: INACTIVE.
 * - After init with the cycle counter: ACTIVE.
 * - After Dwt_Deinit(): INACTIVE.
 * - No register is changed by the getter.
 */
void Ut_Dwt_Get_State_ReportsCycleCounter( void )
{
    dwt_FunctionState_t state = DWT_FUNCTION_ACTIVE;

    TEST_ASSERT_EQUAL( DWT_REQUEST_ERROR, Dwt_Get_State( NULL ) );

    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_State( &state ) );
    TEST_ASSERT_EQUAL( DWT_FUNCTION_INACTIVE, state );

    DCB->DEMCR |= DCB_DEMCR_TRCENA_Msk;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_State( &state ) );
    TEST_ASSERT_EQUAL( DWT_FUNCTION_INACTIVE, state );

    DCB->DEMCR = 0u;
    DWT->CTRL  = DWT_CTRL_CYCCNTENA_Msk;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_State( &state ) );
    TEST_ASSERT_EQUAL( DWT_FUNCTION_INACTIVE, state );

    UtDwt_Start_CycleCounter();
    DWT->CYCCNT = UT_DWT_CYCCNT_PRESET;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_State( &state ) );
    TEST_ASSERT_EQUAL( DWT_FUNCTION_ACTIVE, state );
    TEST_ASSERT_EQUAL_HEX32( DWT_CTRL_CYCCNTENA_Msk, DWT->CTRL );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_CYCCNT_PRESET, DWT->CYCCNT );

    Dwt_Deinit();
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_State( &state ) );
    TEST_ASSERT_EQUAL( DWT_FUNCTION_INACTIVE, state );
}


/**
 * \brief   Dwt_Get_CycleFrequency() returns the core clock.
 *
 * \par Expected results
 * - NULL pointer: DWT_REQUEST_ERROR.
 * - Counter not running: DWT_REQUEST_ERROR, output unchanged.
 * - Counter running: frequency = SystemCoreClock (also after its change).
 * - SystemCoreClock = 0: DWT_REQUEST_ERROR.
 */
void Ut_Dwt_Get_CycleFrequency_ReturnsCoreClock( void )
{
    dwt_Frequency_Hz_t freq = UT_DWT_SENTINEL;

    TEST_ASSERT_EQUAL( DWT_REQUEST_ERROR, Dwt_Get_CycleFrequency( &freq ) );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_SENTINEL, freq );

    UtDwt_Start_CycleCounter();

    TEST_ASSERT_EQUAL( DWT_REQUEST_ERROR, Dwt_Get_CycleFrequency( NULL ) );

    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_CycleFrequency( &freq ) );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_CORE_CLOCK_HZ, freq );

    SystemCoreClock = UT_DWT_FREQ_64MHZ;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_CycleFrequency( &freq ) );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_FREQ_64MHZ, freq );

    SystemCoreClock = 0u;
    TEST_ASSERT_EQUAL( DWT_REQUEST_ERROR, Dwt_Get_CycleFrequency( &freq ) );
}


/**
 * \brief   Dwt_Get_ElapsedTime_us() converts cycles to microseconds.
 *
 * \par Expected results
 * - 250 MHz: 250 cycles = 1 us, 12500 cycles = 50 us, 249 cycles = 0 us.
 * - 12.5 MHz clock: 25 cycles = 2 us.
 * - Zero cycles = 0 us.
 */
void Ut_Dwt_Get_ElapsedTime_ConvertsCyclesToTime( void )
{
    dwt_Time_us_t elapsed = UT_DWT_SENTINEL;

    UtDwt_Start_CycleCounter();

    DWT->CYCCNT = UT_DWT_START_CYCLE;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_ElapsedTime_us( UT_DWT_START_CYCLE, &elapsed ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, elapsed );

    DWT->CYCCNT = UT_DWT_CYCLES_1US_END;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_ElapsedTime_us( UT_DWT_START_CYCLE, &elapsed ) );
    TEST_ASSERT_EQUAL_UINT32( 1u, elapsed );

    DWT->CYCCNT = UT_DWT_CYCLES_BELOW_1US_END;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_ElapsedTime_us( UT_DWT_START_CYCLE, &elapsed ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, elapsed );

    DWT->CYCCNT = UT_DWT_START_CYCLE + UT_DWT_CYCLES_50US;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_ElapsedTime_us( UT_DWT_START_CYCLE, &elapsed ) );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_TIME_50US, elapsed );

    SystemCoreClock = UT_DWT_FREQ_12M5HZ;
    DWT->CYCCNT = UT_DWT_CYCLES_12M5HZ_2US;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_ElapsedTime_us( 0u, &elapsed ) );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_TIME_2US, elapsed );
}


/**
 * \brief   Dwt_Get_ElapsedTime_us() handles wrap-around of the counter.
 *
 * \par Expected results
 * - Start 0xFFFFFF00, now 0x000000FF (wrapped): 511 cycles = 2 us at 250 MHz.
 * - Interval of 2^32 - 1 cycles: 17179869 us at 250 MHz.
 * - 2^32 - 1 cycles at 1 kHz would exceed 32 bits of microseconds, the result is limited.
 */
void Ut_Dwt_Get_ElapsedTime_HandlesWrapAround( void )
{
    dwt_Time_us_t elapsed = 0u;

    UtDwt_Start_CycleCounter();

    DWT->CYCCNT = UT_DWT_WRAP_NOW;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_ElapsedTime_us( UT_DWT_WRAP_START, &elapsed ) );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_TIME_2US, elapsed );

    DWT->CYCCNT = UT_DWT_ALL_ONES;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_ElapsedTime_us( 0u, &elapsed ) );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_TIME_FULL_US, elapsed );

    SystemCoreClock = UT_DWT_FREQ_1KHZ;
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_ElapsedTime_us( 0u, &elapsed ) );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_ALL_ONES, elapsed );
}


/**
 * \brief   Dwt_Get_ElapsedTime_us() reports errors and has no side effects.
 *
 * \par Expected results
 * - Counter not running: DWT_REQUEST_ERROR, output unchanged.
 * - NULL pointer: DWT_REQUEST_ERROR.
 * - The counter and CTRL are not changed by the call.
 */
void Ut_Dwt_Get_ElapsedTime_Errors( void )
{
    dwt_Time_us_t elapsed = UT_DWT_SENTINEL;

    TEST_ASSERT_EQUAL( DWT_REQUEST_ERROR, Dwt_Get_ElapsedTime_us( 0u, &elapsed ) );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_SENTINEL, elapsed );

    UtDwt_Start_CycleCounter();
    DWT->CYCCNT = UT_DWT_CYCCNT_PRESET;

    TEST_ASSERT_EQUAL( DWT_REQUEST_ERROR, Dwt_Get_ElapsedTime_us( 0u, NULL ) );

    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_ElapsedTime_us( 0u, &elapsed ) );
    TEST_ASSERT_EQUAL_UINT32( UT_DWT_CYCCNT_PRESET, DWT->CYCCNT );
    TEST_ASSERT_EQUAL_HEX32( DWT_CTRL_CYCCNTENA_Msk, DWT->CTRL );
}

/* ============================ HELPER FUNCTIONS ============================ */

/** Returns configuration with all counters in required state */
static dwt_Config_t UtDwt_Get_Config( dwt_FunctionState_t state )
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
