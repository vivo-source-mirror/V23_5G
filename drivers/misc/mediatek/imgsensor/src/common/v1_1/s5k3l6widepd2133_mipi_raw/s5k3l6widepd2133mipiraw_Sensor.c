/*
 * Copyright (C) 2016 MediaTek Inc.
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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *	 s5k3l6widepd2133mipi_Sensor.c
 *
 * Project:
 * --------
 *	 ALPS
 *
 * Description:
 * ------------
 *	 Source code of Sensor driver
 *
 *
 *----------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#define PFX "S5K3L6WIDEPD2133_camera_sensor"
#define pr_fmt(fmt) PFX "[%s] " fmt, __func__

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/types.h>

#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "../imgsensor_i2c.h"
#include "s5k3l6widepd2133mipiraw_Sensor.h"
#define LOG_INF(format, args...)    \
	pr_debug(PFX "[%s] " format, __func__, ##args)

static DEFINE_SPINLOCK(imgsensor_drv_lock);
static bool bIsLongExposure = KAL_FALSE;

static struct imgsensor_info_struct imgsensor_info = {
	.sensor_id = S5K3L6WIDEPD2133_SENSOR_ID,

	.checksum_value = 0xb1f1b3cc,

	.pre = {
		.pclk = 479556000,				/* record different mode's pclk */
		.linelength  = 4896,				/* record different mode's linelength */
		.framelength = 3265,			/* record different mode's framelength */
		.startx = 0,
		.starty = 0,
		.grabwindow_width  = 4096,		/* record different mode's width of grabwindow */
		.grabwindow_height = 3072,		/* record different mode's height of grabwindow */
		/* following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/* following for GetDefaultFramerateByScenario()	*/
		.mipi_pixel_rate = 436800000,
		.max_framerate = 300,
	},
	.cap = {
		.pclk = 479556000,				/* record different mode's pclk */
		.linelength  = 4896,				/* record different mode's linelength */
		.framelength = 3265,			/* record different mode's framelength */
		.startx = 0,					/* record different mode's startx of grabwindow */
		.starty = 0,					/* record different mode's starty of grabwindow */
		.grabwindow_width  = 4096,		/* record different mode's width of grabwindow */
		.grabwindow_height = 3072,		/* record different mode's height of grabwindow */
		/* following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/* following for GetDefaultFramerateByScenario()	*/
		.mipi_pixel_rate = 436800000,
		.max_framerate = 300,
	},
	.normal_video = {
		.pclk = 479556000,				/* record different mode's pclk */
		.linelength  = 4896,				/* record different mode's linelength */
		.framelength = 3265,			/* record different mode's framelength */
		.startx = 0,
		.starty = 0,
		.grabwindow_width  = 4096,		/* record different mode's width of grabwindow */
		.grabwindow_height = 2304,		/* record different mode's height of grabwindow */
		/* following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/* following for GetDefaultFramerateByScenario()	*/
		.mipi_pixel_rate = 436800000,
		.max_framerate = 300,
	},
	.hs_video = {
		.pclk = 480000000,				/* record different mode's pclk */
		.linelength  = 4720,				/* record different mode's linelength */
		.framelength = 848,			/* record different mode's framelength */
		.startx = 0,					/* record different mode's startx of grabwindow */
		.starty = 0,					/* record different mode's starty of grabwindow */
		.grabwindow_width  = 1040,		/* record different mode's width of grabwindow */
		.grabwindow_height = 780,		/* record different mode's height of grabwindow */
		/* following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/* following for GetDefaultFramerateByScenario()	*/
		.mipi_pixel_rate = 308000000,		
		.max_framerate = 1200,
	},
	.slim_video = {
		.pclk = 480000000,				/* record different mode's pclk */
		.linelength  = 4896,				/* record different mode's linelength */
		.framelength = 3260,			/* record different mode's framelength */
		.startx = 0,
		.starty = 0,
		.grabwindow_width  = 2080,		/* record different mode's width of grabwindow */
		.grabwindow_height = 1560,		/* record different mode's height of grabwindow */
		/* following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/* following for GetDefaultFramerateByScenario()	*/
		.mipi_pixel_rate = 240000000,
		.max_framerate = 300,
	},
	.custom1 = {		/*EIS Video*/
		.pclk = 479556000,				/* record different mode's pclk */
		.linelength  = 4896,				/* record different mode's linelength */
		.framelength = 3265,			/* record different mode's framelength */
		.startx = 0,					/* record different mode's startx of grabwindow */
		.starty = 0,					/* record different mode's starty of grabwindow */
		.grabwindow_width  = 4096,		/* record different mode's width of grabwindow */
		.grabwindow_height = 3072,		/* record different mode's height of grabwindow */
		/* following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/* following for GetDefaultFramerateByScenario()	*/
		.mipi_pixel_rate = 436800000,
		.max_framerate = 300,
                     	
	},
	.custom2 = {
		.pclk = 479556000,				/* record different mode's pclk */
		.linelength  = 4896,				/* record different mode's linelength */
		.framelength = 3265,			/* record different mode's framelength */
		.startx = 0,					/* record different mode's startx of grabwindow */
		.starty = 0,					/* record different mode's starty of grabwindow */
		.grabwindow_width  = 4096,		/* record different mode's width of grabwindow */
		.grabwindow_height = 3072,		/* record different mode's height of grabwindow */
		/* following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/* following for GetDefaultFramerateByScenario()	*/
		.mipi_pixel_rate = 436800000,
		.max_framerate = 300,
               	
	},
	.custom3 = {
		.pclk = 479556000,				/* record different mode's pclk */
		.linelength  = 4896,				/* record different mode's linelength */
		.framelength = 3265,			/* record different mode's framelength */
		.startx = 0,					/* record different mode's startx of grabwindow */
		.starty = 0,					/* record different mode's starty of grabwindow */
		.grabwindow_width  = 4096,		/* record different mode's width of grabwindow */
		.grabwindow_height = 3072,		/* record different mode's height of grabwindow */
		/* following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/* following for GetDefaultFramerateByScenario()	*/
		.mipi_pixel_rate = 436800000,
		.max_framerate = 300,
	},
	.custom4 = {
		.pclk = 479556000,				/* record different mode's pclk */
		.linelength  = 4896,				/* record different mode's linelength */
		.framelength = 3265,			/* record different mode's framelength */
		.startx = 0,					/* record different mode's startx of grabwindow */
		.starty = 0,					/* record different mode's starty of grabwindow */
		.grabwindow_width  = 4096,		/* record different mode's width of grabwindow */
		.grabwindow_height = 3072,		/* record different mode's height of grabwindow */
		/* following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/* following for GetDefaultFramerateByScenario()	*/
		.mipi_pixel_rate = 436800000,
		.max_framerate = 300,
	},
	.custom5 = {
		.pclk = 479556000,				/* record different mode's pclk */
		.linelength  = 4896,				/* record different mode's linelength */
		.framelength = 3265,			/* record different mode's framelength */
		.startx = 0,					/* record different mode's startx of grabwindow */
		.starty = 0,					/* record different mode's starty of grabwindow */
		.grabwindow_width  = 4096,		/* record different mode's width of grabwindow */
		.grabwindow_height = 3072,		/* record different mode's height of grabwindow */
		/* following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/* following for GetDefaultFramerateByScenario()	*/
		.mipi_pixel_rate = 436800000,
		.max_framerate = 300,
	},
	.custom6 = {		/*Dual Cam slave 2784x2088 30fps*/
		.pclk = 479556000,				/* record different mode's pclk */
		.linelength  = 5536,				/* record different mode's linelength */
		.framelength = 2887,			/* record different mode's framelength */
		.startx = 0,					/* record different mode's startx of grabwindow */
		.starty = 0,					/* record different mode's starty of grabwindow */
		.grabwindow_width  = 2784,		/* record different mode's width of grabwindow */
		.grabwindow_height = 2088,		/* record different mode's height of grabwindow */
		/* following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		/* following for GetDefaultFramerateByScenario()	*/
		.mipi_pixel_rate = 436800000,
		.max_framerate = 300, 
                     	
	},
	.margin = 2,			//sensor framelength & shutter margin
	.min_shutter = 4,               /*min shutter*/
	.min_gain = 64,
	.max_gain = 1024,
	.min_gain_iso = 100,
	.gain_step = 2,
	.gain_type = 2,
	/*max framelength by sensor register's limitation*/
	.max_frame_length = 0xFFFF,//REG0x0202 <=REG0x0340-5//max framelength by sensor register's limitation
	/*shutter delay frame for AE cycle, 2 frame*/
	.ae_shut_delay_frame = 0,
	/*sensor gain delay frame for AE cycle,2 frame*/
	.ae_sensor_gain_delay_frame = 0,
	.ae_ispGain_delay_frame = 2,    /*isp gain delay frame for AE cycle*/
	.frame_time_delay_frame = 2,	/* The delay frame of setting frame length  */
	.ihdr_support = 0,	            /*1, support; 0,not support*/
	.ihdr_le_firstline = 0,         /*1,le first ; 0, se first*/
	.sensor_mode_num = 11,	  //support sensor mode num ,don't support Slow motion
	.cap_delay_frame = 2,		//enter capture delay frame num
	.pre_delay_frame = 2, 		//enter preview delay frame num
	.video_delay_frame = 2,		//enter video delay frame num
	.hs_video_delay_frame = 2,	//enter high speed video  delay frame num
	.slim_video_delay_frame = 2,//enter slim video delay frame num
	.custom1_delay_frame = 2,
	.custom2_delay_frame = 2,
	.custom3_delay_frame = 2,
	.custom4_delay_frame = 2,
	.custom5_delay_frame = 2,
	.custom6_delay_frame = 2,
		
	.isp_driving_current = ISP_DRIVING_4MA,     /*mclk driving current   //5/19 xyw pd2133 modified driving current 4ma*/

	/*sensor_interface_type*/
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,
	/*0,MIPI_OPHY_NCSI2;  1,MIPI_OPHY_CSI2*/
	.mipi_sensor_type = MIPI_OPHY_NCSI2,
	/*0,MIPI_SETTLEDELAY_AUTO; 1,MIPI_SETTLEDELAY_MANNUAL*/
	.mipi_settle_delay_mode = 1,
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_Gr,//sensor output first pixel color
	.mclk = 26,         /*mclk value, suggest 24 or 26 for 24Mhz or 26Mhz*/
	.mipi_lane_num = SENSOR_MIPI_4_LANE,

	/*record sensor support all write id addr*/
	.i2c_addr_table = {0x5a, 0xff},
	.i2c_speed = 400,
};


static struct imgsensor_struct imgsensor = {
	.mirror = IMAGE_NORMAL,		/*mirrorflip information*/

	/*IMGSENSOR_MODE enum value,record current sensor mode*/
	.sensor_mode = IMGSENSOR_MODE_INIT,
	.shutter = 0x200,					//current shutter
	.gain = 0x200,						//current gain
	.dummy_pixel = 0,			/*current dummypixel*/
	.dummy_line = 0,			/*current dummyline*/

	/*full size current fps : 24fps for PIP, 30fps for Normal or ZSD*/
	.current_fps = 0,
	/*auto flicker enable: KAL_FALSE for disable auto flicker*/
	.autoflicker_en = KAL_FALSE,
	/*test pattern mode or not. KAL_FALSE for in test pattern mode*/
	.test_pattern = KAL_FALSE,
	/*current scenario id*/
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,
	/*sensor need support LE, SE with HDR feature*/
	.ihdr_mode = 0,
	.current_ae_effective_frame = 1,
	.i2c_write_id = 0x5a,  /*record current sensor's i2c write id*/

};

