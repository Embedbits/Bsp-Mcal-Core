/**
 * \author Mr.Nobody
 * \file Itm_Port.h
 * \ingroup Itm
 * \brief Instrumentation Trace Macrocell (ITM) module public functionality
 *
 */

#ifndef ITM_PORT_H
#define ITM_PORT_H
/* ============================== INCLUDES ================================== */
#include "Itm_Types.h"                      /* Module types definition        */
/* ============================== TYPEDEFS ================================== */

/* ========================== SYMBOLIC CONSTANTS ============================ */

/* ========================== EXPORTED MACROS =============================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */

void                        Itm_Init                    ( itm_Config_t * const itmConfig );
void                        Itm_Deinit                  ( void );

void                        Itm_Set_TimestampEnable     ( void );
void                        Itm_Set_TimestampDisable    ( void );

void                        Itm_Set_SyncEnable          ( void );
void                        Itm_Set_SyncDisable         ( void );

void                        Itm_Transmit_8b             ( itm_Port_t itmPort, uint8_t data );
void                        Itm_Transmit_16b            ( itm_Port_t itmPort, uint16_t data );
void                        Itm_Transmit_32b            ( itm_Port_t itmPort, uint32_t data);

itm_FlagState_t             Itm_Get_TransmitState       ( itm_Port_t itmPort );
itm_FlagState_t             Itm_Get_BusyState           ( void );


#endif /* ITM_PORT_H */
