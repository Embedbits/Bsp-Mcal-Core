/**
 * \author Mr.Nobody
 * \file Dwt.c
 * \ingroup Dwt
 * \brief Data Watchpoint and Trace Unit (DWT) module implementation file.
 *
 * # Cycle counter and profiling counters
 * The Cortex®-M7 DWT supports a cycle counter and profiling counters.
 *
 * # Cycle counter
 * When enabled in DWT_CTRL, the 32-bit cycle counter, DWT_CYCCNT, increments
 * each cycle unless the processor is in debug halt state. When the cycle
 * counter is disabled, all functionality associated with the cycle counter is
 * also disabled.
 *
 * # Profiling counters
 * The profiling counters can be configured to generate events on overflow using
 * DWT_CTRL fields.
 *
 * ## CPI Counter (DWT_CPICNT)
 * The 8-bit CPI counter is incremented for every additional cycle, that is,
 * greater than one taken to execute a non-load or store instruction. This
 * counter must also be incremented for every cycle where fetch is stalled.
 *
 * ## Exception Overhead Counter (DWT_EXCCNT)
 * The 8-bit Exception Overhead Counter is incremented for every cycle
 * associated with exception entry and return. This includes stacking,
 * unstacking, preemption and tail-chaining. Register stacking associated with
 * floating-point lazy context saving is also included in this counter.
 *
 * ## Sleep Overhead Counter (DWT_SLEEPCNT)
 * The 8-bit Sleep Overhead Counter is incremented for every cycle associated
 * for power saving. For example, WFI and WFE exceptions.
 *
 * ## Load-Store Counter (DWT_LSUCNT)
 * The 8-bit Load-Store Counter is incremented for every additional cycle that
 * is greater than one taken to execute a load-store instruction.
 *
 * ## Fold Counter (DWT_FOLDCNT)
 * The 8-bit Fold Counter counts folded instructions and increments for every
 * instruction executed in zero cycles.
 *
 * # Trace enable (DEMCR.TRCENA)
 * DWT and ITM share the global trace enable bit. Deinitialization of DWT keeps
 * the bit set while ITM is enabled, so ITM keeps working.
 *
 * # Software lock (Cortex-M7)
 * DWT of Cortex-M7 has the CoreSight software lock - without the unlock key in
 * DWT_LAR the counters can not be written (and the cycle counter does not run
 * without a debugger which unlocks the lock itself).
 *
 */
/* ============================== INCLUDES ================================== */
#include "Dwt.h"                            /* Self include                   */
#include "Dwt_Port.h"                       /* Own port file include          */
#include "Dwt_Types.h"                      /* Module types definitions       */
#include "Stm32.h"                          /* STM32 platform definitions     */
/* ============================== TYPEDEFS ================================== */

/* ======================== FORWARD DECLARATIONS ============================ */

/* ========================== SYMBOLIC CONSTANTS ============================ */

/** Value of major version of SW module */
#define DWT_MAJOR_VERSION           ( 1u )

/** Value of minor version of SW module */
#define DWT_MINOR_VERSION           ( 0u )

/** Value of patch version of SW module */
#define DWT_PATCH_VERSION           ( 0u )

/** Reset value of counters */
#define DWT_COUNTER_RESET           ( 0u )

/** CoreSight software lock key unlocking write access to DWT registers */
#define DWT_LAR_UNLOCK_KEY          ( 0xC5ACCE55u )

/* =============================== MACROS =================================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* =========================== LOCAL VARIABLES ============================== */

/* ========================= EXPORTED FUNCTIONS ============================= */

/**
 * \brief Returns module SW version
 *
 * \return Module SW version
 */
dwt_ModuleVersion_t Dwt_Get_ModuleVersion( void )
{
    dwt_ModuleVersion_t retVersion;

    retVersion.Major = DWT_MAJOR_VERSION;
    retVersion.Minor = DWT_MINOR_VERSION;
    retVersion.Patch = DWT_PATCH_VERSION;

    return (retVersion);
}


