/**
 * \author Mr.Nobody
 * \file ItTest_Scb.c
 * \ingroup Scb
 * \brief Integration tests of System Control Block (SCB) module on target.
 *
 * Scb module runs on the MCU (Cortex-M33 core). Tests verify that the SCR
 * bits read back correctly after being set/cleared on real hardware (not
 * emulated by unit tests).
 *
 * \ref Scb_Set_WaitForInterrupt and \ref Scb_Set_WaitForEvent are not called
 * here: entering an actual low power / wait state on target without a
 * wake-up source wired up could hang the board indefinitely during an
 * unattended test run (no board access to recover it). They wrap a single
 * CMSIS intrinsic (`__WFI()` / `__WFE()`) with no other logic, so they are
 * covered by the unit tests (CmsisHost instruction counters) and by code
 * review / compile here (this file links and builds against them).
 *
 * No peripheral and no external wiring is needed.
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "IntegrationTesting.h"             /* Integration testing on target  */
#include "Scb_Port.h"                       /* Module under test              */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================== LOCAL VARIABLES =============================== */

/* ============================= TEST SETUP ================================= */

void setUp( void )
{
    /* Restore both bits to their reset (inactive) state before every test */
    Scb_Set_DeepSleepInactive();
    Scb_Set_SleepOnExitInactive();
}


void tearDown( void )
{
    /* Every test case runs after system reset - extra safety margin: leave no
       low power selection active in SCR even if a later test case is added
       that does not restore it itself. */
    Scb_Set_DeepSleepInactive();
    Scb_Set_SleepOnExitInactive();
}

/* =============================== TESTS ==================================== */

/**
 * \brief   Activating deep sleep reads back as active on real hardware.
 *
 * \par Expected results
 * - Scb_Get_DeepSleepState() returns SCB_FUNCTION_ACTIVE after the set.
 */
void It_Scb_Set_DeepSleepActive_StateIsActive( void )
{
    Scb_Set_DeepSleepActive();

    TEST_ASSERT_EQUAL( SCB_FUNCTION_ACTIVE, Scb_Get_DeepSleepState() );
}


/**
 * \brief   Deactivating deep sleep reads back as inactive on real hardware.
 *
 * \par Expected results
 * - Scb_Get_DeepSleepState() returns SCB_FUNCTION_INACTIVE after the clear.
 */
void It_Scb_Set_DeepSleepInactive_StateIsInactive( void )
{
    Scb_Set_DeepSleepActive();
    Scb_Set_DeepSleepInactive();

    TEST_ASSERT_EQUAL( SCB_FUNCTION_INACTIVE, Scb_Get_DeepSleepState() );
}


/**
 * \brief   Activating sleep-on-exit reads back as active on real hardware.
 *
 * \par Expected results
 * - Scb_Get_SleepOnExitState() returns SCB_FUNCTION_ACTIVE after the set.
 */
void It_Scb_Set_SleepOnExitActive_StateIsActive( void )
{
    Scb_Set_SleepOnExitActive();

    TEST_ASSERT_EQUAL( SCB_FUNCTION_ACTIVE, Scb_Get_SleepOnExitState() );
}


/**
 * \brief   Deactivating sleep-on-exit reads back as inactive on real hardware.
 *
 * \par Expected results
 * - Scb_Get_SleepOnExitState() returns SCB_FUNCTION_INACTIVE after the clear.
 */
void It_Scb_Set_SleepOnExitInactive_StateIsInactive( void )
{
    Scb_Set_SleepOnExitActive();
    Scb_Set_SleepOnExitInactive();

    TEST_ASSERT_EQUAL( SCB_FUNCTION_INACTIVE, Scb_Get_SleepOnExitState() );
}


/**
 * \brief   Deep sleep and sleep-on-exit are independent bits on real hardware.
 *
 * \details Activates deep sleep only and checks sleep-on-exit stays inactive,
 *          then the opposite.
 *
 * \par Expected results
 * - Each bit reads back active only for the one that was set.
 */
void It_Scb_Set_DeepSleepAndSleepOnExit_AreIndependent( void )
{
    Scb_Set_DeepSleepActive();

    TEST_ASSERT_EQUAL( SCB_FUNCTION_ACTIVE,   Scb_Get_DeepSleepState() );
    TEST_ASSERT_EQUAL( SCB_FUNCTION_INACTIVE, Scb_Get_SleepOnExitState() );

    Scb_Set_DeepSleepInactive();
    Scb_Set_SleepOnExitActive();

    TEST_ASSERT_EQUAL( SCB_FUNCTION_INACTIVE, Scb_Get_DeepSleepState() );
    TEST_ASSERT_EQUAL( SCB_FUNCTION_ACTIVE,   Scb_Get_SleepOnExitState() );
}

/* ============================ HELPER FUNCTIONS ============================ */
