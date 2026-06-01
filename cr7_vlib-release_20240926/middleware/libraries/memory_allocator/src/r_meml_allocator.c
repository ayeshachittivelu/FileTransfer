/*************************************************************************************************************
* Memory Allocator
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_meml_allocator.c
* Version :      1.0.0
* Product Name : MEML
* Device(s) :    R-Car
* Description :  Memory Allocator Library
***********************************************************************************************************************/

/**
 * @file r_meml_allocator.c
 */

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "rcar-xos/memory_allocator/r_meml.h"

#if defined(R_MEML_ENABLE_DEBUG) /* DEFINED_DEBUG */
#include "r_print_api.h"
#endif

/*******************************************************************************************************************//**
 * @ingroup MEMORY_ALLOCATOR_LIBRARY
 * @defgroup MEMORY_ALLOCATOR_LIBRARY_Private_Defines Private macro definitions
 *
 * @{
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @def MEML_DEBUG
 * debug print.
***********************************************************************************************************************/
#if defined(R_MEML_ENABLE_DEBUG) /* DEFINED_DEBUG */
#define MEML_DEBUG(...) R_PRINT_Log(__VA_ARGS__);
#else
#define MEML_DEBUG(...)
#endif

/*******************************************************************************************************************//**
 * @def MEML_STATIC
***********************************************************************************************************************/
#define MEML_STATIC static

/** @} */

/*******************************************************************************************************************//**
 * global variable
***********************************************************************************************************************/


/******************************************************************************************************************//**
 * @ingroup MEMORY_ALLOCATOR_LIBRARY
 * @defgroup MEMORY_ALLOCATOR_LIBRARY_Private_Functions Private function definitions for Memory Allocator Library
 *
 * @{
 *********************************************************************************************************************/

/*******************************************************************************************************************//**
* @brief            Stack allocation from the memory area given by R_MEML_Initialize..
* @param[in,out]    p_manage                Handle for Memory Allocator Library
* @param[in]        size                    Request size [n*minimumCachelineSize(64Byte) ... maxBufferSize n = 1,2,3..]
* @param[in]        alignment               Alignment of the memory [range: 2^6, 2^7, ..., 2^n, with n is limited to
                                            the size at initialization].
* @return           uintptr_t
* @retval           Non-NULL                Allocated memory address
* @retval           NULL                    Not success.
                                            If alignment is unacceptable, or free space is less than the required,
                                            or not initialized.
* @note             The memory allocate algorithm is specified by R_MEML_Initialize.
* @see              R_MEML_Alloc()
***********************************************************************************************************************/
MEML_STATIC uintptr_t memory_lib_internal_alloc_stack(st_meml_manage_t* const p_manage, size_t size,
                                                      size_t alignment);

/*******************************************************************************************************************//**
* @brief            FreeList allocation from the memory area given by R_MEML_Initialize..
* @param[in,out]    p_manage                Handle for Memory Allocator Library
* @param[in]        size                    Request size [n*minimumCachelineSize(64Byte) ... maxBufferSize n = 1,2,3..]
* @param[in]        alignment               Alignment of the memory [range: 2^6, 2^7, ..., 2^n, with n is limited to
                                            the size at initialization].
* @return           uintptr_t
* @retval           Non-NULL                Allocated memory address
* @retval           NULL                    Not success.
                                            If alignment is unacceptable, or free space is less than the required,
                                            or not initialized.
* @note             The memory allocate algorithm is specified by R_MEML_Initialize.
* @see              R_MEML_Alloc()
***********************************************************************************************************************/
MEML_STATIC uintptr_t memory_lib_internal_alloc_freelist(st_meml_manage_t* const p_manage, size_t size,
                                                         size_t alignment);
