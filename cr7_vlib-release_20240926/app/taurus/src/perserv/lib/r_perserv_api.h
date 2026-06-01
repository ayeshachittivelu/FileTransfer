/*************************************************************************************************************
* taurus_perserv_api
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#ifndef R_PERSERV_API_H
#define R_PERSERV_API_H

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
   Type: R_TAURUS_PeripheralServer_SendSignalImpl_t

   Pointer to the implementation of the function used to send a signal
   to the Guest

   Parameters:
   Guest         - Guest to which we want to send a signal
   Signal        - The signal id to be sent
 */
typedef int (*R_TAURUS_PeripheralServer_SendSignalImpl_t)(int Guest, R_TAURUS_SignalId_t Signal, void* Option);

/*******************************************************************************
   Type: R_TAURUS_PeripheralServer_t

   TAURUS peripheral server structure.

   Members:
   Name              - Driver name
   Init              - Init function called during CIO server start
   DeInit            - DeInit function called during CIO server ending
   Execute           - Driver command
   SetSendSignalImpl - Set the implementation of the function used to send a signal to the Guest
 */

typedef struct {
    char *Name;
    int (*Init)(int Guest);
    int (*DeInit)(int Guest);
    void (*Execute)(int Guest, R_TAURUS_CmdMsg_t *Cmd, R_TAURUS_ResultMsg_t *Result);
    void (*SetSendSignalImpl)(R_TAURUS_PeripheralServer_SendSignalImpl_t SendSignalImpl);
} R_TAURUS_PeripheralServer_t;

#ifdef __cplusplus
}
#endif

#endif /* R_PERSERV_API_H */
