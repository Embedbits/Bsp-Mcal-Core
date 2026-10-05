/**
 * \author Mr.Nobody
 * \file Itm.c
 * \ingroup Itm
 * \brief Instrumentation Trace Macrocell (ITM) module implementation file.
 *
 * ITM stimulus ports are transmitted over Serial Wire Output (SWO) pin through
 * Trace Port Interface Unit (TPIU) in asynchronous NRZ mode. SWO baudrate is
 * derived from core clock (HCLK) provided by RCC module.
 *
 * On STM32F4 the asynchronous trace pin assignment (TRACE_IOEN) has to be
 * enabled in DBGMCU, otherwise TPIU does not output any data. SWO is output on
 * pin PB3 (TRACESWO, AF0 - pin reset state).
 *
 * Transmit functions write to stimulus port only if ITM and the port are
 * enabled (eg. debugger with SWV support connected) and wait for free stimulus
 * port FIFO for limited time, so the application is never blocked when no trace
 * receiver is present.
 *
 * # Trace enable (DEMCR.TRCENA)
 * DWT and ITM share the global trace enable bit. Deinitialization of ITM keeps
 * the bit set while any DWT counter is enabled, so DWT keeps working.
 *
 */
/* ============================== INCLUDES ================================== */
#include "Itm.h"                            /* Self include                   */
#include "Itm_Port.h"                       /* Own port file include          */
#include "Itm_Types.h"                      /* Module types definitions       */
#include "Stm32.h"                          /* MCU common functionality       */
#include "Stm32_system.h"                   /* DBGMCU trace configuration     */
#include "Rcc_Port.h"                       /* RCC module functionality       */
/* ============================== TYPEDEFS ================================== */

/* ======================== FORWARD DECLARATIONS ============================ */

static itm_RequestState_t   Itm_Set_SwoConfig       ( itm_FreqHz_t swoClk );
static itm_FlagState_t      Itm_Get_PortReady       ( itm_Port_t itmPort );
static itm_RequestState_t   Itm_Wait_PortReady      ( itm_Port_t itmPort );

/* ========================== SYMBOLIC CONSTANTS ============================ */

/** Value of major version of SW module */
#define ITM_MAJOR_VERSION           ( 1u )

/** Value of minor version of SW module */
#define ITM_MINOR_VERSION           ( 0u )

/** Value of patch version of SW module */
#define ITM_PATCH_VERSION           ( 0u )

/** Maximum wait time for free stimulus port FIFO */
#define ITM_TIMEOUT_RAW             ( 0x84FCB )

/** Default SWO communication clock used by \ref Itm_Get_DefaultConfig */
#define ITM_DEFAULT_SWO_CLK_HZ      ( 2000000u )

/** Maximal value of SWO prescaler (TPI ACPR register width) */
#define ITM_SWO_PRESCALER_MAX       ( TPI_ACPR_PRESCALER_Msk )

/** Selected pin protocol - asynchronous SWO, NRZ encoding */
#define ITM_SWO_PROTOCOL_NRZ        ( 2u )

/** Trace bus ID of ITM (non-zero ID required by trace infrastructure) */
#define ITM_TRACE_BUS_ID            ( 1u )

/** CoreSight software lock key unlocking write access to ITM registers */
#define ITM_LAR_UNLOCK_KEY          ( 0xC5ACCE55u )

/** Unlocked ITM - trace control register value with ITM disabled */
#define ITM_TCR_DISABLED            ( 0u )

/** Unprivileged access to all stimulus ports */
#define ITM_TPR_UNPRIVILEGED        ( 0u )

/** All stimulus ports disabled */
#define ITM_TER_DISABLED            ( 0u )

/** Stimulus port read value signaling FIFO ready to accept data */
#define ITM_PORT_FIFO_READY         ( 1u )

/** DWT control register mask of all counter enable bits */
#define ITM_DWT_COUNTERS_MSK        ( DWT_CTRL_CYCCNTENA_Msk   | \
                                      DWT_CTRL_CPIEVTENA_Msk   | \
                                      DWT_CTRL_EXCEVTENA_Msk   | \
                                      DWT_CTRL_SLEEPEVTENA_Msk | \
                                      DWT_CTRL_LSUEVTENA_Msk   | \
                                      DWT_CTRL_FOLDEVTENA_Msk    )

/* =============================== MACROS =================================== */

/** Stimulus port bit mask in trace enable register */
#define ITM_PORT_MSK( portId )      ( 1uL << (uint32_t)( portId ) )

/* ========================== EXPORTED VARIABLES ============================ */