/**
 * \brief Initializes module Dwt
 *
 * Enables trace unit, unlocks the DWT software lock, resets all counters and
 * enables counters required by configuration.
 *
 * \param config [in]: Module configuration
 *
 * \return \ref DWT_REQUEST_OK if module was initialized, otherwise
 *         \ref DWT_REQUEST_ERROR
 */
dwt_RequestState_t Dwt_Init( dwt_Config_t * const config )
{
    dwt_RequestState_t retVal = DWT_REQUEST_ERROR;

    if( DWT_NULL_PTR != config )
    {
        /* Enable trace unit */
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

        /* Unlock write access to DWT registers (CoreSight software lock) */
        DWT->LAR = DWT_LAR_UNLOCK_KEY;

        /* Reset all counters */
        DWT->CYCCNT   = DWT_COUNTER_RESET;
        DWT->CPICNT   = DWT_COUNTER_RESET;
        DWT->EXCCNT   = DWT_COUNTER_RESET;
        DWT->SLEEPCNT = DWT_COUNTER_RESET;
        DWT->LSUCNT   = DWT_COUNTER_RESET;
        DWT->FOLDCNT  = DWT_COUNTER_RESET;

        uint32_t regVal = 0u;

        if( DWT_FUNCTION_ACTIVE == config->CycleCounterState )
        {
            regVal |= DWT_CTRL_CYCCNTENA_Msk;
        }
        else
        {
            /* No action required */
        }

        if( DWT_FUNCTION_ACTIVE == config->CpiCounterState )
        {
            regVal |= DWT_CTRL_CPIEVTENA_Msk;
        }
        else
        {
            /* No action required */
        }

        if( DWT_FUNCTION_ACTIVE == config->ExceptionCounterState )
        {
            regVal |= DWT_CTRL_EXCEVTENA_Msk;
        }
        else
        {
            /* No action required */
        }

        if( DWT_FUNCTION_ACTIVE == config->SleepCounterState )
        {
            regVal |= DWT_CTRL_SLEEPEVTENA_Msk;
        }
        else
        {
            /* No action required */
        }

        if( DWT_FUNCTION_ACTIVE == config->LoadStoreCounterState )
        {
            regVal |= DWT_CTRL_LSUEVTENA_Msk;
        }
        else
        {
            /* No action required */
        }

        if( DWT_FUNCTION_ACTIVE == config->FoldCounterState )
        {
            regVal |= DWT_CTRL_FOLDEVTENA_Msk;
        }
        else
        {
            /* No action required */
        }

        DWT->CTRL = regVal;

        retVal = DWT_REQUEST_OK;
    }
    else
    {
        retVal = DWT_REQUEST_ERROR;
    }

    return ( retVal );
}


/**
 * \brief Deinitializes module Dwt
 *
 * Disables all counters and resets cycle counter. Trace unit is disabled only
 * if ITM is not enabled (shared DEMCR.TRCENA bit).
 */
void Dwt_Deinit( void )
{
    /* Disable all counters */
    DWT->CTRL = 0u;

    /* Reset cycle counter */
    DWT->CYCCNT = DWT_COUNTER_RESET;

    /* Disable trace unit, if not used by ITM */
    if( 0u == ( ITM->TCR & ITM_TCR_ITMENA_Msk ) )
    {
        CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
    }
    else
    {
        /* No action required */
    }
}


/**
 * \brief Main task of module Dwt
 *
 * This function shall be called in the main loop of the application or the task
 * scheduler. It shall be called periodically, depending on the module's
 * requirements.
 */
void Dwt_Task( void )
{
    return;
}


/**
 * \brief Fills default configuration of the module
 *
 * Default configuration enables cycle counter only.
 *
 * \param config [out]: Configuration to be filled
 *
 * \return \ref DWT_REQUEST_OK if configuration was filled, otherwise
 *         \ref DWT_REQUEST_ERROR
 */
