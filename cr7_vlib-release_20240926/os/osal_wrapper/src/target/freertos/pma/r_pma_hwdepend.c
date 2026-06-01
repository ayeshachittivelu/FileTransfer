/*************************************************************************************************************
* Power Management Agent 
* Copyright (c) [2021-2022]  Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* Version      : 1.0.0
* Description  : Hardware dependant function for PM API
***********************************************************************************************************************/

#include "target/freertos/pma/r_pma_statemachine.h"
#include "target/freertos/pma/r_pma_hwdepend.h"
#include <r_cpg_api.h>
#include <r_sysc_api.h>


#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define CPG_MSTP_GEN3_RTDMAC0          CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_0, R_CPG_BIT21)
#define CPG_MSTP_GEN3_RTDMAC1          CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_0, R_CPG_BIT16)

#define CPG_MSTP_GEN3_VCP4_VDPB        CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT31)  
#define CPG_MSTP_GEN3_VCP4_VCPLF       CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT30)  
#define CPG_MSTP_GEN3_iVDP1C           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT30)  
#define CPG_MSTP_GEN3_TMU0             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT25)
#define CPG_MSTP_GEN3_TMU1             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT24)
#define CPG_MSTP_GEN3_TMU2             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT23)
#define CPG_MSTP_GEN3_TMU3             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT22)
#define CPG_MSTP_GEN3_TMU4             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT21)
#define CPG_MSTP_GEN3_FDP10            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT19)  
#define CPG_MSTP_GEN3_FDP11            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT18)  
#define CPG_MSTP_GEN3_3DGE             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT12)  
#define CPG_MSTP_GEN3_SSP1             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT9) 
#define CPG_MSTP_GEN3_TSIF0            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT8) 
#define CPG_MSTP_GEN3_TSIF1            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT7) 
#define CPG_MSTP_GEN3_STBE             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_1, R_CPG_BIT5) 

#define CPG_MSTP_GEN3_SCEG             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_2, R_CPG_BIT26)  
#define CPG_MSTP_GEN3_SYS_DMAC0        CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_2, R_CPG_BIT19)  
#define CPG_MSTP_GEN3_SYSDMAC1         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_2, R_CPG_BIT18)
#define CPG_MSTP_GEN3_SYSDMAC2         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_2, R_CPG_BIT17)
#define CPG_MSTP_GEN3_MFIS             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_2, R_CPG_BIT13)
#define CPG_MSTP_GEN3_MSIOF0           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_2, R_CPG_BIT11)
#define CPG_MSTP_GEN3_MSIOF1           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_2, R_CPG_BIT10)
#define CPG_MSTP_GEN3_MSIOF2           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_2, R_CPG_BIT9)
#define CPG_MSTP_GEN3_MSIOF3           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_2, R_CPG_BIT8)
#define CPG_MSTP_GEN3_SCIF0            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_2, R_CPG_BIT7)
#define CPG_MSTP_GEN3_SCIF1            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_2, R_CPG_BIT6)
#define CPG_MSTP_GEN3_SCIF3            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_2, R_CPG_BIT4)
#define CPG_MSTP_GEN3_SCIF4            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_2, R_CPG_BIT3)
#define CPG_MSTP_GEN3_SCIF5            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_2, R_CPG_BIT2)

#define CPG_MSTP_GEN3_USB_DMAC01       CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT31) 
#define CPG_MSTP_GEN3_USB_DMAC00       CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT30) 
#define CPG_MSTP_GEN3_USB_DMAC31       CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT29) 
#define CPG_MSTP_GEN3_USB30_IF0        CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT28) 
#define CPG_MSTP_GEN3_USB_DMAC30       CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT26) 
#define CPG_MSTP_GEN3_CRC2             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT23)
#define CPG_MSTP_GEN3_CRC3             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT22)
#define CPG_MSTP_GEN3_PCIEC0           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT19)
#define CPG_MSTP_GEN3_PCIEC1           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT18) 
#define CPG_MSTP_GEN3_SD_IF0           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT14)
#define CPG_MSTP_GEN3_SD_IF1           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT13) 
#define CPG_MSTP_GEN3_SD_IF2           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT12) 
#define CPG_MSTP_GEN3_SD_IF3           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT11) 
#define CPG_MSTP_GEN3_SCIF2            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT10) 
#define CPG_MSTP_GEN3_CRC0             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT7)
#define CPG_MSTP_GEN3_CRC1             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT6)
#define CPG_MSTP_GEN3_TPU0             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT4)
#define CPG_MSTP_GEN3_CMT0             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT3)
#define CPG_MSTP_GEN3_CMT1             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT2)
#define CPG_MSTP_GEN3_CMT2             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT1)
#define CPG_MSTP_GEN3_CMT3             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_3, R_CPG_BIT0)

#define CPG_MSTP_GEN3_SUCMT            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_4, R_CPG_BIT31)
#define CPG_MSTP_GEN3_RWDT             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_4, R_CPG_BIT2)

#define CPG_MSTP_GEN3_SECURE_BOOT_ROM  CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT30)
#define CPG_MSTP_GEN3_PWM              CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT23)
#define CPG_MSTP_GEN3_THS              CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT22)
#define CPG_MSTP_GEN3_TSC              CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT22)
#define CPG_MSTP_GEN3_HSCIF0           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT20)
#define CPG_MSTP_GEN3_HSCIF1           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT19)
#define CPG_MSTP_GEN3_HSCIF2           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT18)
#define CPG_MSTP_GEN3_HSCIF3           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT17)
#define CPG_MSTP_GEN3_HSCIF4           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT16)  
#define CPG_MSTP_GEN3_DRIF00           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT15)  
#define CPG_MSTP_GEN3_DRIF01           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT14)  
#define CPG_MSTP_GEN3_DRIF10           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT13)  
#define CPG_MSTP_GEN3_DRIF11           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT12)  
#define CPG_MSTP_GEN3_DRIF20           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT11)  
#define CPG_MSTP_GEN3_DRIF21           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT10)  
#define CPG_MSTP_GEN3_DRIF30           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT9)  
#define CPG_MSTP_GEN3_DRIF31           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT8)  
#define CPG_MSTP_GEN3_ADSP             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT6)  
#define CPG_MSTP_GEN3_Audio_DMAC0      CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT2) 
#define CPG_MSTP_GEN3_Audio_DMAC1      CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_5, R_CPG_BIT1) 

