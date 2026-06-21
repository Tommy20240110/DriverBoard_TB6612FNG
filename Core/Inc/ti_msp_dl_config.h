/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
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

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     80000000
/* Defines for SYSPLL_ERR_01 Workaround */
/* Represent 1.000 as 1000 */
#define FLOAT_TO_INT_SCALE                                               (1000U)
#define FCC_EXPECTED_RATIO                                                  2500
#define FCC_UPPER_BOUND                       (FCC_EXPECTED_RATIO * (1 + 0.003))
#define FCC_LOWER_BOUND                       (FCC_EXPECTED_RATIO * (1 - 0.003))

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);


/* Defines for PWM_TB6612FNG */
#define PWM_TB6612FNG_INST                                                 TIMA0
#define PWM_TB6612FNG_INST_IRQHandler                           TIMA0_IRQHandler
#define PWM_TB6612FNG_INST_INT_IRQN                             (TIMA0_INT_IRQn)
#define PWM_TB6612FNG_INST_CLK_FREQ                                     10000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_TB6612FNG_C0_PORT                                         GPIOA
#define GPIO_PWM_TB6612FNG_C0_PIN                                  DL_GPIO_PIN_0
#define GPIO_PWM_TB6612FNG_C0_IOMUX                               (IOMUX_PINCM1)
#define GPIO_PWM_TB6612FNG_C0_IOMUX_FUNC              IOMUX_PINCM1_PF_TIMA0_CCP0
#define GPIO_PWM_TB6612FNG_C0_IDX                            DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_TB6612FNG_C1_PORT                                         GPIOA
#define GPIO_PWM_TB6612FNG_C1_PIN                                  DL_GPIO_PIN_1
#define GPIO_PWM_TB6612FNG_C1_IOMUX                               (IOMUX_PINCM2)
#define GPIO_PWM_TB6612FNG_C1_IOMUX_FUNC              IOMUX_PINCM2_PF_TIMA0_CCP1
#define GPIO_PWM_TB6612FNG_C1_IDX                            DL_TIMER_CC_1_INDEX

/* Defines for PWM_SERVO */
#define PWM_SERVO_INST                                                     TIMG6
#define PWM_SERVO_INST_IRQHandler                               TIMG6_IRQHandler
#define PWM_SERVO_INST_INT_IRQN                                 (TIMG6_INT_IRQn)
#define PWM_SERVO_INST_CLK_FREQ                                          1000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_SERVO_C0_PORT                                             GPIOB
#define GPIO_PWM_SERVO_C0_PIN                                      DL_GPIO_PIN_2
#define GPIO_PWM_SERVO_C0_IOMUX                                  (IOMUX_PINCM15)
#define GPIO_PWM_SERVO_C0_IOMUX_FUNC                 IOMUX_PINCM15_PF_TIMG6_CCP0
#define GPIO_PWM_SERVO_C0_IDX                                DL_TIMER_CC_0_INDEX



/* Defines for TIMER_0 */
#define TIMER_0_INST                                                    (TIMG12)
#define TIMER_0_INST_IRQHandler                                TIMG12_IRQHandler
#define TIMER_0_INST_INT_IRQN                                  (TIMG12_INT_IRQn)
#define TIMER_0_INST_LOAD_VALUE                                    (3999999999U)




/* Defines for I2C_DRIVERBOARD */
#define I2C_DRIVERBOARD_INST                                                I2C1
#define I2C_DRIVERBOARD_INST_IRQHandler                          I2C1_IRQHandler
#define I2C_DRIVERBOARD_INST_INT_IRQN                              I2C1_INT_IRQn
#define GPIO_I2C_DRIVERBOARD_SDA_PORT                                      GPIOA
#define GPIO_I2C_DRIVERBOARD_SDA_PIN                              DL_GPIO_PIN_16
#define GPIO_I2C_DRIVERBOARD_IOMUX_SDA                           (IOMUX_PINCM38)
#define GPIO_I2C_DRIVERBOARD_IOMUX_SDA_FUNC               IOMUX_PINCM38_PF_I2C1_SDA
#define GPIO_I2C_DRIVERBOARD_SCL_PORT                                      GPIOA
#define GPIO_I2C_DRIVERBOARD_SCL_PIN                              DL_GPIO_PIN_15
#define GPIO_I2C_DRIVERBOARD_IOMUX_SCL                           (IOMUX_PINCM37)
#define GPIO_I2C_DRIVERBOARD_IOMUX_SCL_FUNC               IOMUX_PINCM37_PF_I2C1_SCL


