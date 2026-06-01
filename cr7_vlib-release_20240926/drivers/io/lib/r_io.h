/*************************************************************************************************************
* io
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#ifndef R_IO_H_
#define R_IO_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

#define __arch_getb(a)			(*(volatile unsigned char *)((uint32_t)a))
#define __arch_getw(a)			(*(volatile unsigned short *)((uint32_t)a))
#define __arch_getl(a)			(*(volatile unsigned int *)((uint32_t)a))

#define __arch_putb(v,a)		(*(volatile unsigned char *)((uint32_t)a) = (v))
#define __arch_putw(v,a)		(*(volatile unsigned short *)((uint32_t)a) = (v))
#define __arch_putl(v,a)		(*(volatile unsigned int *)((uint32_t)a) = (v))

#define __raw_writeb(v,a)	__arch_putb(v,a)
#define __raw_writew(v,a)	__arch_putw(v,a)
#define __raw_writel(v,a)	__arch_putl(v,a)

#define __raw_readb(a)		__arch_getb(a)
#define __raw_readw(a)		__arch_getw(a)
#define __raw_readl(a)		__arch_getl(a)

/*
 * TODO: The kernel offers some more advanced versions of barriers, it might
 * have some advantages to use them instead of the simple one here.
 */
#define reg_dmb()		__asm__ __volatile__ ("" : : : "memory")
#define __iormb()	reg_dmb()
#define __iowmb()	reg_dmb()

#define writeb(v,c)	({ uint8_t  __v = v; __iowmb(); __arch_putb(__v,c); __v; })
#define writew(v,c)	({ uint16_t __v = v; __iowmb(); __arch_putw(__v,c); __v; })
#define writel(v,c)	({ uint32_t __v = v; __iowmb(); __arch_putl(__v,c); __v; })

#define readb(c)	({ uint8_t  __v = __arch_getb(c); __iormb(); __v; })
#define readw(c)	({ uint16_t __v = __arch_getw(c); __iormb(); __v; })
#define readl(c)	({ uint32_t __v = __arch_getl(c); __iormb(); __v; })

#ifdef __cplusplus
}
#endif

#endif /* R_AUDIO_CORE_H_ */