#define CPG_MSTP_GEN3_VSPI0            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT31)  
#define CPG_MSTP_GEN3_VSPI1            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT30)  
#define CPG_MSTP_GEN3_VSPBD            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT26)  
#define CPG_MSTP_GEN3_VSPBC            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT24)  
#define CPG_MSTP_GEN3_VSPD0            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT23)
#define CPG_MSTP_GEN3_VSPD1            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT22)  
#define CPG_MSTP_GEN3_VSPD2            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT21)  
#define CPG_MSTP_GEN3_FCPCS            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT19)  
#define CPG_MSTP_GEN3_FCPF0            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT15)  
#define CPG_MSTP_GEN3_FCPF1            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT14)  
#define CPG_MSTP_GEN3_FCPVI0           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT11)  
#define CPG_MSTP_GEN3_FCPVI1           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT10)  
#define CPG_MSTP_GEN3_FCPVB0           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT7)  
#define CPG_MSTP_GEN3_FCPVB1           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT6)  
#define CPG_MSTP_GEN3_FCPVD0           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT3)
#define CPG_MSTP_GEN3_FCPVD1           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT2)  
#define CPG_MSTP_GEN3_FCPVD2           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_6, R_CPG_BIT1)  

#define CPG_MSTP_GEN3_HDMI_IF0         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT29)  
#define CPG_MSTP_GEN3_HDMI_IF1         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT28)  
#define CPG_MSTP_GEN3_LVDS             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT27)
#define CPG_MSTP_GEN3_DU0              CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT24)
#define CPG_MSTP_GEN3_DU1              CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT23)  
#define CPG_MSTP_GEN3_DU2              CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT22)  
#define CPG_MSTP_GEN3_DU3              CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT21)  
#define CPG_MSTP_GEN3_TCON0            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT20)  
#define CPG_MSTP_GEN3_TCON1            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT19)  
#define CPG_MSTP_GEN3_DOC0             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT18)  
#define CPG_MSTP_GEN3_DOC1             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT17)  
#define CPG_MSTP_GEN3_CSI40            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT16)
#define CPG_MSTP_GEN3_CSI41            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT15)
#define CPG_MSTP_GEN3_CSI20            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT14)  
#define CPG_MSTP_GEN3_DCU              CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT12)  
#define CPG_MSTP_GEN3_CMM0             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT11)  
#define CPG_MSTP_GEN3_CMM1             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT10)  
#define CPG_MSTP_GEN3_CMM2             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT9)  
#define CPG_MSTP_GEN3_CMM3             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT8)  
#define CPG_MSTP_GEN3_HS_USB_IF3       CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT5)  
#define CPG_MSTP_GEN3_HS_USB_IF0       CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT4)  
#define CPG_MSTP_GEN3_EHCI_OHCI0       CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT3)  
#define CPG_MSTP_GEN3_EHCI_OHCI1       CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT2)  
#define CPG_MSTP_GEN3_EHCI_OHCI2       CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT1)  
#define CPG_MSTP_GEN3_EHCI_OHCI3       CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_7, R_CPG_BIT0)  

#define CPG_MSTP_GEN3_IMP              CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT24)
#define CPG_MSTP_GEN3_IMR0             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT23)
#define CPG_MSTP_GEN3_IMR1             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT22)
#define CPG_MSTP_GEN3_IMR2             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT21)
#define CPG_MSTP_GEN3_IMR3             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT20)
#define CPG_MSTP_GEN3_SATAIF           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT15)
#define CPG_MSTP_GEN3_EAVBIF           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT12)
#define CPG_MSTP_GEN3_VIN0             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT11)
#define CPG_MSTP_GEN3_VIN1             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT10)
#define CPG_MSTP_GEN3_VIN2             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT9)
#define CPG_MSTP_GEN3_VIN3             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT8)
#define CPG_MSTP_GEN3_VIN4             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT7)
#define CPG_MSTP_GEN3_VIN5             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT6)
#define CPG_MSTP_GEN3_VIN6             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT5)
#define CPG_MSTP_GEN3_VIN7             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT4)
#define CPG_MSTP_GEN3_MLP              CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_8, R_CPG_BIT2)

/* Workaround for DHD */
#define CPG_MSTP_GEN3_DAVE_HD          CPG_MSTP_REG_BIT_TO_ID(20, R_CPG_BIT1)

#define CPG_MSTP_GEN3_I2C0             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT31)
#define CPG_MSTP_GEN3_I2C1             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT30)
#define CPG_MSTP_GEN3_I2C2             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT29)
#define CPG_MSTP_GEN3_I2C3             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT28)
#define CPG_MSTP_GEN3_I2C4             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT27)
#define CPG_MSTP_GEN3_IIC_DVFS         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT26)
#define CPG_MSTP_GEN3_MLM              CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT24)
#define CPG_MSTP_GEN3_DCTP             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT23)
#define CPG_MSTP_GEN3_ADG              CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT22)
#define CPG_MSTP_GEN3_SIM              CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT20)
#define CPG_MSTP_GEN3_I2C5             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT19)
#define CPG_MSTP_GEN3_I2C6             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT18)
#define CPG_MSTP_GEN3_RPC              CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT17)
#define CPG_MSTP_GEN3_CAN_IF0          CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT16)
#define CPG_MSTP_GEN3_CAN_IF1          CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT15)
#define CPG_MSTP_GEN3_CANFD            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT14)
#define CPG_MSTP_GEN3_GPIO0            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT12)
#define CPG_MSTP_GEN3_GPIO1            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT11)
#define CPG_MSTP_GEN3_GPIO2            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT10)
#define CPG_MSTP_GEN3_GPIO3            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT9)
#define CPG_MSTP_GEN3_GPIO4            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT8)
#define CPG_MSTP_GEN3_GPIO5            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT7)
#define CPG_MSTP_GEN3_GPIO6            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT6)
#define CPG_MSTP_GEN3_GPIO7            CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT5)
#define CPG_MSTP_GEN3_FM               CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT4)
#define CPG_MSTP_GEN3_IR               CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT3)
#define CPG_MSTP_GEN3_SPIF             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT2)
#define CPG_MSTP_GEN3_GYROIF           CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_9, R_CPG_BIT1)

