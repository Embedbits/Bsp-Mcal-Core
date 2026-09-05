/**
 * \author Mr.Nobody
 * \file Dwt_Types.h
 * \ingroup Dwt
 * \brief Data Watchpoint and Trace Unit (DWT) module global types definition
 *
 * This file contains the types definitions used across the module and are 
 * available for other modules through Port file.
 *
 */

#ifndef DWT_DWT_TYPES_H
#define DWT_DWT_TYPES_H
/* ============================== INCLUDES ================================== */
#include "stdint.h"                         /* Module types definition        */
/* ========================== SYMBOLIC CONSTANTS ============================ */

/** Null pointer definition */
#define DWT_NULL_PTR                        ( ( void* ) 0u )

/* ========================== EXPORTED MACROS =============================== */

/* ============================== TYPEDEFS ================================== */

/** \brief Type signaling major version of SW module */
typedef uint8_t dwt_MajorVersion_t;


/** \brief Type signaling minor version of SW module */
typedef uint8_t dwt_MinorVersion_t;


/** \brief Type signaling patch version of SW module */
typedef uint8_t dwt_PatchVersion_t;


/** \brief Type signaling actual version of SW module */
typedef struct
{
    dwt_MajorVersion_t Major; /**< Major version */
    dwt_MinorVersion_t Minor; /**< Minor version */
    dwt_PatchVersion_t Patch; /**< Patch version */
}   dwt_ModuleVersion_t;


/** Function status enumeration */
typedef enum
{
    DWT_FUNCTION_INACTIVE = 0u, /**< Function status is inactive */
    DWT_FUNCTION_ACTIVE         /**< Function status is active   */
}   dwt_FunctionState_t;


/** Enumeration used to signal request processing state */
typedef enum
{
    DWT_REQUEST_ERROR = 0u, /**< Processing request failed  */
    DWT_REQUEST_OK          /**< Processing request succeed */
}   dwt_RequestState_t;


/** Flag states enumeration */
typedef enum
{
    DWT_FLAG_INACTIVE = 0u, /**< Inactive flag state */
    DWT_FLAG_ACTIVE         /**< Active flag state   */
}   dwt_FlagState_t;


/** \brief Type used for DWT counter values */
typedef uint32_t dwt_CounterValue_t;


/**
 * \brief DWT configuration struct
 */
typedef struct
{
    dwt_FunctionState_t CycleCounterState;     /**< Cycle counter activation/de-activation                        */
    dwt_FunctionState_t CpiCounterState;       /**< Cycles Per Instruction (CPI) counter activation/de-activation */
    dwt_FunctionState_t ExceptionCounterState; /**< Exception counter activation/de-activation                    */
    dwt_FunctionState_t SleepCounterState;     /**< Sleep counter activation/de-activation                        */
    dwt_FunctionState_t LoadStoreCounterState; /**< Load-Store Counter LSU counter activation/de-activation       */
    dwt_FunctionState_t FoldCounterState;      /**< Folded instruction counter activation/de-activation           */
}   dwt_Config_t;

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */


#endif /* DWT_DWT_TYPES_H */
