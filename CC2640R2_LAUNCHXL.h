/*
 * Copyright (c) 2016-2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** ============================================================================
 *  @file       CC2640R2_LAUNCHXL.h
 *
 *  @brief      CC2640R2 LaunchPad Board Specific header file.
 *
 *  This file is responsible for setting up the board specific items for the
 *  CC2640R2_LAUNCHXL board.
 *
 *  This board file is made for the 7x7 mm QFN package, to convert this board
 *  file to use for other smaller device packages(5x5 mm and 4x4 mm QFN), the
 *  board will need to be modified since all the IO pins are not available for
 *  smaller packages. Note that the 2.7 x 2.7 mm WCSP package should use a
 *  separate board file also included within the SDK.
 *
 *  Refer to the datasheet for all the package options and IO descriptions:
 *  http://www.ti.com/lit/ds/symlink/cc2640r2f.pdf
 *
 *  For example, to change to the 4x4 package, remove all defines for all IOs
 *  not available (IOID_10 and higher) since the 4x4 package
 *  has only 10 DIO pins as listed in the datasheet. Remove the modules/pins
 *  not used including ADC, Display, SPI1, LED, and PIN due to limited pins.
 *  ============================================================================
 */
#ifndef __CC2640R2_LAUNCHXL_BOARD_H__
#define __CC2640R2_LAUNCHXL_BOARD_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include <ti/drivers/PIN.h>
#include <ti/devices/cc26x0r2/driverlib/ioc.h>

#include "sys_cfg.h"

/* Externs */
extern const PIN_Config BoardGpioInitTable[];

/* Defines */
#define CC2640R2_LAUNCHXL
#define Board_EXCLUDE_NVS_INTERNAL_FLASH
/* Mapping of pins to board signals using general board aliases
 *      <board signal alias>                  <pin mapping>
 */

/*cc2592*/
#define PA_EN                                   IOID_7
#define LNA_EN                                  IOID_13
#define HGM                                     IOID_14

/* SPI */
#define CC2640R2_LAUNCHXL_SPI_FLASH_CS          IOID_27
#define CC2640R2_LAUNCHXL_FLASH_CS_ON           0
#define CC2640R2_LAUNCHXL_FLASH_CS_OFF          1

#if defined(PCIE_SPI)
#define CC2640R2_LAUNCHXL_SPI_SLAVE_READY          IOID_3
#elif defined(AP_3)
#define CC2640R2_LAUNCHXL_SPI_SLAVE_READY          IOID_4
#elif defined(PCIE)
#define CC2640R2_LAUNCHXL_SPI_SLAVE_READY          IOID_4
#else

#endif

/* SPI Board */
#define CC2640R2_LAUNCHXL_SPI0_MISO             IOID_19          /* RF1.20 */
#define CC2640R2_LAUNCHXL_SPI0_MOSI             IOID_17          /* RF1.18 */
#define CC2640R2_LAUNCHXL_SPI0_CLK              IOID_18         /* RF1.16 */
#define CC2640R2_LAUNCHXL_SPI0_CSN              IOID_16

/* UART Board */
//#define BOARD2
#ifdef BOARD2
#define CC2640R2_LAUNCHXL_UART_RX               IOID_3          /* RXD */
#define CC2640R2_LAUNCHXL_UART_TX               IOID_2          /* TXD */
#define CC2640R2_LAUNCHXL_SPI1_MISO             IOID_19
#define CC2640R2_LAUNCHXL_SPI1_MOSI             IOID_18
#define CC2640R2_LAUNCHXL_SPI1_CLK              IOID_17
#define CC2640R2_LAUNCHXL_SPI1_CSN              IOID_16
#elif defined(AP_3)
#define CC2640R2_LAUNCHXL_UART_RX                IOID_2          /* RXD */
#define CC2640R2_LAUNCHXL_UART_TX                IOID_3          /* TXD */
#define CC2640R2_LAUNCHXL_SPI1_MISO              IOID_8          //pcb MISO
#define CC2640R2_LAUNCHXL_SPI1_MOSI              IOID_9          //pcb MOSI
#define CC2640R2_LAUNCHXL_SPI1_CLK               IOID_10          //pcb CLK
#define CC2640R2_LAUNCHXL_SPI1_CSN               IOID_11         //pcb CSN
#elif defined(PCIE_SPI)
#define CC2640R2_LAUNCHXL_UART_RX                IOID_UNUSED          /* RXD */
#define CC2640R2_LAUNCHXL_UART_TX                IOID_2          /* TXD */
#define CC2640R2_LAUNCHXL_SPI1_MISO              IOID_8          //pcb MISO
#define CC2640R2_LAUNCHXL_SPI1_MOSI              IOID_9          //pcb MOSI
#define CC2640R2_LAUNCHXL_SPI1_CLK               IOID_10          //pcb CLK
#define CC2640R2_LAUNCHXL_SPI1_CSN               IOID_11         //pcb CSN
#elif defined(PCIE)
#define CC2640R2_LAUNCHXL_UART_RX               IOID_2          /* RXD */
#define CC2640R2_LAUNCHXL_UART_TX               IOID_3          /* TXD */
#define CC2640R2_LAUNCHXL_SPI1_CSN              IOID_8          //pcb MISO
#define CC2640R2_LAUNCHXL_SPI1_MOSI               IOID_9          //pcb MOSI
#define CC2640R2_LAUNCHXL_SPI1_CLK              IOID_10          //pcb CLK
#define CC2640R2_LAUNCHXL_SPI1_MISO               IOID_11         //pcb CSN
else