#define CPG_MSTP_GEN3_SCU_SRC0         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT31)
#define CPG_MSTP_GEN3_SCU_SRC1         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT30)
#define CPG_MSTP_GEN3_SCU_SRC2         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT29)
#define CPG_MSTP_GEN3_SCU_SRC3         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT28)
#define CPG_MSTP_GEN3_SCU_SRC4         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT27)
#define CPG_MSTP_GEN3_SCU_SRC5         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT26)
#define CPG_MSTP_GEN3_SCU_SRC6         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT25)
#define CPG_MSTP_GEN3_SCU_SRC7         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT24)
#define CPG_MSTP_GEN3_SCU_SRC8         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT23)
#define CPG_MSTP_GEN3_SCU_SRC9         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT22)
#define CPG_MSTP_GEN3_SCU0             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT21)
#define CPG_MSTP_GEN3_SCU1             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT20)
#define CPG_MSTP_GEN3_SCU_DCV0         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT19)
#define CPG_MSTP_GEN3_SCU_DCV1         CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT18)
#define CPG_MSTP_GEN3_SCU_ALL          CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT17)
#define CPG_MSTP_GEN3_SSI0             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT15)
#define CPG_MSTP_GEN3_SSI1             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT14)
#define CPG_MSTP_GEN3_SSI2             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT13)
#define CPG_MSTP_GEN3_SSI3             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT12)
#define CPG_MSTP_GEN3_SSI4             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT11)
#define CPG_MSTP_GEN3_SSI5             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT10)
#define CPG_MSTP_GEN3_SSI6             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT9)
#define CPG_MSTP_GEN3_SSI7             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT8)
#define CPG_MSTP_GEN3_SSI8             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT7)
#define CPG_MSTP_GEN3_SSI9             CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT6)
#define CPG_MSTP_GEN3_SSI_ALL          CPG_MSTP_REG_BIT_TO_ID(R_CPG_REGIDX_10, R_CPG_BIT5)

#define R_PMA_HWA_ID_BIT_H3_IVDP1C        0x0000000000000001ULL   /** R_PMA_HWA_ID_IVDP1C      */
#define R_PMA_HWA_ID_BIT_H3_FCPCS         0x0000000000000002ULL   /** R_PMA_HWA_ID_FCPC        */
#define R_PMA_HWA_ID_BIT_H3_IMR0          0x0000000000000004ULL   /** R_PMA_HWA_ID_IMR0        */
#define R_PMA_HWA_ID_BIT_H3_IMR1          0x0000000000000008ULL   /** R_PMA_HWA_ID_IMR1        */
#define R_PMA_HWA_ID_BIT_H3_IMR2          0x0000000000000010ULL   /** R_PMA_HWA_ID_IMR2        */
#define R_PMA_HWA_ID_BIT_H3_IMR3          0x0000000000000020ULL   /** R_PMA_HWA_ID_IMR3        */
                                      /** 0x0000000000000040ULL       R_PMA_HWA_ID_DHD         */
                                      /** 0x0000000000000080ULL       R_PMA_HWA_ID_DU1         */
                                      /** 0x0000000000000100ULL       R_PMA_HWA_ID_DU2         */
                                      /** 0x0000000000000200ULL       R_PMA_HWA_ID_DU3         */
                                      /** 0x0000000000000400ULL       R_PMA_HWA_ID_DU4         */
#define R_PMA_HWA_ID_BIT_H3_VSPBD0        0x0000000000000800ULL   /** R_PMA_HWA_ID_VSPD0       */
#define R_PMA_HWA_ID_BIT_H3_VSPBD1        0x0000000000001000ULL   /** R_PMA_HWA_ID_VSPD1       */
#define R_PMA_HWA_ID_BIT_H3_VSPBD2        0x0000000000002000ULL   /** R_PMA_HWA_ID_VSPD1       */
                                                                  /** R_PMA_HWA_ID_VIN0        */
                                                                  /** R_PMA_HWA_ID_VIN1        */
                                                                  /** R_PMA_HWA_ID_VIN2        */
                                                                  /** R_PMA_HWA_ID_VIN3        */
                                                                  /** R_PMA_HWA_ID_VIN4        */
                                                                  /** R_PMA_HWA_ID_VIN5        */
                                                                  /** R_PMA_HWA_ID_VIN6        */
                                                                  /** R_PMA_HWA_ID_CSI20       */
                                                                  /** R_PMA_HWA_ID_CSI40       */
                                                                  /** R_PMA_HWA_ID_DOC0        */
                                                                  /** R_PMA_HWA_ID_DOC1        */
                                                                  /** R_PMA_HWA_ID_HDMI0       */
                                                                  /** R_PMA_HWA_ID_HDMI1       */
                                                                  /** R_PMA_HWA_ID_LVDS        */
                                                                  /** R_PMA_HWA_ID_CANFD       */
                                                                  /** R_PMA_HWA_ID_ETHERNET    */
                                                                  /** R_PMA_HWA_ID_RTDMAC0     */
                                                                  /** R_PMA_HWA_ID_RTDMAC1     */


#define R_PMA_SYSC_DOMAIN_BIT_H3_MASK_A3IR        (uint32_t)(0)


#define R_PMA_SYSC_DOMAIN_BIT_H3_MASK_A3VP        (uint32_t)(R_PMA_HWA_ID_BIT_H3_VSPBD0 | \
                                                               R_PMA_HWA_ID_BIT_H3_VSPBD1 | \
                                                               R_PMA_HWA_ID_BIT_H3_VSPBD2)

#define R_PMA_SYSC_DOMAIN_BIT_H3_MASK_A2VC1       (uint32_t)(R_PMA_HWA_ID_BIT_H3_IVDP1C)

#define R_PMA_SYSC_DOMAIN_BIT_H3_MASK_A3VC        (uint32_t)(R_PMA_HWA_ID_BIT_H3_IMR0 | \
                                                               R_PMA_HWA_ID_BIT_H3_IMR1 | \
                                                               R_PMA_HWA_ID_BIT_H3_IMR2 | \
                                                               R_PMA_HWA_ID_BIT_H3_IMR3 | \
                                                               R_PMA_HWA_ID_BIT_H3_FCPCS)

/* PRQA S 1514,3408 1 # Because the SoC-dependent processing is divided by the file, it cannot be avoided */
const uint32_t sysc_mask_tbl_H3[R_PMA_SYSC_DOMAIN_ID_NUM] =
{
    (uint32_t)R_PMA_SYSC_DOMAIN_BIT_H3_MASK_A3IR,         /**  R_PMA_SYSC_DOMAIN_ID_A3IR   */
    (uint32_t)R_PMA_SYSC_DOMAIN_BIT_H3_MASK_A3VP,         /**  R_PMA_SYSC_DOMAIN_ID_A3VP   */
    (uint32_t)R_PMA_SYSC_DOMAIN_BIT_H3_MASK_A3VC,         /**  R_PMA_SYSC_DOMAIN_ID_A3VC   */
    (uint32_t)R_PMA_SYSC_DOMAIN_BIT_H3_MASK_A2VC1         /**  R_PMA_SYSC_DOMAIN_ID_A2VC1  */
};


