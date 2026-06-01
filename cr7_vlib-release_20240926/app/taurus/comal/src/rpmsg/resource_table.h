/*************************************************************************************************************
* resource_table.h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#if defined (LINUX_GUEST)
#include "resource_table_linux.h"
#elif defined (INTEGRITY_GUEST)
#include "resource_table_integrity.h"
#endif