#endif

/*!
 *  @brief  Initialize the general board specific settings
 *
 *  This function initializes the general board specific settings.
 */
void CC2640R2_LAUNCHXL_initGeneral(void);

/*!
 *  @brief  Turn off the external flash on LaunchPads
 *
 */
void CC2640R2_LAUNCHXL_shutDownExtFlash(void);

/*!
 *  @brief  Wake up the external flash present on the board files
 *
 *  This function toggles the chip select for the amount of time needed
 *  to wake the chip up.
 */
void CC2640R2_LAUNCHXL_wakeUpExtFlash(void);

/*!
 *  @def    CC2640R2_LAUNCHXL_ADCBufName
 *  @brief  Enum of ADCs
 */
typedef enum CC2640R2_LAUNCHXL_ADCBufName {
    CC2640R2_LAUNCHXL_ADCBUF0 = 0,

    CC2640R2_LAUNCHXL_ADCBUFCOUNT
} CC2640R2_LAUNCHXL_ADCBufName;

/*!
 *  @def    CC2640R2_LAUNCHXL_ADCBuf0SourceName
 *  @brief  Enum of ADCBuf channels
 */
typedef enum CC2640R2_LAUNCHXL_ADCBuf0ChannelName {
    CC2640R2_LAUNCHXL_ADCBUF0CHANNEL0 = 0,
    CC2640R2_LAUNCHXL_ADCBUF0CHANNEL1,
    CC2640R2_LAUNCHXL_ADCBUF0CHANNEL2,
    CC2640R2_LAUNCHXL_ADCBUF0CHANNEL3,
    CC2640R2_LAUNCHXL_ADCBUF0CHANNEL4,
    CC2640R2_LAUNCHXL_ADCBUF0CHANNEL5,
    CC2640R2_LAUNCHXL_ADCBUF0CHANNEL6,
    CC2640R2_LAUNCHXL_ADCBUF0CHANNEL7,
    CC2640R2_LAUNCHXL_ADCBUF0CHANNELVDDS,
    CC2640R2_LAUNCHXL_ADCBUF0CHANNELDCOUPL,
    CC2640R2_LAUNCHXL_ADCBUF0CHANNELVSS,

    CC2640R2_LAUNCHXL_ADCBUF0CHANNELCOUNT
} CC2640R2_LAUNCHXL_ADCBuf0ChannelName;

/*!
 *  @def    CC2640R2_LAUNCHXL_ADCName
 *  @brief  Enum of ADCs
 */
typedef enum CC2640R2_LAUNCHXL_ADCName {
    CC2640R2_LAUNCHXL_ADC0 = 0,
    CC2640R2_LAUNCHXL_ADC1,
    CC2640R2_LAUNCHXL_ADC2,
    CC2640R2_LAUNCHXL_ADC3,
    CC2640R2_LAUNCHXL_ADC4,
    CC2640R2_LAUNCHXL_ADC5,
    CC2640R2_LAUNCHXL_ADC6,
    CC2640R2_LAUNCHXL_ADC7,
    CC2640R2_LAUNCHXL_ADCDCOUPL,
    CC2640R2_LAUNCHXL_ADCVSS,
    CC2640R2_LAUNCHXL_ADCVDDS,

    CC2640R2_LAUNCHXL_ADCCOUNT
} CC2640R2_LAUNCHXL_ADCName;

/*!
 *  @def    CC2640R2_LAUNCHXL_CryptoName
 *  @brief  Enum of Crypto names
 */
typedef enum CC2640R2_LAUNCHXL_CryptoName {
    CC2640R2_LAUNCHXL_CRYPTO0 = 0,

    CC2640R2_LAUNCHXL_CRYPTOCOUNT
} CC2640R2_LAUNCHXL_CryptoName;

/*!
 *  @def    CC2640R2_LAUNCHXL_GPIOName
 *  @brief  Enum of GPIO names
 */
typedef enum CC2640R2_LAUNCHXL_GPIOName {
    CC2640R2_LAUNCHXL_GPIO_SPI_FLASH_CS,
    Board_SPI_SLAVE_READY,
    CC2640R2_LAUNCHXL_GPIOCOUNT
} CC2640R2_LAUNCHXL_GPIOName;

/*!
 *  @def    CC2640R2_LAUNCHXL_GPTimerName
 *  @brief  Enum of GPTimer parts
 */