/* Sensor output window information */
static struct SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[11] = {
 { 4160, 3120, 32,	24, 4096, 3072, 4096, 3072, 0, 0, 4096, 3072, 0, 0, 4096, 3072}, /* preview */
 { 4160, 3120, 32,	24, 4096, 3072, 4096, 3072, 0, 0, 4096, 3072, 0, 0, 4096, 3072}, /* capture */
 { 4160, 3120, 32,408, 4096, 2304, 4096, 2304, 0, 0, 4096, 2304, 0, 0, 4096, 2304},  /* video */
 { 4160, 3120, 0, 	0, 4160, 3120, 1040, 780, 	0, 0, 1040, 780, 0, 0, 1040, 780}, /* HS_hight speed video */
 { 4160, 3120, 0,	0, 4160, 3120, 2080, 1560, 0, 0, 2080, 1560, 0, 0, 2080, 1560}, /* slim video */
 { 4160, 3120, 32,	24, 4096, 3072, 4096, 3072, 0, 0, 4096, 3072, 0, 0, 4096, 3072},//custom1
 { 4160, 3120, 32,	24, 4096, 3072, 4096, 3072, 0, 0, 4096, 3072, 0, 0, 4096, 3072},//custom2
 { 4160, 3120, 32,	24, 4096, 3072, 4096, 3072, 0, 0, 4096, 3072, 0, 0, 4096, 3072},//custom3
 { 4160, 3120, 32,	24, 4096, 3072, 4096, 3072, 0, 0, 4096, 3072, 0, 0, 4096, 3072},//custom4
 { 4160, 3120, 32,	24, 4096, 3072, 4096, 3072, 0, 0, 4096, 3072, 0, 0, 4096, 3072},//custom5
 { 4160, 3120, 688,	516, 2784, 2088, 2784, 2088, 0,	0, 2784, 2088, 0, 0, 2784, 2088},//custom6
};

static struct  SENSOR_RAWINFO_STRUCT imgsensor_raw_info = {
	 4096,//raw_weight 
 	 3072,//raw_height
	 2,//raw_dataBit
	 BAYER_GRBG,//raw_colorFilterValue
	 64,//raw_blackLevel
	 120.0,//raw_viewAngle
	 10,//raw_bitWidth
	 16//raw_maxSensorGain
};
/*PD information update*/
static struct SET_PD_BLOCK_INFO_T imgsensor_pd_info =
{
	.i4OffsetX = 0,
	.i4OffsetY = 24,
	.i4PitchX = 64,
	.i4PitchY = 64,
	.i4PairNum =16,
	.i4SubBlkW =16,
	.i4SubBlkH =16, 
	.i4BlockNumX = 65,
	.i4BlockNumY = 48,
    .i4PosL = { {4, 31}, {20, 35}, {40, 35}, {56, 31}, {8, 51}, {24, 55}, {36, 55}, {52, 51}, {8, 67}, {24, 63}, {36, 63}, {52, 67}, {4, 87}, {20, 83}, {40, 83}, {56, 87} },


    .i4PosR = { {4, 35}, {20, 39}, {40, 39}, {56, 35}, {8, 47}, {24, 51}, {36, 51}, {52, 47}, {8, 71}, {24, 67}, {36, 67}, {52, 71}, {4, 83}, {20, 79}, {40, 79}, {56, 83} },
 	.i4Crop = {{32, 24}, {32, 24},{32, 408},{0, 0},{0, 0},{32, 24},{32, 24},{32, 24},{32, 24},{32, 24},{688, 516}},
	.iMirrorFlip = 0,
};
#if 0
/*PD information update 2784*2088 */
static struct SET_PD_BLOCK_INFO_T imgsensor_pd_info_Dual_Cam =
{
	.i4OffsetX = 0,
	.i4OffsetY = 24,
	.i4PitchX = 64,
	.i4PitchY = 64,
	.i4PairNum =16,
	.i4SubBlkW =16,
	.i4SubBlkH =16, 
	.i4BlockNumX = 43,
	.i4BlockNumY = 32,
    .i4PosL = { {60, 27}, {76, 31}, {96, 31}, {112, 27}, {64, 47}, {80, 51}, {92, 51}, {108, 47}, {64, 63}, {80, 59}, {92, 59}, {108, 63}, {60, 83}, {76, 79}, {96, 79}, {112, 83} },
    .i4PosR = { {60, 31}, {76, 35}, {96, 35}, {112, 31}, {64, 43}, {80, 47}, {92, 47}, {108, 43}, {64, 67}, {80, 63}, {92, 63}, {108, 67}, {60, 79}, {76, 75}, {96, 75}, {112, 79} },
	//.i4Crop = {{0, 0}, {0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0}},
	.iMirrorFlip = 0,
};
#endif
/*vivo hope add for Camera otp errorcode*/
extern int s5k3l6widepd2133_vivo_otp_read(void);
static int vivo_otp_read_when_power_on = 0;
extern otp_error_code_t S5K3L6WIDEPD2133_OTP_ERROR_CODE;
MUINT32  sn_inf_main_s5k3l6widepd2133[13];  /*0 flag   1-12 data*/
MUINT32  material_inf_main_s5k3l6widepd2133[4];
extern unsigned int is_atboot;
extern unsigned  int s5k3l6widepd2133_flag;
/*vivo hope add end*/

static kal_uint16 read_cmos_sensor_16_16(kal_uint32 addr)
{
	kal_uint16 get_byte= 0;
	char pusendcmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
	 /*kdSetI2CSpeed(imgsensor_info.i2c_speed); Add this func to set i2c speed by each sensor*/
	iReadRegI2C(pusendcmd , 2, (u8*)&get_byte, 2, imgsensor.i2c_write_id);
	return ((get_byte<<8)&0xff00)|((get_byte>>8)&0x00ff);
}


static void write_cmos_sensor_16_16(kal_uint16 addr, kal_uint16 para)
{
	char pusendcmd[4] = {(char)(addr >> 8) , (char)(addr & 0xFF) ,(char)(para >> 8),(char)(para & 0xFF)};
	/* kdSetI2CSpeed(imgsensor_info.i2c_speed); Add this func to set i2c speed by each sensor*/
	iWriteRegI2C(pusendcmd , 4, imgsensor.i2c_write_id);
}

static kal_uint16 read_cmos_sensor_16_8(kal_uint16 addr)
{
	kal_uint16 get_byte= 0;
	char pusendcmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
	/*kdSetI2CSpeed(imgsensor_info.i2c_speed);  Add this func to set i2c speed by each sensor*/
	iReadRegI2C(pusendcmd , 2, (u8*)&get_byte,1,imgsensor.i2c_write_id);
	return get_byte;
}

static void write_cmos_sensor_16_8(kal_uint16 addr, kal_uint8 para)
{
	char pusendcmd[4] = {(char)(addr >> 8) , (char)(addr & 0xFF) ,(char)(para & 0xFF)};
	 /* kdSetI2CSpeed(imgsensor_info.i2c_speed);Add this func to set i2c speed by each sensor*/
	iWriteRegI2C(pusendcmd , 3, imgsensor.i2c_write_id);
}

static kal_uint32 streaming_control(kal_bool enable)
{
	int timeout = (10000 / imgsensor.current_fps) + 1;
	int i = 0;
	int framecnt = 0;

	LOG_INF("streaming_enable(0= Sw Standby,1= streaming): %d\n", enable);
	if (enable) {
		write_cmos_sensor_16_16(0x3C1E,0x0100);
		write_cmos_sensor_16_8(0x0100,0x01);
		write_cmos_sensor_16_16(0x3C1E,0x0000);
		mdelay(10);
	} else {
		write_cmos_sensor_16_8(0x0100, 0x00);
		for (i = 0; i < timeout; i++) {
			mdelay(10);
			framecnt = read_cmos_sensor_16_8(0x0005);
			LOG_INF("framecnt= %d.\n", framecnt);
			if ( framecnt == 0xFF) {
				LOG_INF(" Stream Off OK at i=%d.\n", i);
				return ERROR_NONE;
			}
		}
		LOG_INF("Stream Off Fail! framecnt= %d.\n", framecnt);
	}
	return ERROR_NONE;
}


#define MULTI_WRITE 1

#if MULTI_WRITE
#define I2C_BUFFER_LEN 225	/* trans# max is 255, each 3 bytes */
#else
#define I2C_BUFFER_LEN 4

#endif

static kal_uint16 table_write_cmos_sensor(kal_uint16 *para, kal_uint32 len)
{
	char puSendCmd[I2C_BUFFER_LEN];
	kal_uint32 tosend, IDX;
	kal_uint16 addr = 0, addr_last = 0, data;

	tosend = 0;
	IDX = 0;
	while (len > IDX) {
		addr = para[IDX];

		{
			puSendCmd[tosend++] = (char)(addr >> 8);
			puSendCmd[tosend++] = (char)(addr & 0xFF);
			data = para[IDX + 1];
			puSendCmd[tosend++] = (char)(data >> 8);
			puSendCmd[tosend++] = (char)(data & 0xFF);
			IDX += 2;
			addr_last = addr;

		}
#if MULTI_WRITE
		/* Write when remain buffer size is less than 4 bytes or reach end of data */
		if ((I2C_BUFFER_LEN - tosend) < 4 || IDX == len || addr != addr_last) {
			iBurstWriteReg_multi(puSendCmd, tosend, imgsensor.i2c_write_id,
								4, imgsensor_info.i2c_speed);
			tosend = 0;
		}
#else
		iWriteRegI2CTiming(puSendCmd, 4, imgsensor.i2c_write_id, imgsensor_info.i2c_speed);
		tosend = 0;

#endif
	}
	return 0;
}




static void set_dummy(void)
{
	LOG_INF("dummyline = %d, dummypixels = %d \n", imgsensor.dummy_line, imgsensor.dummy_pixel);
	/* you can set dummy by imgsensor.dummy_line and imgsensor.dummy_pixel, or you can set dummy by imgsensor.frame_length and imgsensor.line_length */
	write_cmos_sensor_16_16(0x0340, imgsensor.frame_length & 0xFFFF);	  
	write_cmos_sensor_16_16(0x0342, imgsensor.line_length & 0xFFFF);
}	/*	set_dummy  */
static void set_max_framerate(UINT16 framerate, kal_bool min_framelength_en)
{
	kal_uint32 frame_length = imgsensor.frame_length;

	LOG_INF("framerate = %d, min framelength should enable(%d) \n", framerate,min_framelength_en);
   
	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;
	spin_lock(&imgsensor_drv_lock);
	imgsensor.frame_length = (frame_length > imgsensor.min_frame_length) ? frame_length : imgsensor.min_frame_length; 
	imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	//dummy_line = frame_length - imgsensor.min_frame_length;
	//if (dummy_line < 0)
		//imgsensor.dummy_line = 0;
	//else
		//imgsensor.dummy_line = dummy_line;
	//imgsensor.frame_length = frame_length + imgsensor.dummy_line;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
	{
		imgsensor.frame_length = imgsensor_info.max_frame_length;
		imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	}
	if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);
	set_dummy();
}	/*	set_max_framerate  */

#if 0
static void write_shutter(kal_uint32 shutter)
{
	kal_uint16 realtime_fps = 0;
	   
	/* 0x3500, 0x3501, 0x3502 will increase VBLANK to get exposure larger than frame exposure */
	/* AE doesn't update sensor gain at capture mode, thus extra exposure lines must be updated here. */
	
	// OV Recommend Solution
	// if shutter bigger than frame_length, should extend frame length first
	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)		
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;
	
	if (imgsensor.autoflicker_en) { 
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if(realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296,0);
		else if(realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146,0);	
	} else {
		// Extend frame length
		write_cmos_sensor_16_16(0x0340, imgsensor.frame_length & 0xFFFF);
	}

	// Update Shutter
	write_cmos_sensor_16_16(0x0202, (shutter) & 0xFFFF);
	LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter,imgsensor.frame_length);

	//LOG_INF("frame_length = %d ", frame_length);
	
}	/*	write_shutter  */
#endif

