/*
 * Copyright (C) 2014 PHYTEC Messtechnik GmbH
 *               2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup     drivers_hdc2010
 * @{
 *
 * @file
 * @brief       Driver for the TI HDC2010 Humidity and Temperature Sensor.
 *
 * @author      Johann Fischer <j.fischer@phytec.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <string.h>

#include "assert.h"
#include "xtimer.h"
#include "periph/i2c.h"
#include "hdc2010.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

static int16_t temp_cached, hum_cached;
static uint32_t last_read_time;

int hdc2010_read_regw(const hdc2010_t *dev, uint8_t reg, uint8_t *data)
{
    i2c_acquire(dev->p.i2c);

    if (i2c_read_regs(dev->p.i2c, dev->p.addr,
                      reg, data, 2, 0) < 0) {
        i2c_release(dev->p.i2c);
        return HDC2010_NOBUS;
    }
    i2c_release(dev->p.i2c);
    return HDC2010_OK;
}

int hdc2010_write_reg(const hdc2010_t *dev, uint8_t reg, uint8_t data)
{
    i2c_acquire(dev->p.i2c);

    if (i2c_write_reg(dev->p.i2c, dev->p.addr,
                      reg, data, 0) < 0) {
        i2c_release(dev->p.i2c);
        return HDC2010_NOBUS;
    }
    i2c_release(dev->p.i2c);
    return HDC2010_OK;
}

int hdc2010_init(hdc2010_t *dev, const hdc2010_params_t *params)
{
    uint8_t reg[2];
    uint16_t tmp;
    int status;

    /* write device descriptor */
    dev->p = *params;

    if ((status = hdc2010_read_regw(dev, HDC2010_MANUFACTURER_ID, &reg[0])) != HDC2010_OK)
	    return status;

    tmp = ((uint16_t)reg[0] << 8) | reg[1];
    if (tmp != HDC2010_MID_VALUE) {
        return HDC2010_NODEV;
    }

    /* initial read for caching operation */
    if (hdc2010_read(dev, &temp_cached, &hum_cached) != HDC2010_OK) {
        return HDC2010_BUSERR;
    }
    last_read_time = xtimer_now_usec();

    /* all set */
    return HDC2010_OK;
}

int hdc2010_read_reg(const hdc2010_t *dev, uint8_t reg, uint8_t *data)
{
    i2c_acquire(dev->p.i2c);

    if (i2c_read_reg(dev->p.i2c, dev->p.addr,
                      reg, data, 0) < 0) {
        i2c_release(dev->p.i2c);
        return HDC2010_NOBUS;
    }
    i2c_release(dev->p.i2c);
    return HDC2010_OK;
}

int hdc2010_trigger_conversion(const hdc2010_t *dev)
{
    int status = HDC2010_OK;
    uint8_t configContents;
    assert(dev);

    if ((status = hdc2010_read_reg(dev, HDC2010_MEASUREMENT_CONFIG, &configContents)) != HDC2010_OK)
	    return status;

    configContents = (configContents | 0x01);

    if ((status = hdc2010_write_reg(dev, HDC2010_MEASUREMENT_CONFIG, configContents)) != HDC2010_OK)
	    return status;
    return status;
}

int hdc2010_get_results(const hdc2010_t *dev, int16_t *temp, int16_t *hum)
{
    int status = HDC2010_OK;
    assert(dev);

    uint8_t buf[4];

    if ((status = hdc2010_read_regw(dev, HDC2010_TEMPERATURE, &buf[0])) != HDC2010_OK)
	    return status;
    if ((status = hdc2010_read_regw(dev, HDC2010_HUMIDITY, &buf[2])) != HDC2010_OK)
	    return status;

    /* if all ok, we convert the values to their physical representation */
    if (temp) {
        uint16_t traw = ((uint16_t)buf[1] << 8) | buf[0];
        *temp = (int16_t)((((int32_t)traw * 16500) >> 16) - 4000);
    }
    if (hum) {
        uint16_t hraw = ((uint16_t)buf[3] << 8) | buf[2];
        *hum  = (int16_t)(((int32_t)hraw * 10000) >> 16);
    }
    return status;
}

int hdc2010_read(const hdc2010_t *dev, int16_t *temp, int16_t *hum)
{
    if (hdc2010_trigger_conversion(dev) != HDC2010_OK) {
        return HDC2010_BUSERR;
    }
    xtimer_usleep(HDC2010_CONVERSION_TIME);
    return hdc2010_get_results(dev, temp, hum);
}


int hdc2010_read_cached(const hdc2010_t *dev, int16_t *temp, int16_t *hum)
{
    uint32_t now = xtimer_now_usec();

    /* check if readings are outdated */
    if (now - last_read_time > dev->p.renew_interval) {
        /* update last_read_time */
        if (hdc2010_read(dev, &temp_cached, &hum_cached) != HDC2010_OK) {
            return HDC2010_BUSERR;
        }
        last_read_time = now;
    }

    if (temp) {
        *temp = temp_cached;
    }
    if (hum) {
        *hum = hum_cached;
    }
    return HDC2010_OK;
}