/*******************************************************************************************************************//**
* @brief            Deallocate the specified memory area.
* @param[in,out]    p_manage                Handle for Memory Allocator Library
* @param[in]        address                 Address obtained by R_MEML_Alloc().
* @return           e_meml_return_t
* @retval           MEML_RETURN_OK          on success
* @retval           MEML_RETURN_PAR         invalid parameter
* @note             Only for Stack allocation.
* @see              R_MEML_Dealloc()
***********************************************************************************************************************/
MEML_STATIC e_meml_return_t memory_lib_internal_dealloc_stack(st_meml_manage_t* const p_manage, uintptr_t address);

/*******************************************************************************************************************//**
* @brief            Deallocate the specified memory area.
* @param[in,out]    p_manage                Handle for Memory Allocator Library
* @param[in]        address                 Address obtained by R_MEML_Alloc().
* @return           e_meml_return_t
* @retval           MEML_RETURN_OK          on success
* @retval           MEML_RETURN_PAR         invalid parameter
* @note             Only for FreeList allocation.
* @see              R_MEML_Dealloc()
***********************************************************************************************************************/
MEML_STATIC e_meml_return_t memory_lib_internal_dealloc_freelist(st_meml_manage_t* const p_manage,
                                                                 uintptr_t address);

/** @} */

/***********************************************************************************************************************
* Start of function R_MEML_Initialize()
* XOS2_OSAL_NOS_CD_CD_MEM_001
* [Covers: XOS2_OSAL_NOS_UD_DD_MEM_005]
* [Covers: XOS2_OSAL_NOS_UD_DD_MEM_010]
***********************************************************************************************************************/
e_meml_return_t R_MEML_Initialize(st_meml_manage_t* const p_manage, uintptr_t pa, uintptr_t va, size_t size,
                                  e_meml_alloc_mode_t mode)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_meml_return_t ret = MEML_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
    if ((NULL == p_manage) || (NULL == (void *)pa) || (NULL == (void *)va) || (0U == size))
    {
        ret = MEML_RETURN_PAR;
    }
#if 0
    else if (NULL != manage->buffer)
    {
        if ((NULL != manage->buffer->next) &&
            (NULL != manage->allocate) &&
            (NULL != manage->deallocate))
        {
            ;   /* Aleady initialized, but initial value of the CMEM area is 0? */
        }
    }