static void set_shutter_frame_length(
	kal_uint32 shutter, kal_uint16 frame_length)
{
	//check
	kal_uint16 realtime_fps = 0;
	kal_int32 dummy_line = 0;

	//spin_lock_irqsave(&imgsensor_drv_lock, flags);
	//imgsensor.shutter = shutter;
	//spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

	spin_lock(&imgsensor_drv_lock);
	/*Change frame time*/
	if(frame_length > 1)
		dummy_line = frame_length - imgsensor.frame_length;
	imgsensor.frame_length = imgsensor.frame_length + dummy_line;

	if (shutter > imgsensor.frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;

	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);

	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;

	if (imgsensor.autoflicker_en == KAL_TRUE) //auroflicker:need to avoid 15fps and 30 fps
	{
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if(realtime_fps >= 297 && realtime_fps <= 305)
		{
			realtime_fps = 296;
			set_max_framerate(realtime_fps,0);
		}
		else if(realtime_fps >= 147 && realtime_fps <= 150)
		{
			realtime_fps = 146;
			set_max_framerate(realtime_fps ,0);
		}
		else
		{
			write_cmos_sensor_16_16(0x0340, imgsensor.frame_length & 0xFFFF);
		}
	}
	else
	{
		write_cmos_sensor_16_16(0x0340, imgsensor.frame_length & 0xFFFF);
	}

//	write_cmos_sensor_16_16(0x0350,0x00);		/* Disable auto extend */

	/* Update Shutter */
	write_cmos_sensor_16_16(0x0202, shutter & 0xFFFF);
	LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter, imgsensor.frame_length);
}


/*************************************************************************
 * FUNCTION
 *	set_shutter
 *
 * DESCRIPTION
 *	This function set e-shutter of sensor to change exposure time.
 *
 * PARAMETERS
 *	iShutter : exposured lines
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static void set_shutter(kal_uint32 shutter)
{
	unsigned long flags;
	kal_uint16 realtime_fps = 0;
	//kal_uint32 frame_length = 0;
	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);
	//write_shutter(shutter);
	/* 0x3500, 0x3501, 0x3502 will increase VBLANK to get exposure larger than frame exposure */
	/* AE doesn't update sensor gain at capture mode, thus extra exposure lines must be updated here. */
	// OV Recommend Solution
	// if shutter bigger than frame_length, should extend frame length first
	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	//shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;
	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if(realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296,0);
		else if(realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146,0);
		else {
		// Extend frame length
		write_cmos_sensor_16_16(0x0340, imgsensor.frame_length & 0xFFFF);
		}
	} else {
		// Extend frame length
		write_cmos_sensor_16_16(0x0340, imgsensor.frame_length & 0xFFFF);
	}
	
	if(shutter > 65530) {  //linetime=10160/960000000<< maxshutter=3023622-line=32s
		/*enter long exposure mode */
		kal_uint32 exposure_time;
		kal_uint32 new_framelength;
		kal_uint32 long_shutter;
		kal_uint32 temp1_030F = 0, temp2_0821 = 0, temp3_38c3 = 0, temp4_0342 = 0, temp5_0343 = 0;
		kal_uint32 long_shutter_linelenght = 0;
		int dat, dat1;
		LOG_INF("pengzuo enter long exposure mode\n");
		LOG_INF("pengzuo Calc long exposure  +\n");
		
		bIsLongExposure = KAL_TRUE;
		
		//exposure_time = shutter*imgsensor_info.cap.linelength/960000;//ms
		//exposure_time = shutter/1000*4896/480;//ms
		exposure_time = (shutter*1)/100;//ms
		
		if(exposure_time < 7000) {
			temp1_030F = 0x78;
			temp2_0821 = 0x78;
			temp3_38c3 = 0x06;
			temp4_0342 = 0x13;
			temp5_0343 = 0x20;
			long_shutter_linelenght = 4896;
			LOG_INF("pengzuo   7s exposure_time = %d\n",exposure_time);
		} else if (7000 <= exposure_time  && exposure_time <= 22200) {
			temp1_030F = 0x78;
			temp2_0821 = 0x78;
			temp3_38c3 = 0x05;
			temp4_0342 = 0x3f;
			temp5_0343 = 0x90;
			long_shutter_linelenght = 16272;
			LOG_INF("pengzuo  7s  22.2s exposure_time = %d\n",exposure_time);
		} else if (22200 < exposure_time  && exposure_time <= 23500) {
			temp1_030F = 0x64;
			temp2_0821 = 0x64;
			temp3_38c3 = 0x05;
			temp4_0342 = 0x43;
			temp5_0343 = 0x40;
			long_shutter_linelenght = 17216;
			LOG_INF("pengzuo  22.2s  23.5s exposure_time = %d\n",exposure_time);
		} 

		//long_shutter = (exposure_time*pclk-256)/lineleght/64 = (shutter*linelength-256)/linelength/pow_shift
       		 long_shutter = (shutter*48)/(long_shutter_linelenght/10);//line_lengthpck已经改变需要重新计算longshuter的linelength
		new_framelength = long_shutter+16;
		LOG_INF("pengzuo long exposure Shutter = %d\n",shutter);
       		LOG_INF("pengzuo Calc long exposure_time=%dms,long_shutter=%d, framelength=%d.\n", exposure_time,long_shutter, new_framelength);
		LOG_INF("pengzuo Calc long exposure  -\n");

		streaming_control(KAL_FALSE);		
		//write_cmos_sensor_16_8(0x0100, 0x00); /*stream off */ 
		//mdelay(100);
		write_cmos_sensor_16_8(0x0307, 0x60); 
		write_cmos_sensor_16_8(0x3C1F, 0x03); 
		write_cmos_sensor_16_8(0x030D, 0x03); 
		write_cmos_sensor_16_8(0x030E, 0x00); 
		write_cmos_sensor_16_8(0x030F, temp1_030F); 
		write_cmos_sensor_16_8(0x3C17, 0x04); 
		write_cmos_sensor_16_8(0x0820, 0x00); 
		write_cmos_sensor_16_8(0x0821, temp2_0821); 
		write_cmos_sensor_16_8(0x38C5, 0x03); 
		write_cmos_sensor_16_8(0x38D9, 0x00); 
		write_cmos_sensor_16_8(0x38DB, 0x08); 
		write_cmos_sensor_16_8(0x38DD, 0x13); 
		write_cmos_sensor_16_8(0x38C3, temp3_38c3); 
		write_cmos_sensor_16_8(0x38C1, 0x00); 
		write_cmos_sensor_16_8(0x38D7, 0x0F); 
		write_cmos_sensor_16_8(0x38D5, 0x03); 
		write_cmos_sensor_16_8(0x38B1, 0x01); 
		write_cmos_sensor_16_8(0x3932, 0x20); 
		write_cmos_sensor_16_8(0x3938, 0x20); 
		
		write_cmos_sensor_16_8(0x0340, (new_framelength&0xFF00)>>8); //
		write_cmos_sensor_16_8(0x0341, (new_framelength&0x00FF)); //00000111
		write_cmos_sensor_16_8(0x0342, temp4_0342); 
		write_cmos_sensor_16_8(0x0343, temp5_0343); 
		write_cmos_sensor_16_8(0x0202, (long_shutter&0xFF00)>>8); 
		write_cmos_sensor_16_8(0x0203, (long_shutter&0x00FF)); 
		
		write_cmos_sensor_16_8(0x3C1E, 0x01); 
		write_cmos_sensor_16_8(0x0100, 0x01); 
		write_cmos_sensor_16_8(0x3C1E, 0x00);

		dat = read_cmos_sensor_16_8(0x0340);
		dat1 = read_cmos_sensor_16_8(0x0341);
		LOG_INF("pengzuo long exposure dat = %x, dat1 = %x\n",dat,dat1);
		LOG_INF("pengzuo long exposure (new_framelength&0xFF00)>>8 %x\n",(new_framelength&0xFF00)>>8);
		LOG_INF("pengzuo long exposure (new_framelength&0x00FF) %x\n",(new_framelength&0x00FF));
		//write_cmos_sensor_16_8(0x0100, 0x01); /*stream on */ 
		/*streaming_control(KAL_TRUE);*/
		
		LOG_INF("pengzuo long exposure  stream on-\n");
	} else {
	// Update Shutter
	if (bIsLongExposure == KAL_TRUE) {
		bIsLongExposure = KAL_FALSE;
		
		LOG_INF("[Exit long shutter + ]  shutter =%d, framelength =%d\n", shutter,imgsensor.frame_length);
	      //write_cmos_sensor_16_8(0x0100, 0x00); /*stream off */
	     //mdelay(200);
		streaming_control(KAL_FALSE);

		write_cmos_sensor_16_8(0x0307, 0x78); 
		write_cmos_sensor_16_8(0x3C1F, 0x00); 
		write_cmos_sensor_16_8(0x030D, 0x03); 
		write_cmos_sensor_16_8(0x030E, 0x00); 
		write_cmos_sensor_16_8(0x030F, 0x4B); 
		write_cmos_sensor_16_8(0x3C17, 0x00); 
		write_cmos_sensor_16_8(0x0820, 0x04); 
		write_cmos_sensor_16_8(0x0821, 0xB0); 
		write_cmos_sensor_16_8(0x38C5, 0x09); 
		write_cmos_sensor_16_8(0x38D9, 0x2A); 
		write_cmos_sensor_16_8(0x38DB, 0x0A); 
		write_cmos_sensor_16_8(0x38DD, 0x0B); 
		write_cmos_sensor_16_8(0x38C3, 0x0A); 
		write_cmos_sensor_16_8(0x38C1, 0x0F); 
		write_cmos_sensor_16_8(0x38D7, 0x0A); 
		write_cmos_sensor_16_8(0x38D5, 0x09); 
		write_cmos_sensor_16_8(0x38B1, 0x0F); 
		write_cmos_sensor_16_8(0x3932, 0x18); 
		write_cmos_sensor_16_8(0x3938, 0x00); 
		
		write_cmos_sensor_16_8(0x0340, 0x0C); 
		write_cmos_sensor_16_8(0x0341, 0xBC); 
		write_cmos_sensor_16_8(0x0342, 0x13); 
		write_cmos_sensor_16_8(0x0343, 0x20); 
		write_cmos_sensor_16_8(0x0202, 0x03); 
		write_cmos_sensor_16_8(0x0203, 0xDE); 
		
		//write_cmos_sensor_16_8(0x3C1E, 0x01); 
		//write_cmos_sensor_16_8(0x0100, 0x01); 
		//write_cmos_sensor_16_8(0x3C1E, 0x00);
		
		streaming_control(KAL_TRUE);
		LOG_INF("[Exit long shutter - ] shutter =%d, framelength =%d\n", shutter,imgsensor.frame_length);

	}else {
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;
	write_cmos_sensor_16_16(0x0202, shutter & 0xFFFF);
	LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter,imgsensor.frame_length);
		}
	}
}

static kal_uint16 gain2reg(const kal_uint16 gain)
{
	kal_uint16 iReg = 0x0000;
	//gain = 64 = 1x real gain.
    	iReg = gain/2;
	//reg_gain = reg_gain & 0xFFFF;
	return (kal_uint16)iReg;
}

/*************************************************************************
 * FUNCTION
 * set_gain
 *
 * DESCRIPTION
 *	This function is to set global gain to sensor.
 *
 * PARAMETERS
 *	iGain : sensor global gain(base: 0x40)
 *
 * RETURNS
 *	the actually gain set to sensor.
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 reg_gain;
	
	LOG_INF("set_gain %d \n", gain);
  	//gain = 64 = 1x real gain.
	if (gain < BASEGAIN || gain > 16 * BASEGAIN) {
		LOG_INF("Error gain setting");
		if (gain < BASEGAIN)
			gain = BASEGAIN;
		else if (gain > 16 * BASEGAIN)
			gain = 16 * BASEGAIN;		 
	}

    	reg_gain = gain2reg(gain);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.gain = reg_gain; 
	spin_unlock(&imgsensor_drv_lock);
	LOG_INF("gain = %d , reg_gain = 0x%x\n ", gain, reg_gain);

	write_cmos_sensor_16_16(0x0204, (reg_gain&0xFFFF));    
	return gain;
}	/*	set_gain  */

