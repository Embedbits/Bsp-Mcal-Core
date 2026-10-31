/**
 * \author Mr.Nobody
 * \file Scb_Port.h
 * \ingroup Scb
 * \brief System Control Block (SCB) module public functionality
 *
 * This file contains all available public functionality, any other files shall
 * not used outside of the module.
 *
 */

#ifndef SCB_SCB_PORT_H
#define SCB_SCB_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================== INCLUDES ================================== */
#include "Scb_Types.h"                      /* Module types definition        */
/* ============================== TYPEDEFS ================================== */

/* ========================== SYMBOLIC CONSTANTS ============================ */

/* ========================== EXPORTED MACROS =============================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */

scb_ModuleVersion_t     Scb_Get_ModuleVersion           ( void );

void                    Scb_Set_DeepSleepActive         ( void );
void                    Scb_Set_DeepSleepInactive       ( void );
scb_FunctionState_t     Scb_Get_DeepSleepState          ( void );

void                    Scb_Set_SleepOnExitActive       ( void );
void                    Scb_Set_SleepOnExitInactive     ( void );
scb_FunctionState_t     Scb_Get_SleepOnExitState        ( void );

void                    Scb_Set_WaitForInterrupt        ( void );
void                    Scb_Set_WaitForEvent            ( void );

#ifdef __cplusplus
}
#endif

#endif /* SCB_SCB_PORT_H */
