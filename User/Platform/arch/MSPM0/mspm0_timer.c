#include "../../timers/platform_clock.h"
#include "../../timers/platform_periodic_timer.h"
#include "../../timers/platform_pwm.h"
#include "../../core/platform_irq.h"

#include "mspm0_internal.h"
#include "mspm0_providers.h"
#include "ti_msp_dl_config.h"

#include <stddef.h>

#define MSPM0_PWM_TB_TICK_HZ (10000000U)
#define MSPM0_PWM_SERVO_TICK_HZ (1000000U)
#define MSPM0_CLOCK_TICK_HZ (80000000U)
#define MSPM0_PERIODIC_TIMER_TICK_HZ (1000000U)
#define MSPM0_PERIODIC_DEFAULT_TICKS (10000U)
#define MSPM0_TIMER_ISR_EVENT_LIMIT (8U)

struct mspm0_pwm_channel
{
    GPTIMER_Regs *timer;
    DL_TIMER_CC_INDEX compare_index;
    uint8_t timer_index;
    uint8_t enable_bit;
    uint32_t tick_hz;
};

struct mspm0_pwm_timer
{
    GPTIMER_Regs *timer;
    uint32_t period_ticks;
    uint8_t enabled_mask;
};

struct mspm0_periodic_timer
{
    GPTIMER_Regs *timer;
    IRQn_Type irq;
    uint32_t tick_hz;
    platform_periodic_timer_callback_t volatile callback;
    void *volatile context;
};

struct mspm0_clock
{
    GPTIMER_Regs *timer;
    IRQn_Type irq;
    uint32_t tick_hz;
};

static const struct mspm0_pwm_channel s_pwm_channels[] = {
    {
        .timer = TIMA0,
        .compare_index = DL_TIMER_CC_0_INDEX,
        .timer_index = 0U,
        .enable_bit = 1U,
        .tick_hz = MSPM0_PWM_TB_TICK_HZ,
    },
    {
        .timer = TIMA0,
        .compare_index = DL_TIMER_CC_1_INDEX,
        .timer_index = 0U,
        .enable_bit = 2U,
        .tick_hz = MSPM0_PWM_TB_TICK_HZ,
    },
    {
        .timer = TIMG6,
        .compare_index = DL_TIMER_CC_0_INDEX,
        .timer_index = 1U,
        .enable_bit = 1U,
        .tick_hz = MSPM0_PWM_SERVO_TICK_HZ,
    },
};

static struct mspm0_pwm_timer s_pwm_timers[] = {
    {
        .timer = TIMA0,
        .period_ticks = 1000U,
        .enabled_mask = 0U,
    },
    {
        .timer = TIMG6,
        .period_ticks = 20000U,
        .enabled_mask = 0U,
    },
};

static struct platform_pwm_state
    s_pwm_states[MSPM0_ARRAY_COUNT(s_pwm_channels)];

static const struct mspm0_clock s_clocks[] = {
    {
        .timer = TIMG12,
        .irq = TIMG12_INT_IRQn,
        .tick_hz = MSPM0_CLOCK_TICK_HZ,
    },
};

static struct mspm0_periodic_timer s_periodic_timers[] = {
    {
        .timer = TIMG8,
        .irq = TIMG8_INT_IRQn,
        .tick_hz = MSPM0_PERIODIC_TIMER_TICK_HZ,
        .callback = NULL,
        .context = NULL,
    },
};

static bool mspm0_pwm_channel_is_valid(
    platform_pwm_channel_t channel)
{
    return channel < MSPM0_ARRAY_COUNT(s_pwm_channels);
}