static void set_mirror_flip(kal_uint8 image_mirror)
{
	LOG_INF("image_mirror = %d\n", image_mirror);

	/********************************************************
	   *
	   *   0x3820[2] ISP Vertical flip
	   *   0x3820[1] Sensor Vertical flip
	   *
	   *   0x3821[2] ISP Horizontal mirror
	   *   0x3821[1] Sensor Horizontal mirror
	   *
	   *   ISP and Sensor flip or mirror register bit should be the same!!
	   *
	   ********************************************************/
	spin_lock(&imgsensor_drv_lock);
	imgsensor.mirror= image_mirror;
	spin_unlock(&imgsensor_drv_lock);
	switch (image_mirror) {
		case IMAGE_NORMAL:
			write_cmos_sensor_16_8(0x0101,0X00); //GR
			break;
		case IMAGE_H_MIRROR:
			write_cmos_sensor_16_8(0x0101,0X01); //R
			break;
		case IMAGE_V_MIRROR:
			write_cmos_sensor_16_8(0x0101,0X02); //B
			break;
		case IMAGE_HV_MIRROR:
			write_cmos_sensor_16_8(0x0101,0X03); //GB
			break;
		default:
			LOG_INF("Error image_mirror setting\n");
	}

}

/*************************************************************************
 * FUNCTION
 *	night_mode
 *
 * DESCRIPTION
 *	This function night mode of sensor.
 *
 * PARAMETERS
 *	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
#if 0
static void night_mode(kal_bool enable)
{
/*No Need to implement this function*/
}	/*	night_mode	*/
#endif

static kal_uint16 addr_data_pair_init[] = {
0x3084, 0x1314,
0x3266, 0x0001,
0x3242, 0x2020,
0x306A, 0x2F4C,
0x306C, 0xCA01,
0x307A, 0x0D20,
0x309E, 0x002D,
0x3072, 0x0013,
0x3074, 0x0977,
0x3076, 0x9411,
0x3024, 0x0016,
0x3070, 0x3D00,
0x3002, 0x0E00,
0x3006, 0x1000,
0x300A, 0x0C00,
0x3010, 0x0400,
0x3018, 0xC500,
0x303A, 0x0204,
0x345A, 0x0000,
0x345C, 0x0000,
0x345E, 0x0000,
0x3460, 0x0000,
0x3066, 0x7E00,
0x3004, 0x0800,
0x3C08, 0xFFFF,

};

static kal_uint16 addr_data_pair_preview[] = {
0x0344, 0x0040,
0x0346, 0x0020,
0x0348, 0x103F,
0x034A, 0x0C1F,
0x034C, 0x1000,
0x034E, 0x0C00,
0x0900, 0x0000,
0x0380, 0x0001,
0x0382, 0x0001,
0x0384, 0x0001,
0x0386, 0x0001,
0x0114, 0x0330,
0x0110, 0x0002,
0x0136, 0x1A00,
0x0304, 0x0003,
0x0306, 0x0053,
0x3C1E, 0x0000,
0x030C, 0x0003,
0x030E, 0x003F,
0x3C16, 0x0000,
0x0300, 0x0006,
0x0342, 0x1320,
0x0340, 0x0CC1,
0x38C4, 0x0009,
0x38D8, 0x0026,
0x38DA, 0x0009,
0x38DC, 0x000A,
0x38C2, 0x0009,
0x38C0, 0x000D,
0x38D6, 0x0009,
0x38D4, 0x0008,
0x38B0, 0x000D,
0x3932, 0x1000,
0x3938, 0x000C,
0x0820, 0x0444,
0x380C, 0x0090,
0x3064, 0xFFCF,
0x309C, 0x0640,
0x3090, 0x8800,
0x3238, 0x000C,
0x314A, 0x5F00,
0x3300, 0x0000,
0x3400, 0x0000,
0x3402, 0x4E42,
0x32B2, 0x0000,
0x32B4, 0x0000,
0x32B6, 0x0000,
0x32B8, 0x0000,
0x3C34, 0x0048,
0x3C36, 0x2000,
0x3C38, 0x0028,
0x393E, 0x4000,
0x303A, 0x0204,
0x3034, 0x4B01,
0x3036, 0x0029,
0x3032, 0x4800,
0x320E, 0x049E,

};

static kal_uint16 addr_data_pair_capture[] = {
0x0344, 0x0040,
0x0346, 0x0020,
0x0348, 0x103F,
0x034A, 0x0C1F,
0x034C, 0x1000,
0x034E, 0x0C00,
0x0900, 0x0000,
0x0380, 0x0001,
0x0382, 0x0001,
0x0384, 0x0001,
0x0386, 0x0001,
0x0114, 0x0330,
0x0110, 0x0002,
0x0136, 0x1A00,
0x0304, 0x0003,
0x0306, 0x0053,
0x3C1E, 0x0000,
0x030C, 0x0003,
0x030E, 0x003F,
0x3C16, 0x0000,
0x0300, 0x0006,
0x0342, 0x1320,
0x0340, 0x0CC1,
0x38C4, 0x0009,
0x38D8, 0x0026,
0x38DA, 0x0009,
0x38DC, 0x000A,
0x38C2, 0x0009,
0x38C0, 0x000D,
0x38D6, 0x0009,
0x38D4, 0x0008,
0x38B0, 0x000D,
0x3932, 0x1000,
0x3938, 0x000C,
0x0820, 0x0444,
0x380C, 0x0090,
0x3064, 0xFFCF,
0x309C, 0x0640,
0x3090, 0x8800,
0x3238, 0x000C,
0x314A, 0x5F00,
0x3300, 0x0000,
0x3400, 0x0000,
0x3402, 0x4E42,
0x32B2, 0x0000,
0x32B4, 0x0000,
0x32B6, 0x0000,
0x32B8, 0x0000,
0x3C34, 0x0048,
0x3C36, 0x2000,
0x3C38, 0x0028,
0x393E, 0x4000,
0x303A, 0x0204,
0x3034, 0x4B01,
0x3036, 0x0029,
0x3032, 0x4800,
0x320E, 0x049E,

};                

static kal_uint16 addr_data_pair_normal_video[] = {                                
0x0344, 0x0040,
0x0346, 0x01A0,
0x0348, 0x103F,
0x034A, 0x0A9F,
0x034C, 0x1000,
0x034E, 0x0900,
0x0900, 0x0000,
0x0380, 0x0001,
0x0382, 0x0001,
0x0384, 0x0001,
0x0386, 0x0001,
0x0114, 0x0330,
0x0110, 0x0002,
0x0136, 0x1A00,
0x0304, 0x0003,
0x0306, 0x0053,
0x3C1E, 0x0000,
0x030C, 0x0003,
0x030E, 0x003F,
0x3C16, 0x0000,
0x0300, 0x0006,
0x0342, 0x1320,
0x0340, 0x0CC1,
0x38C4, 0x0009,
0x38D8, 0x0026,
0x38DA, 0x0009,
0x38DC, 0x000A,
0x38C2, 0x0009,
0x38C0, 0x000D,
0x38D6, 0x0009,
0x38D4, 0x0008,
0x38B0, 0x000D,
0x3932, 0x1000,
0x3938, 0x000C,
0x0820, 0x0444,
0x380C, 0x0090,
0x3064, 0xFFCF,
0x309C, 0x0640,
0x3090, 0x8800,
0x3238, 0x000C,
0x314A, 0x5F00,
0x3300, 0x0000,
0x3400, 0x0000,
0x3402, 0x4E42,
0x32B2, 0x0000,
0x32B4, 0x0000,
0x32B6, 0x0000,
0x32B8, 0x0000,
0x3C34, 0x0048,
0x3C36, 0x2000,
0x3C38, 0x0028,
0x393E, 0x4000,
0x303A, 0x0204,
0x3034, 0x4B01,
0x3036, 0x0029,
0x3032, 0x4800,
0x320E, 0x049E,

};                                                                      


static kal_uint16 addr_data_pair_hs_video[] = {
	0x0344, 0x0340,
	0x0346, 0x0350,
	0x0348, 0x0D3F,
	0x034A, 0x08EF,
	0x034C, 0x0500,
	0x034E, 0x02D0,
	0x0900, 0x0122,
	0x0380, 0x0001,
	0x0382, 0x0001,
	0x0384, 0x0001,
	0x0386, 0x0003,
	0x0114, 0x0330,
	0x0110, 0x0002,
	0x0136, 0x1800,
	0x0304, 0x0004,
	0x0306, 0x0078,
	0x3C1E, 0x0000,
	0x030C, 0x0003,
	0x030E, 0x0059,
	0x3C16, 0x0002,
	0x0300, 0x0006,
	0x0342, 0x1320,
	0x0340, 0x0330,
	0x38C4, 0x0002,
	0x38D8, 0x0008,
	0x38DA, 0x0003,
	0x38DC, 0x0004,
	0x38C2, 0x0003,
	0x38C0, 0x0001,
	0x38D6, 0x0003,
	0x38D4, 0x0002,
	0x38B0, 0x0004,
	0x3932, 0x1800,
	0x3938, 0x000C,
	0x0820, 0x0162,
	0x380C, 0x003D,
	0x3064, 0xFFCF,
	0x309C, 0x0640,
	0x3090, 0x8000,
	0x3238, 0x000B,
	0x314A, 0x5F02,
	0x3300, 0x0000,
	0x3400, 0x0000,
	0x3402, 0x4E46,
	0x32B2, 0x0000,
	0x32B4, 0x0000,
	0x32B6, 0x0000,
	0x32B8, 0x0000,
	0x3C34, 0x0008,
	0x3C36, 0x3000,
	0x3C38, 0x0024,
	0x393E, 0x4000,	
};

static kal_uint16 addr_data_pair_slim_video[] = {
	0x0344, 0x0008,
	0x0346, 0x0008,
	0x0348, 0x1077,
	0x034A, 0x0C37,
	0x034C, 0x0838,
	0x034E, 0x0618,
	0x0900, 0x0122,
	0x0380, 0x0001,
	0x0382, 0x0001,
	0x0384, 0x0001,
	0x0386, 0x0003,
	0x0114, 0x0330,
	0x0110, 0x0002,
	0x0136, 0x1800,
	0x0304, 0x0004,
	0x0306, 0x0078,
	0x3C1E, 0x0000,
	0x030C, 0x0003,
	0x030E, 0x004B,
	0x3C16, 0x0001,
	0x0300, 0x0006,
	0x0342, 0x1320,
	0x0340, 0x0CBC,
	0x38C4, 0x0004,
	0x38D8, 0x0012,
	0x38DA, 0x0005,
	0x38DC, 0x0006,
	0x38C2, 0x0005,
	0x38C0, 0x0005,
	0x38D6, 0x0005,
	0x38D4, 0x0004,
	0x38B0, 0x0007,
	0x3932, 0x1000,
	0x3938, 0x000C,
	0x0820, 0x0258,
	0x380C, 0x0049,
	0x3064, 0xFFCF,
	0x309C, 0x0600,
	0x3090, 0x8000,
	0x3238, 0x000B,
	0x314A, 0x5F02,
	0x3300, 0x0000,
	0x3400, 0x0000,
	0x3402, 0x4E46,
	0x32B2, 0x0000,
	0x32B4, 0x0000,
	0x32B6, 0x0000,
	0x32B8, 0x0000,
	0x3C34, 0x0008,
	0x3C36, 0x3000,
	0x3C38, 0x0020,
	0x393E, 0x4000,		
	0x303A, 0x0204,  //carl 20180803
	0x3034, 0x4B01,
	0x3036, 0x0029,
	0x3032, 0x4800,
	0x320E, 0x049E,	

};

