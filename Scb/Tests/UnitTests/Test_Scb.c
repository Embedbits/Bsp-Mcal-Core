/**
 * \author Mr.Nobody
 * \file Test_Scb.c
 * \ingroup Scb
 * \brief Unit tests of System Control Block (SCB) module.
 *
 * Scb.c is compiled unchanged with real CMSIS headers. SCB registers (private
 * peripheral bus) are emulated by RegMem. Core intrinsics `__WFI()` / `__WFE()`
 * are emulated by CmsisHost (no actual low power entry on host) and counted,
 * so tests can check they were executed without blocking the test process.
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "RegMem.h"                         /* Register memory emulation      */
#include "CmsisHost.h"                      /* Host emulation of intrinsics   */
#include "Scb_Port.h"                       /* Module under test              */
#include "Stm32.h"                          /* SCB registers                  */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Preset value of unrelated SCR bits (SEVONPEND), must be preserved by the module */
#define UT_SCB_SCR_OTHER_BITS               ( SCB_SCR_SEVONPEND_Msk )

/** Preset value of SCR with all bits under test already set */
#define UT_SCB_SCR_PRESET_ALL               ( SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk | \
                                               UT_SCB_SCR_OTHER_BITS )

/* ========================== LOCAL VARIABLES =============================== */

/* ============================ TEST FIXTURE ================================ */

void setUp( void )
{
    TEST_ASSERT_EQUAL( REGMEM_REQUEST_OK, RegMem_Reset() );
    CmsisHost_Reset();
}


void tearDown( void )
{
}

/* ========================== MODULE VERSION ================================ */

/**
 * \brief   Scb_Get_ModuleVersion() returns version of the module.
 *
 * \par Expected results
 * - Version is 1.0.0.
 */
void Ut_Scb_Get_ModuleVersion_ReturnsVersion( void )
{
    scb_ModuleVersion_t version = Scb_Get_ModuleVersion();

    TEST_ASSERT_EQUAL_UINT8( 1u, version.Major );
    TEST_ASSERT_EQUAL_UINT8( 0u, version.Minor );
    TEST_ASSERT_EQUAL_UINT8( 0u, version.Patch );
}

/* =============================== DEEP SLEEP ================================ */

/**
 * \brief   Scb_Set_DeepSleepActive() sets SLEEPDEEP without touching other bits.
 *
 * \par Expected results
 * - SLEEPDEEP bit set, other SCR bits (SLEEPONEXIT, SEVONPEND) unchanged.
 */
void Ut_Scb_Set_DeepSleepActive_SetsOnlySleepDeep( void )
{
    SCB->SCR = UT_SCB_SCR_OTHER_BITS;

    Scb_Set_DeepSleepActive();

    TEST_ASSERT_EQUAL_HEX32( SCB_SCR_SLEEPDEEP_Msk | UT_SCB_SCR_OTHER_BITS, SCB->SCR );
}


/**
 * \brief   Scb_Set_DeepSleepInactive() clears SLEEPDEEP without touching other bits.
 *
 * \par Expected results
 * - SLEEPDEEP bit cleared, other SCR bits (SLEEPONEXIT, SEVONPEND) unchanged.
 */
void Ut_Scb_Set_DeepSleepInactive_ClearsOnlySleepDeep( void )
{
    SCB->SCR = UT_SCB_SCR_PRESET_ALL;

    Scb_Set_DeepSleepInactive();

    TEST_ASSERT_EQUAL_HEX32( SCB_SCR_SLEEPONEXIT_Msk | UT_SCB_SCR_OTHER_BITS, SCB->SCR );
}


/**
 * \brief   Scb_Get_DeepSleepState() reads back SLEEPDEEP without side effects.
 *
 * \par Expected results
 * - SCB_FUNCTION_ACTIVE when the bit is set, SCB_FUNCTION_INACTIVE when clear.
 * - SCR is not modified by the getter.
 */
void Ut_Scb_Get_DeepSleepState_ReadsBackBit( void )
{
    SCB->SCR = SCB_SCR_SLEEPDEEP_Msk;
    TEST_ASSERT_EQUAL( SCB_FUNCTION_ACTIVE, Scb_Get_DeepSleepState() );
    TEST_ASSERT_EQUAL_HEX32( SCB_SCR_SLEEPDEEP_Msk, SCB->SCR );

    SCB->SCR = UT_SCB_SCR_OTHER_BITS;
    TEST_ASSERT_EQUAL( SCB_FUNCTION_INACTIVE, Scb_Get_DeepSleepState() );
    TEST_ASSERT_EQUAL_HEX32( UT_SCB_SCR_OTHER_BITS, SCB->SCR );
}

/* ============================== SLEEP ON EXIT =============================== */

/**
 * \brief   Scb_Set_SleepOnExitActive() sets SLEEPONEXIT without touching other bits.
 *
 * \par Expected results
 * - SLEEPONEXIT bit set, other SCR bits (SLEEPDEEP, SEVONPEND) unchanged.
 */
void Ut_Scb_Set_SleepOnExitActive_SetsOnlySleepOnExit( void )
{
    SCB->SCR = UT_SCB_SCR_OTHER_BITS;

    Scb_Set_SleepOnExitActive();

    TEST_ASSERT_EQUAL_HEX32( SCB_SCR_SLEEPONEXIT_Msk | UT_SCB_SCR_OTHER_BITS, SCB->SCR );
}


/**
 * \brief   Scb_Set_SleepOnExitInactive() clears SLEEPONEXIT without touching other bits.
 *
 * \par Expected results
 * - SLEEPONEXIT bit cleared, other SCR bits (SLEEPDEEP, SEVONPEND) unchanged.
 */
