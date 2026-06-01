
#ifndef CIO_BSP_CAN_CONFIG_H
#define CIO_BSP_CAN_CONFIG_H


#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
   Section: private defines
 */



#define PRIV_RCAR_CAN_TIMEOUT           (0x1000000)

#define R_CAN_PRV_MAX_INST_NUM          (2)


void prvSalvatorCanEnableTransceiver(void);
void prvSalvatorCanPFCInit(void);
void prvEbisuCanPFCInit(void);
void prvCanClkEnable(void);
void prvCanSoftReset(uint32_t Unit);


#ifdef __cplusplus
}
#endif

#endif /* CIO_BSP_CAN_CONFIG_H */
