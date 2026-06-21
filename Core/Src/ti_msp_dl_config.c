/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

DL_TimerA_backupConfig gPWM_TB6612FNGBackup;
DL_TimerG_backupConfig gPWM_SERVOBackup;
DL_MCAN_backupConfig gCAN_DRIVERBOARDBackup;

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_PWM_TB6612FNG_init();
    SYSCFG_DL_PWM_SERVO_init();
    SYSCFG_DL_TIMER_0_init();
    SYSCFG_DL_I2C_DRIVERBOARD_init();
    SYSCFG_DL_UART_DRIVERBOARD_init();
    SYSCFG_DL_ADC_TB6612FNG_init();
    SYSCFG_DL_CAN_DRIVERBOARD_init();
    /* Ensure backup structures have no valid state */
	gPWM_TB6612FNGBackup.backupRdy 	= false;
	gPWM_SERVOBackup.backupRdy 	= false;


	gCAN_DRIVERBOARDBackup.backupRdy 	= false;

}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_saveConfiguration(PWM_TB6612FNG_INST, &gPWM_TB6612FNGBackup);
	retStatus &= DL_TimerG_saveConfiguration(PWM_SERVO_INST, &gPWM_SERVOBackup);
	retStatus &= DL_MCAN_saveConfiguration(CAN_DRIVERBOARD_INST, &gCAN_DRIVERBOARDBackup);

    return retStatus;
}


SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_restoreConfiguration(PWM_TB6612FNG_INST, &gPWM_TB6612FNGBackup, false);
	retStatus &= DL_TimerG_restoreConfiguration(PWM_SERVO_INST, &gPWM_SERVOBackup, false);
	retStatus &= DL_MCAN_restoreConfiguration(CAN_DRIVERBOARD_INST, &gCAN_DRIVERBOARDBackup);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerA_reset(PWM_TB6612FNG_INST);
    DL_TimerG_reset(PWM_SERVO_INST);
    DL_TimerG_reset(TIMER_0_INST);
    DL_I2C_reset(I2C_DRIVERBOARD_INST);
    DL_UART_Main_reset(UART_DRIVERBOARD_INST);
    DL_ADC12_reset(ADC_TB6612FNG_INST);
    DL_MCAN_reset(CAN_DRIVERBOARD_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerA_enablePower(PWM_TB6612FNG_INST);
    DL_TimerG_enablePower(PWM_SERVO_INST);
    DL_TimerG_enablePower(TIMER_0_INST);
    DL_I2C_enablePower(I2C_DRIVERBOARD_INST);
    DL_UART_Main_enablePower(UART_DRIVERBOARD_INST);
    DL_ADC12_enablePower(ADC_TB6612FNG_INST);
    DL_MCAN_enablePower(CAN_DRIVERBOARD_INST);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{
    const uint8_t unusedPinIndexes[] =
    {
        IOMUX_PINCM7, IOMUX_PINCM8, IOMUX_PINCM9, IOMUX_PINCM10,
        IOMUX_PINCM11, IOMUX_PINCM14, IOMUX_PINCM16, IOMUX_PINCM21,
        IOMUX_PINCM22, IOMUX_PINCM23, IOMUX_PINCM24, IOMUX_PINCM25,
        IOMUX_PINCM26, IOMUX_PINCM31, IOMUX_PINCM32, IOMUX_PINCM33,
        IOMUX_PINCM36, IOMUX_PINCM43, IOMUX_PINCM44, IOMUX_PINCM45,
        IOMUX_PINCM48, IOMUX_PINCM52
    };

    for(int i = 0; i < sizeof(unusedPinIndexes)/sizeof(unusedPinIndexes[0]); i++)
    {
        DL_GPIO_initDigitalInputFeatures(unusedPinIndexes[i],
            DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
            DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    }

    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_TB6612FNG_C0_IOMUX,GPIO_PWM_TB6612FNG_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_TB6612FNG_C0_PORT, GPIO_PWM_TB6612FNG_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_TB6612FNG_C1_IOMUX,GPIO_PWM_TB6612FNG_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_TB6612FNG_C1_PORT, GPIO_PWM_TB6612FNG_C1_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_SERVO_C0_IOMUX,GPIO_PWM_SERVO_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_SERVO_C0_PORT, GPIO_PWM_SERVO_C0_PIN);

    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_I2C_DRIVERBOARD_IOMUX_SDA,
        GPIO_I2C_DRIVERBOARD_IOMUX_SDA_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_I2C_DRIVERBOARD_IOMUX_SCL,
        GPIO_I2C_DRIVERBOARD_IOMUX_SCL_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(GPIO_I2C_DRIVERBOARD_IOMUX_SDA);
    DL_GPIO_enableHiZ(GPIO_I2C_DRIVERBOARD_IOMUX_SCL);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_DRIVERBOARD_IOMUX_TX, GPIO_UART_DRIVERBOARD_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_DRIVERBOARD_IOMUX_RX, GPIO_UART_DRIVERBOARD_IOMUX_RX_FUNC);

    DL_GPIO_initDigitalOutput(GPIO_GRP_TB6612FNG_PIN_AIN1_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_GRP_TB6612FNG_PIN_AIN2_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_GRP_TB6612FNG_PIN_BIN1_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_GRP_TB6612FNG_PIN_BIN2_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_GRP_TB6612FNG_PIN_STBY_IOMUX);

    DL_GPIO_initDigitalInputFeatures(GPIO_GRP_MOTOR_PIN_ADIR_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_GRP_MOTOR_PIN_BDIR_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_GRP_MOTOR_PIN_AFREQ_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_GRP_MOTOR_PIN_BFREQ_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_clearPins(GPIOA, GPIO_GRP_TB6612FNG_PIN_AIN1_PIN |
		GPIO_GRP_TB6612FNG_PIN_AIN2_PIN |
		GPIO_GRP_TB6612FNG_PIN_BIN1_PIN |
		GPIO_GRP_TB6612FNG_PIN_BIN2_PIN);
    DL_GPIO_setPins(GPIOA, GPIO_GRP_TB6612FNG_PIN_STBY_PIN);
    DL_GPIO_enableOutput(GPIOA, GPIO_GRP_TB6612FNG_PIN_AIN1_PIN |
		GPIO_GRP_TB6612FNG_PIN_AIN2_PIN |
		GPIO_GRP_TB6612FNG_PIN_BIN1_PIN |
		GPIO_GRP_TB6612FNG_PIN_BIN2_PIN |
		GPIO_GRP_TB6612FNG_PIN_STBY_PIN);
    DL_GPIO_setUpperPinsPolarity(GPIOA, DL_GPIO_PIN_28_EDGE_RISE |
		DL_GPIO_PIN_31_EDGE_RISE);
    DL_GPIO_clearInterruptStatus(GPIOA, GPIO_GRP_MOTOR_PIN_AFREQ_PIN |
		GPIO_GRP_MOTOR_PIN_BFREQ_PIN);
    DL_GPIO_enableInterrupt(GPIOA, GPIO_GRP_MOTOR_PIN_AFREQ_PIN |
		GPIO_GRP_MOTOR_PIN_BFREQ_PIN);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_CAN_DRIVERBOARD_IOMUX_CAN_TX, GPIO_CAN_DRIVERBOARD_IOMUX_CAN_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_CAN_DRIVERBOARD_IOMUX_CAN_RX, GPIO_CAN_DRIVERBOARD_IOMUX_CAN_RX_FUNC);

}


