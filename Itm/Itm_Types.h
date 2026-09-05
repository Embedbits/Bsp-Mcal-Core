/**
 * \author Mr.Nobody
 * \file Itm_Types.h
 * \ingroup Itm
 * \brief Instrumentation Trace Macrocell (ITM) module global types definition
 *
 */

#ifndef ITM_TYPES_H
#define ITM_TYPES_H
/* ============================== INCLUDES ================================== */
#include "stdint.h"                         /* Module types definition        */
/* ========================== SYMBOLIC CONSTANTS ============================ */

/** Null pointer definition */
#define ITM_NULL_PTR                    ( ( void* ) 0u )

/* ============================== TYPEDEFS ================================== */

/** \brief Type signaling major version of SW module */
typedef uint8_t itm_MajorVersion_t;


/** \brief Type signaling minor version of SW module */
typedef uint8_t itm_MinorVersion_t;


/** \brief Type signaling patch version of SW module */
typedef uint8_t itm_PatchVersion_t;


/** \brief Type signaling actual version of SW module */
typedef struct
{
    itm_MajorVersion_t Major; /**< Major version */
    itm_MinorVersion_t Minor; /**< Minor version */
    itm_PatchVersion_t Patch; /**< Patch version */
}   itm_ModuleVersion_t;


/** Function status enumeration */
typedef enum
{
    ITM_FUNCTION_INACTIVE = 0u, /**< Function status is inactive */
    ITM_FUNCTION_ACTIVE         /**< Function status is active   */
}   itm_FunctionState_t;


/** Flag states enumeration */
typedef enum
{
    ITM_FLAG_INACTIVE = 0u, /**< Inactive flag state */
    ITM_FLAG_ACTIVE         /**< Active flag state   */
}   itm_FlagState_t;


/** Enumeration used to signal request processing state */
typedef enum
{
    ITM_REQUEST_ERROR = 0u, /**< Processing request failed  */
    ITM_REQUEST_OK          /**< Processing request succeed */
}   itm_RequestState_t;


/** Frequency values type represented in Hz */
typedef uint32_t itm_FreqHz_t;


/** \brief Type signaling Instruction Trace MMacrocell (ITM) communication ports */
typedef enum
{
    ITM_PORT_0 = 0, /**< ITM Port 0      */
    ITM_PORT_1,     /**< ITM Port 1      */
    ITM_PORT_2,     /**< ITM Port 2      */
    ITM_PORT_3,     /**< ITM Port 3      */
    ITM_PORT_4,     /**< ITM Port 4      */
    ITM_PORT_5,     /**< ITM Port 5      */
    ITM_PORT_6,     /**< ITM Port 6      */
    ITM_PORT_7,     /**< ITM Port 7      */
    ITM_PORT_8,     /**< ITM Port 8      */
    ITM_PORT_9,     /**< ITM Port 9      */
    ITM_PORT_10,    /**< ITM Port 10     */
    ITM_PORT_11,    /**< ITM Port 11     */
    ITM_PORT_12,    /**< ITM Port 12     */
    ITM_PORT_13,    /**< ITM Port 13     */
    ITM_PORT_14,    /**< ITM Port 14     */
    ITM_PORT_15,    /**< ITM Port 15     */
    ITM_PORT_16,    /**< ITM Port 16     */
    ITM_PORT_17,    /**< ITM Port 17     */
    ITM_PORT_18,    /**< ITM Port 18     */
    ITM_PORT_19,    /**< ITM Port 19     */
    ITM_PORT_20,    /**< ITM Port 20     */
    ITM_PORT_21,    /**< ITM Port 21     */
    ITM_PORT_22,    /**< ITM Port 22     */
    ITM_PORT_23,    /**< ITM Port 23     */
    ITM_PORT_24,    /**< ITM Port 24     */
    ITM_PORT_25,    /**< ITM Port 25     */
    ITM_PORT_26,    /**< ITM Port 26     */
    ITM_PORT_27,    /**< ITM Port 27     */
    ITM_PORT_28,    /**< ITM Port 28     */
    ITM_PORT_29,    /**< ITM Port 29     */
    ITM_PORT_30,    /**< ITM Port 30     */
    ITM_PORT_31,    /**< ITM Port 31     */
    ITM_PORT_CNT    /**< ITM Ports count */
}   itm_Port_t;


typedef struct
{
    itm_Port_t          PortId;         /**< ITM communication port identification             */
    itm_FunctionState_t TimestampState; /**< Timestamp function activation/de-activation state */
    itm_FunctionState_t SyncState;      /**< Sync function activation/de-activation state      */
    itm_FreqHz_t        SwoClk;         /**< SWO communication clock in Hz                     */
}   itm_Config_t;


/* ========================== EXPORTED MACROS =============================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */


#endif /* ITM_TYPES_H */