/* Defines for UART_DRIVERBOARD */
#define UART_DRIVERBOARD_INST                                              UART1
#define UART_DRIVERBOARD_INST_FREQUENCY                                 40000000
#define UART_DRIVERBOARD_INST_IRQHandler                        UART1_IRQHandler
#define UART_DRIVERBOARD_INST_INT_IRQN                            UART1_INT_IRQn
#define GPIO_UART_DRIVERBOARD_RX_PORT                                      GPIOA
#define GPIO_UART_DRIVERBOARD_TX_PORT                                      GPIOA
#define GPIO_UART_DRIVERBOARD_RX_PIN                              DL_GPIO_PIN_18
#define GPIO_UART_DRIVERBOARD_TX_PIN                              DL_GPIO_PIN_17
#define GPIO_UART_DRIVERBOARD_IOMUX_RX                           (IOMUX_PINCM40)
#define GPIO_UART_DRIVERBOARD_IOMUX_TX                           (IOMUX_PINCM39)
#define GPIO_UART_DRIVERBOARD_IOMUX_RX_FUNC               IOMUX_PINCM40_PF_UART1_RX
#define GPIO_UART_DRIVERBOARD_IOMUX_TX_FUNC               IOMUX_PINCM39_PF_UART1_TX
#define UART_DRIVERBOARD_BAUD_RATE                                      (115200)
#define UART_DRIVERBOARD_IBRD_40_MHZ_115200_BAUD                            (21)
#define UART_DRIVERBOARD_FBRD_40_MHZ_115200_BAUD                            (45)





/* Defines for ADC_TB6612FNG */
#define ADC_TB6612FNG_INST                                                  ADC0
#define ADC_TB6612FNG_INST_IRQHandler                            ADC0_IRQHandler
#define ADC_TB6612FNG_INST_INT_IRQN                              (ADC0_INT_IRQn)
#define ADC_TB6612FNG_ADCMEM_0                                DL_ADC12_MEM_IDX_0
#define ADC_TB6612FNG_ADCMEM_0_REF               DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define ADC_TB6612FNG_ADCMEM_0_REF_VOLTAGE_V                                     3.3
#define ADC_TB6612FNG_ADCMEM_1                                DL_ADC12_MEM_IDX_1
#define ADC_TB6612FNG_ADCMEM_1_REF               DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define ADC_TB6612FNG_ADCMEM_1_REF_VOLTAGE_V                                     3.3
#define GPIO_ADC_TB6612FNG_C0_PORT                                         GPIOA
#define GPIO_ADC_TB6612FNG_C0_PIN                                 DL_GPIO_PIN_27
#define GPIO_ADC_TB6612FNG_IOMUX_C0                              (IOMUX_PINCM60)
#define GPIO_ADC_TB6612FNG_IOMUX_C0_FUNC          (IOMUX_PINCM60_PF_UNCONNECTED)
#define GPIO_ADC_TB6612FNG_C1_PORT                                         GPIOA
#define GPIO_ADC_TB6612FNG_C1_PIN                                 DL_GPIO_PIN_26
#define GPIO_ADC_TB6612FNG_IOMUX_C1                              (IOMUX_PINCM59)
#define GPIO_ADC_TB6612FNG_IOMUX_C1_FUNC          (IOMUX_PINCM59_PF_UNCONNECTED)



/* Port definition for Pin Group GPIO_GRP_TB6612FNG */
#define GPIO_GRP_TB6612FNG_PORT                                          (GPIOA)

/* Defines for PIN_AIN1: GPIOA.21 with pinCMx 46 on package pin 39 */
#define GPIO_GRP_TB6612FNG_PIN_AIN1_PIN                         (DL_GPIO_PIN_21)
#define GPIO_GRP_TB6612FNG_PIN_AIN1_IOMUX                        (IOMUX_PINCM46)
/* Defines for PIN_AIN2: GPIOA.22 with pinCMx 47 on package pin 40 */
#define GPIO_GRP_TB6612FNG_PIN_AIN2_PIN                         (DL_GPIO_PIN_22)
#define GPIO_GRP_TB6612FNG_PIN_AIN2_IOMUX                        (IOMUX_PINCM47)
/* Defines for PIN_BIN1: GPIOA.23 with pinCMx 53 on package pin 43 */
#define GPIO_GRP_TB6612FNG_PIN_BIN1_PIN                         (DL_GPIO_PIN_23)
#define GPIO_GRP_TB6612FNG_PIN_BIN1_IOMUX                        (IOMUX_PINCM53)
/* Defines for PIN_BIN2: GPIOA.24 with pinCMx 54 on package pin 44 */
#define GPIO_GRP_TB6612FNG_PIN_BIN2_PIN                         (DL_GPIO_PIN_24)
#define GPIO_GRP_TB6612FNG_PIN_BIN2_IOMUX                        (IOMUX_PINCM54)
/* Defines for PIN_STBY: GPIOA.25 with pinCMx 55 on package pin 45 */
#define GPIO_GRP_TB6612FNG_PIN_STBY_PIN                         (DL_GPIO_PIN_25)
#define GPIO_GRP_TB6612FNG_PIN_STBY_IOMUX                        (IOMUX_PINCM55)
/* Port definition for Pin Group GPIO_GRP_MOTOR */
#define GPIO_GRP_MOTOR_PORT                                              (GPIOA)