static const DL_SYSCTL_SYSPLLConfig gSYSPLLConfig = {
    .inputFreq              = DL_SYSCTL_SYSPLL_INPUT_FREQ_16_32_MHZ,
	.rDivClk2x              = 1,
	.rDivClk1               = 0,
	.rDivClk0               = 0,
	.enableCLK2x            = DL_SYSCTL_SYSPLL_CLK2X_DISABLE,
	.enableCLK1             = DL_SYSCTL_SYSPLL_CLK1_ENABLE,
	.enableCLK0             = DL_SYSCTL_SYSPLL_CLK0_ENABLE,
	.sysPLLMCLK             = DL_SYSCTL_SYSPLL_MCLK_CLK0,
	.sysPLLRef              = DL_SYSCTL_SYSPLL_REF_SYSOSC,
	.qDiv                   = 9,
	.pDiv                   = DL_SYSCTL_SYSPLL_PDIV_2
};

SYSCONFIG_WEAK bool SYSCFG_DL_SYSCTL_SYSPLL_init(void)
{
    bool fFCCRatioStatus = false;
    uint32_t fFCCSysoscCount;
    uint32_t fFCCPllCount;
    uint32_t fFCCRatio;
    uint32_t fccTimeOutCounter;

    DL_SYSCTL_setFCCPeriods( DL_SYSCTL_FCC_TRIG_CNT_01 );

    /* Measuring PLL. */
    DL_SYSCTL_configFCC(DL_SYSCTL_FCC_TRIG_TYPE_RISE_RISE,
                        DL_SYSCTL_FCC_TRIG_SOURCE_LFCLK,
                        DL_SYSCTL_FCC_CLOCK_SOURCE_SYSPLLCLK0);
    /* Get SYSPLL frequency using FCC */
    fccTimeOutCounter = 0;
    DL_SYSCTL_startFCC();
    while (DL_SYSCTL_isFCCDone() == 0) {
        delay_cycles(977);  /* 1x LFCLK cycle = 32MHz/32.768kHz = 977, 30.5us */
        fccTimeOutCounter++;
        if(fccTimeOutCounter > 65){
            /* Timeout set to approximately 2ms (user-customizable) */
            break;
        }
    }

    /* get measA= SYSPLLCLK0 freq wrt LFOSC*/
    fFCCPllCount = DL_SYSCTL_readFCC();

    /* Measuring SYSPLL Source */
    DL_SYSCTL_configFCC(DL_SYSCTL_FCC_TRIG_TYPE_RISE_RISE,
                        DL_SYSCTL_FCC_TRIG_SOURCE_LFCLK,
                        DL_SYSCTL_FCC_CLOCK_SOURCE_SYSOSC);
    /* Get SYSPLL frequency using FCC */
    fccTimeOutCounter = 0;
    DL_SYSCTL_startFCC();
    while (DL_SYSCTL_isFCCDone() == 0) {
        delay_cycles(977);  /* 1x LFCLK cycle = 32MHz/32.768kHz = 977, 30.5us */
        fccTimeOutCounter++;
        if(fccTimeOutCounter > 65){
            /* Timeout set to approximately 2ms (user-customizable) */
            break;
        }
    }

    /* get measB= SYSOSC freq wrt LFOSC*/
    fFCCSysoscCount = DL_SYSCTL_readFCC();

    /* Get ratio of both measurements*/
    fFCCRatio = (fFCCPllCount * FLOAT_TO_INT_SCALE) / fFCCSysoscCount;
    /* Check ratio is within bounds*/
    if ((FCC_LOWER_BOUND <  fFCCRatio) && (fFCCRatio < FCC_UPPER_BOUND))
    {
        /* ratio is good for proceeding into application code. */
        fFCCRatioStatus = true;
    }

    return fFCCRatioStatus;
}
SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);
    DL_SYSCTL_setFlashWaitState(DL_SYSCTL_FLASH_WAIT_STATE_2);

    
	DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_configSYSPLL((DL_SYSCTL_SYSPLLConfig *) &gSYSPLLConfig);

    /*
     * [SYSPLL_ERR_01]
     * PLL Incorrect locking WA start.
     * Insert after every PLL enable.
     * This can lead an infinite loop if the condition persists
     * and can block entry to the application code.
     */

    while (SYSCFG_DL_SYSCTL_SYSPLL_init() == false)
    {
        /* Toggle SYSPLL enable to re-enable SYSPLL and re-check incorrect locking */
        DL_SYSCTL_disableSYSPLL();
        DL_SYSCTL_enableSYSPLL();

        /* Wait until SYSPLL startup is stabilized*/
        while ((DL_SYSCTL_getClockStatus() & SYSCTL_CLKSTATUS_SYSPLLGOOD_MASK) != DL_SYSCTL_CLK_STATUS_SYSPLL_GOOD){}
    }
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_2);
    DL_SYSCTL_setMCLKSource(SYSOSC, HSCLK, DL_SYSCTL_HSCLK_SOURCE_SYSPLL);
    /* INT_GROUP1 Priority */
    NVIC_SetPriority(GPIOA_INT_IRQn, 3);

}