const int32_t R_PMA_SYSC_PDR_ID_TABLE_H3[R_PMA_SYSC_DOMAIN_ID_NUM] =
{
    R_SYSC_IMP_A3IR,  /* IMP/A3IR        */
    R_SYSC_A3VP,      /* A3VP            */
    R_SYSC_A3VC,      /* A3VC            */
    R_SYSC_A2VC1      /* A2VC1           */
};

/* PRQA S 1514,3408 1 # Because the SoC-dependent processing is divided by the file, it cannot be avoided */
const int32_t R_PMA_CPG_MOD_ID_TABLE_H3[R_PMA_CPG_DOMAIN_ID_NUM] =
{
    CPG_MSTP_GEN3_iVDP1C,              /** R_PMA_CPG_DOMAIN_ID_IVDP1C      */
    CPG_MSTP_GEN3_FCPCS,               /** R_PMA_CPG_DOMAIN_ID_FCPC        */
    CPG_MSTP_GEN3_IMR0,                /** R_PMA_HWA_ID_IMR0               */
    CPG_MSTP_GEN3_IMR1,                /** R_PMA_HWA_ID_IMR1               */
    CPG_MSTP_GEN3_IMR2,                /** R_PMA_HWA_ID_IMR2               */
    CPG_MSTP_GEN3_IMR3,                /** R_PMA_HWA_ID_IMR3               */
    CPG_MSTP_GEN3_DAVE_HD,             /** R_PMA_CPG_DOMAIN_ID_DHD         */
    CPG_MSTP_GEN3_DU0,                 /** R_PMA_CPG_DOMAIN_ID_DU0         */
    CPG_MSTP_GEN3_DU1,                 /** R_PMA_CPG_DOMAIN_ID_DU1         */
    CPG_MSTP_GEN3_DU2,                 /** R_PMA_CPG_DOMAIN_ID_DU2         */
    CPG_MSTP_GEN3_DU3,                 /** R_PMA_CPG_DOMAIN_ID_DU3         */
    CPG_MSTP_GEN3_VSPD0,               /** R_PMA_CPG_DOMAIN_ID_VSPD0       */
    CPG_MSTP_GEN3_VSPD1,               /** R_PMA_CPG_DOMAIN_ID_VSPD1       */
    CPG_MSTP_GEN3_VSPD2,               /** R_PMA_CPG_DOMAIN_ID_VSPD2       */
    CPG_MSTP_GEN3_VIN0,                /** R_PMA_CPG_DOMAIN_ID_VIN0        */
    CPG_MSTP_GEN3_VIN1,                /** R_PMA_CPG_DOMAIN_ID_VIN1        */
    CPG_MSTP_GEN3_VIN2,                /** R_PMA_CPG_DOMAIN_ID_VIN2        */
    CPG_MSTP_GEN3_VIN3,                /** R_PMA_CPG_DOMAIN_ID_VIN3        */
    CPG_MSTP_GEN3_VIN4,                /** R_PMA_CPG_DOMAIN_ID_VIN4        */
    CPG_MSTP_GEN3_VIN5,                /** R_PMA_CPG_DOMAIN_ID_VIN5        */
    CPG_MSTP_GEN3_VIN6,                /** R_PMA_CPG_DOMAIN_ID_VIN6        */
    CPG_MSTP_GEN3_VIN7,                /** R_PMA_CPG_DOMAIN_ID_VIN7        */
    CPG_MSTP_GEN3_CSI20,               /** R_PMA_CPG_DOMAIN_ID_CSI20       */
    CPG_MSTP_GEN3_CSI40,               /** R_PMA_CPG_DOMAIN_ID_CSI40       */
    CPG_MSTP_GEN3_CSI41,               /** R_PMA_CPG_DOMAIN_ID_CSI41       */
    CPG_MSTP_GEN3_DOC0,                /** R_PMA_CPG_DOMAIN_ID_DOC0        */
    CPG_MSTP_GEN3_DOC1,                /** R_PMA_CPG_DOMAIN_ID_DOC1        */
    CPG_MSTP_GEN3_HDMI_IF0,            /** R_PMA_CPG_DOMAIN_ID_HDMI0       */
    CPG_MSTP_GEN3_HDMI_IF1,            /** R_PMA_CPG_DOMAIN_ID_HDMI1       */
    CPG_MSTP_GEN3_LVDS,                /** R_PMA_CPG_DOMAIN_ID_LVDS        */
    CPG_MSTP_GEN3_CANFD,               /** R_PMA_CPG_DOMAIN_ID_CANFD       */
    CPG_MSTP_GEN3_EAVBIF,              /** R_PMA_CPG_DOMAIN_ID_ETHERNET    */
    CPG_MSTP_GEN3_RTDMAC0,             /** R_PMA_CPG_DOMAIN_ID_RTDMAC0     */
    CPG_MSTP_GEN3_RTDMAC1,             /** R_PMA_CPG_DOMAIN_ID_RTDMAC1     */
    CPG_MSTP_GEN3_Audio_DMAC0,         /** R_PMA_CPG_DOMAIN_ID_AUDMAC0     */
    CPG_MSTP_GEN3_Audio_DMAC1,         /** R_PMA_CPG_DOMAIN_ID_AUDMAC1     */
};

// static e_result_t R_PMA_SM_ChangeToPGtypeA(const st_hwa_t *pHwa);
static e_result_t R_PMA_SM_ChangeToPGtypeB(const st_hwa_t *pHwa);
// static e_result_t R_PMA_SM_ChangeToPGtypeC(const st_hwa_t *pHwa);
static e_result_t R_PMA_SM_ChangeToREADYwithoutPwr(const st_hwa_t *pHwa);
static e_result_t R_PMA_SM_ChangeToREADY(const st_hwa_t *pHwa);
// static e_result_t R_PMA_SM_ChangeToREADYtypeA(const st_hwa_t *pHwa);
// static e_result_t R_PMA_SM_ChangeToREADYtypeC(const st_hwa_t *pHwa);

