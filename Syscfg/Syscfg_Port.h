/**
 * \author Mr.Nobody
 * \file Syscfg_Port.h
 * \ingroup Syscfg
 * \brief Syscfg module public functionality
 *
 * This file contains all available public functionality, any other files shall
 * not be used outside of the module.
 *
 */

#ifndef SYSCFG_SYSCFG_PORT_H
#define SYSCFG_SYSCFG_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================== INCLUDES ================================== */
#include "Syscfg_Types.h"                   /* Module types definition        */
/* ============================== TYPEDEFS ================================== */

/* ========================== SYMBOLIC CONSTANTS ============================ */

/* ========================== EXPORTED MACROS =============================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */

syscfg_ModuleVersion_t  Syscfg_Get_ModuleVersion     ( void );

syscfg_RequestState_t   Syscfg_Set_EthPhyInterface   ( syscfg_EthPhyInterface_t phyInterface );
syscfg_RequestState_t   Syscfg_Get_EthPhyInterface   ( syscfg_EthPhyInterface_t * const phyInterface );

#ifdef __cplusplus
}
#endif

#endif /* SYSCFG_SYSCFG_PORT_H */
