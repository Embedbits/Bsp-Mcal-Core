/**
 * \author Mr.Nobody
 * \file Dwt_Port.h
 * \ingroup Dwt
 * \brief Data Watchpoint and Trace Unit (DWT) module public functionality
 *
 * This file contains all available public functionality, any other files shall 
 * not used outside of the module.
 *
 */

#ifndef DWT_DWT_PORT_H
#define DWT_DWT_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================== INCLUDES ================================== */
#include "Dwt_Types.h"                      /* Module types definition        */
/* ============================== TYPEDEFS ================================== */

/* ========================== SYMBOLIC CONSTANTS ============================ */

/* ========================== EXPORTED MACROS =============================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */

dwt_ModuleVersion_t     Dwt_Get_ModuleVersion           ( void );

dwt_RequestState_t      Dwt_Init                        ( dwt_Config_t * const config );
void                    Dwt_Deinit                      ( void );
void                    Dwt_Task                        ( void );

void                    Dwt_Reset_CycleCounter          ( void );
dwt_CounterValue_t      Dwt_Get_CycleCounter            ( void );

void                    Dwt_Reset_CpiCounter            ( void );
dwt_CounterValue_t      Dwt_Get_CpiCounter              ( void );

void                    Dwt_Reset_ExceptionCounter      ( void );
dwt_CounterValue_t      Dwt_Get_ExceptionCounter        ( void );

void                    Dwt_Reset_SleepCounter          ( void );
dwt_CounterValue_t      Dwt_Get_SleepCounter            ( void );

void                    Dwt_Reset_LoadStoreCounter      ( void );
dwt_CounterValue_t      Dwt_Get_LoadStoreCounter        ( void );

void                    Dwt_Reset_FoldCounter           ( void );
dwt_CounterValue_t      Dwt_Get_FoldCounter             ( void );

#ifdef __cplusplus
}
#endif

#endif /* DWT_DWT_PORT_H */