/*
 * Timer clock configuration to be sourced by  / 1 (80000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   10000000 Hz = 80000000 Hz / (1 * (7 + 1))
 */
static const DL_TimerA_ClockConfig gPWM_TB6612FNGClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 7U
};

static const DL_TimerA_PWMConfig gPWM_TB6612FNGConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN_UP,
    .period = 1000,
    .isTimerWithFourCC = true,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_PWM_TB6612FNG_init(void) {

    DL_TimerA_setClockConfig(
        PWM_TB6612FNG_INST, (DL_TimerA_ClockConfig *) &gPWM_TB6612FNGClockConfig);

    DL_TimerA_initPWMMode(
        PWM_TB6612FNG_INST, (DL_TimerA_PWMConfig *) &gPWM_TB6612FNGConfig);

    // Set Counter control to the smallest CC index being used
    DL_TimerA_setCounterControl(PWM_TB6612FNG_INST,DL_TIMER_CZC_CCCTL0_ZCOND,DL_TIMER_CAC_CCCTL0_ACOND,DL_TIMER_CLC_CCCTL0_LCOND);

    DL_TimerA_setCaptureCompareOutCtl(PWM_TB6612FNG_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_TB6612FNG_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_TB6612FNG_INST, 99, DL_TIMER_CC_0_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(PWM_TB6612FNG_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_1_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_TB6612FNG_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_TB6612FNG_INST, 99, DL_TIMER_CC_1_INDEX);

    DL_TimerA_enableClock(PWM_TB6612FNG_INST);


    
    DL_TimerA_setCCPDirection(PWM_TB6612FNG_INST , DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT );
    DL_TimerA_enableShadowFeatures(PWM_TB6612FNG_INST);


}
/*
 * Timer clock configuration to be sourced by  / 1 (80000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   1000000 Hz = 80000000 Hz / (1 * (79 + 1))
 */
static const DL_TimerG_ClockConfig gPWM_SERVOClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 79U
};