#endif
    else
    {
        p_manage->p_allocate = NULL;
        p_manage->p_deallocate = NULL;

        /* check allocation mode */
        if (MEML_ALLOC_MODE_STACK == mode)
        {
            MEML_DEBUG("MEML set STACK\n");
            /* set stack algorithm function */
            p_manage->p_allocate = memory_lib_internal_alloc_stack;
            p_manage->p_deallocate = memory_lib_internal_dealloc_stack;
        }
        else if (MEML_ALLOC_MODE_FREE_LIST == mode)
        {
            MEML_DEBUG("MEML set LIST\n");
            /* set free list algorithm function */
            p_manage->p_allocate = memory_lib_internal_alloc_freelist;
            p_manage->p_deallocate = memory_lib_internal_dealloc_freelist;
        }
        else
        {
            ret = MEML_RETURN_PAR;  /* invalid mode */
        }
    }

    if (MEML_RETURN_OK == ret)
    {
        /* initialize manager object */
        p_manage->top_addr = va;
        p_manage->physical_addr = pa;
        p_manage->total_size = size;

        /* initialize root buffer object */
        p_manage->buffer.addr = p_manage->top_addr;
        p_manage->buffer.size = 0;
        p_manage->buffer.remain = p_manage->total_size;
        p_manage->buffer.p_prev = &(p_manage->buffer);
        p_manage->buffer.p_next = NULL;

        MEML_DEBUG("MEML buffer(root) addr(%p) size(%zu) remain(%zu) prev(%p) next(%p)\n",
                    (void *)p_manage->buffer.addr,
                    p_manage->buffer.size,
                    p_manage->buffer.remain,
                    (void *)p_manage->buffer.p_prev,
                    (void *)p_manage->buffer.p_next);
    }

    MEML_DEBUG("MEML return R_MEML_Initialize(%d)\n", ret);

    return ret;
}
/***********************************************************************************************************************
* End of function R_MEML_Initialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_MEML_Deinitialize()
* XOS2_OSAL_NOS_CD_CD_MEM_002
* [Covers: XOS2_OSAL_NOS_UD_DD_MEM_006]
***********************************************************************************************************************/
e_meml_return_t R_MEML_Deinitialize(st_meml_manage_t* const p_manage)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_meml_return_t ret = MEML_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == p_manage)
    {
        ret = MEML_RETURN_PAR;
    }
    else if ((NULL != p_manage->buffer.p_next) &&
            (NULL != p_manage->p_allocate) &&
            (NULL != p_manage->p_deallocate))
    {
        ret = MEML_RETURN_FAIL;
        ;   /* Aleady initialized, but initial value of the CMEM area is 0? */
    }
    else
    {
        ; /* do nothing. */
    }

    if (MEML_RETURN_OK == ret)
    {
        /* deinitialize root buffer object */
        /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
        p_manage->buffer.addr = (uintptr_t)NULL;
        /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
        p_manage->physical_addr = (uintptr_t)NULL;
        p_manage->buffer.size = 0;
        p_manage->buffer.remain = 0;
        p_manage->buffer.p_prev = NULL;
        p_manage->buffer.p_next = NULL;

        /* deinitialize manager object */
        /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
        p_manage->top_addr = (uintptr_t)NULL;
        p_manage->total_size = 0;
        p_manage->p_allocate = NULL;
        p_manage->p_deallocate = NULL;
    }

    MEML_DEBUG("MEML return R_MEML_Deinitialize(%d)\n", ret);

    return ret;
}
/***********************************************************************************************************************
* End of function R_MEML_Deinitialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_MEML_Alloc()
* XOS2_OSAL_NOS_CD_CD_MEM_003
* [Covers: XOS2_OSAL_NOS_UD_DD_MEM_007]
* [Covers: XOS2_OSAL_NOS_UD_DD_MEM_011]
***********************************************************************************************************************/
uintptr_t R_MEML_Alloc(st_meml_manage_t* const p_manage, size_t size, size_t alignment)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    uintptr_t ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/

    if ((NULL == p_manage) || (NULL == p_manage->p_allocate) || (0U == size))
    {
        /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
        ret = (uintptr_t)NULL;
    }
    else if ((64U > size) || (0U != (size % 64U)))
    {
        /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
        ret = (uintptr_t)NULL;
    }
    else if ((64U > alignment) || (0U != (alignment & (alignment - 1U))))
    {
        /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
        ret = (uintptr_t)NULL;
    }
    else
    {
        /* call allocate algorithm */
        ret = p_manage->p_allocate(p_manage, size, alignment);  /* already set at initialization */
    }

    MEML_DEBUG("MEML return memory_lib_alloc(%p)\n", (void *)ret);
    return ret;
}
/***********************************************************************************************************************
* End of function R_MEML_Alloc()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_MEML_Dealloc()
* XOS2_OSAL_NOS_CD_CD_MEM_004
* [Covers: XOS2_OSAL_NOS_UD_DD_MEM_008]
* [Covers: XOS2_OSAL_NOS_UD_DD_MEM_011]
***********************************************************************************************************************/
e_meml_return_t R_MEML_Dealloc(st_meml_manage_t* const p_manage, uintptr_t address)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_meml_return_t ret = MEML_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if ((NULL == p_manage) || (NULL == p_manage->p_deallocate))
    {
        ret = MEML_RETURN_PAR;
    }
    else
    {
        /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
        if (NULL != (void *)address) /* When address is NULL, do nothing. */
        {
            /* call deallocate algorithm */
            ret = p_manage->p_deallocate(p_manage, address);  /* already set at initialization */
        }
    }

    MEML_DEBUG("MEML return R_MEML_Dealloc(%d)\n", ret);

    return ret;
}
/***********************************************************************************************************************
* End of function R_MEML_Dealloc()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function memory_lib_internal_alloc_stack()
* XOS2_OSAL_NOS_CD_CD_MEM_005
* [Covers: XOS2_OSAL_NOS_UD_DD_MEM_002]
***********************************************************************************************************************/
MEML_STATIC uintptr_t memory_lib_internal_alloc_stack (st_meml_manage_t * const p_manage, size_t size,
                                                       size_t alignment)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
    uintptr_t ret = (uintptr_t)NULL;
    st_meml_buffer_t *p_local_buffer;
    st_meml_buffer_t *p_new_buffer;
    size_t align_offset;
    uintptr_t next_addr;
    uintptr_t next_physical_addr;
