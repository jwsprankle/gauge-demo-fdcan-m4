#pragma once

#include "main.h"
#include <stdint.h>

void obd2_openamp_init();

void obd2_pub_data_to_openamp(struct obd_mail tx_msg);