/* PRQA S 1514,3408 1 # Because the SoC-dependent processing is divided by the file, it cannot be avoided */
const st_hwa_func_tbl_t R_PMA_SM_GstRegStatChangeFuncTbl_H3[R_PMA_HWA_ID_NUM] =
{
    { &R_PMA_SM_ChangeToPGtypeB,     &R_PMA_SM_ChangeToREADY           }, /** R_PMA_CPG_DOMAIN_ID_IVDP1C      */
    { &R_PMA_SM_ChangeToPGtypeB,     &R_PMA_SM_ChangeToREADY           }, /** R_PMA_CPG_DOMAIN_ID_FCPC        */
    { &R_PMA_SM_ChangeToPGtypeB,     &R_PMA_SM_ChangeToREADY           }, /** R_PMA_HWA_ID_IMR0               */
    { &R_PMA_SM_ChangeToPGtypeB,     &R_PMA_SM_ChangeToREADY           }, /** R_PMA_HWA_ID_IMR1               */
    { &R_PMA_SM_ChangeToPGtypeB,     &R_PMA_SM_ChangeToREADY           }, /** R_PMA_HWA_ID_IMR2               */
    { &R_PMA_SM_ChangeToPGtypeB,     &R_PMA_SM_ChangeToREADY           }, /** R_PMA_HWA_ID_IMR3               */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_DHD         */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_DU0         */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_DU1         */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_DU2         */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_DU3         */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_VSPD0       */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_VSPD1       */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_VSPD2       */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_VIN0        */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_VIN1        */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_VIN2        */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_VIN3        */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_VIN4        */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_VIN5        */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_VIN6        */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_VIN7        */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_CSI20       */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_CSI40       */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_CSI41       */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_DOC0        */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_DOC1        */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_HDMI0       */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_HDMI1       */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_LVDS        */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_CANFD       */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_ETHERNET    */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_RTDMAC0     */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_RTDMAC1     */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }, /** R_PMA_CPG_DOMAIN_ID_AUDMAC0     */
    { &R_PMA_SM_ChangeToCG,          &R_PMA_SM_ChangeToREADYwithoutPwr }  /** R_PMA_CPG_DOMAIN_ID_AUDMAC1     */
};

/* PRQA S 3408 4 # Because the SoC-dependent processing is divided by the file, it cannot be avoided */
const int32_t *R_PMA_CPG_MOD_ID_TABLE;
const int32_t *R_PMA_SYSC_PDR_ID_TABLE;
const uint32_t *sysc_mask_tbl;
const st_hwa_func_tbl_t *R_PMA_SM_GstRegStatChangeFuncTbl;

