/**
 * \author Mr.Nobody
 * \file Syscfg.c
 * \ingroup Syscfg
 * \brief Syscfg module implementation file.
 *
 * Minimal getter/setter pair for the SYSCFG PMCR.EPIS_SEL field (Ethernet PHY interface
 * selection), added for the classic STM32H7 Eth module (AB#1863): classic STM32H7 has no Sbs
 * module (unlike STM32H5), and Core / Syscfg is the module of the SYSCFG
 * peripheral on this family (RM0433 chapter "System configuration controller (SYSCFG)",
 * RM0399 for the dual-core H745/H747/H755/H757 lines - PMCR is shared, written by whichever
 * core's firmware image configures the Ethernet interface).
 *
 * STM32H7R / STM32H7S (Ral family STM32H7RS, built with the same Mcal branch) have no SYSCFG
 * peripheral: the same selection is the SBS_PMCR.ETH_PHYSEL field there (RM0477 Rev 10, 8.5.7),
 * with the SBS register interface clock (AB#2204). The register, the field and the clock are
 * selected by the CMSIS definitions (\c SYSCFG_PMCR_EPIS_SEL or \c SBS_PMCR_ETH_PHYSEL).
 *
 * The encoding below (\c 0b000 = MII, \c 0b100 = RMII, bit 2 of the field only) is the same on both
 * register sets and is the one of the ST drivers (\c LL_SYSCFG_ETH_MII = 0,
 * \c LL_SYSCFG_ETH_RMII = \c SYSCFG_PMCR_EPIS_SEL_2 in stm32h7xx_ll_system.h,
 * \c LL_SBS_ETH_PHYSEL_GMII_MII = 0, \c LL_SBS_ETH_PHYSEL_RMII = \c SBS_PMCR_ETH_PHYSEL_2 in
 * stm32h7rsxx_ll_system.h).
 *
 */
/* ============================== INCLUDES ================================== */
#include "Syscfg.h"                         /* Self include                   */
#include "Syscfg_Port.h"                    /* Own port file include          */
#include "Syscfg_Types.h"                   /* Module types definitions       */
#include "Stm32.h"                          /* SYSCFG / SBS registers         */
#include "Rcc_Port.h"                       /* SYSCFG / SBS clock enable      */
/* ============================== TYPEDEFS ================================== */

/* ======================== FORWARD DECLARATIONS ============================ */

/* ========================== SYMBOLIC CONSTANTS ============================ */

/** \brief Module major/minor/patch version */
#define SYSCFG_MODULE_VERSION_MAJOR    ( 0u )
#define SYSCFG_MODULE_VERSION_MINOR    ( 1u )
#define SYSCFG_MODULE_VERSION_PATCH    ( 0u )

#if defined(SYSCFG_PMCR_EPIS_SEL)
/** \brief Register holding the Ethernet PHY interface field (SYSCFG PMCR, classic STM32H7) */
#define SYSCFG_PHY_SEL_REG             ( SYSCFG->PMCR )

/** \brief Ethernet PHY interface field of the register (PMCR.EPIS_SEL[2:0]) */
#define SYSCFG_PHY_SEL_MASK            ( SYSCFG_PMCR_EPIS_SEL )

/** \brief Field value selecting the RMII interface (bit 2 of the field only) */
#define SYSCFG_PHY_SEL_RMII            ( SYSCFG_PMCR_EPIS_SEL_2 )

/** \brief Rcc item of the register interface clock of the SYSCFG peripheral */
#define SYSCFG_PHY_SEL_PERIPH          ( RCC_PERIPH_SYSCFG )
#elif defined(SBS_PMCR_ETH_PHYSEL)
/** \brief Register holding the Ethernet PHY interface field (SBS PMCR, STM32H7R / STM32H7S) */
#define SYSCFG_PHY_SEL_REG             ( SBS->PMCR )

/** \brief Ethernet PHY interface field of the register (PMCR.ETH_PHYSEL[2:0]) */
#define SYSCFG_PHY_SEL_MASK            ( SBS_PMCR_ETH_PHYSEL )

/** \brief Field value selecting the RMII interface (bit 2 of the field only) */
#define SYSCFG_PHY_SEL_RMII            ( SBS_PMCR_ETH_PHYSEL_2 )

/** \brief Rcc item of the register interface clock of the SBS peripheral */
#define SYSCFG_PHY_SEL_PERIPH          ( RCC_PERIPH_SBS )
#endif /* SYSCFG_PMCR_EPIS_SEL / SBS_PMCR_ETH_PHYSEL */

#if defined(SYSCFG_PHY_SEL_MASK)
/** \brief Field value selecting the MII interface (GMII or MII on STM32H7R / STM32H7S) */
#define SYSCFG_PHY_SEL_MII             ( 0u )
#endif /* SYSCFG_PHY_SEL_MASK */

/* =============================== MACROS =================================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* =========================== LOCAL VARIABLES ============================== */

/* ========================= EXPORTED FUNCTIONS ============================= */

/**
 * \brief Returns the version of the module.
 *
 * \return Structure with the major, minor and patch version of the module.
 */
