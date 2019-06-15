/*
 * Copyright (C) 2014 PHYTEC Messtechnik GmbH
 *               2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_hdc2010 HDC2010 Humidity and Temperature Sensor
 * @ingroup     drivers_sensors
 * @ingroup     drivers_saul
 * @brief       Driver for the TI HDC2010 Humidity and Temperature Sensor
 *
 * The driver will initialize the sensor for best resolution (14 bit). Currently
 * the driver doesn't use the heater. Temperature and humidity are acquired in
 * sequence. The sensor is always in sleep mode.
 *
 * The temperature and humidity values can either be acquired using the
 * simplified `hdc2010_read()` function, or the conversion can be triggered
 * manually using the `hdc2010_trigger_conversion()` and `hdc2010_get_results()`
 * functions sequentially. If using the second method, on must wait at least
 * `HDC2010_CONVERSION_TIME` between triggering the conversion and reading the
 * results.
 *
 * @note        The driver does currently not support using the devices heating
 *              unit.
 *
 * This driver provides @ref drivers_saul capabilities.
 *
 * @{
 *
 * @file
 * @brief       Interface definition for the HDC2010 sensor driver
 *
 * @author      Johann Fischer <j.fischer@phytec.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef HDC2010_H
#define HDC2010_H

#include <stdint.h>

#include "periph/i2c.h"
#include "hdc2010_regs.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief   Default I2C bus address of HDC2010 devices
 */
#ifndef HDC2010_I2C_ADDRESS
#define HDC2010_I2C_ADDRESS           (0x40)
#endif

/**
 * @brief   Typical conversion time needed to acquire new values [in us]
 *
 * @note    This time value is chosen twice as large as needed for two 14-bit
 *          conversions (worst case) to allow for timer imprecision:
 *          (convert temp + convert hum) * 2 -> (610.0us + 610.0us) * 2 := 2.4ms.
 */
#ifndef HDC2010_CONVERSION_TIME
#define HDC2010_CONVERSION_TIME       (2400)
#endif

/**
 * @brief   HDC2010 specific return values
 */
enum {
    HDC2010_OK     = 0,     /**< everything went as expected */
    HDC2010_NODEV  = -1,    /**< no HDC2010 device found on the bus */
    HDC2010_NOBUS  = -2,    /**< errors while initializing the I2C bus */
    HDC2010_BUSERR = -3     /**< error during I2C communication */
};

/**
 * @brief   Possible resolution values
 */
typedef enum {
    HDC2010_11BIT = (HDC2010_TRES11 | HDC2010_HRES11),  /**< 11-bit conversion */
    HDC2010_14BIT = (HDC2010_TRES14 | HDC2010_HRES14)   /**< 14-bit conversion */
} hdc2010_res_t;

/**
 * @brief   Parameters needed for device initialization
 */
typedef struct {
    i2c_t i2c;               /**< bus the device is connected to */
    uint8_t addr;            /**< address on that bus */
    hdc2010_res_t res;       /**< resolution used for sampling temp and hum */
    uint32_t renew_interval; /**< interval for cache renewal */
} hdc2010_params_t;

/**
 * @brief   Device descriptor for HDC2010 sensors
 */
typedef struct {
    hdc2010_params_t p;     /**< Configuration parameters */
} hdc2010_t;

/**
 * @brief   Initialize the given HDC2010 device
 *
 * @param[out] dev          device descriptor of sensor to initialize
 * @param[in]  params       configuration parameters
 *
 * @return                  HDC2010_OK on success
 * @return                  HDC2010_NOBUS if initialization of I2C bus fails
 * @return                  HDC2010_NODEV if no HDC2010 device found on bus
 */
int hdc2010_init(hdc2010_t *dev, const hdc2010_params_t *params);

/**
 * @brief   Trigger a new conversion
 *
 * After the conversion is triggered, one has to wait
 * @ref HDC2010_CONVERSION_TIME us until the results can be read using
 * @ref hdc2010_get_results().
 *
 * @param[in]  dev          device descriptor of sensor
 *
 * @return                  HDC2010_OK on success
 * @return                  HDC2010_BUSERR on I2C communication failures
 */
int hdc2010_trigger_conversion(const hdc2010_t *dev);

/**
 * @brief   Read conversion results for temperature and humidity
 *
 * @param[in]  dev          device descriptor of sensor
 * @param[out] temp         temperature [in 100 * degree centigrade]
 * @param[out] hum          humidity [in 100 * percent relative]
 *
 * @return                  HDC2010_OK on success
 * @return                  HDC2010_BUSERR on I2C communication failures
 */
int hdc2010_get_results(const hdc2010_t *dev, int16_t *temp, int16_t *hum);

/**
 * @brief   Convenience function for reading temperature and humidity
 *
 * This function will trigger a new conversion, wait for the conversion to be
 * finished and the get the results from the device.
 *
 * @param[in]  dev          device descriptor of sensor
 * @param[out] temp         temperature [in 100 * degree centigrade]
 * @param[out] hum          humidity [in 100 * percent relative]
 *
 * @return                  HDC2010_OK on success
 * @return                  HDC2010_BUSERR on I2C communication failures
 */
int hdc2010_read(const hdc2010_t *dev, int16_t *temp, int16_t *hum);

/**
 * @brief   Extended read function including caching capability
 *
 * This function will return cached values if they are within the sampling
 * period (HDC2010_PARAM_RENEW_INTERVAL), or will trigger a new conversion,
 * wait for the conversion to be finished and the get the results from the
 * device.
 *
 * @param[in]  dev          device descriptor of sensor
 * @param[out] temp         temperature [in 100 * degree centigrade]
 * @param[out] hum          humidity [in 100 * percent relative]
 *
 * @return                  HDC2010_OK on success
 * @return                  HDC2010_BUSERR on I2C communication failures
 */
int hdc2010_read_cached(const hdc2010_t *dev, int16_t *temp, int16_t *hum);

#ifdef __cplusplus
}
#endif

#endif /* HDC2010_H */
/** @} */

