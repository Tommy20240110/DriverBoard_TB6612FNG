#include "../../communications/platform_can.h"
#include "../../core/platform_irq.h"

#include "mspm0_internal.h"
#include "mspm0_providers.h"
#include "ti_msp_dl_config.h"

#include <stddef.h>
#include <string.h>

#define MSPM0_CAN_MODE_TIMEOUT_TICKS (800000U)
#define MSPM0_CAN_STANDARD_ID_SHIFT (18U)
#define MSPM0_CAN_STANDARD_ID_MASK (0x7FFU)
#define MSPM0_CAN_EXTENDED_ID_MASK (0x1FFFFFFFU)

struct mspm0_can_controller
{
    MCAN_Regs *mcan;
    bool is_busy;
    bool bus_off_latched;
    bool rx_overflow_latched;
    struct platform_can_diagnostics diagnostics;
};

static struct mspm0_can_controller
    s_can_controllers[] = {
        {
            .mcan = CAN_DRIVERBOARD_INST,
            .is_busy = false,
        },
    };

static const uint8_t s_can_dlc_lengths[] = {
    0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U,
    8U, 12U, 16U, 20U, 24U, 32U, 48U, 64U,
};

static struct mspm0_can_controller *
mspm0_can_get_controller(
    platform_can_controller_t controller)
{
    if (controller >=
        MSPM0_ARRAY_COUNT(s_can_controllers)) {
        return NULL;
    }
    return &s_can_controllers[controller];
}

static platform_status_t mspm0_can_acquire(
    struct mspm0_can_controller *controller)
{
    platform_irq_state_t irq_state;

    irq_state = platform_irq_save();
    if (controller->is_busy) {
        platform_irq_restore(irq_state);
        return PLATFORM_STATUS_BUSY;
    }
    controller->is_busy = true;
    platform_irq_restore(irq_state);
    return PLATFORM_STATUS_OK;
}

static void mspm0_can_release(
    struct mspm0_can_controller *controller)
{
    platform_irq_state_t irq_state;

    irq_state = platform_irq_save();
    controller->is_busy = false;
    platform_irq_restore(irq_state);
}

static bool mspm0_can_timeout_expired(uint32_t start)
{
    uint32_t now = DL_Timer_getTimerCount(TIMG12);

    return (uint32_t)(now - start) >=
           MSPM0_CAN_MODE_TIMEOUT_TICKS;
}