syscfg_ModuleVersion_t Syscfg_Get_ModuleVersion( void )
{
    const syscfg_ModuleVersion_t moduleVersion =
    {
        .Major = SYSCFG_MODULE_VERSION_MAJOR,
        .Minor = SYSCFG_MODULE_VERSION_MINOR,
        .Patch = SYSCFG_MODULE_VERSION_PATCH,
    };

    return ( moduleVersion );
}


/**
 * \brief Selects the Ethernet PHY interface on the SYSCFG PMCR.EPIS_SEL field (classic STM32H7)
 *        or on the SBS PMCR.ETH_PHYSEL field (STM32H7R / STM32H7S).
 *
 * Enables the register interface clock of the block (Rcc, RCC_PERIPH_SYSCFG or RCC_PERIPH_SBS)
 * before touching PMCR: nothing else in this module (or in Eth.c, the only caller) enabled it
 * before (AB#1866, finding 5), so on a clean boot the write's read-back used to fail and this
 * function returned \ref SYSCFG_REQUEST_ERROR even though the field/encoding was correct.
 *
 * \param phyInterface [in]: Requested interface
 * \return \ref SYSCFG_REQUEST_OK when the value was read back, otherwise \ref SYSCFG_REQUEST_ERROR
 *         (invalid parameter, the RCC clock could not be enabled, or the device line has no
 *         Ethernet PHY interface field).
 */
syscfg_RequestState_t Syscfg_Set_EthPhyInterface( syscfg_EthPhyInterface_t phyInterface )
{
    syscfg_RequestState_t retState = SYSCFG_REQUEST_ERROR;

#if defined(SYSCFG_PHY_SEL_MASK)
    rcc_RequestState_t clkState = RCC_REQUEST_ERROR;

    if( SYSCFG_ETH_PHY_INTERFACE_CNT > phyInterface )
    {
        clkState = Rcc_Set_PeriphActive( SYSCFG_PHY_SEL_PERIPH );
    }
    else
    {
        /* Invalid parameter, the clock is not touched */
    }

    if( RCC_REQUEST_OK == clkState )
    {
        uint32_t writeValue = SYSCFG_PHY_SEL_MII;

        if( SYSCFG_ETH_PHY_RMII == phyInterface )
        {
            writeValue = SYSCFG_PHY_SEL_RMII;
        }
        else
        {
            /* SYSCFG_ETH_PHY_MII, value already set */
        }

        SYSCFG_PHY_SEL_REG = ( ( SYSCFG_PHY_SEL_REG & ~SYSCFG_PHY_SEL_MASK ) | writeValue );

        if( writeValue == ( SYSCFG_PHY_SEL_REG & SYSCFG_PHY_SEL_MASK ) )
        {
            retState = SYSCFG_REQUEST_OK;
        }
        else
        {
            retState = SYSCFG_REQUEST_ERROR;
        }
    }
    else
    {
        /* Invalid parameter, or the SYSCFG / SBS clock could not be enabled */
        retState = SYSCFG_REQUEST_ERROR;
    }
#else
    (void)phyInterface;
    /* No Ethernet PHY interface field on this device line */
    retState = SYSCFG_REQUEST_ERROR;
#endif /* SYSCFG_PHY_SEL_MASK */

    return ( retState );
}


/**
 * \brief Reads back the Ethernet PHY interface selected on the SYSCFG PMCR.EPIS_SEL field
 *        (classic STM32H7) or on the SBS PMCR.ETH_PHYSEL field (STM32H7R / STM32H7S).
 *
 * \param phyInterface [out]: Pointer to the interface
 * \return \ref SYSCFG_REQUEST_OK, or \ref SYSCFG_REQUEST_ERROR for an invalid pointer or when
 *         the device line has no Ethernet PHY interface field.
 */
syscfg_RequestState_t Syscfg_Get_EthPhyInterface( syscfg_EthPhyInterface_t * const phyInterface )
{
    syscfg_RequestState_t retState = SYSCFG_REQUEST_ERROR;

#if defined(SYSCFG_PHY_SEL_MASK)
    if( SYSCFG_NULL_PTR != phyInterface )
    {
        const uint32_t regValue = ( SYSCFG_PHY_SEL_REG & SYSCFG_PHY_SEL_MASK );

        if( SYSCFG_PHY_SEL_RMII == regValue )
        {
            *phyInterface = SYSCFG_ETH_PHY_RMII;
        }
        else
        {
            *phyInterface = SYSCFG_ETH_PHY_MII;
        }

        retState = SYSCFG_REQUEST_OK;
    }
    else
    {
        /* Invalid pointer */
        retState = SYSCFG_REQUEST_ERROR;
    }
#else
    (void)phyInterface;
    /* No Ethernet PHY interface field on this device line */
    retState = SYSCFG_REQUEST_ERROR;
#endif /* SYSCFG_PHY_SEL_MASK */

    return ( retState );
}

/* =========================== LOCAL FUNCTIONS ============================== */

/* =========================== INTERRUPT HANDLERS =========================== */

/* ================================ TASKS =================================== */
