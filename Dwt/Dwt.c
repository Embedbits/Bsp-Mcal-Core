/**
 * \author Mr.Nobody
 * \file Dwt.c
 * \ingroup Dwt
 * \brief Data Watchpoint and Trace Unit (DWT) module implementation file.
 *
 * # Cycle counter and profiling counters
 * The Cortex®-M33 DWT supports a cycle counter and profiling counters.
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
 * instruction executed in zero cycles.
 *
 * # Trace enable (DEMCR.TRCENA)
 * DWT and ITM share the global trace enable bit. Deinitialization of DWT keeps
 * the bit set while ITM is enabled, so ITM keeps working.
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
#define DWT_MINOR_VERSION           ( 1u )

/** Value of patch version of SW module */
#define DWT_PATCH_VERSION           ( 0u )

/** Reset value of counters */
#define DWT_COUNTER_RESET           ( 0u )

/** Number of microseconds in one second */
#define DWT_US_PER_S                ( 1000000ull )

/** Maximal value of time in microseconds (saturation of the conversion) */
#define DWT_TIME_US_MAX             ( 0xFFFFFFFFu )

/* =============================== MACROS =================================== */

/* ========================== EXPORTED VARIABLES ============================ */

/** CMSIS core clock (HCLK) in Hz, maintained by the Rcc module */
extern uint32_t SystemCoreClock;

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
 * Enables trace unit, resets all counters and enables counters required by
 * configuration.
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
        DCB->DEMCR |= DCB_DEMCR_TRCENA_Msk;

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
        DCB->DEMCR &= ~DCB_DEMCR_TRCENA_Msk;
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
 * \brief Returns whether the cycle counter runs.
 *
 * The counter runs when the trace unit is enabled (DEMCR.TRCENA) and the cycle
 * counter is enabled (DWT_CTRL.CYCCNTENA). No register is changed.
 *
 * \param state [out]: \ref DWT_FUNCTION_ACTIVE when the cycle counter runs,
 *                     otherwise \ref DWT_FUNCTION_INACTIVE
 *
 * \return \ref DWT_REQUEST_OK if the state was filled, otherwise
 *         \ref DWT_REQUEST_ERROR (NULL pointer)
 */
dwt_RequestState_t Dwt_Get_State( dwt_FunctionState_t * const state )
{
    dwt_RequestState_t retVal = DWT_REQUEST_ERROR;

    if( DWT_NULL_PTR != state )
    {
        if( ( 0u != ( DCB->DEMCR & DCB_DEMCR_TRCENA_Msk ) ) &&
            ( 0u != ( DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk ) )    )
        {
            *state = DWT_FUNCTION_ACTIVE;
        }
        else
        {
            *state = DWT_FUNCTION_INACTIVE;
        }

        retVal = DWT_REQUEST_OK;
    }
    else
    {
        retVal = DWT_REQUEST_ERROR;
    }

    return ( retVal );
}


/**
 * \brief Returns frequency of the cycle counter.
 *
 * The cycle counter runs with the core clock (HCLK). The value is taken from
 * the CMSIS variable SystemCoreClock, so the module does not depend on Rcc API.
 *
 * \param freq [out]: Frequency of the cycle counter in Hz
 *
 * \return \ref DWT_REQUEST_OK if the frequency was filled, otherwise
 *         \ref DWT_REQUEST_ERROR (NULL pointer, cycle counter does not run or
 *         core clock unknown)
 */
dwt_RequestState_t Dwt_Get_CycleFrequency( dwt_Frequency_Hz_t * const freq )
{
    dwt_RequestState_t  retVal   = DWT_REQUEST_ERROR;
    dwt_FunctionState_t state    = DWT_FUNCTION_INACTIVE;
    dwt_RequestState_t  getState = Dwt_Get_State( &state );

    if( ( DWT_NULL_PTR        != freq            ) &&
        ( DWT_REQUEST_OK      == getState        ) &&
        ( DWT_FUNCTION_ACTIVE == state           ) &&
        ( 0u                  != SystemCoreClock )    )
    {
        *freq  = (dwt_Frequency_Hz_t) SystemCoreClock;
        retVal = DWT_REQUEST_OK;
    }
    else
    {
        retVal = DWT_REQUEST_ERROR;
    }

    return ( retVal );
}


/**
 * \brief Returns time elapsed since the given value of the cycle counter.
 *
 * The difference is computed in unsigned 32-bit arithmetic, so one wrap-around
 * of the counter between the start and now is handled. A longer interval than
 * 2^32 cycles cannot be detected. The result saturates at the maximal value.
 *
 * \param startCycle [in]: Value of the cycle counter at the start of the measurement
 * \param elapsed    [out]: Elapsed time in microseconds
 *
 * \return \ref DWT_REQUEST_OK if the time was filled, otherwise
 *         \ref DWT_REQUEST_ERROR (NULL pointer or cycle counter does not run)
 */
dwt_RequestState_t Dwt_Get_ElapsedTime_us( dwt_CounterValue_t startCycle, dwt_Time_us_t * const elapsed )
{
    dwt_RequestState_t retVal  = DWT_REQUEST_ERROR;
    dwt_Frequency_Hz_t freq    = 0u;
    dwt_RequestState_t freqReq = Dwt_Get_CycleFrequency( &freq );

    if( ( DWT_NULL_PTR   != elapsed ) &&
        ( DWT_REQUEST_OK == freqReq )    )
    {
        const dwt_CounterValue_t cycles = (dwt_CounterValue_t) ( Dwt_Get_CycleCounter() - startCycle );
        const uint64_t           timeUs = ( (uint64_t) cycles * DWT_US_PER_S ) / (uint64_t) freq;

        if( timeUs > (uint64_t) DWT_TIME_US_MAX )
        {
            *elapsed = (dwt_Time_us_t) DWT_TIME_US_MAX;
        }
        else
        {
            *elapsed = (dwt_Time_us_t) timeUs;
        }

        retVal = DWT_REQUEST_OK;
    }
    else
    {
        retVal = DWT_REQUEST_ERROR;
    }

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
