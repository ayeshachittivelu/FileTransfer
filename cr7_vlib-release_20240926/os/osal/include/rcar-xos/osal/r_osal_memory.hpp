/*************************************************************************************************************
* OSAL Common Header
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
 * @Version      :
 * @Description  :
 ***********************************************************************************************************************/

#ifndef R_OSAL_MEMORY_HPP
#define R_OSAL_MEMORY_HPP

#include "r_osal.h"
#include <iostream>

namespace renesas
{
namespace rcarxos
{
namespace osal
{

static constexpr st_osal_mmngr_config_t MMNGR_CONFIG_INIT_VALUE()
{
    return {OSAL_MMNGR_ALLOC_MODE_INVALID, 0, 0, 0};
}

#if !defined(NDEBUG) || defined(R_OSAL_ENABLE_DEBUG)
namespace dbg
{
inline std::ostream & operator<<(std::ostream & os, const e_osal_mmngr_event_t & obj)
{
    switch (obj)
    {
        case OSAL_MMNGR_EVENT_ALLOCATE:
            os << "OSAL_MMNGR_EVENT_ALLOCATE";
            break;
        case OSAL_MMNGR_EVENT_DEALLOCATE:
            os << "OSAL_MMNGR_EVENT_DEALLOCATE";
            break;
        case OSAL_MMNGR_EVENT_OVERFLOW_DETECTED:
            os << "OSAL_MMNGR_EVENT_OVERFLOW_DETECTED";
            break;
        case OSAL_MMNGR_EVENT_UNDERFLOW_DETECTED:
            os << "OSAL_MMNGR_EVENT_UNDERFLOW_DETECTED";
            break;
        case OSAL_MMNGR_EVENT_ERROR:
            os << "OSAL_MMNGR_EVENT_ERROR";
            break;
        default: /* should not be reached */
            os << "UNKOWN e_osal_mmngr_event_t value(" << static_cast<std::int64_t>(obj) << ")";
            break;
    }
    return os;
}

inline std::ostream & operator<<(std::ostream & os, const e_osal_mmngr_allocator_mode_t & obj)
{
    switch (obj)
    {
        case OSAL_MMNGR_ALLOC_MODE_STACK:
            os << "OSAL_MMNGR_ALLOC_MODE_STACK";
            break;
        case OSAL_MMNGR_ALLOC_MODE_STACK_PROT:
            os << "OSAL_MMNGR_ALLOC_MODE_STACK_PROT";
            break;
        case OSAL_MMNGR_ALLOC_MODE_FREE_LIST:
            os << "OSAL_MMNGR_ALLOC_MODE_FREE_LIST";
            break;
        case OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT:
            os << "OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT";
            break;
        case OSAL_MMNGR_ALLOC_MODE_UNSPECIFIED:
            os << "OSAL_MMNGR_ALLOC_MODE_UNSPECIFIED";
            break;
        case OSAL_MMNGR_ALLOC_MODE_UNSPECIFIED_PROT:
            os << "OSAL_MMNGR_ALLOC_MODE_UNSPECIFIED_PROT";
            break;
        default: /* should not be reached */
            os << "UNKOWN e_osal_mmngr_allocator_mode_t value(" << static_cast<std::int64_t>(obj) << ")";
            break;
    }
    return os;
}

inline std::ostream & operator<<(std::ostream & os, const e_osal_return_t & obj)
{
    switch (obj)
    {
        case OSAL_RETURN_OK:
            os << "OSAL_RETURN_OK";
            break;
        case OSAL_RETURN_PAR:
            os << "OSAL_RETURN_PAR";
            break;
        case OSAL_RETURN_MEM:
            os << "OSAL_RETURN_MEM";
            break;
        case OSAL_RETURN_DEV:
            os << "OSAL_RETURN_DEV";
            break;
        case OSAL_RETURN_TIME:
            os << "OSAL_RETURN_TIME";
            break;
        case OSAL_RETURN_CONF:
            os << "OSAL_RETURN_CONF";
            break;
        case OSAL_RETURN_FAIL:
            os << "OSAL_RETURN_FAIL";
            break;
        case OSAL_RETURN_IO:
            os << "OSAL_RETURN_IO";
            break;
        case OSAL_RETURN_BUSY:
            os << "OSAL_RETURN_BUSY";
            break;
        case OSAL_RETURN_ID:
            os << "OSAL_RETURN_ID";
            break;
        case OSAL_RETURN_HANDLE:
            os << "OSAL_RETURN_HANDLE";
            break;
        case OSAL_RETURN_STATE:
            os << "OSAL_RETURN_STATE";
            break;
        case OSAL_RETURN_PROHIBITED:
            os << "OSAL_RETURN_PROHIBITED";
            break;
        case OSAL_RETURN_UNSUPPORTED_OPERATION:
            os << "OSAL_RETURN_UNSUPPORTED_OPERATION";
            break;
        case OSAL_RETURN_HANDLE_NOT_SUPPORTED:
            os << "OSAL_RETURN_HANDLE_NOT_SUPPORTED";
            break;
        default: /* should not be reached */
            os << "UNKOWN e_osal_return_t value(" << static_cast<std::int64_t>(obj) << ")";
            break;
    }
    return os;
}

inline std::ostream & operator<<(std::ostream & os, const st_osal_mmngr_config_t & obj)
{
    os << "[mode: " << obj.mode << ", mem limit: " << (obj.memory_limit / 1024)
       << "KB, max # alloc: " << obj.max_allowed_allocations
       << ", max event monitors: " << obj.max_registered_monitors_cbs << "]";
    return os;
}
} /* namespace dbg */
#endif /* #if !defined(NDEBUG) || defined(R_OSAL_ENABLE_DEBUG) */
} /* namespace osal */
} /* namespace rcarxos */
} /* namespace renesas */

#endif /* R_OSAL_MEMORY_HPP */
