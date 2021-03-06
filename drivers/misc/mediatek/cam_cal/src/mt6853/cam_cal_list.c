/*
 * Copyright (C) 2018 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */
#include <linux/kernel.h>
#include "cam_cal_list.h"
#include "eeprom_i2c_common_driver.h"
#include "eeprom_i2c_custom_driver.h"
#include "kd_imgsensor.h"

#define MAX_EEPROM_SIZE_16K 0x4000

struct stCAM_CAL_LIST_STRUCT g_camCalList[] = {
	/*Below is commom sensor */
#if defined(CONFIG_MTK_CAM_PD2083F_EX)
	{S5KGW3SP13PD2083F_EX_SENSOR_ID, 0xA4, Common_read_region,MAX_EEPROM_SIZE_16K},
	{S5KGH1SM24PD2083F_EX_SENSOR_ID, 0xA2, Common_read_region,MAX_EEPROM_SIZE_16K},
	{OV8856PD2083F_EX_SENSOR_ID, 0xA8, Common_read_region,MAX_EEPROM_SIZE_16K},
	{OV02B10PD2083F_EX_SENSOR_ID, 0xA4, Common_read_region,MAX_EEPROM_SIZE_16K},
#elif defined(CONFIG_MTK_CAM_PD2123)
	{S5KJN1SQ03PD2131_SENSOR_ID, 0xA0, Common_read_region, MAX_EEPROM_SIZE_16K},
	{S5KGW3SP13PD2123_SENSOR_ID, 0xA0, Common_read_region, MAX_EEPROM_SIZE_16K},
	{IMX355PD2131_SENSOR_ID, 0xA2, Common_read_region, MAX_EEPROM_SIZE_16K},
	{OV02B10PD2131_SENSOR_ID, 0xA4, Common_read_region, MAX_EEPROM_SIZE_16K},
#elif defined(CONFIG_MTK_CAM_PD2159F_EX)
	//add by vivo chenjunliang  2021.8.24 start
	{S5KJN1SQ03PD2159_SENSOR_ID, 0xA0, Common_read_region},
	{S5KGH1SM24PD2159_SENSOR_ID, 0xA2, Common_read_region},
	{OV8856PD2159_SENSOR_ID, 0xA8, Common_read_region},
	{GC02M1PD2159_SENSOR_ID, 0xA4, Common_read_region},
	//add by vivo chenjunliang  2021.8.24 end
#elif defined(CONFIG_MTK_CAM_PD2166)
    {S5KJN1SQ03PD2166_SENSOR_ID, 0xA0, Common_read_region},
	{GC02M1PD2166_SENSOR_ID, 0xA4, Common_read_region},
	{HI1634QPD2166_SENSOR_ID, 0xA2, Common_read_region},
	{OV13B10PD2166_SENSOR_ID, 0xA4, Common_read_region},
	{S5K4H7PD2166_SENSOR_ID, 0xA2, Common_read_region},
#elif defined(CONFIG_MTK_CAM_PD2166A)
    {S5KJN1SQ03PD2166A_SENSOR_ID, 0xA0, Common_read_region},
	{GC02M1PD2166A_SENSOR_ID, 0xA4, Common_read_region},
	{HI1634QPD2166A_SENSOR_ID, 0xA2, Common_read_region},
	{OV13B10PD2166A_SENSOR_ID, 0xA4, Common_read_region},
	{OV13B10V1PD2166A_SENSOR_ID, 0xA4, Common_read_region},
	{S5K4H7PD2166A_SENSOR_ID, 0xA2, Common_read_region},
	{S5K4H7V1PD2166A_SENSOR_ID, 0xA2, Common_read_region},
#elif defined(CONFIG_MTK_CAM_PD2167F_EX)
	{S5KGW1SD03PD2167F_SENSOR_ID, 0xA0, Common_read_region, MAX_EEPROM_SIZE_16K},
	{S5KJNVSQ04PD2167F_SENSOR_ID, 0xA2, Common_read_region, MAX_EEPROM_SIZE_16K},
	{S5KJN1SQ03PD2167F_SENSOR_ID, 0xA2, Common_read_region, MAX_EEPROM_SIZE_16K},
	{OV8856PD2167F_SENSOR_ID, 0xA8, Common_read_region,MAX_EEPROM_SIZE_16K},
	{HI846PD2167F_SENSOR_ID, 0xA6, Common_read_region, MAX_EEPROM_SIZE_16K},
	{OV02B10PD2167F_SENSOR_ID, 0xA4, Common_read_region, MAX_EEPROM_SIZE_16K},
#else
	{IMX586_SENSOR_ID, 0xA0, Common_read_region, MAX_EEPROM_SIZE_16K,
		BL24SA64_write_region},
	{IMX576_SENSOR_ID, 0xA2, Common_read_region},
	{IMX519_SENSOR_ID, 0xA0, Common_read_region},
	{IMX319_SENSOR_ID, 0xA2, Common_read_region, MAX_EEPROM_SIZE_16K},
	{S5K3M5SX_SENSOR_ID, 0xA2, Common_read_region, MAX_EEPROM_SIZE_16K,
		BL24SA64_write_region},
	{IMX686_SENSOR_ID, 0xA0, Common_read_region, MAX_EEPROM_SIZE_16K},
	{HI846_SENSOR_ID, 0xA0, Common_read_region, MAX_EEPROM_SIZE_16K},
	{S5KGD1SP_SENSOR_ID, 0xA8, Common_read_region, MAX_EEPROM_SIZE_16K},
	{OV16A10_SENSOR_ID, 0xA0, Common_read_region, MAX_EEPROM_SIZE_16K},
	{S5K3P9SP_SENSOR_ID, 0xA8, Common_read_region},
	{S5K2T7SP_SENSOR_ID, 0xA4, Common_read_region},
	{IMX386_SENSOR_ID, 0xA0, Common_read_region},
	{S5K2L7_SENSOR_ID, 0xA0, Common_read_region},
	{IMX398_SENSOR_ID, 0xA0, Common_read_region},
	{IMX350_SENSOR_ID, 0xA0, Common_read_region},
	{IMX386_MONO_SENSOR_ID, 0xA0, Common_read_region},
	{S5KJD1_SENSOR_ID, 0xB0, Common_read_region, DEFAULT_MAX_EEPROM_SIZE_8K,
		DW9763_write_region},
	{IMX499_SENSOR_ID, 0xA0, Common_read_region},
	{IMX481_SENSOR_ID, 0xA4, Common_read_region, DEFAULT_MAX_EEPROM_SIZE_8K,
		BL24SA64_write_region},
	/*  ADD before this line */
	{0, 0, 0}       /*end of list */
#endif
};

unsigned int cam_cal_get_sensor_list(
	struct stCAM_CAL_LIST_STRUCT **ppCamcalList)
{
	if (ppCamcalList == NULL)
		return 1;

	*ppCamcalList = &g_camCalList[0];
	return 0;
}