static kal_uint16 addr_data_pair_custom6[] = {
	0x0344,	0x02d0,
	0x0346,	0x020c,
	0x0348,	0x0daf,
	0x034A,	0x0A33,
	0x034C,	0x0ae0,
	0x034E,	0x0828,
	0x0900,	0x0000,
	0x0380,	0x0001,
	0x0382,	0x0001,
	0x0384,	0x0001,
	0x0386,	0x0001,
	0x0114,	0x0330,
	0x0110,	0x0002,
	0x0136,	0x1A00,
	0x0304,	0x0003,
	0x0306,	0x0053,
	0x3C1E,	0x0000,
	0x030C,	0x0003,
	0x030E,	0x003F,
	0x3C16,	0x0000,
	0x0300,	0x0006,
	0x0342,	0x15A0,
	0x0340,	0x0B47,
	0x38C4,	0x0009,
	0x38D8,	0x0026,
	0x38DA,	0x0009,
	0x38DC,	0x000A,
	0x38C2,	0x0009,
	0x38C0,	0x000D,
	0x38D6,	0x0009,
	0x38D4,	0x0008,
	0x38B0, 0x000D,
	0x3932,	0x1000,
	0x3938,	0x000C,
	0x0820, 0x0444,
	0x380C,	0x0090,
	0x3064,	0xFFCF,
	0x309C,	0x0640,
	0x3090,	0x8800,
	0x3238,	0x000C,
	0x314A,	0x5F00,
	0x3300,	0x0000,
	0x3400,	0x0000,
	0x3402,	0x4E42,
	0x32B2,	0x0000,
	0x32B4,	0x0000,
	0x32B6,	0x0000,
	0x32B8,	0x0000,
	0x3C34, 0x0048,
	0x3C36, 0x2000,
	0x3C38, 0x0028,
	0x393E,	0x4000,
	0x303A, 0x0204,
	0x3034, 0x4B01,
	0x3036, 0x0029,
	0x3032, 0x4800,
	0x320E, 0x049E,


};                 

#if 0
static kal_uint16 addr_data_pair_custom2[] = {                                 
	0x0344, 0x01E0,
	0x0346, 0x0158,
	0x0348, 0x0E9F,
	0x034A, 0x0AE7,
	0x034C, 0x0CC0,
	0x034E, 0x0990,
	0x0900, 0x0000,
	0x0380, 0x0001,
	0x0382, 0x0001,
	0x0384, 0x0001,
	0x0386, 0x0001,
	0x0114, 0x0330,
	0x0110, 0x0002,
	0x0136, 0x1800,
	0x0304, 0x0004,
	0x0306, 0x0078,
	0x3C1E, 0x0000,
	0x030C, 0x0003,
	0x030E, 0x0046,
	0x3C16, 0x0000,
	0x0300, 0x0006,
	0x0342, 0x1320,
	0x0340, 0x1326,
	0x38C4, 0x0009,
	0x38D8, 0x0027,
	0x38DA, 0x0009,
	0x38DC, 0x000A,
	0x38C2, 0x0009,
	0x38C0, 0x000D,
	0x38D6, 0x0009,
	0x38D4, 0x0008,
	0x38B0, 0x000E,
	0x3932, 0x1000,
	0x3938, 0x000C,
	0x0820, 0x0460,
	0x380C, 0x0090,
	0x3064, 0xFFCF,
	0x309C, 0x0640,
	0x3090, 0x8800,
	0x3238, 0x000C,
	0x314A, 0x5F00,
	0x3300, 0x0000,
	0x3400, 0x0000,
	0x3402, 0x4E42,
	0x32B2, 0x0000,
	0x32B4, 0x0000,
	0x32B6, 0x0000,
	0x32B8, 0x0000,
	0x3C34, 0x0008,
	0x3C36, 0x3000,
	0x3C38, 0x0020,
	0x393E, 0x4000,	
	0x303A, 0x0204,  //carl 20180803
	0x3034, 0x4B01,
	0x3036, 0x0029,
	0x3032, 0x4800,
	0x320E, 0x049E,
};                                                                           
#endif
static void sensor_init(void)
{
	pr_debug("sensor_init() E\n");
	write_cmos_sensor_16_16(0x0100, 0x0000);
	write_cmos_sensor_16_16(0x0000, 0x0050);
  	write_cmos_sensor_16_16(0x0000,0x30C6);
    mdelay(3);
	write_cmos_sensor_16_16(0x0A02, 0x3400);
    table_write_cmos_sensor(addr_data_pair_init,
		   sizeof(addr_data_pair_init) / sizeof(kal_uint16));
	
	//write_cmos_sensor_16_8(0x3268, 0x00);/* affect test patten */
	/*write_cmos_sensor_16_16(0x0100, 0x0000);*/
	pr_debug("sensor_init() end\n");
}	/*	sensor_init  */

static void preview_setting(void)
{
	/*Preview 2320*1744 30fps 24M MCLK 4lane 1488Mbps/lane*/
	/*preview 30.01fps*/
	pr_debug("preview_setting() E\n");
	table_write_cmos_sensor(addr_data_pair_preview,
		   sizeof(addr_data_pair_preview) / sizeof(kal_uint16));
	pr_debug("preview_setting() end\n");
}	/*	preview_setting  */


static void capture_setting(kal_uint16 currefps)
{
	pr_debug("capture_setting() E! currefps:%d\n", currefps);
	table_write_cmos_sensor(addr_data_pair_capture,
		   sizeof(addr_data_pair_capture) / sizeof(kal_uint16));
}


static void normal_video_setting(kal_uint16 currefps)
{
	pr_debug("normal_video_setting() E! currefps:%d\n", currefps);
	table_write_cmos_sensor(addr_data_pair_normal_video,
		   sizeof(addr_data_pair_normal_video) / sizeof(kal_uint16));
	pr_debug("normal_video_setting() end! currefps:%d\n", currefps);
}

static void hs_video_setting(void)
{
	pr_debug("hs_video_setting() E\n");
	table_write_cmos_sensor(addr_data_pair_hs_video,
		   sizeof(addr_data_pair_hs_video) / sizeof(kal_uint16));
}


static void slim_video_setting(void)
{
	pr_debug("slim_video_setting() E\n");
	table_write_cmos_sensor(addr_data_pair_slim_video,
			   sizeof(addr_data_pair_slim_video) / sizeof(kal_uint16));
	pr_debug("slim_video_setting() end\n");

}

static void custom6_setting(kal_uint16 currefps)
{
	
	table_write_cmos_sensor(addr_data_pair_custom6,
		   sizeof(addr_data_pair_custom6) / sizeof(kal_uint16));
	pr_debug("custom6_setting() end! \n");
}




static kal_uint32 return_sensor_id(void)
{
    return ((read_cmos_sensor_16_8(0x0000) << 8) | read_cmos_sensor_16_8(0x0001));
}

/*************************************************************************
 * FUNCTION
 *	get_imgsensor_id
 *
 * DESCRIPTION
 *	This function get the sensor ID
 *
 * PARAMETERS
 *	*sensorID : return the sensor ID
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	/* sensor have two i2c address 0x6c 0x6d & 0x21 0x20,
	 * we should detect the module used i2c address
	 */

	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
				*sensor_id = return_sensor_id() + 4;
				if (*sensor_id == imgsensor_info.sensor_id) {
				/*vivo hope add for AT cammand start*/
				pr_debug("i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id,*sensor_id);
				if (is_atboot == 1) {
					pr_info("i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id, *sensor_id);
					pr_info("AT mode skip now return\n");
					return ERROR_NONE;
				}
				/*vivo hope add for AT cammand end*/
				/*vivo hope add for CameraEM otp errorcode*/
				pr_debug("add:start read eeprom  - when_power_on  = %d\n",vivo_otp_read_when_power_on);
				vivo_otp_read_when_power_on = s5k3l6widepd2133_vivo_otp_read();
			
				pr_debug("add:end read eeprom - when_power_on = %d\n",vivo_otp_read_when_power_on);
			   	return ERROR_NONE;
				/*vivo hope add end*/
				if (s5k3l6widepd2133_flag == 1){
					pr_debug("i2c write id: 0x%x, sensor id: 0x%x, s5k3l6widepd2133_flag = %d\n", imgsensor.i2c_write_id,*sensor_id, s5k3l6widepd2133_flag);
					return ERROR_NONE;
				}
			}
			pr_debug("Read sensor id fail, id: 0x%x, *sensor_id =%d\n",
				imgsensor.i2c_write_id, *sensor_id);
			retry--;
		} while (retry > 0);
		i++;
		retry = 2;
	}
	if (*sensor_id != imgsensor_info.sensor_id || s5k3l6widepd2133_flag != 1) {
	/* if Sensor ID is not correct, Must set *sensor_id to 0xFFFFFFFF*/
		*sensor_id = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	return ERROR_NONE;
}


/*************************************************************************
 * FUNCTION
 *	open
 *
 * DESCRIPTION
 *	This function initialize the registers of CMOS sensor
 *
 * PARAMETERS
 *	None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 open(void)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint16 sensor_id = 0;

    pr_debug("PLATFORM:MT6797,MIPI 4LANE +++++ ++++ \n");
    pr_debug("preview 2112*1568@30fps,1080Mbps/lane; video 1024*768@120fps,864Mbps/lane; capture 4224*3136@30fps,1080Mbps/lane\n");

	/* sensor have two i2c address 0x6c 0x6d & 0x21 0x20,
	 * we should detect the module used i2c address
	 */

	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			sensor_id = return_sensor_id() + 4;
			if (sensor_id == imgsensor_info.sensor_id) {
				pr_debug("i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id,sensor_id);
				break;
			}
			pr_debug("Read sensor id fail: 0x%x, id: 0x%x\n", imgsensor.i2c_write_id,sensor_id);
			retry--;
		} while (retry > 0);
		i++;
		if (sensor_id == imgsensor_info.sensor_id)
			break;
		retry = 2;
	}
	if (imgsensor_info.sensor_id != sensor_id)
		return ERROR_SENSOR_CONNECT_FAIL;

	/* initail sequence write in  */
	sensor_init();

	spin_lock(&imgsensor_drv_lock);

	imgsensor.autoflicker_en = KAL_FALSE;
	imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.dummy_pixel = 0;
	imgsensor.dummy_line = 0;
	imgsensor.ihdr_mode = 0;
	imgsensor.test_pattern = KAL_FALSE;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}	/*	open  */



/*************************************************************************
 * FUNCTION
 *	close
 *
 * DESCRIPTION
 *
 *
 * PARAMETERS
 *	None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 close(void)
{
	pr_debug("E\n");

	/*No Need to implement this function*/

	return ERROR_NONE;
}	/*	close  */


/*************************************************************************
 * FUNCTION
 * preview
 *
 * DESCRIPTION
 *	This function start the sensor preview.
 *
 * PARAMETERS
 *  *image_window : address pointer of pixel numbers in one period of HSYNC
 *  *sensor_config_data : address pointer of line numbers in one period of VSYNC
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
				  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	pr_debug("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	preview_setting();
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/*	preview   */

