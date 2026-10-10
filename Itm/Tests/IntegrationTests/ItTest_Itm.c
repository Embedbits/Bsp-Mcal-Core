/**
 * \author Mr.Nobody
 * \file ItTest_Itm.c
 * \ingroup Itm
 * \brief Integration tests of Instrumentation Trace Macrocell (ITM) module on target.
 *
 * Itm module runs on the MCU together with real RCC module (core clock) and
 * hardware (DBGMCU, TPIU, ITM). Tests verify behavior which cannot be verified
 * by unit tests (emulated registers): stimulus port FIFO is drained by TPIU over
 * SWO pin, so transmission never runs into the timeout and ITM is not busy after
 * the data were sent. Transmission duration is measured by DWT cycle counter.
 *
 * SWO output is not received by the host (data content is not checked), SWO pin
 * (PB3) is connected to the ST-LINK of NUCLEO-144 and 32F746GDISCOVERY boards, no
 * external wiring is needed. Cortex-M7 ITM is unlocked by ITM_LAR.
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "IntegrationTesting.h"             /* Integration testing on target  */
#include "Itm_Port.h"                       /* Module under test              */
#include "Dwt_Port.h"                       /* Cycle counter                  */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

static void                 It_Itm_Init_Default     ( itm_Port_t portId );
static void                 It_Itm_Init_CycleCounter( void );

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Stimulus port used by tests */
#define IT_ITM_PORT                         ( ITM_PORT_0 )

/** Count of 32-bit words transmitted by tests */
#define IT_ITM_WORD_CNT                     ( 16u )

/**
 * Maximal count of core cycles of the transmission of all words. 16 words at
 * 2 MHz SWO take less than 0.5 ms (< 108000 cycles at 216 MHz), one timeout of
 * the module (FIFO not drained) takes more than 1000000 cycles.
 */
#define IT_ITM_TRANSMIT_CYCLES_MAX          ( 1000000u )

/** Maximal count of wait loop iterations for ITM not busy */
#define IT_ITM_BUSY_WAIT_LOOPS              ( 1000000u )

/* ============================== MACROS ==================================== */

/* ========================== LOCAL VARIABLES =============================== */

/* ============================= TEST SETUP ================================= */

void setUp( void )
{
}


void tearDown( void )
{
    /* Every test case runs after system reset */
}

/* =============================== TESTS ==================================== */

/**
 * \brief   Initialized ITM port is ready to transmit.
 *
 * \details Initializes the module with default configuration (port 0, 2 MHz).
 *
 * \par Expected results
 * - ITM_REQUEST_OK.
 * - Port 0 ready (ITM_FLAG_ACTIVE), port 1 not enabled (ITM_FLAG_INACTIVE).
 */
void It_Itm_Init_DefaultConfig_PortReady( void )
{
    It_Itm_Init_Default( IT_ITM_PORT );

    TEST_ASSERT_EQUAL( ITM_FLAG_ACTIVE,   Itm_Get_TransmitState( IT_ITM_PORT ) );
    TEST_ASSERT_EQUAL( ITM_FLAG_INACTIVE, Itm_Get_TransmitState( ITM_PORT_1 ) );
}


/**
 * \brief   Transmission is drained by TPIU without timeout.
 *
 * \details Transmits 16 32-bit words to port 0, measures duration by DWT cycle
 *          counter, then waits for ITM not busy.
 *
 * \par Expected results
 * - Transmission takes less than one timeout of the module.
 * - ITM is not busy after the transmission, port is ready again.
 */
