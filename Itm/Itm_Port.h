/**
 * \author Mr.Nobody
 * \file Itm_Port.h
 * \ingroup Itm
 * \brief Instrumentation Trace Macrocell (ITM) module public functionality
 *
 * This file contains all available public functionality, any other files shall
 * not used outside of the module.
 *
 */

#ifndef ITM_ITM_PORT_H
#define ITM_ITM_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================== INCLUDES ================================== */
#include "Itm_Types.h"                      /* Module types definition        */
/* ============================== TYPEDEFS ================================== */

/* ========================== SYMBOLIC CONSTANTS ============================ */

/* ========================== EXPORTED MACROS =============================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */

itm_ModuleVersion_t         Itm_Get_ModuleVersion       ( void );

itm_RequestState_t          Itm_Init                    ( itm_Config_t * const itmConfig );
void                        Itm_Deinit                  ( void );

itm_RequestState_t          Itm_Get_DefaultConfig       ( itm_Config_t * const itmConfig );

itm_RequestState_t          Itm_PortEnable              ( itm_Port_t portId );
itm_RequestState_t          Itm_PortDisable             ( itm_Port_t portId );

void                        Itm_Set_TimestampEnable     ( void );
void                        Itm_Set_TimestampDisable    ( void );

void                        Itm_Set_SyncEnable          ( void );
void                        Itm_Set_SyncDisable         ( void );

void                        Itm_Transmit_8b             ( itm_Port_t itmPort, uint8_t data );
void                        Itm_Transmit_16b            ( itm_Port_t itmPort, uint16_t data );
void                        Itm_Transmit_32b            ( itm_Port_t itmPort, uint32_t data );

itm_FlagState_t             Itm_Get_TransmitState       ( itm_Port_t itmPort );
itm_FlagState_t             Itm_Get_BusyState           ( void );

#ifdef __cplusplus
}
#endif

#endif /* ITM_ITM_PORT_H */