/*************************************************************************
 * FUNCTION
 *	capture
 *
 * DESCRIPTION
 *	This function setup the CMOS sensor in capture MY_OUTPUT mode
 *
 * PARAMETERS
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
				  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	pr_debug("E\n");
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	/*PIP capture: 24fps for less than 13M, 20fps for 16M,15fps for 20M*/
		if (imgsensor.current_fps != imgsensor_info.cap.max_framerate)
			pr_debug(
		"Warning: current_fps %d fps is not support, so use cap's setting: %d fps!\n",
			imgsensor.current_fps,
			imgsensor_info.cap.max_framerate/10);

		imgsensor.pclk = imgsensor_info.cap.pclk;
		imgsensor.line_length = imgsensor_info.cap.linelength;
		imgsensor.frame_length = imgsensor_info.cap.framelength;
		imgsensor.min_frame_length = imgsensor_info.cap.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	 capture_setting(imgsensor.current_fps);
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/* capture() */
static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
				  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	pr_debug("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	normal_video_setting(imgsensor.current_fps);
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/*	normal_video   */

static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
				  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	pr_debug("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
	imgsensor.pclk = imgsensor_info.hs_video.pclk;
	/*imgsensor.video_mode = KAL_TRUE;*/
	imgsensor.line_length = imgsensor_info.hs_video.linelength;
	imgsensor.frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	/*imgsensor.current_fps = 300;*/
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	hs_video_setting();
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/*	hs_video   */

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
				  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	pr_debug("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;
	/*imgsensor.video_mode = KAL_TRUE;*/
	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	/*imgsensor.current_fps = 300;*/
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	slim_video_setting();
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/*	slim_video	 */

static kal_uint32 custom6(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
				  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	pr_debug("E\n");
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM6;
	/*PIP capture: 24fps for less than 13M, 20fps for 16M,15fps for 20M*/
		if (imgsensor.current_fps != imgsensor_info.custom6.max_framerate)
			pr_debug(
		"Warning: current_fps %d fps is not support, so use custom6's setting: %d fps!\n",
			imgsensor.current_fps,
			imgsensor_info.custom6.max_framerate/10);

		imgsensor.pclk = imgsensor_info.custom6.pclk;
		imgsensor.line_length = imgsensor_info.custom6.linelength;
		imgsensor.frame_length = imgsensor_info.custom6.framelength;
		imgsensor.min_frame_length = imgsensor_info.custom6.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	custom6_setting(imgsensor.current_fps);
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}

static kal_uint32 get_resolution(
	MSDK_SENSOR_RESOLUTION_INFO_STRUCT(*sensor_resolution))
{
	pr_debug("E\n");
	sensor_resolution->SensorFullWidth =
		imgsensor_info.cap.grabwindow_width;
	sensor_resolution->SensorFullHeight =
		imgsensor_info.cap.grabwindow_height;

	sensor_resolution->SensorPreviewWidth =
		imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorPreviewHeight =
		imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorVideoWidth =
		imgsensor_info.normal_video.grabwindow_width;
	sensor_resolution->SensorVideoHeight =
		imgsensor_info.normal_video.grabwindow_height;

	sensor_resolution->SensorHighSpeedVideoWidth =
		imgsensor_info.hs_video.grabwindow_width;
	sensor_resolution->SensorHighSpeedVideoHeight =
		imgsensor_info.hs_video.grabwindow_height;

	sensor_resolution->SensorSlimVideoWidth	 =
		imgsensor_info.slim_video.grabwindow_width;
	sensor_resolution->SensorSlimVideoHeight =
		imgsensor_info.slim_video.grabwindow_height;

	sensor_resolution->SensorCustom1Width = 
		imgsensor_info.custom1.grabwindow_width;
	sensor_resolution->SensorCustom1Height =
		imgsensor_info.custom1.grabwindow_height;
		
	sensor_resolution->SensorCustom2Width = 
		imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorCustom2Height =
		imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorCustom3Width = 
		imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorCustom3Height =
		imgsensor_info.pre.grabwindow_height;
		
	sensor_resolution->SensorCustom4Width = 
		imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorCustom4Height =
		imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorCustom5Width = 
		imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorCustom5Height =
		imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorCustom6Width = 
		imgsensor_info.custom6.grabwindow_width;
	sensor_resolution->SensorCustom6Height =
		imgsensor_info.custom6.grabwindow_height;
	return ERROR_NONE;
}	/*	get_resolution	*/

static kal_uint32 get_info(enum MSDK_SCENARIO_ID_ENUM scenario_id,
				  MSDK_SENSOR_INFO_STRUCT *sensor_info,
				  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	pr_debug("scenario_id = %d\n", scenario_id);

	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;

	/* not use */
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW;
	/* inverse with datasheet*/
	sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4; /* not use */
	sensor_info->SensorResetActiveHigh = FALSE; /* not use */
	sensor_info->SensorResetDelayCount = 5; /* not use */

	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;

	sensor_info->SensorOutputDataFormat =
		imgsensor_info.sensor_output_dataformat;

	sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
	sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;

	sensor_info->HighSpeedVideoDelayFrame =
		imgsensor_info.hs_video_delay_frame;
	sensor_info->SlimVideoDelayFrame =
		imgsensor_info.slim_video_delay_frame;
		
	sensor_info->Custom1DelayFrame = imgsensor_info.custom1_delay_frame;
	sensor_info->Custom2DelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->Custom3DelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->Custom4DelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->Custom5DelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->Custom6DelayFrame = imgsensor_info.custom6_delay_frame;
	
	sensor_info->SensorMasterClockSwitch = 0; /* not use */
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;

	sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame;

	sensor_info->AESensorGainDelayFrame =
		imgsensor_info.ae_sensor_gain_delay_frame;
	sensor_info->AEISPGainDelayFrame =
		imgsensor_info.ae_ispGain_delay_frame;
	sensor_info->FrameTimeDelayFrame = imgsensor_info.frame_time_delay_frame;

	sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;
	sensor_info->PDAF_Support = 1; /*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode*/
	/* 0: NO PDAF, 1: PDAF Raw Data mode,
	 * 2:PDAF VC mode(Full),
	 * 3:PDAF VC mode(Binning)
	 */
	sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num;
	sensor_info->SensorClockFreq = imgsensor_info.mclk;
	sensor_info->SensorClockDividCount = 3; /* not use */
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2; /* not use */
	sensor_info->SensorPixelClockCount = 3; /* not use */
	sensor_info->SensorDataLatchCount = 2; /* not use */

	sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->SensorWidthSampling = 0;  /* 0 is default 1x*/
	sensor_info->SensorHightSampling = 0;	/* 0 is default 1x*/
	sensor_info->SensorPacketECCOrder = 1;

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.pre.mipi_data_lp2hs_settle_dc;

			break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		sensor_info->SensorGrabStartX = imgsensor_info.cap.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.cap.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.cap.mipi_data_lp2hs_settle_dc;

			break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:

	    sensor_info->SensorGrabStartX = imgsensor_info.normal_video.startx;
	    sensor_info->SensorGrabStartY = imgsensor_info.normal_video.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc;

			break;

	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		sensor_info->SensorGrabStartX = imgsensor_info.hs_video.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.hs_video.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc;

			break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
	    sensor_info->SensorGrabStartX = imgsensor_info.slim_video.startx;
	    sensor_info->SensorGrabStartY = imgsensor_info.slim_video.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc;

			break;
    case MSDK_SCENARIO_ID_CUSTOM1:
	    sensor_info->SensorGrabStartX = imgsensor_info.custom1.startx;
	    sensor_info->SensorGrabStartY = imgsensor_info.custom1.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.custom1.mipi_data_lp2hs_settle_dc;

			break;
    case MSDK_SCENARIO_ID_CUSTOM2:
	    sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
	    sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.pre.mipi_data_lp2hs_settle_dc;

			break;
    case MSDK_SCENARIO_ID_CUSTOM3:
	    sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
	    sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.pre.mipi_data_lp2hs_settle_dc;

			break;
    case MSDK_SCENARIO_ID_CUSTOM4:
	    sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
	    sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.pre.mipi_data_lp2hs_settle_dc;

			break;
    case MSDK_SCENARIO_ID_CUSTOM5:
	    sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
	    sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.pre.mipi_data_lp2hs_settle_dc;

			break;
    case MSDK_SCENARIO_ID_CUSTOM6:
	    sensor_info->SensorGrabStartX = imgsensor_info.custom6.startx;
	    sensor_info->SensorGrabStartY = imgsensor_info.custom6.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.custom6.mipi_data_lp2hs_settle_dc;

			break;
	default:
		sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
			break;
	}

	return ERROR_NONE;
}	/* get_info  */


static kal_uint32 control(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			  MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	pr_debug("scenario_id = %d\n", scenario_id);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.current_scenario_id = scenario_id;
	spin_unlock(&imgsensor_drv_lock);
	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			preview(image_window, sensor_config_data);
			break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			capture(image_window, sensor_config_data);
			break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			normal_video(image_window, sensor_config_data);
			break;

	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			hs_video(image_window, sensor_config_data);
			break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
			slim_video(image_window, sensor_config_data);
			break;
    case MSDK_SCENARIO_ID_CUSTOM6:
			custom6(image_window, sensor_config_data);
			break;
	default:
			pr_debug("Error ScenarioId setting");
			preview(image_window, sensor_config_data);
			return ERROR_INVALID_SCENARIO_ID;
	}
	return ERROR_NONE;
}	/* control() */



static kal_uint32 set_video_mode(UINT16 framerate)
{
	pr_debug("framerate = %d\n ", framerate);
	/* SetVideoMode Function should fix framerate*/
	if (framerate == 0)
		/* Dynamic frame rate*/
		return ERROR_NONE;
	spin_lock(&imgsensor_drv_lock);
	if ((framerate == 300) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 296;
	else if ((framerate == 150) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 146;
	else
		imgsensor.current_fps = framerate;
	spin_unlock(&imgsensor_drv_lock);
	set_max_framerate(imgsensor.current_fps, 1);

	return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
	pr_debug("enable = %d, framerate = %d\n", enable, framerate);
	spin_lock(&imgsensor_drv_lock);
	if (enable) /*enable auto flicker*/
		imgsensor.autoflicker_en = KAL_TRUE;
	else /*Cancel Auto flick*/
		imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}


static kal_uint32 set_max_framerate_by_scenario(
	enum MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 framerate)
{
	kal_uint32 frame_length;
  
	LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);

	switch (scenario_id) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();

			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			if(framerate == 0)
				return ERROR_NONE;
			frame_length = imgsensor_info.normal_video.pclk / framerate * 10 / imgsensor_info.normal_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.normal_video.framelength) ? (frame_length - imgsensor_info.normal_video.framelength) : 0;			
			imgsensor.frame_length = imgsensor_info.normal_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			frame_length = imgsensor_info.cap.pclk / framerate * 10 / imgsensor_info.cap.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.cap.framelength) ? (frame_length - imgsensor_info.cap.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.cap.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();			
			break;

		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			frame_length = imgsensor_info.hs_video.pclk / framerate * 10 / imgsensor_info.hs_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.hs_video.framelength) ? (frame_length - imgsensor_info.hs_video.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.hs_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			frame_length = imgsensor_info.slim_video.pclk / framerate * 10 / imgsensor_info.slim_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.slim_video.framelength) ? (frame_length - imgsensor_info.slim_video.framelength): 0;	
			imgsensor.frame_length = imgsensor_info.slim_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();			
			break;
		case MSDK_SCENARIO_ID_CUSTOM6:
			frame_length = imgsensor_info.custom6.pclk / framerate * 10 / imgsensor_info.custom6.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.custom6.framelength) ? (frame_length - imgsensor_info.custom6.framelength) : 0;
			if (imgsensor.dummy_line < 0)
			imgsensor.dummy_line = 0;
			imgsensor.frame_length = imgsensor_info.custom6.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();			
		break;

	default:  //coding with  preview scenario by default
		frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
		imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
		set_dummy();			
		LOG_INF("error scenario_id = %d, we use preview scenario \n", scenario_id);
		break;
	}	
	return ERROR_NONE;
}