typedef enum CC2640R2_LAUNCHXL_GPTimerName {
    CC2640R2_LAUNCHXL_GPTIMER0A = 0,
    CC2640R2_LAUNCHXL_GPTIMER0B,
    CC2640R2_LAUNCHXL_GPTIMER1A,
    CC2640R2_LAUNCHXL_GPTIMER1B,
    CC2640R2_LAUNCHXL_GPTIMER2A,
    CC2640R2_LAUNCHXL_GPTIMER2B,
    CC2640R2_LAUNCHXL_GPTIMER3A,
    CC2640R2_LAUNCHXL_GPTIMER3B,

    CC2640R2_LAUNCHXL_GPTIMERPARTSCOUNT
} CC2640R2_LAUNCHXL_GPTimerName;

/*!
 *  @def    CC2640R2_LAUNCHXL_GPTimers
 *  @brief  Enum of GPTimers
 */
typedef enum CC2640R2_LAUNCHXL_GPTimers {
    CC2640R2_LAUNCHXL_GPTIMER0 = 0,
    CC2640R2_LAUNCHXL_GPTIMER1,
    CC2640R2_LAUNCHXL_GPTIMER2,
    CC2640R2_LAUNCHXL_GPTIMER3,

    CC2640R2_LAUNCHXL_GPTIMERCOUNT
} CC2640R2_LAUNCHXL_GPTimers;

/*!
 *  @def    CC2640R2_LAUNCHXL_I2CName
 *  @brief  Enum of I2C names
 */
typedef enum CC2640R2_LAUNCHXL_I2CName {
    CC2640R2_LAUNCHXL_I2C0 = 0,

    CC2640R2_LAUNCHXL_I2CCOUNT
} CC2640R2_LAUNCHXL_I2CName;

/*!
 *  @def    CC2640R2_LAUNCHXL_NVSName
 *  @brief  Enum of NVS names
 */
typedef enum CC2640R2_LAUNCHXL_NVSName {
#ifndef Board_EXCLUDE_NVS_INTERNAL_FLASH
    CC2640R2_LAUNCHXL_NVSCC26XX0 = 0,
#endif
#ifndef Board_EXCLUDE_NVS_EXTERNAL_FLASH
    CC2640R2_LAUNCHXL_NVSSPI25X0,
#endif

    CC2640R2_LAUNCHXL_NVSCOUNT
} CC2640R2_LAUNCHXL_NVSName;

/*!
 *  @def    CC2640R2_LAUNCHXL_PWM
 *  @brief  Enum of PWM outputs
 */
typedef enum CC2640R2_LAUNCHXL_PWMName {
    CC2640R2_LAUNCHXL_PWM0 = 0,
    CC2640R2_LAUNCHXL_PWM1,
    CC2640R2_LAUNCHXL_PWM2,
    CC2640R2_LAUNCHXL_PWM3,
    CC2640R2_LAUNCHXL_PWM4,
    CC2640R2_LAUNCHXL_PWM5,
    CC2640R2_LAUNCHXL_PWM6,
    CC2640R2_LAUNCHXL_PWM7,

    CC2640R2_LAUNCHXL_PWMCOUNT
} CC2640R2_LAUNCHXL_PWMName;
/*!
 *  @def    CC2640R2_LAUNCHXL_SPIName
 *  @brief  Enum of SPI names
 */
typedef enum CC2640R2_LAUNCHXL_SPIName {
    CC2640R2_LAUNCHXL_SPI0 = 0,
    CC2640R2_LAUNCHXL_SPI1,

    CC2640R2_LAUNCHXL_SPICOUNT
} CC2640R2_LAUNCHXL_SPIName;

/*!
 *  @def    CC2640R2_LAUNCHXL_UARTName
 *  @brief  Enum of UARTs
 */
typedef enum CC2640R2_LAUNCHXL_UARTName {
    CC2640R2_LAUNCHXL_UART0 = 0,

    CC2640R2_LAUNCHXL_UARTCOUNT
} CC2640R2_LAUNCHXL_UARTName;

/*!
 *  @def    CC2640R2_LAUNCHXL_UDMAName
 *  @brief  Enum of DMA buffers
 */
typedef enum CC2640R2_LAUNCHXL_UDMAName {
    CC2640R2_LAUNCHXL_UDMA0 = 0,

    CC2640R2_LAUNCHXL_UDMACOUNT
} CC2640R2_LAUNCHXL_UDMAName;

/*!
 *  @def    CC2640R2_LAUNCHXL_WatchdogName
 *  @brief  Enum of Watchdogs
 */
typedef enum CC2640R2_LAUNCHXL_WatchdogName {
    CC2640R2_LAUNCHXL_WATCHDOG0 = 0,

    CC2640R2_LAUNCHXL_WATCHDOGCOUNT
} CC2640R2_LAUNCHXL_WatchdogName;

#ifdef __cplusplus
}
#endif

#endif /* __CC2640R2_LAUNCHXL_BOARD_H__ */