static platform_status_t mspm0_pwm_apply(
    platform_pwm_channel_t channel,
    const struct platform_pwm_state *state)
{
    const struct mspm0_pwm_channel *channel_cfg;
    struct mspm0_pwm_timer *timer;
    uint8_t other_enabled;
    uint32_t compare_value;

    if (!mspm0_pwm_channel_is_valid(channel) || !state) {
        return !state
                   ? PLATFORM_STATUS_INVALID_ARGUMENT
                   : PLATFORM_STATUS_NO_DEVICE;
    }

    channel_cfg = &s_pwm_channels[channel];
    timer = &s_pwm_timers[channel_cfg->timer_index];
    other_enabled =
        timer->enabled_mask &
        (uint8_t)~channel_cfg->enable_bit;
    if (other_enabled != 0U &&
        timer->period_ticks != state->period_ticks) {
        return PLATFORM_STATUS_BUSY;
    }

    if (timer->period_ticks != state->period_ticks) {
        DL_Timer_setLoadValue(
            timer->timer, state->period_ticks - 1U);
        timer->period_ticks = state->period_ticks;
    }

    compare_value = (state->pulse_ticks == 0U)
                        ? 0U
                        : state->pulse_ticks - 1U;
    DL_Timer_setCaptureCompareValue(
        channel_cfg->timer,
        compare_value,
        channel_cfg->compare_index);

    if (!state->is_enabled || state->pulse_ticks == 0U) {
        DL_Timer_overrideCCPOut(
            channel_cfg->timer,
            DL_TIMER_FORCE_OUT_LOW,
            DL_TIMER_FORCE_CMPL_OUT_LOW,
            channel_cfg->compare_index);
    } else {
        DL_Timer_overrideCCPOut(
            channel_cfg->timer,
            DL_TIMER_FORCE_OUT_DISABLED,
            DL_TIMER_FORCE_CMPL_OUT_DISABLED,
            channel_cfg->compare_index);
    }

    if (state->is_enabled) {
        timer->enabled_mask |= channel_cfg->enable_bit;
        DL_Timer_startCounter(timer->timer);
    } else {
        timer->enabled_mask &=
            (uint8_t)~channel_cfg->enable_bit;
        if (timer->enabled_mask == 0U) {
            DL_Timer_stopCounter(timer->timer);
        }
    }

    s_pwm_states[channel] = *state;
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_pwm_get_state(
    platform_pwm_channel_t channel,
    struct platform_pwm_state *state)
{
    if (!mspm0_pwm_channel_is_valid(channel) || !state) {
        return !state
                   ? PLATFORM_STATUS_INVALID_ARGUMENT
                   : PLATFORM_STATUS_NO_DEVICE;
    }

    *state = s_pwm_states[channel];
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_pwm_get_tick_hz(
    platform_pwm_channel_t channel,
    uint32_t *tick_hz)
{
    if (!mspm0_pwm_channel_is_valid(channel) || !tick_hz) {
        return !tick_hz
                   ? PLATFORM_STATUS_INVALID_ARGUMENT
                   : PLATFORM_STATUS_NO_DEVICE;
    }

    *tick_hz = s_pwm_channels[channel].tick_hz;
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_clock_read(
    platform_clock_id_t clock,
    uint32_t *ticks)
{
    if (clock >= MSPM0_ARRAY_COUNT(s_clocks) || !ticks) {
        return !ticks
                   ? PLATFORM_STATUS_INVALID_ARGUMENT
                   : PLATFORM_STATUS_NO_DEVICE;
    }

    *ticks = DL_Timer_getTimerCount(
        s_clocks[clock].timer);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_clock_get_frequency(
    platform_clock_id_t clock,
    uint32_t *frequency_hz)
{
    if (clock >= MSPM0_ARRAY_COUNT(s_clocks) ||
        !frequency_hz) {
        return !frequency_hz
                   ? PLATFORM_STATUS_INVALID_ARGUMENT
                   : PLATFORM_STATUS_NO_DEVICE;
    }

    *frequency_hz = s_clocks[clock].tick_hz;
    return PLATFORM_STATUS_OK;
}

static bool mspm0_periodic_timer_is_valid(
    platform_periodic_timer_id_t timer)
{
    return timer <
           MSPM0_ARRAY_COUNT(s_periodic_timers);
}

static platform_status_t mspm0_periodic_timer_set_period(
    platform_periodic_timer_id_t timer,
    uint32_t period_ticks)
{
    struct mspm0_periodic_timer *timer_cfg;

    if (!mspm0_periodic_timer_is_valid(timer) ||
        period_ticks == 0U) {
        return period_ticks == 0U
                   ? PLATFORM_STATUS_INVALID_ARGUMENT
                   : PLATFORM_STATUS_NO_DEVICE;
    }

    timer_cfg = &s_periodic_timers[timer];
    if (DL_Timer_isRunning(timer_cfg->timer)) {
        return PLATFORM_STATUS_BUSY;
    }

    DL_Timer_setLoadValue(
        timer_cfg->timer, period_ticks - 1U);
    DL_Timer_setTimerCount(timer_cfg->timer, 0U);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_periodic_timer_start(
    platform_periodic_timer_id_t timer)
{
    struct mspm0_periodic_timer *timer_cfg;

    if (!mspm0_periodic_timer_is_valid(timer)) {
        return PLATFORM_STATUS_NO_DEVICE;
    }

    timer_cfg = &s_periodic_timers[timer];
    if (!timer_cfg->callback) {
        return PLATFORM_STATUS_INVALID_STATE;
    }

    DL_Timer_clearInterruptStatus(
        timer_cfg->timer, UINT32_MAX);
    DL_Timer_enableInterrupt(
        timer_cfg->timer,
        DL_TIMER_INTERRUPT_LOAD_EVENT);
    NVIC_ClearPendingIRQ(timer_cfg->irq);
    NVIC_EnableIRQ(timer_cfg->irq);
    DL_Timer_startCounter(timer_cfg->timer);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_periodic_timer_stop(
    platform_periodic_timer_id_t timer)
{
    struct mspm0_periodic_timer *timer_cfg;

    if (!mspm0_periodic_timer_is_valid(timer)) {
        return PLATFORM_STATUS_NO_DEVICE;
    }

    timer_cfg = &s_periodic_timers[timer];
    DL_Timer_stopCounter(timer_cfg->timer);
    DL_Timer_disableInterrupt(
        timer_cfg->timer,
        DL_TIMER_INTERRUPT_LOAD_EVENT);
    DL_Timer_clearInterruptStatus(
        timer_cfg->timer, UINT32_MAX);
    NVIC_DisableIRQ(timer_cfg->irq);
    NVIC_ClearPendingIRQ(timer_cfg->irq);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_periodic_timer_attach(
    platform_periodic_timer_id_t timer,
    platform_periodic_timer_callback_t callback,
    void *context)
{
    struct mspm0_periodic_timer *timer_cfg;
    platform_irq_state_t state;

    if (!mspm0_periodic_timer_is_valid(timer) ||
        !callback) {
        return !callback
                   ? PLATFORM_STATUS_INVALID_ARGUMENT
                   : PLATFORM_STATUS_NO_DEVICE;
    }

    timer_cfg = &s_periodic_timers[timer];
    if (timer_cfg->callback &&
        timer_cfg->callback != callback) {
        return PLATFORM_STATUS_BUSY;
    }

    state = platform_irq_save();
    timer_cfg->callback = callback;
    timer_cfg->context = context;
    platform_irq_restore(state);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_periodic_timer_detach(
    platform_periodic_timer_id_t timer)
{
    struct mspm0_periodic_timer *timer_cfg;
    platform_irq_state_t state;
    platform_status_t rc;

    if (!mspm0_periodic_timer_is_valid(timer)) {
        return PLATFORM_STATUS_NO_DEVICE;
    }

    timer_cfg = &s_periodic_timers[timer];
    rc = mspm0_periodic_timer_stop(timer);
    if (rc != 0) {
        return rc;
    }

    state = platform_irq_save();
    timer_cfg->callback = NULL;
    timer_cfg->context = NULL;
    platform_irq_restore(state);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_periodic_timer_get_tick_hz(
    platform_periodic_timer_id_t timer,
    uint32_t *tick_hz)
{
    if (!mspm0_periodic_timer_is_valid(timer) ||
        !tick_hz) {
        return !tick_hz
                   ? PLATFORM_STATUS_INVALID_ARGUMENT
                   : PLATFORM_STATUS_NO_DEVICE;
    }

    *tick_hz = s_periodic_timers[timer].tick_hz;
    return PLATFORM_STATUS_OK;
}

void TIMG8_IRQHandler(void)
{
    struct mspm0_periodic_timer *timer =
        &s_periodic_timers[0];
    platform_periodic_timer_callback_t callback;
    DL_TIMER_IIDX index;
    uint32_t event_count = 0U;
    void *context;

    while (event_count < MSPM0_TIMER_ISR_EVENT_LIMIT &&
           (index = DL_Timer_getPendingInterrupt(
                timer->timer)) != 0) {
        event_count++;
        DL_Timer_clearInterruptStatus(
            timer->timer, UINT32_MAX);
        callback = timer->callback;
        context = timer->context;
        if (index == DL_TIMER_IIDX_LOAD && callback) {
            callback(0U, context);
        }
    }
}

static const struct platform_pwm_ops s_mspm0_pwm_ops = {
    .apply = mspm0_pwm_apply,
    .get_state = mspm0_pwm_get_state,
    .get_tick_hz = mspm0_pwm_get_tick_hz,
};

static const struct platform_clock_ops s_mspm0_clock_ops = {
    .read = mspm0_clock_read,
    .get_frequency = mspm0_clock_get_frequency,
};

static const struct platform_periodic_timer_ops
    s_mspm0_periodic_timer_ops = {
        .set_period = mspm0_periodic_timer_set_period,
        .start = mspm0_periodic_timer_start,
        .stop = mspm0_periodic_timer_stop,
        .attach = mspm0_periodic_timer_attach,
        .detach = mspm0_periodic_timer_detach,
        .get_tick_hz =
            mspm0_periodic_timer_get_tick_hz,
    };

platform_status_t mspm0_pwm_provider_init(void)
{
    uint8_t channel;

    DL_Timer_stopCounter(TIMA0);
    DL_Timer_stopCounter(TIMG6);
    for (channel = 0U;
         channel < MSPM0_ARRAY_COUNT(s_pwm_channels);
         channel++) {
        s_pwm_states[channel].period_ticks =
            s_pwm_timers[
                s_pwm_channels[channel].timer_index]
                .period_ticks;
        s_pwm_states[channel].pulse_ticks = 0U;
        s_pwm_states[channel].is_enabled = false;
        DL_Timer_setCaptureCompareValue(
            s_pwm_channels[channel].timer,
            0U,
            s_pwm_channels[channel].compare_index);
        DL_Timer_overrideCCPOut(
            s_pwm_channels[channel].timer,
            DL_TIMER_FORCE_OUT_LOW,
            DL_TIMER_FORCE_CMPL_OUT_LOW,
            s_pwm_channels[channel].compare_index);
    }

    return platform_pwm_register(&s_mspm0_pwm_ops);
}

platform_status_t mspm0_clock_provider_init(void)
{
    const struct mspm0_clock *clock = &s_clocks[0];

    NVIC_DisableIRQ(clock->irq);
    NVIC_ClearPendingIRQ(clock->irq);
    DL_Timer_stopCounter(clock->timer);
    DL_Timer_disableInterrupt(
        clock->timer, UINT32_MAX);
    DL_Timer_clearInterruptStatus(
        clock->timer, UINT32_MAX);
    DL_Timer_setLoadValue(clock->timer, UINT32_MAX);
    DL_Timer_setTimerCount(clock->timer, 0U);
    DL_Timer_startCounter(clock->timer);

    return platform_clock_register(&s_mspm0_clock_ops);
}

platform_status_t mspm0_periodic_timer_provider_init(void)
{
    const DL_TimerG_ClockConfig clock_config = {
        .clockSel = DL_TIMER_CLOCK_BUSCLK,
        .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
        .prescale = 79U,
    };
    const DL_TimerG_TimerConfig timer_config = {
        .period = MSPM0_PERIODIC_DEFAULT_TICKS - 1U,
        .timerMode = DL_TIMER_TIMER_MODE_PERIODIC_UP,
        .startTimer = DL_TIMER_STOP,
    };

    DL_TimerG_reset(TIMG8);
    DL_TimerG_enablePower(TIMG8);
    delay_cycles(POWER_STARTUP_DELAY);
    DL_TimerG_setClockConfig(
        TIMG8,
        (DL_TimerG_ClockConfig *)&clock_config);
    DL_TimerG_initTimerMode(
        TIMG8,
        (DL_TimerG_TimerConfig *)&timer_config);
    DL_TimerG_enableClock(TIMG8);
    NVIC_DisableIRQ(TIMG8_INT_IRQn);
    NVIC_ClearPendingIRQ(TIMG8_INT_IRQn);
    NVIC_SetPriority(TIMG8_INT_IRQn, 3U);

    return platform_periodic_timer_register(
        &s_mspm0_periodic_timer_ops);
}