static kal_uint32 get_default_framerate_by_scenario(
	enum MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 *framerate)
{
	pr_debug("scenario_id = %d\n", scenario_id);

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			*framerate = imgsensor_info.pre.max_framerate;
			break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*framerate = imgsensor_info.normal_video.max_framerate;
			break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*framerate = imgsensor_info.cap.max_framerate;
			break;

	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*framerate = imgsensor_info.hs_video.max_framerate;
			break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*framerate = imgsensor_info.slim_video.max_framerate;
			break;
	case MSDK_SCENARIO_ID_CUSTOM6:
			*framerate = imgsensor_info.custom6.max_framerate;	
			break;


	default:
			*framerate = imgsensor_info.pre.max_framerate;	
			break;
	}

	return ERROR_NONE;
}

static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
	LOG_INF("enable: %d\n", enable);

	if (enable) {
		write_cmos_sensor_16_16(0x0600, 0x0002);
	} else {
		write_cmos_sensor_16_16(0x0600, 0x0000);
	}
	spin_lock(&imgsensor_drv_lock);
	imgsensor.test_pattern = enable;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}
static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
				 UINT8 *feature_para, UINT32 *feature_para_len)
{
	UINT16 *feature_return_para_16 = (UINT16 *) feature_para;
	UINT16 *feature_data_16 = (UINT16 *) feature_para;
	UINT32 *feature_return_para_32 = (UINT32 *) feature_para;
	UINT32 *feature_data_32 = (UINT32 *) feature_para;
	unsigned long long *feature_data = (unsigned long long *) feature_para;
	                                   //  0      1         2         3        4         5         6         7         8         9         10        11        12        13        14        15      
	MUINT32 i4localPosLVivo[16][2] = {{36, 7}, {52, 11}, {72, 11}, {88, 7}, {40, 27}, {56, 31}, {68, 31}, {84, 27}, {40, 43}, {56, 39}, {68, 39}, {84, 43}, {36, 63}, {52, 59}, {72, 59}, {88, 63}};	/* left pd pixel position in one block*/
	MUINT32 i4localPosRVivo[16][2] = {{36, 11},{52, 15}, {72, 15}, {88, 11}, {40, 23}, {56, 27}, {68, 27}, {84, 23}, {40, 47}, {56, 43}, {68, 43}, {84, 47}, {36, 59}, {52, 55}, {72, 55}, {88, 59}};	/* right pd pixel position in one block*/
#if 0
/*
		Start 	x	56	X Block Number	39	Unit Block X	64											
			    y 	20	y Block Number	30	Unit Block Y	64											
				        0	1	2	3	4	5	6	7	8	9	10	11	12	13	14	15
2608	Left	AF1	X	60	76	96	112	64	80	92	108	64	80	92	108	60	76	96	112
			        Y	27	31	31	27	47	51	51	47	63	59	59	63	83	79	79	83
1960	right	AF2	X	60	76	96	112	64	80	92	108	64	80	92	108	60	76	96	112
			        Y	31	35	35	31	43	47	47	43	67	63	63	67	79	75	75	79
*/
/*
       MUINT32 i4localPosLVivo_custom6[16][2] = {{60, 27}, {76, 31}, {96, 31}, {112, 27}, {64, 47}, {80, 51}, {92, 51}, {108, 47}, {64, 63}, {80, 59}, {92, 59}, {108, 63}, {60, 83}, {76, 79}, {96, 79}, {112, 83}};  /* left pd pixel position in one block*/
       MUINT32 i4localPosRVivo_custom6[16][2] = {{60, 31},{76, 35}, {96, 35}, {112, 31}, {64, 43}, {80, 47}, {92, 47}, {108, 43}, {64, 67}, {80, 63}, {92, 63}, {108, 67}, {60, 79}, {76, 75}, {96, 75}, {112, 79}};   /* right pd pixel position in one block*/
#endif
/*
		Start 	x	16	X Block Number	43	Unit Block X	64											
				y 	20	y Block Number	32	Unit Block Y	64											
						0	1	2	3	4	5	6	7	8	9	10	11	12	13	14	15
2784	Left	AF1	X	23	39	59	75	27	43	55	71	27	43	55	71	23	39	59	75
					Y	20	24	24	20	40	44	44	40	56	52	52	56	76	72	72	76
2088	right	AF2	X	23	39	59	75	27	43	55	71	27	43	55	71	23	39	59	75
					Y	24	28	28	24	36	40	40	36	60	56	56	60	72	68	68	72

*/
	                                   			//  0      1         2         3        4         5         6         7         8         9         10        11        12        13        14        15      
	MUINT32 i4localPosLVivo_custom6[16][2] = {{23, 20}, {39, 24}, {59, 24}, {75, 20}, {27, 40}, {43, 44}, {55, 44}, {71, 40}, {27, 56}, {43, 52}, {55, 52}, {71, 56}, {23, 76}, {39, 72}, {59, 72}, {75, 76}};	/* left pd pixel position in one block*/
	MUINT32 i4localPosRVivo_custom6[16][2] = {{23, 24}, {39, 28}, {59, 28}, {75, 24}, {27, 40}, {43, 40}, {55, 40}, {71, 36}, {27, 60}, {43, 56}, {55, 56}, {71, 60}, {23, 72}, {39, 68}, {59, 68}, {75, 72}};	/* right pd pixel position in one block*/

	struct SET_PD_BLOCK_INFO_T *PDAFinfo;
	/*struct SENSOR_VC_INFO_STRUCT *pvcinfo;*/
	struct SENSOR_WINSIZE_INFO_STRUCT *wininfo;
	struct SENSOR_RAWINFO_STRUCT *rawinfo;

	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data =
		(MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;

	pr_debug("feature_id = %d\n", feature_id);
	switch (feature_id) {
	case SENSOR_FEATURE_GET_PERIOD:
			*feature_return_para_16++ = imgsensor.line_length;
			*feature_return_para_16 = imgsensor.frame_length;
			*feature_para_len = 4;
			break;
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			*feature_return_para_32 = imgsensor.pclk;
			*feature_para_len = 4;
			break;
	case SENSOR_FEATURE_SET_ESHUTTER:
		set_shutter(*feature_data);
			break;
	case SENSOR_FEATURE_SET_NIGHTMODE:
	    /*night_mode((BOOL) *feature_data); no need to implement this mode*/
			break;
	case SENSOR_FEATURE_SET_GAIN:
		set_gain((UINT16) *feature_data);
			break;
	case SENSOR_FEATURE_SET_FLASHLIGHT:
			break;
	case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			break;
	case SENSOR_FEATURE_SET_REGISTER:
			write_cmos_sensor_16_8(
			    sensor_reg_data->RegAddr, sensor_reg_data->RegData);
			break;
	case SENSOR_FEATURE_GET_REGISTER:
			sensor_reg_data->RegData =
				read_cmos_sensor_16_16(sensor_reg_data->RegAddr);
			break;
	case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
		/* get the lens driver ID from EEPROM or
		 * just return LENS_DRIVER_ID_DO_NOT_CARE
		 */

		/* if EEPROM does not exist in camera module.*/
		*feature_return_para_32 = LENS_DRIVER_ID_DO_NOT_CARE;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_VIDEO_MODE:
		set_video_mode(*feature_data);
		break;
	case SENSOR_FEATURE_CHECK_SENSOR_ID:
			get_imgsensor_id(feature_return_para_32);
			break;
	case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
		set_auto_flicker_mode(
			(BOOL)*feature_data_16, *(feature_data_16+1));
		break;
	case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
		set_max_framerate_by_scenario(
		  (enum MSDK_SCENARIO_ID_ENUM)*feature_data, *(feature_data+1));
		break;
	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
		get_default_framerate_by_scenario(
			(enum MSDK_SCENARIO_ID_ENUM)*(feature_data),
			(MUINT32 *)(uintptr_t)(*(feature_data+1)));
		break;
	case SENSOR_FEATURE_SET_TEST_PATTERN:
		set_test_pattern_mode((BOOL)*feature_data);
		break;

		/*for factory mode auto testing*/
	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
			*feature_return_para_32 = imgsensor_info.checksum_value;
			*feature_para_len = 4;
			break;
	case SENSOR_FEATURE_SET_FRAMERATE:
		spin_lock(&imgsensor_drv_lock);
			imgsensor.current_fps = (UINT16)*feature_data_32;
		spin_unlock(&imgsensor_drv_lock);
			break;
	case SENSOR_FEATURE_GET_BINNING_TYPE:	
		switch (*(feature_data + 1)) {	/*2sum = 2; 4sum = 4; 4avg = 1 not 4cell sensor is 4avg*/
			
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CUSTOM1:
		case MSDK_SCENARIO_ID_CUSTOM2:	
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		default:
			*feature_return_para_32 = 1; /*BINNING_NONE,*/ 
			break;
		}
		PK_DBG("SENSOR_FEATURE_GET_BINNING_TYPE AE_binning_type:%d,\n",
			*feature_return_para_32);
		*feature_para_len = 4;
		break;		
	case SENSOR_FEATURE_GET_RAW_INFO:	
		pr_debug("SENSOR_FEATURE_GET_RAW_INFO scenarioId:%d\n",
			(UINT32) *feature_data);
		rawinfo = (struct SENSOR_RAWINFO_STRUCT *) (uintptr_t) (*(feature_data + 1));	
		memcpy((void *)rawinfo,
				(void *)&imgsensor_raw_info,
				sizeof(struct SENSOR_RAWINFO_STRUCT));
		break;			
	case SENSOR_FEATURE_GET_CROP_INFO:
		pr_debug("SENSOR_FEATURE_GET_CROP_INFO scenarioId:%d\n",
			*feature_data_32);

		wininfo =
	    (struct SENSOR_WINSIZE_INFO_STRUCT *)(uintptr_t)(*(feature_data+1));

		switch (*feature_data_32) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			memcpy(
				(void *)wininfo,
				(void *)&imgsensor_winsize_info[1],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			memcpy(
				(void *)wininfo,
				(void *)&imgsensor_winsize_info[2],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			memcpy(
				(void *)wininfo,
				(void *)&imgsensor_winsize_info[3],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			memcpy(
				(void *)wininfo,
				(void *)&imgsensor_winsize_info[4],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM6:
			memcpy(
				(void *)wininfo,
				(void *)&imgsensor_winsize_info[10],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;

		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			memcpy(
				(void *)wininfo,
				(void *)&imgsensor_winsize_info[0],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		}
		break;
	case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
		pr_debug("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n",
			(UINT16)*feature_data,
			(UINT16)*(feature_data+1), (UINT16)*(feature_data+2));

		/* ihdr_write_shutter_gain((UINT16)*feature_data,
		 * (UINT16)*(feature_data+1),(UINT16)*(feature_data+2));
		 */
		break;
	case SENSOR_FEATURE_SET_AWB_GAIN:
		break;
	case SENSOR_FEATURE_SET_HDR_SHUTTER:
		pr_debug("SENSOR_FEATURE_SET_HDR_SHUTTER LE=%d, SE=%d\n",
			(UINT16)*feature_data, (UINT16)*(feature_data+1));
		/* ihdr_write_shutter(
		 * (UINT16)*feature_data,(UINT16)*(feature_data+1));
		 */
		break;
		/******************** PDAF START >>> *********/
	case SENSOR_FEATURE_GET_PDAF_INFO:
			pr_debug("SENSOR_FEATURE_GET_PDAF_INFO scenarioId:%lld\n", *feature_data);
			PDAFinfo= (struct SET_PD_BLOCK_INFO_T *)(uintptr_t)(*(feature_data+1));

		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG: /*full*/
		case MSDK_SCENARIO_ID_CUSTOM1:
		case MSDK_SCENARIO_ID_CUSTOM2:
		case MSDK_SCENARIO_ID_CUSTOM3:
		case MSDK_SCENARIO_ID_CUSTOM4:
		case MSDK_SCENARIO_ID_CUSTOM5:
			imgsensor_pd_info.i4BlockNumX = 63,
			imgsensor_pd_info.i4BlockNumY = 48,
			imgsensor_pd_info.i4OffsetXVivo = 32;
			imgsensor_pd_info.i4OffsetYVivo = 0;
			memcpy(imgsensor_pd_info.i4PosLVivo, i4localPosLVivo, sizeof(imgsensor_pd_info.i4PosLVivo));
			memcpy(imgsensor_pd_info.i4PosRVivo, i4localPosRVivo, sizeof(imgsensor_pd_info.i4PosRVivo));
			memcpy((void *)PDAFinfo,(void *)&imgsensor_pd_info,sizeof(struct SET_PD_BLOCK_INFO_T));
			pr_debug("i4PosLVivo-i4localPosRVivo MSDK_SCENARIO_ID_CUSTOM5\n");
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			imgsensor_pd_info.i4BlockNumX = 63,
			imgsensor_pd_info.i4BlockNumY = 36,
			imgsensor_pd_info.i4OffsetXVivo = 32;
			imgsensor_pd_info.i4OffsetYVivo = 0;
			memcpy(imgsensor_pd_info.i4PosLVivo, i4localPosLVivo, sizeof(imgsensor_pd_info.i4PosLVivo));
			memcpy(imgsensor_pd_info.i4PosRVivo, i4localPosRVivo, sizeof(imgsensor_pd_info.i4PosRVivo));
			memcpy((void *)PDAFinfo,(void *)&imgsensor_pd_info,sizeof(struct SET_PD_BLOCK_INFO_T));
			pr_debug("i4PosLVivo-i4localPosRVivo MSDK_SCENARIO_ID_VIDEO_PREVIEW\n");
			break;
		case MSDK_SCENARIO_ID_CUSTOM6:
			imgsensor_pd_info.i4BlockNumX = 43,
			imgsensor_pd_info.i4BlockNumY = 32,
			imgsensor_pd_info.i4OffsetXVivo = 32;
			imgsensor_pd_info.i4OffsetYVivo = 0;
			memcpy(imgsensor_pd_info.i4PosLVivo, i4localPosLVivo_custom6, sizeof(imgsensor_pd_info.i4PosLVivo));
			memcpy(imgsensor_pd_info.i4PosRVivo, i4localPosRVivo_custom6, sizeof(imgsensor_pd_info.i4PosRVivo));
			memcpy((void *)PDAFinfo,(void *)&imgsensor_pd_info,sizeof(struct SET_PD_BLOCK_INFO_T));
			pr_debug("pdaf_info MSDK_SCENARIO_ID_CUSTOM6\n");
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		

		default:
		break;
		}
		break;

		case SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY:
			pr_debug("SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY scenarioId:%lld\n", *feature_data);
			//PDAF capacity enable or not, 2p8 only full size support PDAF
			switch (*feature_data) {
				case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
				case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
				case MSDK_SCENARIO_ID_CUSTOM1:
				case MSDK_SCENARIO_ID_CUSTOM2:
				case MSDK_SCENARIO_ID_CUSTOM3:
				case MSDK_SCENARIO_ID_CUSTOM4:
				case MSDK_SCENARIO_ID_CUSTOM5:
					*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 1;
					break;
				case MSDK_SCENARIO_ID_CUSTOM6:
					*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 1;
					break;
				case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 1;// xxx is not supported
					break;
				case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
					*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
					break;
				case MSDK_SCENARIO_ID_SLIM_VIDEO:
					*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
					break;

				default:
					*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
					break;
			}
			break;
#if 0	/*hope modify*/	
	case SENSOR_FEATURE_GET_PDAF_DATA:	/*get cal data from eeprom*/
			pr_debug("SENSOR_FEATURE_GET_PDAF_DATA\n");
			pr_debug("SENSOR_FEATURE_GET_PDAF_DATA success\n");
			break;
	
	case SENSOR_FEATURE_SET_PDAF:
			pr_debug("PDAF mode :%d\n", imgsensor.pdaf_mode);
			break;
#endif			
	case SENSOR_FEATURE_SET_SHUTTER_FRAME_TIME:/*lzl*/
			set_shutter_frame_length((UINT32)*feature_data,
					(UINT16)*(feature_data+1));
			break;
	case SENSOR_FEATURE_GET_FRAME_CTRL_INFO_BY_SCENARIO:
		/*
		 * 1, if driver support new sw frame sync
		 * set_shutter_frame_length() support third para auto_extend_en
		 */
		switch (*feature_data) {
			case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
				*(feature_data + 1) = 1;
		    *(feature_data + 2) = 4;   ////imgsensor_info.margin
				break;
			case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
				*(feature_data + 1) = 1;
		    *(feature_data + 2) = 4;   ////imgsensor_info.margin
				break;
			case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
				*(feature_data + 1) = 1;
		    *(feature_data + 2) = 8;   ////imgsensor_info.margin
				break;
			case MSDK_SCENARIO_ID_SLIM_VIDEO:
				*(feature_data + 1) = 1;
		    *(feature_data + 2) = 8;   ////imgsensor_info.margin
				break;
			case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
				*(feature_data + 1) = 1;
		    *(feature_data + 2) = 4;   ////imgsensor_info.margin
				break;
			 case MSDK_SCENARIO_ID_CUSTOM2:
				 *(feature_data + 1) = 1;
				 *(feature_data + 2) = 8;	////imgsensor_info.margin
				 break;
	 
			default:
				*(feature_data + 1) = 1;
		    *(feature_data + 2) = 4;   ////imgsensor_info.margin
				break;
		}
		break;
	case SENSOR_FEATURE_GET_AE_FRAME_MODE_FOR_LE:
		pr_debug("SENSOR_FEATURE_GET_AE_FRAME_MODE_FOR_LE\n");
		memcpy(feature_return_para_32,
		&imgsensor.ae_frm_mode, sizeof(struct IMGSENSOR_AE_FRM_MODE));
		break;
	case SENSOR_FEATURE_GET_AE_EFFECTIVE_FRAME_FOR_LE:
		pr_debug("SENSOR_FEATURE_GET_AE_EFFECTIVE_FRAME_FOR_LE\n");
		*feature_return_para_32 =  imgsensor.current_ae_effective_frame;
		break;

	case SENSOR_FEATURE_GET_PIXEL_RATE:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.cap.pclk /
			(imgsensor_info.cap.linelength - 80))*
			imgsensor_info.cap.grabwindow_width;
			break;
			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.normal_video.pclk /
			(imgsensor_info.normal_video.linelength - 80))*
			imgsensor_info.normal_video.grabwindow_width;
			break;
			
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.hs_video.pclk /
			(imgsensor_info.hs_video.linelength - 80))*
			imgsensor_info.hs_video.grabwindow_width;
			break;
			
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.slim_video.pclk /
			(imgsensor_info.slim_video.linelength - 80))*
			imgsensor_info.slim_video.grabwindow_width;
			break;
		
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.custom1.pclk /
			(imgsensor_info.custom1.linelength - 80))*
			imgsensor_info.custom1.grabwindow_width;
			break;
		case MSDK_SCENARIO_ID_CUSTOM6:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.custom6.pclk /
			(imgsensor_info.custom6.linelength - 80))*
			imgsensor_info.custom6.grabwindow_width;
			break;
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.pre.pclk /
			(imgsensor_info.pre.linelength - 80))*
			imgsensor_info.pre.grabwindow_width;
			break;
		}
		break;