#if defined(R_MEML_ENABLE_DEBUG) /* DEFINED_DEBUG */
    uint32_t cnt = 0;
#endif

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    MEML_DEBUG("MEML start memory_lib_internal_alloc_stack manage:%p size:%zu align:%zu\n",
                (void *)p_manage, size, alignment);

    if ((NULL == p_manage) || (0U == size) || (NULL == p_manage->buffer.p_prev))
    {
        /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
        ret = (uintptr_t)NULL;
    }
    else
    {
        p_local_buffer = p_manage->buffer.p_prev;
        /* get tail */
        if (NULL == p_manage->buffer.p_next)
        {
            next_addr = (uintptr_t)p_manage->buffer.addr;
        }
        else
        {
            /* PRQA S 0306 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
            next_addr = (uintptr_t)&p_local_buffer[1];
        }

        next_physical_addr = p_manage->physical_addr + (next_addr - p_manage->top_addr);

        align_offset = 0;
        if (0U != alignment)
        {
            align_offset = ((next_physical_addr + alignment - 1U) & ~ (alignment - 1U)) - next_physical_addr;
        }

        /* check remain size */
        if (p_local_buffer->remain >= (size + align_offset + sizeof(st_meml_buffer_t)))
        {
            /* allocate */
            /* PRQA S 0306 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
            p_new_buffer = (st_meml_buffer_t *)(next_addr + align_offset + size);

            p_new_buffer->addr = next_addr + align_offset;
            p_new_buffer->size = size;
            p_new_buffer->remain = p_local_buffer->remain -
                                   (align_offset + size + sizeof(st_meml_buffer_t)); /* Reduce remaining area */
            p_new_buffer->p_next = NULL;
            p_new_buffer->p_prev = p_local_buffer;

            /* update list */
            p_local_buffer->remain = align_offset;
            p_local_buffer->p_next = p_new_buffer;

            /* Replace new tail */
            /* PRQA S 2916 2 # "p_new_buffer" is a pointer to a continuous memory area that lifetime of */
            /* the continuous area is persistent, verified OK. */
            p_manage->buffer.p_prev = p_new_buffer;

            ret = (uintptr_t)p_new_buffer->addr;
        }

#if defined(R_MEML_ENABLE_DEBUG) /* DEFINED_DEBUG */
        cnt = 0;
        p_local_buffer = &(p_manage->buffer);

        MEML_DEBUG("MEML buffer(root %p) addr(%p) size(%zu) remain(%zu) prev(%p) next(%p)\n",
            (void *)p_local_buffer,
            (void *)p_local_buffer->addr,
            p_local_buffer->size,
            p_local_buffer->remain,
            (void *)p_local_buffer->p_prev,
            (void *)p_local_buffer->p_next);
        p_local_buffer = p_local_buffer->p_next;

        while(NULL != p_local_buffer)
        {
            MEML_DEBUG("MEML buffer(%d %p) addr(%p) size(%zu) remain(%zu) prev(%p) next(%p)\n",
                cnt++,
                (void *)p_local_buffer,
                (void *)p_local_buffer->addr,
                p_local_buffer->size,
                p_local_buffer->remain,
                (void *)p_local_buffer->p_prev,
                (void *)p_local_buffer->p_next);
            p_local_buffer = p_local_buffer->p_next;
        }
#endif
    }

    MEML_DEBUG("MEML return memory_lib_internal_alloc_stack(%p)\n", (void *)ret);
    return ret;
}
/***********************************************************************************************************************
* End of function memory_lib_internal_alloc_stack()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function memory_lib_internal_alloc_freelist()
* XOS2_OSAL_NOS_CD_CD_MEM_006
* [Covers: XOS2_OSAL_NOS_UD_DD_MEM_001]
***********************************************************************************************************************/
MEML_STATIC uintptr_t memory_lib_internal_alloc_freelist (st_meml_manage_t* const p_manage, size_t size,
                                                          size_t alignment)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
    uintptr_t ret = (uintptr_t)NULL;
    st_meml_buffer_t *p_local_buffer;
    st_meml_buffer_t *p_new_buffer = NULL;
    st_meml_buffer_t *p_prev_buffer = NULL;
    /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
    uintptr_t next_addr = (uintptr_t)NULL;
    uintptr_t next_physical_addr;
    size_t align_offset = 0;
    uintptr_t next_addr_tmp;
    size_t align_offset_tmp;
    size_t remain_min;
