/**
 * \author Mr.Nobody
 * \file Scb.c
 * \ingroup Scb
 * \brief System Control Block (SCB) module implementation file.
 *
 * # System Control Register (SCB_SCR)
 * The Cortex-M33 SCB->SCR register controls the sleep behavior entered by the
 * core intrinsics `__WFI()` (Wait For Interrupt) and `__WFE()` (Wait For
 * Event):
 *
 * - SLEEPDEEP: selects deep sleep (Stop / Standby) instead of normal sleep
 *   when the core enters a low power state.
 * - SLEEPONEXIT: enters sleep immediately on return from an interrupt
 *   handler to Thread mode, instead of returning to the interrupted code.
 *
 * This module provides read/modify/read-back access to these two bits and
 * wraps the two entry intrinsics. It owns no other state and does not need
 * initialization - SCB is always clocked and accessible.
 */
/* ============================== INCLUDES ================================== */
#include "Scb.h"                            /* Self include                   */
#include "Scb_Port.h"                       /* Own port file include          */
#include "Scb_Types.h"                      /* Module types definitions       */
#include "Stm32.h"                          /* STM32 platform definitions     */
/* ============================== TYPEDEFS =================================== */

/* ======================== FORWARD DECLARATIONS ============================ */

/* ========================== SYMBOLIC CONSTANTS ============================ */

/** Value of major version of SW module */
#define SCB_MAJOR_VERSION           ( 1u )

/** Value of minor version of SW module */
#define SCB_MINOR_VERSION           ( 0u )

/** Value of patch version of SW module */
#define SCB_PATCH_VERSION           ( 0u )

/* =============================== MACROS =================================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* =========================== LOCAL VARIABLES ============================== */

/* ========================= EXPORTED FUNCTIONS ============================= */

/**
 * \brief Returns module SW version
 *
 * \return Module SW version
 */
scb_ModuleVersion_t Scb_Get_ModuleVersion( void )
{
    scb_ModuleVersion_t retVersion;

    retVersion.Major = SCB_MAJOR_VERSION;
    retVersion.Minor = SCB_MINOR_VERSION;
    retVersion.Patch = SCB_PATCH_VERSION;

    return (retVersion);
}


/**
 * \brief Selects deep sleep for the next low power entry.
 *
 * Sets SCB_SCR.SLEEPDEEP. Takes effect on the following `__WFI()` / `__WFE()`
 * (or sleep-on-exit), selecting deep sleep (Stop / Standby) instead of normal
 * sleep.
 */
void Scb_Set_DeepSleepActive( void )
{
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
}


/**
 * \brief Selects normal sleep for the next low power entry.
 *
 * Clears SCB_SCR.SLEEPDEEP.
 */
void Scb_Set_DeepSleepInactive( void )
{
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
}


/**
 * \brief Returns actual state of the deep sleep selection.
 *
 * Read-only, no side effects.
 *
 * \return \ref SCB_FUNCTION_ACTIVE if SCB_SCR.SLEEPDEEP is set, otherwise
 *         \ref SCB_FUNCTION_INACTIVE
 */
scb_FunctionState_t Scb_Get_DeepSleepState( void )
{
    scb_FunctionState_t retVal = SCB_FUNCTION_INACTIVE;

    if( 0u != ( SCB->SCR & SCB_SCR_SLEEPDEEP_Msk ) )
    {
        retVal = SCB_FUNCTION_ACTIVE;
    }
    else
    {
        retVal = SCB_FUNCTION_INACTIVE;
    }

    return ( retVal );
}


/**
 * \brief Enables sleep-on-exit.
 *
 * Sets SCB_SCR.SLEEPONEXIT - the core enters the selected sleep immediately on
 * return from an interrupt handler to Thread mode.
 */
void Scb_Set_SleepOnExitActive( void )
{
    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
}


/**
 * \brief Disables sleep-on-exit.
 *
 * Clears SCB_SCR.SLEEPONEXIT - the core returns to the interrupted code after
 * an interrupt handler.
 */
void Scb_Set_SleepOnExitInactive( void )
{
    SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;
}


/**
 * \brief Returns actual state of sleep-on-exit.
 *
 * Read-only, no side effects.
 *
 * \return \ref SCB_FUNCTION_ACTIVE if SCB_SCR.SLEEPONEXIT is set, otherwise
 *         \ref SCB_FUNCTION_INACTIVE
 */
scb_FunctionState_t Scb_Get_SleepOnExitState( void )
{
    scb_FunctionState_t retVal = SCB_FUNCTION_INACTIVE;

    if( 0u != ( SCB->SCR & SCB_SCR_SLEEPONEXIT_Msk ) )
    {
        retVal = SCB_FUNCTION_ACTIVE;
    }
    else
    {
        retVal = SCB_FUNCTION_INACTIVE;
    }

    return ( retVal );
}


/**
 * \brief Enters low power state and waits for an interrupt.
 *
 * Executes the `__WFI()` core intrinsic. Low power state entered (normal or
 * deep sleep) depends on SCB_SCR.SLEEPDEEP, set by \ref Scb_Set_DeepSleepActive
 * / \ref Scb_Set_DeepSleepInactive. Returns when an interrupt wakes the core.
 */
void Scb_Set_WaitForInterrupt( void )
{
    __WFI();
}


/**
 * \brief Enters low power state and waits for an event.
 *
 * Executes the `__WFE()` core intrinsic. Low power state entered (normal or
 * deep sleep) depends on SCB_SCR.SLEEPDEEP, set by \ref Scb_Set_DeepSleepActive
 * / \ref Scb_Set_DeepSleepInactive. Returns when an event wakes the core.
 */
void Scb_Set_WaitForEvent( void )
{
    __WFE();
}

/* =========================== LOCAL FUNCTIONS ============================== */

/* =========================== INTERRUPT HANDLERS =========================== */

/* ================================ TASKS =================================== */
