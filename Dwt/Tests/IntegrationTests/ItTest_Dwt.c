/**
 * \author Mr.Nobody
 * \file ItTest_Dwt.c
 * \ingroup Dwt
 * \brief Integration tests of Data Watchpoint and Trace Unit (DWT) module on target.
 *
 * Dwt module runs on the MCU (Cortex-M7 core). Tests verify behavior which
 * cannot be verified by unit tests (emulated registers): counting of the cycle
 * counter (Cortex-M7 DWT is unlocked by DWT_LAR - locked DWT does not count),
 * reset of the running counter and stop of the counter after deinitialization.
 *
 * No peripheral and no external wiring is needed.
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "IntegrationTesting.h"             /* Integration testing on target  */
#include "Dwt_Port.h"                       /* Module under test              */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

static void                 It_Dwt_Wait         ( void );
static dwt_Config_t         It_Dwt_Get_Config   ( dwt_FunctionState_t cycleCounterState );

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Count of wait loop iterations */
#define IT_DWT_WAIT_LOOPS                   ( 1000u )

/** Minimal count of cycles of the wait loop (at least one cycle per iteration) */
#define IT_DWT_WAIT_CYCLES_MIN              ( IT_DWT_WAIT_LOOPS )

/* ============================== MACROS ==================================== */

/* ========================== LOCAL VARIABLES =============================== */

/** Wait loop counter (volatile - loop is not optimized out) */
static volatile uint32_t    itDwt_WaitCnt;

/* ============================= TEST SETUP ================================= */

void setUp( void )
{
    itDwt_WaitCnt = 0u;
}


void tearDown( void )
{
    /* Every test case runs after system reset */
}

/* =============================== TESTS ==================================== */

/**
 * \brief   Enabled cycle counter counts core cycles.
 *
 * \details Initializes the module with cycle counter active and executes wait loop.
 *
 * \par Expected results
 * - DWT_REQUEST_OK.
 * - Counter difference over the wait loop is at least count of loop iterations.
 */
void It_Dwt_Init_CycleCounterActive_Counts( void )
{
    dwt_Config_t config = It_Dwt_Get_Config( DWT_FUNCTION_ACTIVE );

    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );

    dwt_CounterValue_t start = Dwt_Get_CycleCounter();
    It_Dwt_Wait();
    dwt_CounterValue_t end   = Dwt_Get_CycleCounter();

    TEST_ASSERT_GREATER_OR_EQUAL_UINT32( IT_DWT_WAIT_CYCLES_MIN, end - start );
}


/**
 * \brief   Disabled cycle counter does not count.
 *
 * \details Initializes the module with all counters inactive and executes wait loop.
 *
 * \par Expected results
 * - Cycle counter value does not change.
 */
void It_Dwt_Init_CycleCounterInactive_DoesNotCount( void )
{
    dwt_Config_t config = It_Dwt_Get_Config( DWT_FUNCTION_INACTIVE );

    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );

    dwt_CounterValue_t start = Dwt_Get_CycleCounter();
    It_Dwt_Wait();

    TEST_ASSERT_EQUAL_UINT32( start, Dwt_Get_CycleCounter() );
}


/**
 * \brief   Reset of running cycle counter restarts counting from zero.
 *
 * \details Counts wait loop, resets the counter and reads it immediately.
 *
 * \par Expected results
 * - Counter value after reset is lower than count of the wait loop cycles.
 */
void It_Dwt_Reset_CycleCounter_RestartsFromZero( void )
{
    dwt_Config_t config = It_Dwt_Get_Config( DWT_FUNCTION_ACTIVE );

    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );

    It_Dwt_Wait();
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32( IT_DWT_WAIT_CYCLES_MIN, Dwt_Get_CycleCounter() );

    Dwt_Reset_CycleCounter();

    TEST_ASSERT_LESS_THAN_UINT32( IT_DWT_WAIT_CYCLES_MIN, Dwt_Get_CycleCounter() );
}


/**
 * \brief   Deinitialization stops the cycle counter.
 *
 * \details Initializes counting module, deinitializes it and executes wait loop.
 *
 * \par Expected results
 * - Cycle counter is reset to 0 and does not count.
 */
void It_Dwt_Deinit_StopsCycleCounter( void )
{
    dwt_Config_t config = It_Dwt_Get_Config( DWT_FUNCTION_ACTIVE );

    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );
    It_Dwt_Wait();

    Dwt_Deinit();
    It_Dwt_Wait();

    TEST_ASSERT_EQUAL_UINT32( 0u, Dwt_Get_CycleCounter() );
}


/**
 * \brief   Initialization with NULL configuration fails.
 *
 * \par Expected results
 * - DWT_REQUEST_ERROR.
 */
void It_Dwt_Init_NullConfig_ReturnsError( void )
{
    TEST_ASSERT_EQUAL( DWT_REQUEST_ERROR, Dwt_Init( DWT_NULL_PTR ) );
}

/* ============================ HELPER FUNCTIONS ============================ */

/** Busy wait loop of \ref IT_DWT_WAIT_LOOPS iterations */
static void It_Dwt_Wait( void )
{
    for( itDwt_WaitCnt = 0u; IT_DWT_WAIT_LOOPS > itDwt_WaitCnt; itDwt_WaitCnt++ )
    {
        /* Wait */
    }
}


/** Returns configuration with required cycle counter state, other counters inactive */
static dwt_Config_t It_Dwt_Get_Config( dwt_FunctionState_t cycleCounterState )
{
    dwt_Config_t config;

    config.CycleCounterState     = cycleCounterState;
    config.CpiCounterState       = DWT_FUNCTION_INACTIVE;
    config.ExceptionCounterState = DWT_FUNCTION_INACTIVE;
    config.SleepCounterState     = DWT_FUNCTION_INACTIVE;
    config.LoadStoreCounterState = DWT_FUNCTION_INACTIVE;
    config.FoldCounterState      = DWT_FUNCTION_INACTIVE;

    return ( config );
}