dwt_RequestState_t Dwt_Get_DefaultConfig( dwt_Config_t * const config )
{
    dwt_RequestState_t retVal = DWT_REQUEST_ERROR;

    if( DWT_NULL_PTR != config )
    {
        config->CycleCounterState     = DWT_FUNCTION_ACTIVE;
        config->CpiCounterState       = DWT_FUNCTION_INACTIVE;
        config->ExceptionCounterState = DWT_FUNCTION_INACTIVE;
        config->SleepCounterState     = DWT_FUNCTION_INACTIVE;
        config->LoadStoreCounterState = DWT_FUNCTION_INACTIVE;
        config->FoldCounterState      = DWT_FUNCTION_INACTIVE;

        retVal = DWT_REQUEST_OK;
    }
    else
    {
        retVal = DWT_REQUEST_ERROR;
    }

    return ( retVal );
}


/**
 * \brief Resets cycle counter.
 *
 */
void Dwt_Reset_CycleCounter( void )
{
    DWT->CYCCNT = DWT_COUNTER_RESET;
}


/**
 * \brief Returns actual value of cycle counter.
 *
 * \returns Actual value of cycle counter
 */
dwt_CounterValue_t Dwt_Get_CycleCounter( void )
{
    dwt_CounterValue_t retVal = (dwt_CounterValue_t) DWT->CYCCNT;

    return ( retVal );
}


/**
 * \brief Resets Cycles Per Instruction (CPI) counter.
 *
 */
void Dwt_Reset_CpiCounter( void )
{
    DWT->CPICNT = DWT_COUNTER_RESET;
}


/**
 * \brief Returns actual value of Cycles Per Instruction (CPI) counter.
 *
 * \returns Actual value of Cycles Per Instruction (CPI) counter
 */
dwt_CounterValue_t Dwt_Get_CpiCounter( void )
{
    dwt_CounterValue_t retVal = (dwt_CounterValue_t) DWT->CPICNT;

    return ( retVal );
}


/**
 * \brief Resets Exception Overhead counter.
 *
 */
void Dwt_Reset_ExceptionCounter( void )
{
    DWT->EXCCNT = DWT_COUNTER_RESET;
}


/**
 * \brief Returns actual value of Exception Overhead counter.
 *
 * \returns Actual value of Exception Overhead counter
 */
dwt_CounterValue_t Dwt_Get_ExceptionCounter( void )
{
    dwt_CounterValue_t retVal = (dwt_CounterValue_t) DWT->EXCCNT;

    return ( retVal );
}


/**
 * \brief Resets Sleep Overhead counter.
 *
 */
void Dwt_Reset_SleepCounter( void )
{
    DWT->SLEEPCNT = DWT_COUNTER_RESET;
}


/**
 * \brief Returns actual value of Sleep Overhead counter.
 *
 * \returns Actual value of Sleep Overhead counter
 */
dwt_CounterValue_t Dwt_Get_SleepCounter( void )
{
    dwt_CounterValue_t retVal = (dwt_CounterValue_t) DWT->SLEEPCNT;

    return ( retVal );
}


/**
 * \brief Resets Load-Store counter.
 *
 */
void Dwt_Reset_LoadStoreCounter( void )
{
    DWT->LSUCNT = DWT_COUNTER_RESET;
}


/**
 * \brief Returns actual value of Load-Store counter.
 *
 * \returns Actual value of Load-Store counter
 */
dwt_CounterValue_t Dwt_Get_LoadStoreCounter( void )
{
    dwt_CounterValue_t retVal = (dwt_CounterValue_t) DWT->LSUCNT;

    return ( retVal );
}


/**
 * \brief Resets Folded Instruction counter.
 *
 */
void Dwt_Reset_FoldCounter( void )
{
    DWT->FOLDCNT = DWT_COUNTER_RESET;
}


/**
 * \brief Returns actual value of Folded Instruction counter.
 *
 * \returns Actual value of Folded Instruction counter
 */
dwt_CounterValue_t Dwt_Get_FoldCounter( void )
{
    dwt_CounterValue_t retVal = (dwt_CounterValue_t) DWT->FOLDCNT;

    return ( retVal );
}

/* =========================== LOCAL FUNCTIONS ============================== */

/* =========================== INTERRUPT HANDLERS =========================== */

/* ================================ TASKS =================================== */