void Ut_Scb_Set_SleepOnExitInactive_ClearsOnlySleepOnExit( void )
{
    SCB->SCR = UT_SCB_SCR_PRESET_ALL;

    Scb_Set_SleepOnExitInactive();

    TEST_ASSERT_EQUAL_HEX32( SCB_SCR_SLEEPDEEP_Msk | UT_SCB_SCR_OTHER_BITS, SCB->SCR );
}


/**
 * \brief   Scb_Get_SleepOnExitState() reads back SLEEPONEXIT without side effects.
 *
 * \par Expected results
 * - SCB_FUNCTION_ACTIVE when the bit is set, SCB_FUNCTION_INACTIVE when clear.
 * - SCR is not modified by the getter.
 */
void Ut_Scb_Get_SleepOnExitState_ReadsBackBit( void )
{
    SCB->SCR = SCB_SCR_SLEEPONEXIT_Msk;
    TEST_ASSERT_EQUAL( SCB_FUNCTION_ACTIVE, Scb_Get_SleepOnExitState() );
    TEST_ASSERT_EQUAL_HEX32( SCB_SCR_SLEEPONEXIT_Msk, SCB->SCR );

    SCB->SCR = UT_SCB_SCR_OTHER_BITS;
    TEST_ASSERT_EQUAL( SCB_FUNCTION_INACTIVE, Scb_Get_SleepOnExitState() );
    TEST_ASSERT_EQUAL_HEX32( UT_SCB_SCR_OTHER_BITS, SCB->SCR );
}

/* ============================== INDEPENDENCE =============================== */

/**
 * \brief   Deep sleep and sleep-on-exit setters/getters do not affect each other.
 *
 * \details Sets each bit alone and checks the other stays at its previous state.
 *
 * \par Expected results
 * - Setting SLEEPDEEP leaves SLEEPONEXIT state unchanged and vice versa.
 */
void Ut_Scb_Set_DeepSleepAndSleepOnExit_AreIndependent( void )
{
    SCB->SCR = 0u;

    Scb_Set_DeepSleepActive();
    TEST_ASSERT_EQUAL( SCB_FUNCTION_INACTIVE, Scb_Get_SleepOnExitState() );

    Scb_Set_SleepOnExitActive();
    TEST_ASSERT_EQUAL( SCB_FUNCTION_ACTIVE, Scb_Get_DeepSleepState() );

    Scb_Set_DeepSleepInactive();
    TEST_ASSERT_EQUAL( SCB_FUNCTION_ACTIVE, Scb_Get_SleepOnExitState() );
    TEST_ASSERT_EQUAL( SCB_FUNCTION_INACTIVE, Scb_Get_DeepSleepState() );

    Scb_Set_SleepOnExitInactive();
    TEST_ASSERT_EQUAL( SCB_FUNCTION_INACTIVE, Scb_Get_SleepOnExitState() );
}

/* ======================== WAIT FOR INTERRUPT / EVENT ========================= */

/**
 * \brief   Scb_Set_WaitForInterrupt() executes the WFI core intrinsic exactly once.
 *
 * \par Expected results
 * - CmsisHost WFI instruction counter increments by one, WFE counter unchanged.
 */
void Ut_Scb_Set_WaitForInterrupt_ExecutesWfiOnce( void )
{
    uint32_t wfiBefore = CmsisHost_Get_InstrCnt( CMSISHOST_INSTR_WFI );
    uint32_t wfeBefore = CmsisHost_Get_InstrCnt( CMSISHOST_INSTR_WFE );

    Scb_Set_WaitForInterrupt();

    TEST_ASSERT_EQUAL_UINT32( wfiBefore + 1u, CmsisHost_Get_InstrCnt( CMSISHOST_INSTR_WFI ) );
    TEST_ASSERT_EQUAL_UINT32( wfeBefore, CmsisHost_Get_InstrCnt( CMSISHOST_INSTR_WFE ) );
}


/**
 * \brief   Scb_Set_WaitForEvent() executes the WFE core intrinsic exactly once.
 *
 * \par Expected results
 * - CmsisHost WFE instruction counter increments by one, WFI counter unchanged.
 */
void Ut_Scb_Set_WaitForEvent_ExecutesWfeOnce( void )
{
    uint32_t wfiBefore = CmsisHost_Get_InstrCnt( CMSISHOST_INSTR_WFI );
    uint32_t wfeBefore = CmsisHost_Get_InstrCnt( CMSISHOST_INSTR_WFE );

    Scb_Set_WaitForEvent();

    TEST_ASSERT_EQUAL_UINT32( wfeBefore + 1u, CmsisHost_Get_InstrCnt( CMSISHOST_INSTR_WFE ) );
    TEST_ASSERT_EQUAL_UINT32( wfiBefore, CmsisHost_Get_InstrCnt( CMSISHOST_INSTR_WFI ) );
}


/**
 * \brief   Scb_Set_WaitForInterrupt() does not modify SCR (no implicit configuration).
 *
 * \par Expected results
 * - SCR keeps the preset value after the call.
 */
void Ut_Scb_Set_WaitForInterrupt_DoesNotModifyScr( void )
{
    SCB->SCR = UT_SCB_SCR_PRESET_ALL;

    Scb_Set_WaitForInterrupt();

    TEST_ASSERT_EQUAL_HEX32( UT_SCB_SCR_PRESET_ALL, SCB->SCR );
}

/* ============================ HELPER FUNCTIONS ============================ */