/* Defines for PIN_ADIR: GPIOA.8 with pinCMx 19 on package pin 16 */
#define GPIO_GRP_MOTOR_PIN_ADIR_PIN                              (DL_GPIO_PIN_8)
#define GPIO_GRP_MOTOR_PIN_ADIR_IOMUX                            (IOMUX_PINCM19)
/* Defines for PIN_BDIR: GPIOA.9 with pinCMx 20 on package pin 17 */
#define GPIO_GRP_MOTOR_PIN_BDIR_PIN                              (DL_GPIO_PIN_9)
#define GPIO_GRP_MOTOR_PIN_BDIR_IOMUX                            (IOMUX_PINCM20)
/* Defines for PIN_AFREQ: GPIOA.28 with pinCMx 3 on package pin 3 */
// pins affected by this interrupt request:["PIN_AFREQ","PIN_BFREQ"]
#define GPIO_GRP_MOTOR_INT_IRQN                                 (GPIOA_INT_IRQn)
#define GPIO_GRP_MOTOR_INT_IIDX                 (DL_INTERRUPT_GROUP1_IIDX_GPIOA)
#define GPIO_GRP_MOTOR_PIN_AFREQ_IIDX                       (DL_GPIO_IIDX_DIO28)
#define GPIO_GRP_MOTOR_PIN_AFREQ_PIN                            (DL_GPIO_PIN_28)
#define GPIO_GRP_MOTOR_PIN_AFREQ_IOMUX                            (IOMUX_PINCM3)
/* Defines for PIN_BFREQ: GPIOA.31 with pinCMx 6 on package pin 5 */
#define GPIO_GRP_MOTOR_PIN_BFREQ_IIDX                       (DL_GPIO_IIDX_DIO31)
#define GPIO_GRP_MOTOR_PIN_BFREQ_PIN                            (DL_GPIO_PIN_31)
#define GPIO_GRP_MOTOR_PIN_BFREQ_IOMUX                            (IOMUX_PINCM6)


/* Defines for CAN_DRIVERBOARD */
#define CAN_DRIVERBOARD_INST                                              CANFD0
#define GPIO_CAN_DRIVERBOARD_CAN_TX_PORT                                   GPIOA
#define GPIO_CAN_DRIVERBOARD_CAN_TX_PIN                           DL_GPIO_PIN_12
#define GPIO_CAN_DRIVERBOARD_IOMUX_CAN_TX                         (IOMUX_PINCM34)
#define GPIO_CAN_DRIVERBOARD_IOMUX_CAN_TX_FUNC           IOMUX_PINCM34_PF_CANFD0_CANTX
#define GPIO_CAN_DRIVERBOARD_CAN_RX_PORT                                   GPIOA
#define GPIO_CAN_DRIVERBOARD_CAN_RX_PIN                           DL_GPIO_PIN_13
#define GPIO_CAN_DRIVERBOARD_IOMUX_CAN_RX                         (IOMUX_PINCM35)
#define GPIO_CAN_DRIVERBOARD_IOMUX_CAN_RX_FUNC           IOMUX_PINCM35_PF_CANFD0_CANRX


/* Defines for CAN_DRIVERBOARD MCAN RAM configuration */
#define CAN_DRIVERBOARD_INST_MCAN_STD_ID_FILT_START_ADDR     (0)
#define CAN_DRIVERBOARD_INST_MCAN_STD_ID_FILTER_NUM          (1)
#define CAN_DRIVERBOARD_INST_MCAN_EXT_ID_FILT_START_ADDR     (48)
#define CAN_DRIVERBOARD_INST_MCAN_EXT_ID_FILTER_NUM          (1)
#define CAN_DRIVERBOARD_INST_MCAN_TX_BUFF_START_ADDR         (148)
#define CAN_DRIVERBOARD_INST_MCAN_TX_BUFF_SIZE               (2)
#define CAN_DRIVERBOARD_INST_MCAN_FIFO_1_START_ADDR          (192)
#define CAN_DRIVERBOARD_INST_MCAN_FIFO_1_NUM                 (2)
#define CAN_DRIVERBOARD_INST_MCAN_TX_EVENT_START_ADDR        (164)
#define CAN_DRIVERBOARD_INST_MCAN_TX_EVENT_SIZE              (2)
#define CAN_DRIVERBOARD_INST_MCAN_EXT_ID_AND_MASK            (0x1FFFFFFFU)
#define CAN_DRIVERBOARD_INST_MCAN_RX_BUFF_START_ADDR         (208)
#define CAN_DRIVERBOARD_INST_MCAN_FIFO_0_START_ADDR          (172)
#define CAN_DRIVERBOARD_INST_MCAN_FIFO_0_NUM                 (3)





/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);
void SYSCFG_DL_PWM_TB6612FNG_init(void);
void SYSCFG_DL_PWM_SERVO_init(void);
void SYSCFG_DL_TIMER_0_init(void);
void SYSCFG_DL_I2C_DRIVERBOARD_init(void);
void SYSCFG_DL_UART_DRIVERBOARD_init(void);
void SYSCFG_DL_ADC_TB6612FNG_init(void);

void SYSCFG_DL_CAN_DRIVERBOARD_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