static const DL_TimerG_PWMConfig gPWM_SERVOConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN_UP,
    .period = 20000,
    .isTimerWithFourCC = false,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_PWM_SERVO_init(void) {

    DL_TimerG_setClockConfig(
        PWM_SERVO_INST, (DL_TimerG_ClockConfig *) &gPWM_SERVOClockConfig);

    DL_TimerG_initPWMMode(
        PWM_SERVO_INST, (DL_TimerG_PWMConfig *) &gPWM_SERVOConfig);

    // Set Counter control to the smallest CC index being used
    DL_TimerG_setCounterControl(PWM_SERVO_INST,DL_TIMER_CZC_CCCTL0_ZCOND,DL_TIMER_CAC_CCCTL0_ACOND,DL_TIMER_CLC_CCCTL0_LCOND);

    DL_TimerG_setCaptureCompareOutCtl(PWM_SERVO_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERG_CAPTURE_COMPARE_0_INDEX);

    DL_TimerG_setCaptCompUpdateMethod(PWM_SERVO_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCaptureCompareValue(PWM_SERVO_INST, 1499, DL_TIMER_CC_0_INDEX);

    DL_TimerG_enableClock(PWM_SERVO_INST);


    
    DL_TimerG_setCCPDirection(PWM_SERVO_INST , DL_TIMER_CC0_OUTPUT );


}



/*
 * Timer clock configuration to be sourced by BUSCLK /  (80000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   80000000 Hz = 80000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gTIMER_0ClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * TIMER_0_INST_LOAD_VALUE = (50 s * 80000000 Hz) - 1
 */
static const DL_TimerG_TimerConfig gTIMER_0TimerConfig = {
    .period     = TIMER_0_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC_UP,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_TIMER_0_init(void) {

    DL_TimerG_setClockConfig(TIMER_0_INST,
        (DL_TimerG_ClockConfig *) &gTIMER_0ClockConfig);

    DL_TimerG_initTimerMode(TIMER_0_INST,
        (DL_TimerG_TimerConfig *) &gTIMER_0TimerConfig);
    DL_TimerG_enableInterrupt(TIMER_0_INST , DL_TIMERG_INTERRUPT_LOAD_EVENT);
	NVIC_SetPriority(TIMER_0_INST_INT_IRQN, 3);
    DL_TimerG_enableClock(TIMER_0_INST);





}


static const DL_I2C_ClockConfig gI2C_DRIVERBOARDClockConfig = {
    .clockSel = DL_I2C_CLOCK_BUSCLK,
    .divideRatio = DL_I2C_CLOCK_DIVIDE_1,
};

SYSCONFIG_WEAK void SYSCFG_DL_I2C_DRIVERBOARD_init(void) {

    DL_I2C_setClockConfig(I2C_DRIVERBOARD_INST,
        (DL_I2C_ClockConfig *) &gI2C_DRIVERBOARDClockConfig);
    DL_I2C_setAnalogGlitchFilterPulseWidth(I2C_DRIVERBOARD_INST,
        DL_I2C_ANALOG_GLITCH_FILTER_WIDTH_50NS);
    DL_I2C_enableAnalogGlitchFilter(I2C_DRIVERBOARD_INST);




}

static const DL_UART_Main_ClockConfig gUART_DRIVERBOARDClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_DRIVERBOARDConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_DRIVERBOARD_init(void)
{
    DL_UART_Main_setClockConfig(UART_DRIVERBOARD_INST, (DL_UART_Main_ClockConfig *) &gUART_DRIVERBOARDClockConfig);

    DL_UART_Main_init(UART_DRIVERBOARD_INST, (DL_UART_Main_Config *) &gUART_DRIVERBOARDConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(UART_DRIVERBOARD_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_DRIVERBOARD_INST, UART_DRIVERBOARD_IBRD_40_MHZ_115200_BAUD, UART_DRIVERBOARD_FBRD_40_MHZ_115200_BAUD);



    DL_UART_Main_enable(UART_DRIVERBOARD_INST);
}

/* ADC_TB6612FNG Initialization */
static const DL_ADC12_ClockConfig gADC_TB6612FNGClockConfig = {
    .clockSel       = DL_ADC12_CLOCK_SYSOSC,
    .divideRatio    = DL_ADC12_CLOCK_DIVIDE_1,
    .freqRange      = DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32,
};
SYSCONFIG_WEAK void SYSCFG_DL_ADC_TB6612FNG_init(void)
{
    DL_ADC12_setClockConfig(ADC_TB6612FNG_INST, (DL_ADC12_ClockConfig *) &gADC_TB6612FNGClockConfig);

    DL_ADC12_initSeqSample(ADC_TB6612FNG_INST,
        DL_ADC12_REPEAT_MODE_ENABLED, DL_ADC12_SAMPLING_SOURCE_AUTO, DL_ADC12_TRIG_SRC_SOFTWARE,
        DL_ADC12_SEQ_START_ADDR_00, DL_ADC12_SEQ_END_ADDR_01, DL_ADC12_SAMP_CONV_RES_12_BIT,
        DL_ADC12_SAMP_CONV_DATA_FORMAT_UNSIGNED);
    DL_ADC12_configConversionMem(ADC_TB6612FNG_INST, ADC_TB6612FNG_ADCMEM_0,
        DL_ADC12_INPUT_CHAN_0, DL_ADC12_REFERENCE_VOLTAGE_VDDA, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_configConversionMem(ADC_TB6612FNG_INST, ADC_TB6612FNG_ADCMEM_1,
        DL_ADC12_INPUT_CHAN_1, DL_ADC12_REFERENCE_VOLTAGE_VDDA, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_enableConversions(ADC_TB6612FNG_INST);
}

static const DL_MCAN_ClockConfig gCAN_DRIVERBOARDClockConf = {
    .clockSel = DL_MCAN_FCLK_SYSPLLCLK1,
    .divider  = DL_MCAN_FCLK_DIV_1,
};

static const DL_MCAN_InitParams gCAN_DRIVERBOARDInitParams= {

/* Initialize MCAN Init parameters.    */
    .fdMode            = true,
    .brsEnable         = true,
    .txpEnable         = false,
    .efbi              = false,
    .pxhddisable       = false,
    .darEnable         = false,
    .wkupReqEnable     = false,
    .autoWkupEnable    = false,
    .emulationEnable   = false,
    .tdcEnable         = false,
    .wdcPreload        = 255,

/* Transmitter Delay Compensation parameters. */
    .tdcConfig.tdcf    = 10,
    .tdcConfig.tdco    = 6,
};


static const DL_MCAN_MsgRAMConfigParams gCAN_DRIVERBOARDMsgRAMConfigParams ={

    /* Standard ID Filter List Start Address. */
    .flssa                = CAN_DRIVERBOARD_INST_MCAN_STD_ID_FILT_START_ADDR,
    /* List Size: Standard ID. */
    .lss                  = CAN_DRIVERBOARD_INST_MCAN_STD_ID_FILTER_NUM,
    /* Extended ID Filter List Start Address. */
    .flesa                = CAN_DRIVERBOARD_INST_MCAN_EXT_ID_FILT_START_ADDR,
    /* List Size: Extended ID. */
    .lse                  = CAN_DRIVERBOARD_INST_MCAN_EXT_ID_FILTER_NUM,
    /* Tx Buffers Start Address. */
    .txStartAddr          = CAN_DRIVERBOARD_INST_MCAN_TX_BUFF_START_ADDR,
    /* Number of Dedicated Transmit Buffers. */
    .txBufNum             = CAN_DRIVERBOARD_INST_MCAN_TX_BUFF_SIZE,
    .txFIFOSize           = 0,
    /* Tx Buffer Element Size. */
    .txBufMode            = 0,
    .txBufElemSize        = DL_MCAN_ELEM_SIZE_64BYTES,
    /* Tx Event FIFO Start Address. */
    .txEventFIFOStartAddr = CAN_DRIVERBOARD_INST_MCAN_TX_EVENT_START_ADDR,
    /* Event FIFO Size. */
    .txEventFIFOSize      = CAN_DRIVERBOARD_INST_MCAN_TX_EVENT_SIZE,
    /* Level for Tx Event FIFO watermark interrupt. */
    .txEventFIFOWaterMark = 3,
    /* Rx FIFO0 Start Address. */
    .rxFIFO0startAddr     = CAN_DRIVERBOARD_INST_MCAN_FIFO_0_START_ADDR,
    /* Number of Rx FIFO elements. */
    .rxFIFO0size          = CAN_DRIVERBOARD_INST_MCAN_FIFO_0_NUM,
    /* Rx FIFO0 Watermark. */
    .rxFIFO0waterMark     = 3,
    .rxFIFO0OpMode        = 0,
    /* Rx FIFO1 Start Address. */
    .rxFIFO1startAddr     = CAN_DRIVERBOARD_INST_MCAN_FIFO_1_START_ADDR,
    /* Number of Rx FIFO elements. */
    .rxFIFO1size          = CAN_DRIVERBOARD_INST_MCAN_FIFO_1_NUM,
    /* Level for Rx FIFO 1 watermark interrupt. */
    .rxFIFO1waterMark     = 3,
    /* FIFO blocking mode. */
    .rxFIFO1OpMode        = 0,
    /* Rx Buffer Start Address. */
    .rxBufStartAddr       = CAN_DRIVERBOARD_INST_MCAN_RX_BUFF_START_ADDR,
    /* Rx Buffer Element Size. */
    .rxBufElemSize        = DL_MCAN_ELEM_SIZE_64BYTES,
    /* Rx FIFO0 Element Size. */
    .rxFIFO0ElemSize      = DL_MCAN_ELEM_SIZE_64BYTES,
    /* Rx FIFO1 Element Size. */
    .rxFIFO1ElemSize      = DL_MCAN_ELEM_SIZE_64BYTES,
};



static const DL_MCAN_BitTimingParams   gCAN_DRIVERBOARDBitTimes = {
    /* Arbitration Baud Rate Pre-scaler. */
    .nomRatePrescalar   = 3,
    /* Arbitration Time segment before sample point. */
    .nomTimeSeg1        = 33,
    /* Arbitration Time segment after sample point. */
    .nomTimeSeg2        = 4,
    /* Arbitration (Re)Synchronization Jump Width Range. */
    .nomSynchJumpWidth  = 4,
    /* Data Baud Rate Pre-scaler. */
    .dataRatePrescalar  = 3,
    /* Data Time segment before sample point. */
    .dataTimeSeg1       = 16,
    /* Data Time segment after sample point. */
    .dataTimeSeg2       = 1,
    /* Data (Re)Synchronization Jump Width.   */
    .dataSynchJumpWidth = 1,
};


SYSCONFIG_WEAK void SYSCFG_DL_CAN_DRIVERBOARD_init(void) {
    DL_MCAN_RevisionId revid_CAN_DRIVERBOARD;

    DL_MCAN_enableModuleClock(CAN_DRIVERBOARD_INST);

    DL_MCAN_setClockConfig(CAN_DRIVERBOARD_INST, (DL_MCAN_ClockConfig *) &gCAN_DRIVERBOARDClockConf);

    /* Get MCANSS Revision ID. */
    DL_MCAN_getRevisionId(CAN_DRIVERBOARD_INST, &revid_CAN_DRIVERBOARD);

    /* Wait for Memory initialization to be completed. */
    while(false == DL_MCAN_isMemInitDone(CAN_DRIVERBOARD_INST));

    /* Put MCAN in SW initialization mode. */

    DL_MCAN_setOpMode(CAN_DRIVERBOARD_INST, DL_MCAN_OPERATION_MODE_SW_INIT);

    /* Wait till MCAN is not initialized. */
    while (DL_MCAN_OPERATION_MODE_SW_INIT != DL_MCAN_getOpMode(CAN_DRIVERBOARD_INST));

    /* Initialize MCAN module. */
    DL_MCAN_init(CAN_DRIVERBOARD_INST, (DL_MCAN_InitParams *) &gCAN_DRIVERBOARDInitParams);


    /* Configure Bit timings. */
    DL_MCAN_setBitTime(CAN_DRIVERBOARD_INST, (DL_MCAN_BitTimingParams*) &gCAN_DRIVERBOARDBitTimes);

    /* Configure Message RAM Sections */
    DL_MCAN_msgRAMConfig(CAN_DRIVERBOARD_INST, (DL_MCAN_MsgRAMConfigParams*) &gCAN_DRIVERBOARDMsgRAMConfigParams);



    /* Set Extended ID Mask. */
    DL_MCAN_setExtIDAndMask(CAN_DRIVERBOARD_INST, CAN_DRIVERBOARD_INST_MCAN_EXT_ID_AND_MASK );

    /* Loopback mode */

    /* Take MCAN out of the SW initialization mode */
    DL_MCAN_setOpMode(CAN_DRIVERBOARD_INST, DL_MCAN_OPERATION_MODE_NORMAL);

    while (DL_MCAN_OPERATION_MODE_NORMAL != DL_MCAN_getOpMode(CAN_DRIVERBOARD_INST));


}

