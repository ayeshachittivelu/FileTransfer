/*************************************************************************************************************
* Memory Allocator
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_meml.h
* Version :      1.0.0
* Product Name : MEML
* Device(s) :    R-Car
* Description :  Memory allocator Library header file
***********************************************************************************************************************/

/**
 * @file r_meml.h
 */

#ifndef R_MEML_H
#define R_MEML_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*******************************************************************************************************************//**
 * @ingroup middleware_memalloc
 * @defgroup MEML_DEFINITIONS MEML definitions
 * Definitions for MEML API
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * XOS2_OSAL_NOS_CD_CD_MEM_010
 * [Covers: XOS2_OSAL_NOS_UD_DD_MEM_013]
***********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @def MEML_VERSION_MAJOR
 * Memory Allocator Library major version
***********************************************************************************************************************/
#define MEML_VERSION_MAJOR   (1U)

/***********************************************************************************************************************
 * XOS2_OSAL_NOS_CD_CD_MEM_011
 * [Covers: XOS2_OSAL_NOS_UD_DD_MEM_013]
***********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @def MEML_VERSION_MINOR
 * Memory Allocator Library minor version
***********************************************************************************************************************/
#define MEML_VERSION_MINOR   (0U)

/***********************************************************************************************************************
 * XOS2_OSAL_NOS_CD_CD_MEM_012
 * [Covers: XOS2_OSAL_NOS_UD_DD_MEM_013]
***********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @def MEML_VERSION_PATCH
 * Memory Allocator Library patch version
***********************************************************************************************************************/
#define MEML_VERSION_PATCH   (0U)

/** @} */

/*******************************************************************************************************************//**
 * @ingroup middleware_memalloc
 * @defgroup MEML_GENERIC_ENUM  MEML Generic Enumeration
 * Enumeration for MEML API
 * @{
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @enum e_meml_return_t
 * @brief meml return value.
***********************************************************************************************************************/
typedef enum
{
    MEML_RETURN_OK = 0,             /*!< correct function execution */
    MEML_RETURN_PAR,                /*!< invalid input parameter */
    MEML_RETURN_FAIL,               /*!< internal non-categorized failure */
} e_meml_return_t;

/***********************************************************************************************************************
 * XOS2_OSAL_NOS_CD_CD_MEM_013
 * [Covers: XOS2_OSAL_NOS_UD_DD_MEM_012]
***********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @enum e_meml_alloc_mode_t
 * @brief meml allocation mode values
***********************************************************************************************************************/
typedef enum
{
    MEML_ALLOC_MODE_INVALID = 0,    /*!< Invalid/error value */
    MEML_ALLOC_MODE_STACK,          /*!< Stack Allocation mode */
    MEML_ALLOC_MODE_FREE_LIST,      /*!< FreeList(BestFit) Allocation mode. */
} e_meml_alloc_mode_t;

/** @} */

/*******************************************************************************************************************//**
 * @ingroup middleware_memalloc
 * @defgroup MEML_GENERIC_ST  MEML Generic Struct
 * Struct for MEML API
 * @{
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @struct st_meml_buffer_t
 * @brief Buffer handle for Memory Allocator Library
***********************************************************************************************************************/
typedef struct st_meml_buffer
{
    uintptr_t addr;                 /*!< The address of allocated memory. */
    size_t size;                    /*!< The size of allocated memory. */
    size_t remain;                  /*!< The size of the until next allocated memory. */
    struct st_meml_buffer *p_prev;  /*!< The pointer of the previous memory buffer structure. */
    struct st_meml_buffer *p_next;  /*!< The pointer of the next memory buffer structure. */
} st_meml_buffer_t;


/*******************************************************************************************************************//**
 * @struct st_meml_manage_t
 * @brief Handle for Memory Allocator Library
***********************************************************************************************************************/
typedef struct st_meml_manage
{
    uintptr_t top_addr;             /*!< The top address of memory manager
                                    This value is set in R_MEML_Initialize with the starting address of the allocated
                                    contiguous memory area */
    uintptr_t physical_addr;        /*!< The physical address of memory manager
                                    This value is set in R_MEML_Initialize with the starting physical address of the
                                    allocated contiguous memory area */
    size_t total_size;              /*!< The size of the memory manager.
                                    This value is set in R_MEML_Initialize with the size of the allocated
                                    contiguous memory area */
    uintptr_t (*p_allocate)(struct st_meml_manage * const, size_t, size_t); /*!< The function pointer for allocating. */
    e_meml_return_t (*p_deallocate)(struct st_meml_manage * const, uintptr_t); /*!< The function pointer for deallocating. */
    st_meml_buffer_t buffer;        /*!< The link to HEAD of memory buffer. */
} st_meml_manage_t;