#if defined(R_MEML_ENABLE_DEBUG) /* DEFINED_DEBUG */
    uint32_t cnt = 0;
#endif

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    MEML_DEBUG("MEML start memory_lib_internal_alloc_freelist manage:%p size:%zu align:%zu\n",
                (void *)p_manage, size, alignment);

    if ((NULL == p_manage) || (0U == size))
    {
        /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
        ret = (uintptr_t)NULL;
    }
    else
    {
        remain_min = p_manage->total_size + 1U;
        p_local_buffer = &(p_manage->buffer);

        /* check remain size and proceed to tail */
        while(NULL != p_local_buffer)
        {
            if (p_local_buffer->p_prev == p_local_buffer)
            {
                next_addr_tmp = (uintptr_t)p_manage->buffer.addr;
            }
            else
            {
                /* PRQA S 0306 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
                next_addr_tmp = (uintptr_t)&p_local_buffer[1];
            }

            next_physical_addr = p_manage->physical_addr + (next_addr_tmp - p_manage->top_addr);

            /* There is no problem because intptr_t is a type that represents a pointer type as an integer. */
            align_offset_tmp = 0;
            if (0U != alignment)
            {
                align_offset_tmp = ((next_physical_addr + alignment - 1U) & ~ (alignment - 1U)) - next_physical_addr;
            }

            /* check remain size */
            if (p_local_buffer->remain >= (size + align_offset_tmp + sizeof(st_meml_buffer_t)))
            {
                /* check mininum remain size */
                if (remain_min > p_local_buffer->remain)
                {
                    next_addr = next_addr_tmp;
                    align_offset = align_offset_tmp;
                    /* Update candidate */
                    /* PRQA S 0306 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
                    p_new_buffer = (st_meml_buffer_t *)(next_addr + align_offset + size);
                    remain_min = p_local_buffer->remain;  /* Update remain_min */
                    p_prev_buffer = p_local_buffer;
                }
            }

            p_local_buffer = p_local_buffer->p_next;
        }

        /* find? */
        if (NULL != p_new_buffer)
        {
            /* allocate */
            p_new_buffer->addr = next_addr + align_offset;
            p_new_buffer->size = size;
            p_new_buffer->remain = p_prev_buffer->remain -
                                   (align_offset + size + sizeof(st_meml_buffer_t)); /* Reduce remaining area */
            p_new_buffer->p_next = p_prev_buffer->p_next;

            /* update list */
            p_prev_buffer->remain = align_offset;
            p_prev_buffer->p_next = p_new_buffer;

            ret = (uintptr_t)p_new_buffer->addr;
        }

#if defined(R_MEML_ENABLE_DEBUG) /* DEFINED_DEBUG */
        MEML_DEBUG("MEML p_prev_buffer(%p) p_prev_buffer(%p)\n", (void *)p_prev_buffer, (void *)p_new_buffer);

        cnt = 0;
        p_local_buffer = &(p_manage->buffer);

        MEML_DEBUG("MEML buffer(root %p) addr(%p) size(%zu) remain(%zu) prev(%p) next(%p)\n",
            (void *)p_local_buffer,
            (void *)p_local_buffer->addr,
            p_local_buffer->size,
            p_local_buffer->remain,
            (void *)p_local_buffer->p_prev,
            (void *)p_local_buffer->p_next);
        p_local_buffer = p_local_buffer->p_next;

        while(NULL != p_local_buffer)
        {
            MEML_DEBUG("MEML buffer(%d %p) addr(%p) size(%zu) remain(%zu) prev(%p) next(%p)\n",
                cnt++,
                (void *)p_local_buffer,
                (void *)p_local_buffer->addr,
                p_local_buffer->size,
                p_local_buffer->remain,
                (void *)p_local_buffer->p_prev,
                (void *)p_local_buffer->p_next);
            p_local_buffer = p_local_buffer->p_next;
        }
#endif
    }

    MEML_DEBUG("MEML return memory_lib_internal_alloc_freelist(%p)\n", (void *)ret);
    return ret;
}
/***********************************************************************************************************************
* End of function memory_lib_internal_alloc_freelist()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function memory_lib_internal_dealloc_stack()
* XOS2_OSAL_NOS_CD_CD_MEM_007
* [Covers: XOS2_OSAL_NOS_UD_DD_MEM_004]
***********************************************************************************************************************/
MEML_STATIC e_meml_return_t memory_lib_internal_dealloc_stack(st_meml_manage_t* const p_manage, uintptr_t address)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_meml_return_t ret = MEML_RETURN_OK;
    st_meml_buffer_t *p_local_buffer;
    st_meml_buffer_t *p_del_buffer;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
    if ((NULL == p_manage) || (NULL == (void *)address) || (NULL == p_manage->buffer.p_next))
    {
        ret = MEML_RETURN_PAR;
    }
    else
    {
        p_del_buffer = p_manage->buffer.p_prev;
        p_local_buffer = p_del_buffer->p_prev;

        /* check deallocate target */
        if (address == p_del_buffer->addr)
        {
            /* deallocate */
            p_local_buffer->remain += (p_del_buffer->size + p_del_buffer->remain +
                                                            sizeof(st_meml_buffer_t)); /* Consolidate remaining area */

            /* update list */
            p_local_buffer->p_next = NULL;
            p_manage->buffer.p_prev = p_del_buffer->p_prev;
        }
        else
        {
            ret = MEML_RETURN_PAR;
        }

#if defined(R_MEML_ENABLE_DEBUG) /* DEFINED_DEBUG */
        uint32_t cnt = 0;
        p_local_buffer = &(p_manage->buffer);

        MEML_DEBUG("MEML buffer(root %p) addr(%p) size(%zu) remain(%zu) prev(%p) next(%p)\n",
            (void *)p_local_buffer,
            (void *)p_local_buffer->addr,
            p_local_buffer->size,
            p_local_buffer->remain,
            (void *)p_local_buffer->p_prev,
            (void *)p_local_buffer->p_next);
        p_local_buffer = p_local_buffer->p_next;

        while(NULL != p_local_buffer)
        {
            MEML_DEBUG("MEML buffer(%d %p) addr(%p) size(%zu) remain(%zu) prev(%p) next(%p)\n",
                cnt++,
                (void *)p_local_buffer,
                (void *)p_local_buffer->addr,
                p_local_buffer->size,
                p_local_buffer->remain,
                (void *)p_local_buffer->p_prev,
                (void *)p_local_buffer->p_next);
            p_local_buffer = p_local_buffer->p_next;
        }
#endif
    }

    MEML_DEBUG("MEML return R_MEML_Dealloc(%d)\n", ret);

    return ret;
}
/***********************************************************************************************************************
* End of function memory_lib_internal_dealloc_stack()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function memory_lib_internal_dealloc_freelist()
* XOS2_OSAL_NOS_CD_CD_MEM_008
* [Covers: XOS2_OSAL_NOS_UD_DD_MEM_003]
***********************************************************************************************************************/
MEML_STATIC e_meml_return_t memory_lib_internal_dealloc_freelist(st_meml_manage_t* const p_manage,
                                                                 uintptr_t address)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_meml_return_t ret = MEML_RETURN_OK;
    st_meml_buffer_t *p_local_buffer;
    st_meml_buffer_t *p_del_buffer;
    st_meml_buffer_t *p_prev_buffer;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    /* PRQA S 0326 1 # intptr_t is a type that represents a pointer type as an integer, verified OK. */
    if ((NULL == p_manage) || (NULL == (void *)address) || (NULL == p_manage->buffer.p_next))
    {
        ret = MEML_RETURN_PAR;
    }
    else
    {
        p_local_buffer = &(p_manage->buffer);
        p_del_buffer = NULL;
        p_prev_buffer = p_local_buffer;

        /* find deallocate target */
        while (NULL != p_local_buffer->p_next)
        {
            if (address == p_local_buffer->addr)
            {
                if (p_local_buffer->p_prev == p_local_buffer) /* root? */
                {
                    p_del_buffer = p_local_buffer->p_next;  /* When the root, the beginning is the target */
                }
                else
                {
                    p_del_buffer = p_local_buffer;
                }
                break;
            }
            p_prev_buffer = p_local_buffer;
            p_local_buffer = p_local_buffer->p_next;
        }

        /* check list tail */
        if ((NULL == p_del_buffer ) && (address == p_local_buffer->addr))
        {
            p_del_buffer = p_local_buffer;
        }

        if (NULL != p_del_buffer )
        {
            /* deallocate */
            p_prev_buffer->remain += (p_del_buffer->size + p_del_buffer->remain +
                                                           sizeof(st_meml_buffer_t)); /* Consolidate remaining area */

            /* update list */
            p_prev_buffer->p_next = p_del_buffer->p_next;
        }
        else
        {
            ret = MEML_RETURN_PAR;
        }

#if defined(R_MEML_ENABLE_DEBUG) /* DEFINED_DEBUG */
        uint32_t cnt = 0;
        p_local_buffer = &(p_manage->buffer);

        MEML_DEBUG("MEML buffer(root %p) addr(%p) size(%zu) remain(%zu) prev(%p) next(%p)\n",
            (void *)p_local_buffer,
            (void *)p_local_buffer->addr,
            p_local_buffer->size,
            p_local_buffer->remain,
            (void *)p_local_buffer->p_prev,
            (void *)p_local_buffer->p_next);
        p_local_buffer = p_local_buffer->p_next;

        while(NULL != p_local_buffer)
        {
            MEML_DEBUG("MEML buffer(%d %p) addr(%p) size(%zu) remain(%zu) prev(%p) next(%p)\n",
                cnt++,
                (void *)p_local_buffer,
                (void *)p_local_buffer->addr,
                p_local_buffer->size,
                p_local_buffer->remain,
                (void *)p_local_buffer->p_prev,
                (void *)p_local_buffer->p_next);
            p_local_buffer = p_local_buffer->p_next;
        }
#endif
    }

    MEML_DEBUG("MEML return R_MEML_Dealloc(%d)\n", ret);

    return ret;
}
/***********************************************************************************************************************
* End of function memory_lib_internal_dealloc_freelist()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_MEML_GetVersion()
* XOS2_OSAL_NOS_CD_CD_MEM_009
* [Covers: XOS2_OSAL_NOS_UD_DD_MEM_009]
***********************************************************************************************************************/
const st_meml_version_t* R_MEML_GetVersion(void)
{
    static const st_meml_version_t version =
    {
        MEML_VERSION_MAJOR,
        MEML_VERSION_MINOR,
        MEML_VERSION_PATCH
    };

    return &version;
}
/***********************************************************************************************************************
* End of function R_MEML_GetVersion()
***********************************************************************************************************************/