void It_Itm_Transmit_32b_DrainedWithoutTimeout( void )
{
    It_Itm_Init_CycleCounter();
    It_Itm_Init_Default( IT_ITM_PORT );

    dwt_CounterValue_t start = Dwt_Get_CycleCounter();

    for( uint32_t wordIdx = 0u; IT_ITM_WORD_CNT > wordIdx; wordIdx++ )
    {
        Itm_Transmit_32b( IT_ITM_PORT, wordIdx );
    }

    dwt_CounterValue_t end = Dwt_Get_CycleCounter();

    TEST_ASSERT_LESS_THAN_UINT32( IT_ITM_TRANSMIT_CYCLES_MAX, end - start );

    uint32_t        waitLoops = 0u;
    itm_FlagState_t busyState = Itm_Get_BusyState();

    while( ( ITM_FLAG_ACTIVE        == busyState ) &&
           ( IT_ITM_BUSY_WAIT_LOOPS >  waitLoops )    )
    {
        waitLoops++;
        busyState = Itm_Get_BusyState();
    }

    TEST_ASSERT_EQUAL( ITM_FLAG_INACTIVE, Itm_Get_BusyState() );
    TEST_ASSERT_EQUAL( ITM_FLAG_ACTIVE,   Itm_Get_TransmitState( IT_ITM_PORT ) );
}


/**
 * \brief   Transmission to disabled port returns immediately.
 *
 * \details Port 0 initialized, transmits 16 words to disabled port 1.
 *
 * \par Expected results
 * - Transmission takes less than one timeout of the module (no waiting).
 */
void It_Itm_Transmit_DisabledPort_ReturnsImmediately( void )
{
    It_Itm_Init_CycleCounter();
    It_Itm_Init_Default( IT_ITM_PORT );

    dwt_CounterValue_t start = Dwt_Get_CycleCounter();

    for( uint32_t wordIdx = 0u; IT_ITM_WORD_CNT > wordIdx; wordIdx++ )
    {
        Itm_Transmit_32b( ITM_PORT_1, wordIdx );
    }

    TEST_ASSERT_LESS_THAN_UINT32( IT_ITM_TRANSMIT_CYCLES_MAX, Dwt_Get_CycleCounter() - start );
}


/**
 * \brief   Enabled port at runtime is ready to transmit, disabled is not.
 *
 * \par Expected results
 * - Port 1 ready after Itm_PortEnable(), not ready after Itm_PortDisable().
 */
void It_Itm_PortEnable_PortDisable_ChangeTransmitState( void )
{
    It_Itm_Init_Default( IT_ITM_PORT );

    TEST_ASSERT_EQUAL( ITM_REQUEST_OK,  Itm_PortEnable( ITM_PORT_1 ) );
    TEST_ASSERT_EQUAL( ITM_FLAG_ACTIVE, Itm_Get_TransmitState( ITM_PORT_1 ) );

    TEST_ASSERT_EQUAL( ITM_REQUEST_OK,    Itm_PortDisable( ITM_PORT_1 ) );
    TEST_ASSERT_EQUAL( ITM_FLAG_INACTIVE, Itm_Get_TransmitState( ITM_PORT_1 ) );
}


/**
 * \brief   Deinitialized ITM does not transmit.
 *
 * \par Expected results
 * - Port 0 not ready after Itm_Deinit().
 * - DWT cycle counter keeps counting (shared trace enable kept for DWT).
 */
void It_Itm_Deinit_PortNotReady_DwtKeepsCounting( void )
{
    It_Itm_Init_CycleCounter();
    It_Itm_Init_Default( IT_ITM_PORT );

    Itm_Deinit();

    TEST_ASSERT_EQUAL( ITM_FLAG_INACTIVE, Itm_Get_TransmitState( IT_ITM_PORT ) );

    dwt_CounterValue_t start = Dwt_Get_CycleCounter();
    Itm_Transmit_8b( IT_ITM_PORT, 0u );
    TEST_ASSERT_NOT_EQUAL_UINT32( start, Dwt_Get_CycleCounter() );
}

/* ============================ HELPER FUNCTIONS ============================ */

/** Initializes ITM with default configuration on given port */
static void It_Itm_Init_Default( itm_Port_t portId )
{
    itm_Config_t config;

    TEST_ASSERT_EQUAL( ITM_REQUEST_OK, Itm_Get_DefaultConfig( &config ) );

    config.PortId = portId;

    TEST_ASSERT_EQUAL( ITM_REQUEST_OK, Itm_Init( &config ) );
}


/** Initializes DWT with cycle counter active */
static void It_Itm_Init_CycleCounter( void )
{
    dwt_Config_t config;

    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Get_DefaultConfig( &config ) );
    TEST_ASSERT_EQUAL( DWT_REQUEST_OK, Dwt_Init( &config ) );
}
