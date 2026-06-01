

#ifndef R_RTE_COMAL_API_H_
#define R_RTE_COMAL_API_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
   Title: COMAL

   Communication (COM) Abstraction Layer

   This API is used by the generic COM server application.
   For porting that server to a new communication method with the guest, only
   a new implementation of these functions is required.
 */


/**
 * Function R_RTE_COMAL_Init
 * @brief   Initialises the COMAL.
 *
 * @param[in] Guest - number of the guest application (only 1 is supported)
 *
 * @return  == 0 - OK
 * @return  != 0 - NG
 */
int R_RTE_COMAL_Init(int Guest);


/**
 * Function R_RTE_COMAL_DeInit
 * @brief   Deinitialises the COMAL.
 *
 * @param[in] Guest - number of the guest application (only 1 is supported)
 *
 * @return  == 0 - OK
 * @return  != 0 - NG
 */
int R_RTE_COMAL_DeInit(int Guest);


/**
 * Function R_RTE_COMAL_ReadCmd
 * @brief   Wait to read command from the specified guest application.
 *
 * @param[in] Guest - number of the guest application (only 1 is supported)
 * @param[out] Cmd - output buffer for the command from the guest application (must not be NULL)
 *
 * @return  == 0 - OK
 * @return  != 0 - NG
 */
int R_RTE_COMAL_ReadCmd(int Guest, R_TAURUS_CmdMsg_t *Cmd);

/**
 * Function R_RTE_COMAL_SendAck
 * @brief   Send the ack to the guest application.
 *
 * @param[in] Guest - number of the guest application (only 1 is supported)
 * @param[in] Result - Ack data to be sent to the guest application (must not be NULL)
 *
 * @return  == 0 - OK
 * @return  != 0 - NG
 */
int R_RTE_COMAL_SendAck(int Guest, const R_TAURUS_ResultMsg_t *Result);
    
/**
 * Function R_RTE_COMAL_SendResult
 * @brief   Send the result to the guest application.
 *
 * @param[in] Guest - number of the guest application (only 1 is supported)
 * @param[in] Result - result data to be sent to the guest application (must not be NULL)
 *
 * @return  == 0 - OK
 * @return  != 0 - NG
 */
int R_RTE_COMAL_SendResult(int Guest, const R_TAURUS_ResultMsg_t *Result);

/**
 * Function R_RTE_COMAL_SendSignal
 * @brief   Send signal to the service specified by Signal.
 *
 * @param[in] Guest - number of the guest application (only 1 is supported)
 * @param[in] Signal - identifier of signal sent to the guest application
 * @param[in] Option - Para-Ether data's address and size. For other Para-Server, not used.
 *
 * @return  == 0 - OK
 * @return  != 0 - NG
 */
int R_RTE_COMAL_SendSignal(int Guest, R_TAURUS_SignalId_t Signal, void* Option);

#ifdef __cplusplus
}
#endif

#endif /* R_RTE_COMAL_API_H_ */
