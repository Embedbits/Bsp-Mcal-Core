/**
 * \defgroup Syscfg Syscfg
 * \brief Syscfg module
 */

/**
 * \author Mr.Nobody
 * \file Syscfg_Types.h
 * \ingroup Syscfg
 * \brief Syscfg module global types definition
 *
 * This file contains the types definitions used across the module and are
 * available for other modules through the Port file.
 *
 */

#ifndef SYSCFG_SYSCFG_TYPES_H
#define SYSCFG_SYSCFG_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================== INCLUDES ================================== */
#include "stdint.h"                         /* Module types definition        */
/* ========================== SYMBOLIC CONSTANTS ============================ */

/** Null pointer definition */
#define SYSCFG_NULL_PTR                       ( ( void* ) 0u )

/* ========================== EXPORTED MACROS =============================== */

/* ============================== TYPEDEFS ================================== */

/** \brief Type signaling major version of SW module */
typedef uint8_t syscfg_MajorVersion_t;


/** \brief Type signaling minor version of SW module */
typedef uint8_t syscfg_MinorVersion_t;


/** \brief Type signaling patch version of SW module */
typedef uint8_t syscfg_PatchVersion_t;


/** \brief Type signaling actual version of SW module */
typedef struct
{
    syscfg_MajorVersion_t Major; /**< Major version */
    syscfg_MinorVersion_t Minor; /**< Minor version */
    syscfg_PatchVersion_t Patch; /**< Patch version */
}   syscfg_ModuleVersion_t;


/** \brief Enumeration used to signal request processing state */
typedef enum
{
    SYSCFG_REQUEST_ERROR = 0u, /**< Processing request failed  */
    SYSCFG_REQUEST_OK          /**< Processing request succeed */
}   syscfg_RequestState_t;


/**
 * \brief Ethernet PHY interface selected on the SYSCFG PMCR.EPIS_SEL field (RM0433 / RM0399
 *        chapter "System configuration controller (SYSCFG)"), or on the SBS PMCR.ETH_PHYSEL field
 *        on STM32H7R / STM32H7S (RM0477 Rev 10, 8.5.7). Classic STM32H7 has no Sbs module
 *        (unlike STM32H5): this is the module that owns the selection on this Mcal branch.
 */
typedef enum
{
    SYSCFG_ETH_PHY_MII = 0u,     /**< Media independent interface (field = 0b000, GMII or MII on H7R / H7S) */
    SYSCFG_ETH_PHY_RMII,         /**< Reduced media independent interface (field = 0b100)                   */
    SYSCFG_ETH_PHY_INTERFACE_CNT /**< Count of available options                                  */
}   syscfg_EthPhyInterface_t;

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */

#ifdef __cplusplus
}
#endif

#endif /* SYSCFG_SYSCFG_TYPES_H */