/*******************************************************************************************************************//**
 * @struct st_meml_version_t
 * @brief Version for Memory Allocator Library
***********************************************************************************************************************/
/* PRQA S 3630 2 # This definition must be published because the user references the member after calling */
/* R_MEML_GetVersion, verified OK. */
typedef struct
{
    uint32_t major;      /*!< major version */
    uint32_t minor;      /*!< minor version */
    uint32_t patch;      /*!< patch version */
} st_meml_version_t;
/** @} */

/******************************************************************************************************************//**
 * @defgroup MEMORY_ALLOCATOR_LIBRARY_API Memory Allocator Library API
 * @ingroup middleware_memalloc
 * @{
 *********************************************************************************************************************/

/***********************************************************************************************************************
* XOS2_OSAL_NOS_CD_CD_MEM_014
* [Covers: XOS2_OSAL_NOS_UD_DD_MEM_010]
***********************************************************************************************************************/
/*******************************************************************************************************************//**
* @brief            Initialize the Memory Allocator library with the allocated memory and allocation mode.
* @param[in,out]    p_manage                Handle for Memory Allocator Library
* @param[in]        pa                      Start phisical address of allocated continuous memory area.
                                            This pointer value should be able to HW access.
* @param[in]        va                      Start logical address of allocated continuous memory area.
                                            This pointer value should be able to CPU access.
* @param[in]        size                    Size of allocated continuous memory area.
* @param[in]        mode                    Allocation algorithm.
* @return           e_meml_return_t
* @retval           MEML_RETURN_OK          on success
* @retval           MEML_RETURN_PAR         invalid parameter
* @note             - Pleaes allocate the manage structure at the coller.
* @see
*                   - st_meml_manage_t
*                   - e_meml_alloc_mode_t
***********************************************************************************************************************/
e_meml_return_t R_MEML_Initialize(st_meml_manage_t* const p_manage, uintptr_t pa, uintptr_t va, size_t size,
                                  e_meml_alloc_mode_t mode);

/*******************************************************************************************************************//**
* @brief            Deinitialize Memory Allocator Library.
* @param[in,out]    p_manage                Handle for Memory Allocator Library
* @return           e_meml_return_t
* @retval           MEML_RETURN_OK          on success
* @retval           MEML_RETURN_FAIL        not success
* @see              R_MEML_Initialize()
***********************************************************************************************************************/
e_meml_return_t R_MEML_Deinitialize(st_meml_manage_t* const p_manage);

/*******************************************************************************************************************//**
* @brief            Allocate according to the argument from the memory area given by R_MEML_Initialize.
* @param[in,out]    p_manage                Handle for Memory Allocator Library
* @param[in]        size                    Request size [n*minimumCachelineSize(64Byte) ... maxBufferSize n = 1,2,3..]
* @param[in]        alignment               Alignment of the memory [range: 2^6, 2^7, ..., 2^n, with n is limited to
                                            the size at initialization].
* @return           uintptr_t
* @retval           Non-NULL                Allocated memory address
* @retval           NULL                    Not success.
                                            If alignment is unacceptable, or free space is less than the required,
                                            or not initialized.
* @note             - The memory allocate algorithm is specified by R_MEML_Initialize.
*                   - For each allocate, The Memory Allocator Library uses "sizeof(st_meml_buffer_t)" bytes of
                      space from the contiguous area given at initialization.
* @see              R_MEML_Initialize()
***********************************************************************************************************************/
uintptr_t R_MEML_Alloc(st_meml_manage_t* const p_manage, size_t size, size_t alignment);

/*******************************************************************************************************************//**
* @brief            Deallocate the specified memory area.
* @param[in,out]    p_manage                Handle for Memory Allocator Library
* @param[in]        address                 Address obtained by R_MEML_Alloc().
* @return           e_meml_return_t
* @retval           MEML_RETURN_OK          on success
* @retval           MEML_RETURN_PAR         invalid parameter
* @see              R_MEML_Alloc()
***********************************************************************************************************************/
e_meml_return_t R_MEML_Dealloc(st_meml_manage_t* const p_manage, uintptr_t address);

/*******************************************************************************************************************//**
* @brief            Return Memory Allocator library verion.
* @return           const st_meml_version_t*
* @retval           version structure
* @see              none
***********************************************************************************************************************/
const st_meml_version_t* R_MEML_GetVersion(void);

/** @} MEMORY_ALLOCATOR_LIBRARY_API */

#endif /* R_MEML_H */

