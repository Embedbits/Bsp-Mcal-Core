/**
 * \author Mr.Nobody
 * \file Dwt.h
 * \ingroup Dwt
 * \brief Data Watchpoint and Trace Unit (DWT) module implementation file.
 *
 * # Cycle counter and profiling counters
 * The Cortex®-M55 DWT supports a cycle counter and profiling counters.
 *
 * # Cycle counter
 * When enabled in DWT_CTRL, the 32-bit cycle counter, DWT_CYCCNT, increments
 * each cycle unless the processor is in debug halt state. When the cycle
 * counter is disabled, all functionality associated with the cycle counter is
 * also disabled.
 *
 * If the processor includes support for the Security Extension then the
 * DWT_CTRL.CYCDISS bit field disables the cycle counter increment when the
 * processor is executing secure code. This can be useful for generating CPI
 * measurements for Non-secure applications.
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
 * unstacking, and preemption and tail-chaining, in cases where additional
 * registers must be stacked due to a change in Security state between
 * exceptions. Register stacking associated with floating-point lazy context
 * saving is also included in this counter.
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
 * instruction executed in zero cycles. All folded instructions are dual-issued.
 * For example, for a dual-issued pair of instructions, the counter increments
 * by one to reflect this.
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
 * This function shall call every necessary sub-module initialization function 
 * and set up all the necessary resources for the module to work. In case of
 * failure, the function shall handle it by itself and shall not be transferred
 * to AppMain layer.
 */
dwt_RequestState_t Dwt_Init( dwt_Config_t * const config )
{
    dwt_RequestState_t retVal = DWT_REQUEST_OK;

    if( DWT_NULL_PTR != config )
    {
        /* Enable trace unit */
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

        /* Reset cycle counter */
        DWT->CYCCNT = 0;

        uint32_t regVal = 0u;

        if( DWT_FUNCTION_ACTIVE == config->CycleCounterState )
        {
            regVal |= DWT_CTRL_CYCCNTENA_Msk;
        }

        if( DWT_FUNCTION_ACTIVE == config->CpiCounterState )
        {
            regVal |= DWT_CTRL_CPIEVTENA_Msk;
        }

        if( DWT_FUNCTION_ACTIVE == config->ExceptionCounterState )
        {
            regVal |= DWT_CTRL_EXCEVTENA_Msk;
        }

        if( DWT_FUNCTION_ACTIVE == config->SleepCounterState )
        {
            regVal |= DWT_CTRL_SLEEPEVTENA_Msk;
        }

        if( DWT_FUNCTION_ACTIVE == config->LoadStoreCounterState )
        {
            regVal |= DWT_CTRL_LSUEVTENA_Msk;
        }

        if( DWT_FUNCTION_ACTIVE == config->FoldCounterState )
        {
            regVal |= DWT_CTRL_FOLDEVTENA_Msk;
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
 * This function shall call every necessary sub-module deinitialization function 
 * and free all the resources allocated by the module. In case of failure, the 
 * function shall handle it by itself and shall not be transferred to AppMain 
 * layer.
 */
void Dwt_Deinit( void )
{
    /* Disable all counters */
    DWT->CTRL = 0;

    /* Disable trace unit */
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;

    /* Reset cycle counter */
    DWT->CYCCNT = 0;
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
 * \brief Resets cycle counter.
 *
 */
void Dwt_Reset_CycleCounter( void )
{
    DWT->CYCCNT = 0;
}


/**
 * \brief Returns actual value of cycle counter.
 *
 * \returns Actual value of cycle counter
 */
dwt_CounterValue_t Dwt_Get_CycleCounter( void )
{
    dwt_CounterValue_t retVal = (dwt_CounterValue_t)DWT->CYCCNT;

    return ( retVal );
}


/**
 * \brief Resets Cycles Per Instruction (CPI) counter.
 *
 */
void Dwt_Reset_CpiCounter( void )
{
    DWT->CPICNT = 0;
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
    DWT->EXCCNT = 0;
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
    DWT->SLEEPCNT = 0;
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
    DWT->LSUCNT = 0;
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
    DWT->FOLDCNT = 0;
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
