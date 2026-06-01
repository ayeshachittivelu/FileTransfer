/*************************************************************************************************************
* print_api_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#ifndef R_PRINT_API_H_
#define R_PRINT_API_H_

/**
 * Function R_PRINT_Log
 * @brief Output function
 *
 * @param[in] fmt - Format(must not be NULL)
 * @param[in] ... - Variable-length arguments
 */
void R_PRINT_Log(const char *fmt, ...);

#endif /* R_PRINT_API_H_ */
