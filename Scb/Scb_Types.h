/**
 * \author Mr.Nobody
 * \defgroup Scb Scb
 * \file Scb_Types.h
 * \ingroup Scb
 * \brief System Control Block (SCB) module global types definition
 *
 * This file contains the types definitions used across the module and are
 * available for other modules through Port file.
 *
 */

#ifndef SCB_SCB_TYPES_H
#define SCB_SCB_TYPES_H
/* ============================== INCLUDES ================================== */
#include "stdint.h"                         /* Module types definition        */
/* ========================== SYMBOLIC CONSTANTS ============================ */

/* ========================== EXPORTED MACROS =============================== */

/* ============================== TYPEDEFS ================================== */

/** \brief Type signaling major version of SW module */
typedef uint8_t scb_MajorVersion_t;


/** \brief Type signaling minor version of SW module */
typedef uint8_t scb_MinorVersion_t;


/** \brief Type signaling patch version of SW module */
typedef uint8_t scb_PatchVersion_t;


/** \brief Type signaling actual version of SW module */
typedef struct
{
    scb_MajorVersion_t Major; /**< Major version */
    scb_MinorVersion_t Minor; /**< Minor version */
    scb_PatchVersion_t Patch; /**< Patch version */
}   scb_ModuleVersion_t;


/** Function status enumeration */
typedef enum
{
    SCB_FUNCTION_INACTIVE = 0u, /**< Function status is inactive */
    SCB_FUNCTION_ACTIVE         /**< Function status is active   */
}   scb_FunctionState_t;

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */


#endif /* SCB_SCB_TYPES_H */