/* =========================== LOCAL VARIABLES ============================== */

/* ========================= EXPORTED FUNCTIONS ============================= */

/**
 * \brief Returns module SW version
 *
 * \return Module SW version
 */
itm_ModuleVersion_t Itm_Get_ModuleVersion( void )
{
    itm_ModuleVersion_t retVersion;

    retVersion.Major = ITM_MAJOR_VERSION;
    retVersion.Minor = ITM_MINOR_VERSION;
    retVersion.Patch = ITM_PATCH_VERSION;

    return (retVersion);
}


/**
 * \brief Initialize Instrumentation Trace Macrocell (ITM) periphery.
 *
 * Enables trace, configures SWO output (DBGMCU, TPIU) and enables ITM with
 * configured stimulus port.
 *
 * \param itmConfig [in]: Module configuration
 *
 * \return \ref ITM_REQUEST_OK if ITM was initialized, otherwise
 *         \ref ITM_REQUEST_ERROR
 */
itm_RequestState_t Itm_Init( itm_Config_t * const itmConfig )
{
    itm_RequestState_t retState = ITM_REQUEST_ERROR;

    if( ( ITM_NULL_PTR != itmConfig               ) &&
        ( ITM_PORT_CNT >  itmConfig->PortId       ) &&
        ( 0u           <  itmConfig->SwoClk       )    )
    {
        /* Enable trace in DEMCR */
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

        retState = Itm_Set_SwoConfig( itmConfig->SwoClk );

        if( ITM_REQUEST_OK == retState )
        {
            /* Unlock write access to ITM registers (CoreSight software lock) */
            ITM->LAR = ITM_LAR_UNLOCK_KEY;

            /* Disable ITM before configuration */
            ITM->TCR = ITM_TCR_DISABLED;

            /* Configure stimulus ports */
            ITM->TER = ITM_PORT_MSK( itmConfig->PortId );

            /* Configure trace privilege (allow user mode access) */
            ITM->TPR = ITM_TPR_UNPRIVILEGED;

            if( ITM_FUNCTION_ACTIVE == itmConfig->TimestampState )
            {
                Itm_Set_TimestampEnable();
            }

            if( ITM_FUNCTION_ACTIVE == itmConfig->SyncState )
            {
                Itm_Set_SyncEnable();
            }

            /* Configure trace bus ID and enable ITM */
            ITM->TCR |= ( ( ITM_TRACE_BUS_ID << ITM_TCR_TraceBusID_Pos ) & ITM_TCR_TraceBusID_Msk ) |
                        ITM_TCR_ITMENA_Msk;
        }
    }
    else
    {
        retState = ITM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief De-initialize Instrumentation Trace Macrocell (ITM) periphery.
 *
 * Disables ITM, all stimulus ports and SWO output. Trace is disabled only if
 * no DWT counter is enabled (shared DEMCR.TRCENA bit).
 */
void Itm_Deinit( void )
{
    /* Disable ITM */
    ITM->TCR = ITM_TCR_DISABLED;

    /* Disable all stimulus ports */
    ITM->TER = ITM_TER_DISABLED;

    /* Release trace pins */
    LL_DBGMCU_SetTracePinAssignment( LL_DBGMCU_TRACE_NONE );

    /* Disable trace in DEMCR, if not used by DWT */
    if( 0u == ( DWT->CTRL & ITM_DWT_COUNTERS_MSK ) )
    {
        CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
    }
}


/**
 * \brief Fills default configuration of the module
 *
 * Default configuration uses stimulus port 0 (printf like output of debuggers),
 * SWO clock 2 MHz, timestamps and synchronization packets disabled.
 *
 * \param itmConfig [out]: Configuration to be filled
 *
 * \return \ref ITM_REQUEST_OK if configuration was filled, otherwise
 *         \ref ITM_REQUEST_ERROR
 */
itm_RequestState_t Itm_Get_DefaultConfig( itm_Config_t * const itmConfig )
{
    itm_RequestState_t retState = ITM_REQUEST_ERROR;

    if( ITM_NULL_PTR != itmConfig )
    {
        itmConfig->PortId         = ITM_PORT_0;
        itmConfig->TimestampState = ITM_FUNCTION_INACTIVE;
        itmConfig->SyncState      = ITM_FUNCTION_INACTIVE;
        itmConfig->SwoClk         = ITM_DEFAULT_SWO_CLK_HZ;

        retState = ITM_REQUEST_OK;
    }
    else
    {
        retState = ITM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Enables Instrumentation Trace Macrocell (ITM) stimulus port.
 *
 * \param portId [in]: Stimulus port identification
 *
 * \return \ref ITM_REQUEST_OK if port was enabled, otherwise
 *         \ref ITM_REQUEST_ERROR
 */
itm_RequestState_t Itm_PortEnable( itm_Port_t portId )
{
    itm_RequestState_t retValue = ITM_REQUEST_ERROR;

    if( ITM_PORT_CNT > portId )
    {
        /* Enable given port */
        ITM->TER |= ITM_PORT_MSK( portId );

        retValue = ITM_REQUEST_OK;
    }
    else
    {
        retValue = ITM_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Disables Instrumentation Trace Macrocell (ITM) stimulus port.
 *
 * \param portId [in]: Stimulus port identification
 *
 * \return \ref ITM_REQUEST_OK if port was disabled, otherwise
 *         \ref ITM_REQUEST_ERROR
 */
itm_RequestState_t Itm_PortDisable( itm_Port_t portId )
{
    itm_RequestState_t retValue = ITM_REQUEST_ERROR;

    if( ITM_PORT_CNT > portId )
    {
        /* Disable given port */
        ITM->TER &= ~ITM_PORT_MSK( portId );

        retValue = ITM_REQUEST_OK;
    }
    else
    {
        retValue = ITM_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Enables time-stamp in output frame.
 */
void Itm_Set_TimestampEnable( void )
{
    ITM->TCR |= ITM_TCR_TSENA_Msk;
}


/**
 * \brief Disables time-stamp in output frame.
 */
void Itm_Set_TimestampDisable( void )
{
    ITM->TCR &= ~ITM_TCR_TSENA_Msk;
}


/**
 * \brief Enables synchronization packets.
 */
void Itm_Set_SyncEnable( void )
{
    ITM->TCR |= ITM_TCR_SYNCENA_Msk;
}


/**
 * \brief Disables synchronization packets.
 */
void Itm_Set_SyncDisable( void )
{
    ITM->TCR &= ~ITM_TCR_SYNCENA_Msk;
}


/**
 * \brief Transmits 8-bit data to given channel.
 *
 * Data are dropped if ITM or the port is disabled, or the port FIFO is not
 * free in time.
 *
 * \param itmPort [in]: Communication port identification.
 * \param data    [in]: 8-bits data to be transmitted
 */
void Itm_Transmit_8b( itm_Port_t itmPort, uint8_t data )
{
    if( ITM_REQUEST_OK == Itm_Wait_PortReady( itmPort ) )
    {
        ITM->PORT[ itmPort ].u8 = data;
    }
}


/**
 * \brief Transmits 16-bit data to given channel.
 *
 * Data are dropped if ITM or the port is disabled, or the port FIFO is not
 * free in time.
 *
 * \param itmPort [in]: Communication port identification.
 * \param data    [in]: 16-bits data to be transmitted
 */
void Itm_Transmit_16b( itm_Port_t itmPort, uint16_t data )
{
    if( ITM_REQUEST_OK == Itm_Wait_PortReady( itmPort ) )
    {
        ITM->PORT[ itmPort ].u16 = data;
    }
}


/**
 * \brief Transmits 32-bit data to given channel.
 *
 * Data are dropped if ITM or the port is disabled, or the port FIFO is not
 * free in time.
 *
 * \param itmPort [in]: Communication port identification.
 * \param data    [in]: 32-bits data to be transmitted
 */
void Itm_Transmit_32b( itm_Port_t itmPort, uint32_t data )
{
    if( ITM_REQUEST_OK == Itm_Wait_PortReady( itmPort ) )
    {
        ITM->PORT[ itmPort ].u32 = data;
    }
}


/**
 * \brief Returns transmit state of given stimulus port.
 *
 * \param itmPort [in]: Communication port identification.
 *
 * \return \ref ITM_FLAG_ACTIVE if ITM and the port are enabled and the port
 *         FIFO can accept new data. Otherwise returns \ref ITM_FLAG_INACTIVE.
 */
itm_FlagState_t Itm_Get_TransmitState( itm_Port_t itmPort )
{
    return ( Itm_Get_PortReady( itmPort ) );
}


/**
 * \brief Returns busy state of ITM module.
 *
 * \return Processing state of ITM. Returns \ref ITM_FLAG_ACTIVE if ITM events
 *         present and being drained. Otherwise returns \ref ITM_FLAG_INACTIVE.
 */
itm_FlagState_t Itm_Get_BusyState( void )
{
    itm_FlagState_t retState = ITM_FLAG_INACTIVE;
    uint32_t        regValue = ITM->TCR & ( ITM_TCR_BUSY_Msk );

    if( 0u != regValue )
    {
        retState = ITM_FLAG_ACTIVE;
    }
    else
    {
        retState = ITM_FLAG_INACTIVE;
    }

    return ( retState );
}

/* =========================== LOCAL FUNCTIONS ============================== */

/**
 * \brief Configures SWO output - trace clock, trace pin and TPIU.
 *
 * SWO frequency = HCLK / ( ACPR + 1 ). Prescaler is limited by ACPR register
 * width.
 *
 * \param swoClk [in]: Required SWO communication clock in Hz
 *
 * \return \ref ITM_REQUEST_OK if SWO was configured, \ref ITM_REQUEST_ERROR if
 *         core clock is not available
 */
static itm_RequestState_t Itm_Set_SwoConfig( itm_FreqHz_t swoClk )
{
    itm_RequestState_t retState = ITM_REQUEST_ERROR;
    rcc_FreqHz_t       coreClk  = 0u;

    if( RCC_REQUEST_OK == Rcc_Get_ClkBusClk( RCC_CLK_BUS_AHB1, &coreClk ) )
    {
        /* Asynchronous trace pin (TRACESWO) */
        LL_DBGMCU_SetTracePinAssignment( LL_DBGMCU_TRACE_ASYNCH );

        uint32_t prescaler = ( coreClk / swoClk );

        if( 0u == prescaler )
        {
            prescaler = 1u;
        }

        prescaler -= 1u;

        if( ITM_SWO_PRESCALER_MAX < prescaler )
        {
            prescaler = ITM_SWO_PRESCALER_MAX;
        }

        TPI->ACPR = prescaler;

        /* Protocol: asynchronous SWO, NRZ encoding */
        TPI->SPPR = ITM_SWO_PROTOCOL_NRZ;

        /* Formatter bypassed (continuous formatting disabled) */
        TPI->FFCR = TPI_FFCR_TrigIn_Msk;

        retState = ITM_REQUEST_OK;
    }
    else
    {
        retState = ITM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Returns ready state of stimulus port.
 *
 * \param itmPort [in]: Communication port identification.
 *
 * \return \ref ITM_FLAG_ACTIVE if ITM and the port are enabled and the port
 *         FIFO can accept new data. Otherwise returns \ref ITM_FLAG_INACTIVE.
 */
static itm_FlagState_t Itm_Get_PortReady( itm_Port_t itmPort )
{
    itm_FlagState_t retState = ITM_FLAG_INACTIVE;

    if( ( ITM_PORT_CNT >  itmPort                                        ) &&
        ( 0u           != ( ITM->TCR & ITM_TCR_ITMENA_Msk )              ) &&
        ( 0u           != ( ITM->TER & ITM_PORT_MSK( itmPort ) )         ) &&
        ( 0u           != ( ITM->PORT[ itmPort ].u32 & ITM_PORT_FIFO_READY ) ) )
    {
        retState = ITM_FLAG_ACTIVE;
    }
    else
    {
        retState = ITM_FLAG_INACTIVE;
    }

    return ( retState );
}


/**
 * \brief Waits for free FIFO of stimulus port.
 *
 * \param itmPort [in]: Communication port identification.
 *
 * \return \ref ITM_REQUEST_OK if port is ready to accept data, otherwise
 *         \ref ITM_REQUEST_ERROR (ITM or port disabled, timeout)
 */
static itm_RequestState_t Itm_Wait_PortReady( itm_Port_t itmPort )
{
    itm_RequestState_t retState = ITM_REQUEST_ERROR;

    if( ( ITM_PORT_CNT >  itmPort                                ) &&
        ( 0u           != ( ITM->TCR & ITM_TCR_ITMENA_Msk )      ) &&
        ( 0u           != ( ITM->TER & ITM_PORT_MSK( itmPort ) ) )    )
    {
        uint32_t timeout = ITM_TIMEOUT_RAW;

        while( ( ITM_FLAG_INACTIVE == Itm_Get_PortReady( itmPort ) ) &&
               ( 0u                <  timeout                     )    )
        {
            timeout--;
        }

        if( ITM_FLAG_ACTIVE == Itm_Get_PortReady( itmPort ) )
        {
            retState = ITM_REQUEST_OK;
        }
    }
    else
    {
        retState = ITM_REQUEST_ERROR;
    }

    return ( retState );
}

/* =========================== INTERRUPT HANDLERS =========================== */

/* ================================ TASKS =================================== */