/***********************************************************************************************************************
* Start of function R_PMA_SM_InitSyscDomain()
***********************************************************************************************************************/
e_result_t R_PMA_SM_InitSyscDomain(void)
{
    e_result_t         LenResult = R_PMA_OK;    /* PRQA S 2981 # False positives for QAC */
    st_sysc_domain_t * LpDomain;

    static const e_sysc_domain_id_t LstSyscDepInfoTbl_H3[R_PMA_SYSC_DOMAIN_ID_NUM] =
    {
        R_PMA_SYSC_DOMAIN_ID_NONE,  /** R_PMA_SYSC_DOMAIN_ID_A3IR    */
        R_PMA_SYSC_DOMAIN_ID_NONE,  /** R_PMA_SYSC_DOMAIN_ID_A3VP    */
        R_PMA_SYSC_DOMAIN_ID_NONE,  /** R_PMA_SYSC_DOMAIN_ID_A3VC    */        
        R_PMA_SYSC_DOMAIN_ID_A3VC   /** R_PMA_SYSC_DOMAIN_ID_A2VC1   */
    };

    static const e_sysc_domain_id_t *LstSyscDepInfoTbl;
    /** set soc dependant table. */
    if (OSAL_DEVICE_SOC_TYPE_E3 == pma_soc_type
     || OSAL_DEVICE_SOC_TYPE_H3 == pma_soc_type
     || OSAL_DEVICE_SOC_TYPE_M3 == pma_soc_type
     || OSAL_DEVICE_SOC_TYPE_M3N == pma_soc_type)
    {
        LstSyscDepInfoTbl = LstSyscDepInfoTbl_H3;
        R_PMA_SYSC_PDR_ID_TABLE = R_PMA_SYSC_PDR_ID_TABLE_H3;
        sysc_mask_tbl = sysc_mask_tbl_H3;
    }
    else
    {
        LenResult = R_PMA_NG;
    }

   
    if (R_PMA_OK == LenResult)
    {
        /* PRQA S 1880 2 # No problem because it is a comparison between a non-negative enum and a positive integer */
        /* PRQA S 4442,4424 1 # No problem because it is an enum and an integer cast */
        for (uint32_t domainId = (uint32_t)sysc_idmin; domainId < (uint32_t)sysc_idmax; domainId++)
        {
            /* PRQA S 4342 1 # No problem because it is an enum and an integer cast */
            LpDomain             = R_PMA_SM_GetSyscDomain((e_sysc_domain_id_t)domainId);
            LpDomain->pDepDomain = R_PMA_SM_GetSyscDomain(LstSyscDepInfoTbl[domainId]);
            /* PRQA S 4342 1 # No problem because it is an enum and an integer cast */
            LpDomain->id         = (e_sysc_domain_id_t)domainId;
            LpDomain->refCnt     = 0U;
        }
    }
    /* PRQA S 2880,2992,2996 -- */

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_InitSyscDomain()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_InitCpgDomain()
***********************************************************************************************************************/
e_result_t R_PMA_SM_InitCpgDomain(void)
{
    e_result_t        LenResult = R_PMA_OK; /* PRQA S 2981 # False positives for QAC */
    st_cpg_domain_t * LpDomain;

    /* PRQA S 1514,3408 1 # Because the SoC-dependent processing is divided by the file, it cannot be avoided */
    static const e_cpg_domain_id_t LstCpgDepInfoTbl_H3[R_PMA_CPG_DOMAIN_ID_NUM] =
    {
        R_PMA_CPG_DOMAIN_ID_FCPC, /** R_PMA_CPG_DOMAIN_ID_IVDP1C      */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_FCPC        */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_HWA_ID_IMR0               */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_HWA_ID_IMR1               */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_HWA_ID_IMR2               */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_HWA_ID_IMR3               */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_DHD         */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_DU0         */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_DU1         */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_DU2         */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_DU3         */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_VSPD0       */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_VSPD1       */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_VSPD2       */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_VIN0        */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_VIN1        */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_VIN2        */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_VIN3        */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_VIN4        */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_VIN5        */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_VIN6        */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_VIN7        */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_CSI20       */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_CSI40       */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_CSI41       */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_DOC0        */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_DOC1        */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_HDMI0       */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_HDMI1       */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_LVDS        */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_CANFD       */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_ETHERNET    */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_RTDMAC0     */
        R_PMA_CPG_DOMAIN_ID_NONE,  /** R_PMA_CPG_DOMAIN_ID_RTDMAC1     */
        R_PMA_CPG_DOMAIN_ID_NONE, /** R_PMA_CPG_DOMAIN_ID_AUDMAC0     */
        R_PMA_CPG_DOMAIN_ID_NONE  /** R_PMA_CPG_DOMAIN_ID_AUDMAC1     */
    };

    const e_cpg_domain_id_t *LstCpgDepInfoTbl;

    /** set soc dependant table. */
    if (OSAL_DEVICE_SOC_TYPE_E3 == pma_soc_type
     || OSAL_DEVICE_SOC_TYPE_H3 == pma_soc_type
     || OSAL_DEVICE_SOC_TYPE_M3 == pma_soc_type
     || OSAL_DEVICE_SOC_TYPE_M3N == pma_soc_type)
    {
        LstCpgDepInfoTbl = LstCpgDepInfoTbl_H3;
        R_PMA_CPG_MOD_ID_TABLE = R_PMA_CPG_MOD_ID_TABLE_H3;
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    if (R_PMA_OK == LenResult)
    {
        /* PRQA S 1880 2 # No problem because it is a comparison between a non-negative enum and a positive integer */
        /* PRQA S 4442,4424 1 # No problem because it is an enum and an integer cast */
        for (uint32_t domainId = (uint32_t)cpg_idmin; domainId < (uint32_t)cpg_idmax; domainId++)
        {
            /* PRQA S 4342 1 # No problem because it is an enum and an integer cast */
            LpDomain             = R_PMA_SM_GetCpgDomain((e_cpg_domain_id_t)domainId);
            LpDomain->pDepDomain = R_PMA_SM_GetCpgDomain(LstCpgDepInfoTbl[domainId]);
            /* PRQA S 4342 1 # No problem because it is an enum and an integer cast */
            LpDomain->id         = (e_cpg_domain_id_t)domainId;
            LpDomain->refCnt     = 0U;
            LpDomain->enRstStat  = R_PMA_RELEASED_RESET;
        }
    }
    /* PRQA S 2880,2992,2996 -- */

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_InitCpgDomain()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_InitHwa()
***********************************************************************************************************************/
e_result_t R_PMA_SM_InitHwa(void)
{
    e_result_t LenResult = R_PMA_OK;    /* PRQA S 2981 # False positives for QAC */
    st_hwa_t * LpHwa;

    static const st_hwa_dep_info_t LstHwaDepInfoTbl_H3[R_PMA_HWA_ID_NUM] =
    {
        /** enDepSyscId, enDepCpgId */
        { R_PMA_SYSC_DOMAIN_ID_A2VC1,         R_PMA_CPG_DOMAIN_ID_IVDP1C },       /** R_PMA_CPG_DOMAIN_ID_IVDP1C      */
        { R_PMA_SYSC_DOMAIN_ID_A3VC,          R_PMA_CPG_DOMAIN_ID_FCPC },         /** R_PMA_CPG_DOMAIN_ID_FCPC        */
        { R_PMA_SYSC_DOMAIN_ID_A3VC,          R_PMA_CPG_DOMAIN_ID_IMR0 },         /** R_PMA_HWA_ID_IMR0               */
        { R_PMA_SYSC_DOMAIN_ID_A3VC,          R_PMA_CPG_DOMAIN_ID_IMR1 },         /** R_PMA_HWA_ID_IMR1               */
        { R_PMA_SYSC_DOMAIN_ID_A3VC,          R_PMA_CPG_DOMAIN_ID_IMR2 },         /** R_PMA_HWA_ID_IMR2               */
        { R_PMA_SYSC_DOMAIN_ID_A3VC,          R_PMA_CPG_DOMAIN_ID_IMR3 },         /** R_PMA_HWA_ID_IMR3               */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_DHD },          /** R_PMA_CPG_DOMAIN_ID_DHD         */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_DU0 },          /** R_PMA_CPG_DOMAIN_ID_DU0         */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_DU1 },          /** R_PMA_CPG_DOMAIN_ID_DU1         */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_DU2 },          /** R_PMA_CPG_DOMAIN_ID_DU2         */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_DU3 },          /** R_PMA_CPG_DOMAIN_ID_DU3         */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_VSPD0 },        /** R_PMA_CPG_DOMAIN_ID_VSPD0       */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_VSPD1 },        /** R_PMA_CPG_DOMAIN_ID_VSPD1       */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_VSPD2 },        /** R_PMA_CPG_DOMAIN_ID_VSPD2       */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_VIN0 },         /** R_PMA_CPG_DOMAIN_ID_VIN0        */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_VIN1 },         /** R_PMA_CPG_DOMAIN_ID_VIN1        */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_VIN2 },         /** R_PMA_CPG_DOMAIN_ID_VIN2        */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_VIN3 },         /** R_PMA_CPG_DOMAIN_ID_VIN3        */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_VIN4 },         /** R_PMA_CPG_DOMAIN_ID_VIN4        */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_VIN5 },         /** R_PMA_CPG_DOMAIN_ID_VIN5        */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_VIN6 },         /** R_PMA_CPG_DOMAIN_ID_VIN6        */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_VIN7 },         /** R_PMA_CPG_DOMAIN_ID_VIN7        */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_CSI20 },        /** R_PMA_CPG_DOMAIN_ID_CSI20       */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_CSI40 },        /** R_PMA_CPG_DOMAIN_ID_CSI40       */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_CSI41 },        /** R_PMA_CPG_DOMAIN_ID_CSI41       */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_DOC0 },         /** R_PMA_CPG_DOMAIN_ID_DOC0        */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_DOC1 },         /** R_PMA_CPG_DOMAIN_ID_DOC1        */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_HDMI0 },        /** R_PMA_CPG_DOMAIN_ID_HDMI0       */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_HDMI1 },        /** R_PMA_CPG_DOMAIN_ID_HDMI1       */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_LVDS },         /** R_PMA_CPG_DOMAIN_ID_LVDS        */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_CANFD },        /** R_PMA_CPG_DOMAIN_ID_CANFD       */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_ETHERNET },     /** R_PMA_CPG_DOMAIN_ID_ETHERNET    */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_RTDMAC0 },      /** R_PMA_CPG_DOMAIN_ID_RTDMAC0     */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_RTDMAC1 },       /** R_PMA_CPG_DOMAIN_ID_RTDMAC1     */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_AUDMAC0 },      /** R_PMA_CPG_DOMAIN_ID_RTDMAC0     */
        { R_PMA_SYSC_DOMAIN_ID_NONE,          R_PMA_CPG_DOMAIN_ID_AUDMAC1 }       /** R_PMA_CPG_DOMAIN_ID_RTDMAC1     */
    };

    const st_hwa_dep_info_t *LstHwaDepInfoTbl;
    /** set soc dependant table. */
    if (OSAL_DEVICE_SOC_TYPE_E3 == pma_soc_type
     || OSAL_DEVICE_SOC_TYPE_H3 == pma_soc_type
     || OSAL_DEVICE_SOC_TYPE_M3 == pma_soc_type
     || OSAL_DEVICE_SOC_TYPE_M3N == pma_soc_type)
    {
        LstHwaDepInfoTbl = LstHwaDepInfoTbl_H3;
        R_PMA_SM_GstRegStatChangeFuncTbl = R_PMA_SM_GstRegStatChangeFuncTbl_H3;
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    if (R_PMA_OK == LenResult)
    {
        /* PRQA S 1880 2 # No problem because it is a comparison between a non-negative enum and a positive integer */
        /* PRQA S 4442,4424 1 # No problem because it is an enum and an integer cast */
        for (uint32_t hwaId = (uint32_t)hwa_idmin; hwaId < (uint32_t)hwa_idmax; hwaId++)
        {
            /* PRQA S 4342 1 # No problem because it is an enum and an integer cast */
            LpHwa              = R_PMA_SM_GetHwa((e_pma_hwa_id_t)hwaId);
            LpHwa->handle      = NULL;
            LpHwa->enReqStat   = R_PMA_HWA_RELEASED;
            LpHwa->refCnt      = 0ULL; /* PRQA S 1257 # False positives by QAC */
            LpHwa->pSyscDomain = R_PMA_SM_GetSyscDomain(LstHwaDepInfoTbl[hwaId].enDepSyscId);
            LpHwa->pCpgDomain  = R_PMA_SM_GetCpgDomain(LstHwaDepInfoTbl[hwaId].enDepCpgId);

            if (NULL == LpHwa->pSyscDomain)
            {
                LpHwa->enPwrPolicy      = R_PMA_CG_MODE;
                LpHwa->enPrevStbMode    = R_PMA_CG_MODE;
                LpHwa->enApparentMode   = R_PMA_PG_MODE;
                LpHwa->enLowestPwrState = R_PMA_STATE_CG;
            }
            else
            {
                LpHwa->enPwrPolicy      = R_PMA_PG_MODE;
                LpHwa->enPrevStbMode    = R_PMA_PG_MODE;
                LpHwa->enApparentMode   = R_PMA_PG_MODE;
                LpHwa->enLowestPwrState = R_PMA_STATE_PG;
            }
        }
    }
    /* PRQA S 2880,2992,2996 -- */

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_InitHwa()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_ChangeToCG()
***********************************************************************************************************************/
/*static*/ e_result_t R_PMA_SM_ChangeToCG(const st_hwa_t *pHwa)
{
    e_result_t LenResult;
    if (NULL != pHwa)
    {
        if (NULL != pHwa->pCpgDomain)
        {
            LenResult = R_PMA_OK;
        }
        else
        {
            LenResult = R_PMA_NG;
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    if (R_PMA_OK == LenResult)
    {
        /** Clock Off */
        LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_OFF);
    }

    if ((R_PMA_OK == LenResult) && (NULL != pHwa->pCpgDomain->pDepDomain))
    {
        /** Clock Off */
        LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain->pDepDomain, R_PMA_CPG_CLOCK_OFF);
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_ChangeToCG()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_ChangeToPGtypeA()
***********************************************************************************************************************/
#if 0
static e_result_t R_PMA_SM_ChangeToPGtypeA(const st_hwa_t *pHwa)
{
    e_result_t LenResult;

    /** parameter check */
    if (NULL != pHwa)
    {
        if ((NULL != pHwa->pSyscDomain) && (NULL != pHwa->pCpgDomain))
        {
            LenResult = R_PMA_OK;
        }
        else
        {
            LenResult = R_PMA_NG;
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    /** Figure 9.4(a) */
    if (R_PMA_OK == LenResult)
    {
        /** Clock Off */
        LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_OFF);
    }

    if (R_PMA_OK == LenResult)
    {
        /** Clock On */
        LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_ON);
    }

    if (R_PMA_OK == LenResult)
    {
        /** Power Off *//* PRQA S 2812 1 # Not checked here as it has been checked by the caller */
        LenResult = R_PMA_SM_SetPowerStat(pHwa->pSyscDomain, R_PMA_SYSC_POWER_OFF);
    }

    /** because can't turn clock off **/
    if (R_PMA_OK == LenResult)
    {
        /** Clock Off */
        LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_OFF);
    }

    return LenResult;
}
#endif
/***********************************************************************************************************************
* End of function R_PMA_SM_ChangeToPGtypeA()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_ChangeToPGtypeB()
***********************************************************************************************************************/
static e_result_t R_PMA_SM_ChangeToPGtypeB(const st_hwa_t *pHwa)
{
    e_result_t LenResult;

    /** parameter check */
    if (NULL != pHwa)
    {
        if ((NULL != pHwa->pSyscDomain) && (NULL != pHwa->pCpgDomain))
        {
            /** Figure 9.4(b) */
            /** Clock Off */
            LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_OFF);

            if ((NULL != pHwa->handle) && (R_PMA_OK == LenResult))
            {
                /** Power Off *//* PRQA S 2812 1 # Not checked here as it has been checked by the caller */
                LenResult = R_PMA_SM_SetPowerStat(pHwa->pSyscDomain, R_PMA_SYSC_POWER_OFF);
            }
            else
            {
                LenResult = R_PMA_NG;
            }
        }
        else
        {
            LenResult = R_PMA_NG;
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_ChangeToPGtypeB()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_ChangeToPGtypeC()
***********************************************************************************************************************/
#if 0
static e_result_t R_PMA_SM_ChangeToPGtypeC(const st_hwa_t *pHwa)
{
    e_result_t LenResult;

    /** parameter check */
    if (NULL != pHwa)
    {
        if ((NULL != pHwa->pSyscDomain) && (NULL != pHwa->pCpgDomain))
        {
            LenResult = R_PMA_OK;
        }
        else
        {
            LenResult = R_PMA_NG;
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    /** Figure 9.4(c) */
    if (R_PMA_OK == LenResult)
    {
        /** Clock ON */
        LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_ON);
    }

    if (R_PMA_OK == LenResult)
    {
        if (0U < pHwa->pCpgDomain->refCnt)
        {
            pHwa->pCpgDomain->refCnt--;
        }
        /** Power Off *//* PRQA S 2812 1 # Not checked here as it has been checked by the caller */
        LenResult = R_PMA_SM_SetPowerStat(pHwa->pSyscDomain, R_PMA_SYSC_POWER_OFF);
    }

    if (R_PMA_OK == LenResult)
    {
        /** Clock OFF */
        LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_OFF);
    }

    if ((R_PMA_OK == LenResult) && (NULL != pHwa->pCpgDomain->pDepDomain))
    {
        /** Parent domain Clock Off */
        LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain->pDepDomain, R_PMA_CPG_CLOCK_OFF);
        if (((R_PMA_OK == LenResult) ||  (R_PMA_BSP_ETIMEDOUT == LenResult))
        && (NULL != pHwa->pSyscDomain->pDepDomain))
        {
            /** Parent domain Power Off *//* PRQA S 2812 1 # Not checked here as it has been checked by the caller */
            LenResult = R_PMA_SM_SetPowerStat(pHwa->pSyscDomain->pDepDomain, R_PMA_SYSC_POWER_OFF);
        }
    }
    return LenResult;
}
#endif
/***********************************************************************************************************************
* End of function R_PMA_SM_ChangeToPGtypeC()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_ChangeToREADYwithoutPwr()
***********************************************************************************************************************/
static e_result_t R_PMA_SM_ChangeToREADYwithoutPwr(const st_hwa_t *pHwa)
{
    e_result_t LenResult = R_PMA_OK;

    if ((NULL != pHwa) && (NULL != pHwa->pCpgDomain))
    {
        switch (pHwa->enPrevStbMode)
        {
            /** These 2 cases are intentionally combined. because power domain is always ON */
            case R_PMA_PG_MODE:
            case R_PMA_CG_MODE:
                /** Parent domain Clock On */
                if (NULL != pHwa->pCpgDomain->pDepDomain)
                {
                    LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain->pDepDomain, R_PMA_CPG_CLOCK_ON);
                }

                /** Clock On */
                if (R_PMA_OK == LenResult)
                {
                    LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_ON);
                }
                break;

            default:
                /** do nothing from HP-MODE */
                break;
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_ChangeToREADYwithoutPwr()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_ChangeToREADY()
***********************************************************************************************************************/
static e_result_t R_PMA_SM_ChangeToREADY(const st_hwa_t *pHwa)
{
    e_result_t LenResult = R_PMA_OK;

    if ((NULL != pHwa) && (NULL != pHwa->pSyscDomain) && (NULL != pHwa->pCpgDomain))
    {
        switch (pHwa->enPrevStbMode)
        {
            case R_PMA_PG_MODE:

                /** Clock On */
                /** 8A.2.1.6  */
                /** Note: Clear module stop bit of the module within A3IR power domain before A3IR power domain ON. */
                /** Refer to 9. */
                /** System Controller (SYSC). [R-Car V3H] */

                /** 8A.2.1.9  */
                /** Note: Clear module stop bit of the module within A3IR power domain before A3IR power domain ON. */
                /** Refer to 9. */
                /** System Controller (SYSC). [R-Car V3M, R-Car V3H] */

                /** Power On */
                //LenResult = R_PMA_SM_PowerONA3IR();
                LenResult = R_PMA_SM_SetPowerStat(pHwa->pSyscDomain, R_PMA_SYSC_POWER_ON);
                /** Clock On */
                if (R_PMA_OK == LenResult)
                {
                    LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_ON);
                }

                break;

            case R_PMA_CG_MODE:
                /** Clock On */
                LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_ON);
                break;

            default:
                /** do nothing from HP-MODE */
                break;
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_ChangeToREADY()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_ChangeToREADYtypeA()
***********************************************************************************************************************/
#if 0
static e_result_t R_PMA_SM_ChangeToREADYtypeA(const st_hwa_t *pHwa)
{
    e_result_t LenResult = R_PMA_OK;

    if ((NULL != pHwa) && (NULL != pHwa->pSyscDomain) && (NULL != pHwa->pCpgDomain))
    {
        switch (pHwa->enPrevStbMode)
        {
            case R_PMA_PG_MODE:
                /** Figure 9.4(a) */
                /** Clock Off */
                if (0xffUL > pHwa->pCpgDomain->refCnt)
                {
                    pHwa->pCpgDomain->refCnt++;
                }
                else
                {
                    LenResult = R_PMA_NG;
                }

                if (R_PMA_OK == LenResult)
                {
                    LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_OFF);
                    if (R_PMA_OK != LenResult)
                    {
                        pHwa->pCpgDomain->refCnt--;
                    }
                }

                /** Clock On */
                if (R_PMA_OK == LenResult)
                {
                    LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_ON);
                }

                /** Power On */
                if (R_PMA_OK == LenResult)
                {
                    LenResult = R_PMA_SM_SetPowerStat(pHwa->pSyscDomain, R_PMA_SYSC_POWER_ON);
                }

                break;

            case R_PMA_CG_MODE:
                /** Clock On */
                LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_ON);
                break;

            default:
                /** do nothing from HP-MODE */
                break;
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    return LenResult;
}
#endif
/***********************************************************************************************************************
* End of function R_PMA_SM_ChangeToREADYtypeA()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_ChangeToREADYtypeC()
***********************************************************************************************************************/
#if 0
static e_result_t R_PMA_SM_ChangeToREADYtypeC(const st_hwa_t *pHwa)
{
    e_result_t LenResult = R_PMA_OK;

    if ((NULL != pHwa) && (NULL != pHwa->pSyscDomain) && (NULL != pHwa->pCpgDomain))
    {
        switch (pHwa->enPrevStbMode)
        {
            case R_PMA_PG_MODE:
                if ((NULL != pHwa->pSyscDomain->pDepDomain) && (NULL != pHwa->pCpgDomain->pDepDomain))
                {
                    /** parent domain Power On */
                    LenResult = R_PMA_SM_PowerONA3IR();

                    if (R_PMA_OK == LenResult)
                    {
                        /** parent domain Clock On */
                        LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain->pDepDomain, R_PMA_CPG_CLOCK_ON);
                    }
                }
                
                /** Figure 9.4(c) */
                /** Clock On */
                if (R_PMA_OK == LenResult)
                {
                    LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_ON);
                }

                /** Power On */
                if (R_PMA_OK == LenResult)
                {
                    if (0U < pHwa->pCpgDomain->refCnt)
                    {
                        pHwa->pCpgDomain->refCnt--;
                    }
                    LenResult = R_PMA_SM_SetPowerStat(pHwa->pSyscDomain, R_PMA_SYSC_POWER_ON);
                }

                /** Clock On */
                if (R_PMA_OK == LenResult)
                {
                    LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_ON);
                }
                break;

            case R_PMA_CG_MODE:
                /** Clock On */
                LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_ON);
                if ((R_PMA_OK == LenResult) && (NULL != pHwa->pCpgDomain->pDepDomain))
                {
                    /** parent domain Clock On */
                    LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain->pDepDomain, R_PMA_CPG_CLOCK_ON);
                }
                break;

            default:
                /** do nothing from HP-MODE */
                break;
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    return LenResult;
}
#endif
/***********************************************************************************************************************
* End of function R_PMA_SM_ChangeToREADYtypeC()
***********************************************************************************************************************/