static platform_status_t mspm0_can_set_mode(
    const struct mspm0_can_controller *controller,
    uint32_t mode)
{
    uint32_t start;

    DL_MCAN_setOpMode(controller->mcan, mode);
    start = DL_Timer_getTimerCount(TIMG12);
    while (DL_MCAN_getOpMode(controller->mcan) != mode) {
        if (mspm0_can_timeout_expired(start)) {
            return PLATFORM_STATUS_TIMEOUT;
        }
    }
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_can_start(
    platform_can_controller_t controller_id)
{
    struct mspm0_can_controller *controller =
        mspm0_can_get_controller(controller_id);
    platform_status_t rc = PLATFORM_STATUS_OK;

    if (!controller) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    rc = mspm0_can_acquire(controller);
    if (rc != 0) {
        return rc;
    }
    rc = mspm0_can_set_mode(
        controller, DL_MCAN_OPERATION_MODE_NORMAL);
    mspm0_can_release(controller);
    return rc;
}

static platform_status_t mspm0_can_stop(
    platform_can_controller_t controller_id)
{
    struct mspm0_can_controller *controller =
        mspm0_can_get_controller(controller_id);
    platform_status_t rc;

    if (!controller) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    rc = mspm0_can_acquire(controller);
    if (rc != 0) {
        return rc;
    }
    rc = mspm0_can_set_mode(
        controller, DL_MCAN_OPERATION_MODE_SW_INIT);
    mspm0_can_release(controller);
    return rc;
}

static uint32_t mspm0_can_encode_dlc(uint8_t length)
{
    uint32_t dlc;

    for (dlc = 0U;
         dlc < (uint32_t)sizeof(s_can_dlc_lengths);
         dlc++) {
        if (length <= s_can_dlc_lengths[dlc]) {
            return dlc;
        }
    }
    return 15U;
}

static uint8_t mspm0_can_decode_dlc(uint32_t dlc)
{
    if (dlc >= (uint32_t)sizeof(s_can_dlc_lengths)) {
        return PLATFORM_CAN_MAX_DATA_LENGTH;
    }
    return s_can_dlc_lengths[dlc];
}

static platform_status_t mspm0_can_find_tx_buffer(
    const struct mspm0_can_controller *controller,
    uint32_t *buffer_index)
{
    uint32_t pending;
    uint32_t index;

    pending = DL_MCAN_getTxBufReqPend(controller->mcan);
    for (index = 0U;
         index <
             CAN_DRIVERBOARD_INST_MCAN_TX_BUFF_SIZE;
         index++) {
        if ((pending & (1UL << index)) == 0U) {
            *buffer_index = index;
            return PLATFORM_STATUS_OK;
        }
    }
    return PLATFORM_STATUS_BUSY;
}

static void mspm0_can_build_tx_element(
    const struct platform_can_frame *frame,
    DL_MCAN_TxBufElement *element)
{
    bool is_extended =
        (frame->flags & PLATFORM_CAN_FRAME_EXTENDED) != 0U;

    (void)memset(element, 0, sizeof(*element));
    element->id = is_extended
                      ? frame->id
                      : frame->id <<
                            MSPM0_CAN_STANDARD_ID_SHIFT;
    element->xtd = is_extended ? 1U : 0U;
    element->rtr =
        (frame->flags & PLATFORM_CAN_FRAME_REMOTE) != 0U;
    element->esi =
        (frame->flags & PLATFORM_CAN_FRAME_ESI) != 0U;
    element->fdf =
        (frame->flags & PLATFORM_CAN_FRAME_FD) != 0U;
    element->brs =
        (frame->flags & PLATFORM_CAN_FRAME_BRS) != 0U;
    element->dlc = mspm0_can_encode_dlc(frame->length);
    element->efc = 0U;
    if (frame->length > 0U &&
        (frame->flags & PLATFORM_CAN_FRAME_REMOTE) == 0U) {
        (void)memcpy(
            element->data, frame->data, frame->length);
    }
}

static platform_status_t mspm0_can_send(
    platform_can_controller_t controller_id,
    const struct platform_can_frame *frame)
{
    struct mspm0_can_controller *controller =
        mspm0_can_get_controller(controller_id);
    DL_MCAN_TxBufElement element;
    uint32_t buffer_index;
    platform_status_t rc;

    if (!controller || !frame) {
        return !controller
                   ? PLATFORM_STATUS_NO_DEVICE
                   : PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    rc = mspm0_can_acquire(controller);
    if (rc != 0) {
        if (rc == PLATFORM_STATUS_BUSY) {
            controller->diagnostics.tx_busy_count++;
        }
        return rc;
    }
    if (DL_MCAN_getOpMode(controller->mcan) !=
        DL_MCAN_OPERATION_MODE_NORMAL) {
        rc = PLATFORM_STATUS_INVALID_STATE;
    } else {
        rc = mspm0_can_find_tx_buffer(
            controller, &buffer_index);
    }
    if (rc == 0) {
        mspm0_can_build_tx_element(frame, &element);
        DL_MCAN_writeMsgRam(
            controller->mcan,
            DL_MCAN_MEM_TYPE_BUF,
            buffer_index,
            &element);
        if (DL_MCAN_TXBufAddReq(
                controller->mcan, buffer_index) != 0) {
            rc = PLATFORM_STATUS_IO_ERROR;
        }
    }

    if (rc == PLATFORM_STATUS_OK) {
        controller->diagnostics.send_count++;
    } else if (rc == PLATFORM_STATUS_BUSY) {
        controller->diagnostics.tx_busy_count++;
    } else if (rc == PLATFORM_STATUS_IO_ERROR) {
        controller->diagnostics.io_error_count++;
    }

    mspm0_can_release(controller);
    return rc;
}

static void mspm0_can_parse_rx_element(
    const DL_MCAN_RxBufElement *element,
    struct platform_can_frame *frame)
{
    uint8_t length = mspm0_can_decode_dlc(element->dlc);

    (void)memset(frame, 0, sizeof(*frame));
    frame->id = element->xtd
                    ? element->id &
                          MSPM0_CAN_EXTENDED_ID_MASK
                    : (element->id >>
                       MSPM0_CAN_STANDARD_ID_SHIFT) &
                          MSPM0_CAN_STANDARD_ID_MASK;
    frame->timestamp = element->rxts;
    frame->length = length;
    if (element->xtd != 0U) {
        frame->flags |= PLATFORM_CAN_FRAME_EXTENDED;
    }
    if (element->rtr != 0U) {
        frame->flags |= PLATFORM_CAN_FRAME_REMOTE;
    }
    if (element->fdf != 0U) {
        frame->flags |= PLATFORM_CAN_FRAME_FD;
    }
    if (element->brs != 0U) {
        frame->flags |= PLATFORM_CAN_FRAME_BRS;
    }
    if (element->esi != 0U) {
        frame->flags |= PLATFORM_CAN_FRAME_ESI;
    }
    if (length > 0U && element->rtr == 0U) {
        (void)memcpy(frame->data, element->data, length);
    }
}

static platform_status_t mspm0_can_receive(
    platform_can_controller_t controller_id,
    struct platform_can_frame *frame)
{
    struct mspm0_can_controller *controller =
        mspm0_can_get_controller(controller_id);
    DL_MCAN_RxBufElement element;
    DL_MCAN_RxFIFOStatus fifo_status = {
        .num = DL_MCAN_RX_FIFO_NUM_0,
    };
    platform_status_t rc = PLATFORM_STATUS_OK;

    if (!controller || !frame) {
        return !controller
                   ? PLATFORM_STATUS_NO_DEVICE
                   : PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    rc = mspm0_can_acquire(controller);
    if (rc != 0) {
        return rc;
    }

    DL_MCAN_getRxFIFOStatus(
        controller->mcan, &fifo_status);
    if (fifo_status.fillLvl == 0U) {
        rc = PLATFORM_STATUS_TRY_AGAIN;
        controller->diagnostics.no_frame_count++;
    } else {
        DL_MCAN_readMsgRam(
            controller->mcan,
            DL_MCAN_MEM_TYPE_FIFO,
            0U,
            fifo_status.num,
            &element);
        if (DL_MCAN_writeRxFIFOAck(
                controller->mcan,
                fifo_status.num,
                fifo_status.getIdx) != 0) {
            rc = PLATFORM_STATUS_IO_ERROR;
            controller->diagnostics.io_error_count++;
        } else {
            mspm0_can_parse_rx_element(&element, frame);
            controller->diagnostics.receive_count++;
        }
    }

    mspm0_can_release(controller);
    return rc;
}

static platform_status_t mspm0_can_get_status(
    platform_can_controller_t controller_id,
    struct platform_can_status *status)
{
    struct mspm0_can_controller *controller =
        mspm0_can_get_controller(controller_id);
    DL_MCAN_ErrCntStatus error_status;
    DL_MCAN_ProtocolStatus protocol_status;
    DL_MCAN_RxFIFOStatus fifo_status = {
        .num = DL_MCAN_RX_FIFO_NUM_0,
    };

    if (!controller || !status) {
        return !controller
                   ? PLATFORM_STATUS_NO_DEVICE
                   : PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (mspm0_can_acquire(controller) != 0) {
        return PLATFORM_STATUS_BUSY;
    }

    DL_MCAN_getErrCounters(
        controller->mcan, &error_status);
    DL_MCAN_getProtocolStatus(
        controller->mcan, &protocol_status);
    DL_MCAN_getRxFIFOStatus(
        controller->mcan, &fifo_status);
    status->tx_error_count = error_status.transErrLogCnt;
    status->rx_error_count = error_status.recErrCnt;
    status->last_error_code =
        protocol_status.lastErrCode;
    status->is_error_passive =
        protocol_status.errPassive != 0U;
    status->is_warning =
        protocol_status.warningStatus != 0U;
    status->is_bus_off =
        protocol_status.busOffStatus != 0U;
    status->has_rx_overflow =
        fifo_status.msgLost != 0U;
    if (status->is_bus_off && !controller->bus_off_latched) {
        controller->diagnostics.bus_off_observation_count++;
    }
    if (status->has_rx_overflow &&
        !controller->rx_overflow_latched) {
        controller->diagnostics.rx_overflow_count++;
    }
    controller->bus_off_latched = status->is_bus_off;
    controller->rx_overflow_latched =
        status->has_rx_overflow;
    mspm0_can_release(controller);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_can_get_diagnostics(
    platform_can_controller_t controller_id,
    struct platform_can_diagnostics *diagnostics)
{
    struct mspm0_can_controller *controller =
        mspm0_can_get_controller(controller_id);
    platform_irq_state_t state;

    if (!controller || !diagnostics) {
        return !controller
                   ? PLATFORM_STATUS_NO_DEVICE
                   : PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    state = platform_irq_save();
    *diagnostics = controller->diagnostics;
    platform_irq_restore(state);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_can_reset_diagnostics(
    platform_can_controller_t controller_id)
{
    struct mspm0_can_controller *controller =
        mspm0_can_get_controller(controller_id);
    platform_irq_state_t state;

    if (!controller) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    state = platform_irq_save();
    controller->diagnostics =
        (struct platform_can_diagnostics){0};
    platform_irq_restore(state);
    return PLATFORM_STATUS_OK;
}

static const struct platform_can_ops s_mspm0_can_ops = {
    .start = mspm0_can_start,
    .stop = mspm0_can_stop,
    .send = mspm0_can_send,
    .receive = mspm0_can_receive,
    .get_status = mspm0_can_get_status,
    .get_diagnostics = mspm0_can_get_diagnostics,
    .reset_diagnostics = mspm0_can_reset_diagnostics,
};

platform_status_t mspm0_can_provider_init(void)
{
    return platform_can_register(&s_mspm0_can_ops);
}
