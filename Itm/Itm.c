/**
 * \author Mr.Nobody
 * \file Itm.h
 * \ingroup Itm
 * \brief Itm module common functionality
 *
 */
/* ============================== INCLUDES ================================== */
#include "Itm.h"                            /* Self include                   */
#include "Itm_Port.h"                       /* Own port file include          */
#include "Itm_Types.h"                      /* Module types definitions       */
#include "Stm32.h"                          /* MCU common functionality       */
#include "Rcc_Port.h"                       /* RCC module functionality       */
/* ============================== TYPEDEFS ================================== */

/* ======================== FORWARD DECLARATIONS ============================ */

/* ========================== SYMBOLIC CONSTANTS ============================ */

/** Value of major version of SW module */
#define ITM_MAJOR_VERSION           ( 1u )

/** Value of minor version of SW module */
#define ITM_MINOR_VERSION           ( 0u )

/** Value of patch version of SW module */
#define ITM_PATCH_VERSION           ( 0u )

/* =============================== MACROS =================================== */

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
 */
void Itm_Init( itm_Config_t * const itmConfig )
{
    if( ITM_NULL_PTR != itmConfig )
    {
        if( ( ITM_PORT_CNT > itmConfig->PortId ) &&
            ( 0u           < itmConfig->SwoClk )    )
        {
            /* Enable trace in DEMCR */
            CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

            rcc_FreqHz_t sysClk = 0u;

            rcc_RequestState_t rccState = Rcc_Get_ClkFreq( RCC_CLK_SRC_SYSCLK, &sysClk );

            if( RCC_REQUEST_OK == rccState )
            {
#if defined(TPI)
                /* Prescaler: SWO_freq = coreClockHz / (ACPR + 1) */
                uint32_t prescaler = ( sysClk / itmConfig->SwoClk );
                if (prescaler == 0u)
                {
                    prescaler = 1u;
                }

                prescaler -= 1u;

                TPI->ACPR = prescaler;

                /* Protocol: 2 = NRZ/Async SWO (Manchester = 1, trace port = 0) */
                TPI->SPPR = 0x00000002UL;

                /* Formatter & Flush Control – allow formatter */
                TPI->FFCR = 0x00000100UL;   /* DISFOLD = 1 – typical value */
#endif
            }
            else
            {
                /* Error in RCC module */
            }

            /* Unlock ITM */
            ITM->TCR = 0x00000000;

            /* Configure stimulus ports */
            ITM->TER = ( 1u << itmConfig->PortId );

            /* Configure trace privilege (allow user mode access) */
            ITM->TPR = 0x00000000;

            if( ITM_FUNCTION_ACTIVE == itmConfig->TimestampState )
            {
                Itm_Set_TimestampEnable();       /* Enable timestamps */
            }

            if( ITM_FUNCTION_ACTIVE == itmConfig->SyncState )
            {
                Itm_Set_SyncEnable();     /* Enable sync packets */
            }

            /* Configure and enable ITM */
            ITM->TCR |= ITM_TCR_ITMENA_Msk;
        }
    }
}


/**
 * \brief De-initialize Instrumentation Trace Macrocell (ITM) periphery.
 */
void Itm_Deinit( void )
{
    /* Disable ITM */
    ITM->TCR = 0x00000000;

    /* Disable all stimulus ports */
    ITM->TER = 0x00000000;

    /* Disable trace in DEMCR */
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Pos;
}



/**
 * \brief Enables Instrumentation Trace Macrocell (ITM) periphery.
 */
itm_RequestState_t Itm_PortEnable( itm_Port_t portId )
{
    itm_RequestState_t retValue = ITM_REQUEST_ERROR;

    if( ITM_PORT_CNT > portId )
    {
        /* Enable given port */
        ITM->TER |= ( 1u << portId );

        retValue = ITM_REQUEST_OK;
    }
    else
    {
        retValue = ITM_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Disables Instrumentation Trace Macrocell (ITM) periphery.
 */
itm_RequestState_t Itm_PortDisable( itm_Port_t portId )
{
    itm_RequestState_t retValue = ITM_REQUEST_ERROR;

    if( ITM_PORT_CNT > portId )
    {
        /* Enable given port */
        ITM->TER &= ~( 1u << portId );

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
 * \param itmPort [in]: Communication port identification.
 * \param data    [in]: 8-bits data to be transmitted
 */
void Itm_Transmit_8b( itm_Port_t itmPort, uint8_t data )
{
    if( ITM_PORT_CNT > itmPort )
    {
        ITM->PORT[itmPort].u8 = data;
    }
}


/**
 * \brief Transmits 16-bit data to given channel.
 *
 * \param itmPort [in]: Communication port identification.
 * \param data    [in]: 16-bits data to be transmitted
 */
void Itm_Transmit_16b( itm_Port_t itmPort, uint16_t data )
{
  ITM->PORT[itmPort].u16 = data;
}


/**
 * \brief Transmits 32-bit data to given channel.
 *
 * \param itmPort [in]: Communication port identification.
 * \param data    [in]: 32-bits data to be transmitted
 */
void Itm_Transmit_32b( itm_Port_t itmPort, uint32_t data)
{
    ITM->PORT[ itmPort ].u32 = data;
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

/* =========================== INTERRUPT HANDLERS =========================== */

/* ================================ TASKS =================================== */
