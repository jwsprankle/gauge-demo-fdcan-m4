#pragma once

#include <stdint.h>

enum fdcan_msg_id {
    FDCAN_START,
    FDCAN_STOP,
    FDCAN_SPEED_XMT,
    FDCAN_RPM_XMT,
    FDCAN_SPEED_RCV,
    FDCAN_RPM_RCV
};

struct fdcan_msg
{
    enum fdcan_msg_id id;
    uint32_t data;
};


void fdcan_init();

void publish_fdcan_msg(struct fdcan_msg msg);