#if 1
	case SENSOR_FEATURE_GET_CUSTOM_INFO:
		pr_debug("SENSOR_FEATURE_GET_CUSTOM_INFO information type:%lld  S5K3L6WIDEPD2133_OTP_ERROR_CODE:%d \n", *feature_data,S5K3L6WIDEPD2133_OTP_ERROR_CODE);
			switch (*feature_data) {
				case 0:    //info type: otp state
				pr_debug("*feature_para_len = %d, sizeof(MUINT32)*13 + 2 =%ld, \n", *feature_para_len, sizeof(MUINT32)*13 + 2);
				if (*feature_para_len >= sizeof(MUINT32)*13 + 2) {
					*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = S5K3L6WIDEPD2133_OTP_ERROR_CODE;//otp_state
					memcpy( feature_data+2, sn_inf_main_s5k3l6widepd2133, sizeof(MUINT32)*13); 
					memcpy( feature_data+10, material_inf_main_s5k3l6widepd2133, sizeof(MUINT32)*4); 					
					#if 0
						for (i = 0 ; i<12 ; i++ ){
						printk("sn_inf_main_s5k3l6widepd2133[%d]= 0x%x\n", i, sn_inf_main_s5k3l6widepd2133[i]);
						}
								
					#endif
					}
				break;
				}
			break;
#endif
	/******************** STREAMING RESUME/SUSPEND *********/
	case SENSOR_FEATURE_SET_STREAMING_SUSPEND:
		pr_debug("SENSOR_FEATURE_SET_STREAMING_SUSPEND\n");
		streaming_control(KAL_FALSE);
		break;
	case SENSOR_FEATURE_GET_PERIOD_BY_SCENARIO:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.cap.framelength << 16)
				+ imgsensor_info.cap.linelength;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.normal_video.framelength << 16)
				+ imgsensor_info.normal_video.linelength;
			break;

		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.hs_video.framelength << 16)
				+ imgsensor_info.hs_video.linelength;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.slim_video.framelength << 16)
				+ imgsensor_info.slim_video.linelength;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.custom1.framelength << 16)
				+ imgsensor_info.custom1.linelength;
			break;
		case MSDK_SCENARIO_ID_CUSTOM6:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.custom6.framelength << 16)
				+ imgsensor_info.custom6.linelength;
			break;
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.pre.framelength << 16)
				+ imgsensor_info.pre.linelength;
			break;
		}
		break;
	case SENSOR_FEATURE_GET_OFFSET_TO_START_OF_EXPOSURE:
		*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 0;
		break;
	case SENSOR_FEATURE_GET_GAIN_RANGE_BY_SCENARIO:
		*(feature_data + 1) = imgsensor_info.min_gain;
		*(feature_data + 2) = imgsensor_info.max_gain;
		break;
	case SENSOR_FEATURE_GET_BASE_GAIN_ISO_AND_STEP:
		*(feature_data + 0) = imgsensor_info.min_gain_iso;
		*(feature_data + 1) = imgsensor_info.gain_step;
		*(feature_data + 2) = imgsensor_info.gain_type;
		break;
	case SENSOR_FEATURE_GET_MIN_SHUTTER_BY_SCENARIO:
		*(feature_data + 1) = imgsensor_info.min_shutter;
		break;
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.cap.pclk;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.normal_video.pclk;
			break;

		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.hs_video.pclk;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.slim_video.pclk;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.custom1.pclk;
			break;
		case MSDK_SCENARIO_ID_CUSTOM6:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.custom6.pclk;
			break;
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.pre.pclk;
			break;
		}
		break;
	case SENSOR_FEATURE_SET_STREAMING_RESUME:
		pr_debug("SENSOR_FEATURE_SET_STREAMING_RESUME, shutter:%llu\n",
			*feature_data);
		if (*feature_data != 0)
			set_shutter(*feature_data);
		streaming_control(KAL_TRUE);
		break;
	case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.cap.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.normal_video.mipi_pixel_rate;
			break;

		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.hs_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.slim_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.custom1.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM6:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.custom6.mipi_pixel_rate;
			break;
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.pre.mipi_pixel_rate;
			break;
		}
		break;

	default:
			break;
	}

	return ERROR_NONE;
}	/*	feature_control()  */

static struct SENSOR_FUNCTION_STRUCT sensor_func = {
	open,
	get_info,
	get_resolution,
	feature_control,
	control,
	close
};

UINT32 S5K3L6WIDEPD2133_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc != NULL)
		*pfFunc = &sensor_func;
	return ERROR_NONE;
}
