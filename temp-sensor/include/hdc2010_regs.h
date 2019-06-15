/*
 * Copyright (C) 2014 PHYTEC Messtechnik GmbH
 *               2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_hdc2010
 * @{
 *
 * @file
 * @brief       Register definitions for HDC2010 devices
 *
 * @author      Johann Fischer <j.fischer@phytec.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef HDC2010_REGS_H
#define HDC2010_REGS_H

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @name    Manufacturer and Device IDs
 * @{
 */
#define HDC2010_MID_VALUE          0x4954
#define HDC2010_DID_VALUE          0x2010
/** @} */

/**
 * @name    Register Map
 * @{
 */
#define HDC2010_TEMPERATURE             (0x00)
#define HDC2010_HUMIDITY                (0x02)
#define HDC2010_CONFIG                  (0x0E)
#define HDC2010_MEASUREMENT_CONFIG      (0x0F)
#define HDC2010_MANUFACTURER_ID         (0xFC)
#define HDC2010_DEVICE_ID               (0xFE)

/*
        #define TEMP_LOW 0x00
        #define TEMP_HIGH 0x01
        #define HUMID_LOW 0x02
        #define HUMID_HIGH 0x03
        #define INTERRUPT_DRDY 0x04
        #define TEMP_MAX 0x05
        #define HUMID_MAX 0x06
        #define INTERRUPT_CONFIG 0x07
        #define TEMP_OFFSET_ADJUST 0x08
        #define HUM_OFFSET_ADJUST 0x09
        #define TEMP_THR_L 0x0A
        #define TEMP_THR_H 0x0B
        #define HUMID_THR_L 0x0C
        #define HUMID_THR_H 0x0D
        #define CONFIG 0x0E
        #define MEASUREMENT_CONFIG 0x0F
        #define MID_L 0xFC
        #define MID_H 0xFD
        #define DEVICE_ID_L 0xFE
        #define DEVICE_ID_H 0xFF
*/

/** @} */

/**
 * @name    Configuration register bitmap
 * @{
 */
#define HDC2010_RST                 (1 << 15)
#define HDC2010_HEAT                (1 << 13)
#define HDC2010_SEQ_MOD             (1 << 12)
#define HDC2010_BTST_LOW            (1 << 11)
#define HDC2010_TRES_MSK            (1 << 10)
#define HDC2010_TRES11              (1 << 10)
#define HDC2010_TRES14              (0)
#define HDC2010_HRES_MSK            (1 << 9 | 1 << 8)
#define HDC2010_HRES14              (0)
#define HDC2010_HRES11              (1 << 8)
#define HDC2010_HRES8               (1 << 9)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* HDC2010_REGS_H */
/** @} */
