/*
 *
 * Aptina MT9V113 sensor driver
 *
 * Copyright (C) 2012 Aptina Imaging
 *
 * Contributor Prashanth Subramanya <sprashanth@aptina.com>
 *
 * Based on MT9P031 driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/videodev2.h>

#include <media/mt9v113.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>

#define MT9V113_PIXEL_ARRAY_WIDTH        640
#define MT9V113_PIXEL_ARRAY_HEIGHT        480

#define        MT9V113_ROW_START_MIN                0
#define        MT9V113_ROW_START_MAX                480
#define        MT9V113_ROW_START_DEF                0
#define        MT9V113_COLUMN_START_MIN        0
#define        MT9V113_COLUMN_START_MAX        640
#define        MT9V113_COLUMN_START_DEF        0
#define        MT9V113_WINDOW_HEIGHT_MIN        2
#define        MT9V113_WINDOW_HEIGHT_MAX        480
#define        MT9V113_WINDOW_HEIGHT_DEF        480
#define        MT9V113_WINDOW_WIDTH_MIN        2
#define        MT9V113_WINDOW_WIDTH_MAX        640
#define        MT9V113_WINDOW_WIDTH_DEF        640
#define MT9V113_ENABLE                        1
#define MT9V113_DISABLE                        0

#define MT9V113_CHIP_ID_REG                0x0000
#define MT9V113_CHIP_ID                        0x2280

#define MT9V113_PLL_CONTROL                0x0014
#define MT9V113_PLL_DIVIDERS                0x0010
#define MT9V113_PLL_P_DIVIDERS                0x0012

#define MT9V113_RESET_AND_MISC_CONTROL        0x001A
#define MT9V113_STANDBY_CONTROL                0x0018
#define MT9V113_CLOCKS_CONTROL                0x0016
#define MT9V113_PAD_SLEW                0x001E
#define MT9V113_MCU_ADDRESS                0x098C
#define MT9V113_MCU_DATA_0                0x0990
#define MT9V113_PIX_DEF_ID                0x31E0
#define MT9V113_LSC_START1                0x2703
#define MT9V113_LSC_START2                0x270D
#define MT9V113_LSC_START3                0x2747
#define MT9V113_LSC_START4                0xA408
#define MT9V113_LSC_START5                0x2411
#define MT9V113_COLOR_PIPELINE_CONTROL        0x3210
#define MT9V113_AWB_ADDR_1                0x2306
#define MT9V113_AWB_ADDR_2                0xA348
#define MT9V113_AWB_ADDR_3                0xA35D
#define MT9V113_AWB_POSITION_MIN        0xA351
#define MT9V113_AWB_POSITION_MAX        0xA352
#define MT9V113_AWB_SATURATION                0xA354
#define MT9V113_AWB_MODE                0xA355
#define MT9V113_AWB_CNT_PXL_TH                0x2361
#define MT9V113_AWB_TG_MIN0                0xA363
#define MT9V113_AWB_TG_MAX0                0xA364
#define MT9V113_AWB_WINDOW_POS                0xA302
#define MT9V113_AWB_WINDOW_SIZE                0xA303

#define MT9V113_MODE_DEC_CTRL_B                0x274F
#define MT9V113_MODE_DEC_CTRL_A                0x2741
#define MT9V113_LLMODE                        0xAB1F
#define MT9V113_NR_STOP_G                0xAB31
#define MT9V113_LL_SAT1                        0xAB20
#define MT9V113_LL_INTERPTHRESH1        0xAB21
#define MT9V113_LL_APCORR1                0xAB22
#define MT9V113_LL_SAT2                        0xAB24
#define MT9V113_LL_BRIGHTNESSSTART        0x2B28
#define MT9V113_LL_BRIGHTNESSSTOP        0x2B2A

#define MT9V113_SEQ_CMD                        0x2103
#define MT9V113_REFRESH_MODE                0x0006
#define MT9V113_REFRESH                        0x0005
#define MT9V113_CROP_X0_A                0x2739
#define MT9V113_CROP_X1_A                0x273B
#define MT9V113_CROP_Y0_A                0x273D
#define MT9V113_CROP_Y1_A                0x273F
#define MT9V113_OUTPUT_WIDTH_A                0x2703
#define MT9V113_OUTPUT_HEIGHT_A                0x2705
#define MT9V113_CROP_X0_B                0x2747
#define MT9V113_CROP_X1_B                0x2749
#define MT9V113_CROP_Y0_B                0x274B
#define MT9V113_CROP_Y1_B                0x274D
#define MT9V113_OUTPUT_WIDTH_B                0x2707
#define MT9V113_OUTPUT_HEIGHT_B                0x2709
#define MT9V113_READ_MODE_A                0x2717
#define MT9V113_READ_MODE_B                0x272D
#define MT9V113_SPEC_EFFECTS_A                0x2759
#define MT9V113_SPEC_EFFECTS_B                0x275B
#define MT9V113_PATTERN_SELECT                0x3290
#define MT9V113_8BIT_WALKING1                0x0020
#define MT9V113_10BIT_WALKING1                0x0060

#undef MT9V113_I2C_DEBUG

struct mt9v113_frame_size {
        u16 width;
        u16 height;
};

struct mt9v113_priv {
        struct v4l2_subdev subdev;
        struct media_pad pad;
        struct v4l2_rect crop;  /* Sensor window */
        struct v4l2_mbus_framefmt format;
        struct v4l2_ctrl_handler ctrls;
        struct mt9v113_platform_data *pdata;
        struct mutex power_lock; /* lock to protect power_count */
        struct mt9v113_pll_divs *pll;
        int power_count;
};

static unsigned int mt9v113_lsc1[4] = {
        0x0280, 0x01E0, 0x0280, 0x01E0
};

static unsigned int mt9v113_lsc2[26] = {
        0x0000, 0x0000, 0x01E7, 0x0287, 0x0001, 0x0026, 0x001A, 0x006B,
        0x006B, 0x0206, 0x0363, 0x0000, 0x0000, 0x01E7, 0x0287, 0x0001,
        0x0026, 0x001A, 0x006B, 0x006B, 0x0206, 0x0364, 0x0000, 0x027F,
        0x0000, 0x01DF
};

static unsigned int mt9v113_lsc3[4] = {
        0x0000, 0x027F,        0x0000, 0x01DF
};
        
static unsigned int mt9v113_lsc4[4] = {
        0x1F, 0x21, 0x26, 0x28
};

static unsigned int mt9v113_lsc5[4] = {
        0x0082, 0x009C, 0x0082, 0x009C
};

static unsigned int mt9v113_awb_1[22] = {
        0x0315, 0xFDDC, 0x003A, 0xFF58, 0x02B7, 0xFF31, 0xFF4C, 0xFE4C,
        0x039E, 0x001C, 0x0039, 0x007F, 0xFF77, 0x000A, 0x0020, 0x001B,
        0xFFC6, 0x0086, 0x00B5, 0xFEC3, 0x0001, 0xFFEF
};

static unsigned int mt9v113_awb_2[6] = {
        0x08, 0x02, 0x90, 0xFF, 0x75, 0xEF
};

static unsigned int mt9v113_awb_3[4] = {
        0x78, 0x86, 0x7E, 0x82
};

/************************************************************************
                        Helper Functions
************************************************************************/
/**
 * to_mt9v113 - A helper function which returns pointer to the
 * private data structure
 * @client: pointer to i2c client
 *
 */
static struct mt9v113_priv *to_mt9v113(const struct i2c_client *client)
{
        return container_of(i2c_get_clientdata(client),
                        struct mt9v113_priv, subdev);
}

/**
 * mt9v113_read - reads the data from the given register
 * @client: pointer to i2c client
 * @addr: address of the register which is to be read
 *
 */
static int mt9v113_read(struct i2c_client *client, u16 addr)
{
        struct i2c_msg msg[2];
        u8 buf[2];
        u16 __addr;
        u16 ret;

        /* 16 bit addressable register */
        __addr = cpu_to_be16(addr);

        msg[0].addr  = client->addr;
        msg[0].flags = 0;
        msg[0].len   = 2;
        msg[0].buf   = (u8 *)&__addr;

        msg[1].addr  = client->addr;
        msg[1].flags = I2C_M_RD; /* 1 */
        msg[1].len   = 2;
        msg[1].buf   = buf;

        /*
        * if return value of this function is < 0,
        * it means error.
        * else, under 16bit is valid data.
        */
        ret = i2c_transfer(client->adapter, msg, 2);

        if (ret < 0) {
                v4l_err(client, "Read from offset 0x%x error %d", addr, ret);
                return ret;
        }

        return (buf[0] << 8) | buf[1];
}

/**
 * mt9v113_write - writes the data into the given register
 * @client: pointer to i2c client
 * @addr: address of the register in which to write
 * @data: data to be written into the register
 *
 */
static int mt9v113_write(struct i2c_client *client, u16 addr,
                                u16 data)
{
        struct i2c_msg msg;
        u8 buf[4];
        u16 __addr, __data;
        int ret;

        /* 16-bit addressable register */

        __addr = cpu_to_be16(addr);
        __data = cpu_to_be16(data);

        buf[0] = __addr & 0xff;
        buf[1] = __addr >> 8;
        buf[2] = __data & 0xff;
        buf[3] = __data >> 8;
        msg.addr  = client->addr;
        msg.flags = 0;
        msg.len   = 4;
        msg.buf   = buf;

        /* i2c_transfer returns message length, but function should return 0 */
        ret = i2c_transfer(client->adapter, &msg, 1);
        if (ret >= 0){
#ifdef MT9V113_I2C_DEBUG
                printk(KERN_INFO"REG=0x%04X, 0x%04X\n", addr,data);
#endif
                return 0;
        }

        v4l_err(client, "Write failed at 0x%x error %d\n", addr, ret);
        return ret;
}

/**
 * mt9v113_write_8 - writes the data into the given register
 * @client: pointer to i2c client
 * @addr: address of the register in which to write
 * @data: data to be written into the register
 *
 */
static int mt9v113_write_8(struct i2c_client *client, u16 addr,
                                u8 data)
{
        struct i2c_msg msg;
        u8 buf[3];
        u16 __addr;
        int ret;

        /* 16-bit addressable register */

        __addr = cpu_to_be16(addr);

        buf[0] = __addr & 0xff;
        buf[1] = __addr >> 8;
        buf[2] = data;
        msg.addr  = client->addr;
        msg.flags = 0;
        msg.len   = 3;
        msg.buf   = buf;

        /* i2c_transfer returns message length, but function should return 0 */
        ret = i2c_transfer(client->adapter, &msg, 1);
        if (ret >= 0){
#ifdef MT9V113_I2C_DEBUG
                printk(KERN_INFO"REG8=0x%04X, 0x%04X\n", addr, data);
#endif
                return 0;
        }

        v4l_err(client, "Write failed at 0x%x error %d\n", addr, ret);
        return ret;
}

/**
 * mt9v113_reset - Soft resets the sensor
 * @client: pointer to the i2c client
 *
*/
static int mt9v113_reset(struct i2c_client *client)
{
        int ret;

        ret = mt9v113_write(client, MT9V113_RESET_AND_MISC_CONTROL, 0x0011);
        if(ret < 0)
                return ret;

        msleep(1);

        return mt9v113_write(client, MT9V113_RESET_AND_MISC_CONTROL, 0x0010);
}

/**
 * mt9v113_pll_setup - enable the sensor pll
 * @client: pointer to the i2c client
 *
 */
static int mt9v113_pll_setup(struct i2c_client *client)
{
        int ret, count;
        u16 data;
        
        ret = mt9v113_write(client, MT9V113_PLL_CONTROL, 0x2145);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_PLL_DIVIDERS, 0x0120);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_PLL_P_DIVIDERS, 0x0000);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_PLL_CONTROL, 0x244B);
        msleep(1);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_PLL_CONTROL, 0x304B);
        if (ret < 0)
                return ret;

        /* wait for PLL lock */
        data = mt9v113_read(client, MT9V113_PLL_CONTROL);
        count = 0;
        while(!(data & 0x8000)){
                if(count++ > 5){
                        printk(KERN_ERR"Failed to lock PLL\n");
                        break;
                }
                msleep(1);
                data = mt9v113_read(client, MT9V113_PLL_CONTROL);
        }
        return mt9v113_write(client, MT9V113_PLL_CONTROL, 0xB04A);
}

/**
 * mt9v113_power_on - power on the sensor
 * @mt9v113: pointer to private data structure
 *
 */
void mt9v113_power_on(struct mt9v113_priv *mt9v113)
{
        /* Ensure RESET_BAR is low */
        if (mt9v113->pdata->reset) {
                mt9v113->pdata->reset(&mt9v113->subdev, 1);
                msleep(1);
        }

        /* Enable clock */
        if (mt9v113->pdata->set_xclk) {
                mt9v113->pdata->set_xclk(&mt9v113->subdev,
                mt9v113->pdata->ext_freq);
                msleep(1);
        }

        /* Now RESET_BAR must be high */
        if (mt9v113->pdata->reset) {
                mt9v113->pdata->reset(&mt9v113->subdev, 0);
                msleep(1);
        }
}

/**
 * mt9v113_power_off - power off the sensor
 * @mt9v113: pointer to private data structure
 *
 */
void mt9v113_power_off(struct mt9v113_priv *mt9v113)
{
        if (mt9v113->pdata->set_xclk)
                mt9v113->pdata->set_xclk(&mt9v113->subdev, 0);
}

/************************************************************************
                        v4l2_subdev_core_ops
************************************************************************/
#define V4L2_CID_TEST_PATTERN           (V4L2_CID_USER_BASE | 0x1001)
#define V4L2_CID_EFFECTS                (V4L2_CID_USER_BASE | 0x1002)

static int mt9v113_update_read_mode(struct i2c_client *client, u16 data)
{
        int ret;

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_READ_MODE_A);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, data);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_READ_MODE_B);
        if (ret < 0)
                return ret;
        return mt9v113_write(client, MT9V113_MCU_DATA_0, data);
}

static int mt9v113_s_ctrl(struct v4l2_ctrl *ctrl)
{
        struct mt9v113_priv *mt9v113 = container_of(ctrl->handler,
                                        struct mt9v113_priv, ctrls);
        struct i2c_client *client = v4l2_get_subdevdata(&mt9v113->subdev);
        u16 data;
        int ret = 0;

        switch (ctrl->id) {
        case V4L2_CID_HFLIP:
                data = mt9v113_read(client, MT9V113_READ_MODE_A);
                if (ctrl->val){
                        data |= 0x0001;
                        ret = mt9v113_update_read_mode(client, data);
                        if (ret < 0)
                                return ret;

                        break;
                }
                data &= 0xfffe;
                ret = mt9v113_update_read_mode(client, data);
                if (ret < 0)
                        return ret;
                break;
        
        case V4L2_CID_VFLIP:
                data = mt9v113_read(client, MT9V113_READ_MODE_A);
                if (ctrl->val) {
                        data |= 0x0002;
                        ret = mt9v113_update_read_mode(client, data);
                        if (ret < 0)
                                return ret;
                        break;
                }
                data &= 0xfffc;
                ret = mt9v113_update_read_mode(client, data);
                if (ret < 0)
                        return ret;
                break;

        case V4L2_CID_TEST_PATTERN:
                if (!ctrl->val){
                        ret = mt9v113_write(client, MT9V113_PATTERN_SELECT, 0x0000);
                        if(ret < 0)
                                return ret;
                        break;
                }
                switch(ctrl->val)
                case 1:
                        ret = mt9v113_write(client, MT9V113_PATTERN_SELECT, MT9V113_8BIT_WALKING1);
                        if(ret < 0)
                                return ret;
                        break;
                default:
                        ret = mt9v113_write(client, MT9V113_PATTERN_SELECT, MT9V113_10BIT_WALKING1);
                        if(ret < 0)
                                return ret;
                        break;
                break;

        case V4L2_CID_EFFECTS:
                data = 0x6640 | ctrl->val;
                ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_SPEC_EFFECTS_A);
                if (ret < 0)
                        return ret;
                ret = mt9v113_write(client, MT9V113_MCU_DATA_0, data);
                if (ret < 0)
                        return ret;
                ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_SPEC_EFFECTS_B);
                if (ret < 0)
                        return ret;
                ret = mt9v113_write(client, MT9V113_MCU_DATA_0, data);
                if (ret < 0)
                        return ret;
                break;
        }

        /* Refresh */
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_SEQ_CMD);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, MT9V113_REFRESH_MODE);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_SEQ_CMD);
        if (ret < 0)
                return ret;
        return mt9v113_write(client, MT9V113_MCU_DATA_0, MT9V113_REFRESH);
}

static struct v4l2_ctrl_ops mt9v113_ctrl_ops = {
        .s_ctrl = mt9v113_s_ctrl,
};

/*
MT9V113_TEST_PATTERN
0 = Disabled. Normal operation. Generate output data from pixel array
1 = Marching 1s test pattern (8 bit)
2 = Marching 1s test pattern (10 bit)
*/
static const char * const mt9v113_test_pattern_menu[] = {
        "Disabled",
        "8-bit Marching 1s",
        "10-bit Marching 1s",
};

/*
MT9V113_EFFECTS
0 = Disabled
1 = Monochrome
2 = Sepia
3 = Negative
4 = Solarize with unmodified UV
5 = Solarize with -UV
*/
static const char * const mt9v113_effects_menu[] = {
        "Disabled",
        "Monochrome",
        "Sepia",
        "Negative",
        "Solarize1",
        "Solarize2",
};

static const struct v4l2_ctrl_config mt9v113_ctrls[] = {
        {
                .ops            = &mt9v113_ctrl_ops,
                .id             = V4L2_CID_TEST_PATTERN,
                .type           = V4L2_CTRL_TYPE_MENU,
                .name           = "Test Pattern",
                .min            = 0,
                .max            = ARRAY_SIZE(mt9v113_test_pattern_menu) - 1,
                .step           = 0,
                .def            = 0,
                .flags          = 0,
                .menu_skip_mask = 0,
                .qmenu          = mt9v113_test_pattern_menu,
        },
        {
                .ops            = &mt9v113_ctrl_ops,
                .id             = V4L2_CID_EFFECTS,
                .type           = V4L2_CTRL_TYPE_MENU,
                .name           = "Effects",
                .min            = 0,
                .max            = ARRAY_SIZE(mt9v113_effects_menu) - 1,
                .step           = 0,
                .def            = 0,
                .flags          = 0,
                .menu_skip_mask = 0,
                .qmenu          = mt9v113_effects_menu,
        }
};

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int mt9v113_g_reg(struct v4l2_subdev *sd,
                                struct v4l2_dbg_register *reg)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        int data;

        reg->size = 2;
        data = mt9v113_read(client, reg->reg);
        if (data < 0)
                return data;

        reg->val = data;
        return 0;
}

static int mt9v113_s_reg(struct v4l2_subdev *sd,
                                struct v4l2_dbg_register *reg)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);

        return mt9v113_write(client, reg->reg, reg->val);
}
#endif

static int mt9v113_s_power(struct v4l2_subdev *sd, int on)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        struct mt9v113_priv *mt9v113 = container_of(sd,
                                struct mt9v113_priv, subdev);
        int ret = 0;

        mutex_lock(&mt9v113->power_lock);

        /*
        * If the power count is modified from 0 to != 0 or from != 0 to 0,
        * update the power state.
        */
        if (mt9v113->power_count == !on) {
                if (on) {
                                mt9v113_power_on(mt9v113);
                                ret = mt9v113_reset(client);
                                if (ret < 0) {
                                        dev_err(mt9v113->subdev.v4l2_dev->dev,
                                        "Failed to reset the camera\n");
                                        goto out;
                                }
                                ret = v4l2_ctrl_handler_setup(&mt9v113->ctrls);
                                if (ret < 0)
                                        goto out;
                } else
                        mt9v113_power_off(mt9v113);
        }
        /* Update the power count. */
        mt9v113->power_count += on ? 1 : -1;
        WARN_ON(mt9v113->power_count < 0);
out:
        mutex_unlock(&mt9v113->power_lock);
        return ret;
}

/***************************************************
                v4l2_subdev_video_ops
****************************************************/
static unsigned int mt9v113_lsc_setup(struct i2c_client *client)
{
        int ret;
        int i;

        ret = mt9v113_write(client, MT9V113_RESET_AND_MISC_CONTROL, 0x0210);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_STANDBY_CONTROL, 0x402C);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_CLOCKS_CONTROL, 0x42DF);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_PAD_SLEW, 0x0777);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, 0x02F0);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 0x0000);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, 0x02F2);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 0x0210);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, 0x02F4);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 0x001A);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, 0x2145);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 0x02F4);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, 0xA134);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 0x0001);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_PIX_DEF_ID, 0x0001);
        if (ret < 0)
                return ret;

        for(i = 0; i < ARRAY_SIZE(mt9v113_lsc1); i++){
                ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_LSC_START1 + 2*i);
                if (ret < 0)
                        return ret;
                ret = mt9v113_write(client, MT9V113_MCU_DATA_0, mt9v113_lsc1[i]);
                if (ret < 0)
                        return ret;
        }

        for(i = 0; i < ARRAY_SIZE(mt9v113_lsc2); i++){
                ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_LSC_START2 + 2*i);
                if (ret < 0)
                        return ret;
                ret = mt9v113_write(client, MT9V113_MCU_DATA_0, mt9v113_lsc2[i]);
                if (ret < 0)
                        return ret;
        }

        for(i = 0; i < ARRAY_SIZE(mt9v113_lsc3); i++){
                ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_LSC_START3 + 2*i);
                if (ret < 0)
                        return ret;
                ret = mt9v113_write(client, MT9V113_MCU_DATA_0, mt9v113_lsc3[i]);
                if (ret < 0)
                        return ret;
        }

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, 0x222D);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 0x0082);
        if (ret < 0)
                return ret;

        for(i = 0; i < ARRAY_SIZE(mt9v113_lsc4); i++){
                ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_LSC_START4 + i);
                if (ret < 0)
                        return ret;
                ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, mt9v113_lsc4[i]);
                if (ret < 0)
                        return ret;
        }

        for(i = 0; i < ARRAY_SIZE(mt9v113_lsc5); i++){
                ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_LSC_START5 + 2*i);
                if (ret < 0)
                        return ret;
                ret = mt9v113_write(client, MT9V113_MCU_DATA_0, mt9v113_lsc5[i]);
                if (ret < 0)
                        return ret;
        }

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, 0xA404);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0x10);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, 0xA40D);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0x02);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, 0xA40E);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0x03);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, 0xA410);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0x0A);
        if (ret < 0)
                return ret;

        return mt9v113_write(client, MT9V113_COLOR_PIPELINE_CONTROL, 0x09B8);
}

static unsigned int mt9v113_awb_ccm(struct i2c_client *client)
{
        int i;
        int ret;
        
        for(i = 0; i < ARRAY_SIZE(mt9v113_awb_1); i++){
                ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_AWB_ADDR_1 + 2*i);
                if (ret < 0)
                        return ret;
                ret = mt9v113_write(client, MT9V113_MCU_DATA_0, mt9v113_awb_1[i]);
                if (ret < 0)
                        return ret;
        }
        
        for(i = 0; i < ARRAY_SIZE(mt9v113_awb_2); i++){
                ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_AWB_ADDR_2 + i);
                if (ret < 0)
                        return ret;
                ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, mt9v113_awb_2[i]);
                if (ret < 0)
                        return ret;
        }

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_AWB_POSITION_MIN);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0x00);
        if (ret < 0)
                return ret;

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_AWB_POSITION_MAX);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0x7F);
        if (ret < 0)
                return ret;

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_AWB_SATURATION);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0x43);
        if (ret < 0)
                return ret;

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_AWB_MODE);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0x01); 
        if (ret < 0)
                return ret;

        for(i = 0; i < ARRAY_SIZE(mt9v113_awb_3); i++){
                ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_AWB_ADDR_3 + i);
                if (ret < 0)
                        return ret;
                ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, mt9v113_awb_3[i]);
                if (ret < 0)
                        return ret;
        }

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_AWB_CNT_PXL_TH);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 0x0040);
        if (ret < 0)
                return ret;

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_AWB_TG_MIN0);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0xD2);
        if (ret < 0)
                return ret;

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_AWB_TG_MAX0);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0xF6);
        if (ret < 0)
                return ret;

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_AWB_WINDOW_POS);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0x00);
        if (ret < 0)
                return ret;

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_AWB_WINDOW_SIZE);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0xEF);
        if (ret < 0)
                return ret;

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_LL_SAT1);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0x24);
        if (ret < 0)
                return ret;
        
        /* AWB Setting for FW bootup */
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, 0xA353);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0x0020);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, 0xA34E);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0x009A);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, 0xA34F);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0x0080);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, 0xA350);
        if (ret < 0)
                return ret;
        return mt9v113_write_8(client, MT9V113_MCU_DATA_0, 0x0082);
}

static unsigned int mt9v113_cpipe_setup(struct i2c_client *client)
{
        int ret;

        /* CPIPE calibration */
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_MODE_DEC_CTRL_B);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0,  0x0004);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_MODE_DEC_CTRL_A);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0,  0x0004);
        if (ret < 0)
                return ret;

        /* CPIPE preferences */
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_LLMODE);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0,  0x00C7);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_NR_STOP_G);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0,  0x001E);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_LL_SAT1);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0,  0x0054);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_LL_INTERPTHRESH1);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0,  0x0046);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_LL_APCORR1);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0,  0x0002);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_LL_SAT2);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0,  0x0005);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_LL_BRIGHTNESSSTART);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0,  0x170C);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_LL_BRIGHTNESSSTOP);
        if (ret < 0)
                return ret;
        return mt9v113_write(client, MT9V113_MCU_DATA_0,  0x3E80);
}

static int mt9v113_set_resolution(struct i2c_client *client, struct mt9v113_frame_size *frame)
{
        int ret;

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_CROP_X0_A);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 0);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_CROP_X1_A);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 639);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_CROP_Y0_A);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 0);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_CROP_Y1_A);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 479);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_CROP_X0_B);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 0);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_CROP_X1_B);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 639);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_CROP_Y0_B);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 0);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_CROP_Y1_B);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, 479);
        if (ret < 0)
                return ret;

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_OUTPUT_WIDTH_A);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, frame->width);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_OUTPUT_HEIGHT_A);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, frame->height);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_OUTPUT_WIDTH_B);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, frame->width);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_OUTPUT_HEIGHT_B);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, frame->height);
        if (ret < 0)
                return ret;
        /* Refresh */
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_SEQ_CMD);
        if (ret < 0)
                return ret;
        ret = mt9v113_write_8(client, MT9V113_MCU_DATA_0, MT9V113_REFRESH_MODE);
        if (ret < 0)
                return ret;

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_SEQ_CMD);
        if (ret < 0)
                return ret;
        return mt9v113_write(client, MT9V113_MCU_DATA_0, MT9V113_REFRESH);
}

static int mt9v113_s_stream(struct v4l2_subdev *sd, int enable)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        struct mt9v113_priv *mt9v113 = container_of(sd,
                                        struct mt9v113_priv, subdev);
        struct mt9v113_frame_size frame;
        int ret;

        if (!enable)
                return 0;

        ret = mt9v113_pll_setup(client);
        if (ret < 0){
                printk(KERN_ERR"%s: Failed to setup PLL\n",__func__);
                return ret;
        }

        ret = mt9v113_lsc_setup(client);
        if (ret < 0){
                printk(KERN_ERR"%s: Failed to setup LSC\n",__func__);
                return ret;
        }

        /* Make sure MCU will be turned on after LSC */
        ret = mt9v113_write(client, MT9V113_STANDBY_CONTROL, 0x0028);
        if (ret < 0)
                return ret;
        msleep(20);

        ret = mt9v113_awb_ccm(client);
        if (ret < 0){
                printk(KERN_ERR"%s: Failed to setup AWB CCM\n",__func__);
                return ret;
        }

        ret = mt9v113_cpipe_setup(client);
        if (ret < 0){
                printk(KERN_ERR"%s: Failed to setup color pipe\n",__func__);
                return ret;
        }
        
        frame.width = mt9v113->format.width;
        frame.height = mt9v113->format.height;
        ret = mt9v113_set_resolution(client, &frame);
        if(ret < 0){
                printk(KERN_ERR"%s: Failed to setup resolution:%dx%d\n",
                        __func__, frame.width, frame.height);
                return ret;
        }
                
        /* Refresh */
        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_SEQ_CMD);
        if (ret < 0)
                return ret;
        ret = mt9v113_write(client, MT9V113_MCU_DATA_0, MT9V113_REFRESH_MODE);
        if (ret < 0)
                return ret;

        ret = mt9v113_write(client, MT9V113_MCU_ADDRESS, MT9V113_SEQ_CMD);
        if (ret < 0)
                return ret;
        return mt9v113_write(client, MT9V113_MCU_DATA_0, MT9V113_REFRESH);
}


/***************************************************
                v4l2_subdev_pad_ops
****************************************************/
static int mt9v113_enum_mbus_code(struct v4l2_subdev *sd,
                                struct v4l2_subdev_fh *fh,
                                struct v4l2_subdev_mbus_code_enum *code)
{
        struct mt9v113_priv *mt9v113 = container_of(sd,
                                        struct mt9v113_priv, subdev);

        if (code->pad || code->index)
                return -EINVAL;

        code->code = mt9v113->format.code;
        return 0;
}

static int mt9v113_enum_frame_size(struct v4l2_subdev *sd,
                                struct v4l2_subdev_fh *fh,
                                struct v4l2_subdev_frame_size_enum *fse)
{
        struct mt9v113_priv *mt9v113 = container_of(sd,
                                        struct mt9v113_priv, subdev);

        if (fse->index != 0 || fse->code != mt9v113->format.code)
                return -EINVAL;

        fse->min_width = MT9V113_WINDOW_WIDTH_MIN;
        fse->max_width = MT9V113_WINDOW_WIDTH_MAX;
        fse->min_height = MT9V113_WINDOW_HEIGHT_MIN;
        fse->max_height = MT9V113_WINDOW_HEIGHT_MAX;

        return 0;
}

static struct v4l2_mbus_framefmt *
__mt9v113_get_pad_format(struct mt9v113_priv *mt9v113, struct v4l2_subdev_fh *fh,
                        unsigned int pad, u32 which)
{
        switch (which) {
        case V4L2_SUBDEV_FORMAT_TRY:
                return v4l2_subdev_get_try_format(fh, pad);
        case V4L2_SUBDEV_FORMAT_ACTIVE:
                return &mt9v113->format;
        default:
                return NULL;
        }
}

static struct v4l2_rect *
__mt9v113_get_pad_crop(struct mt9v113_priv *mt9v113, struct v4l2_subdev_fh *fh,
        unsigned int pad, u32 which)
{
        switch (which) {
        case V4L2_SUBDEV_FORMAT_TRY:
                return v4l2_subdev_get_try_crop(fh, pad);
        case V4L2_SUBDEV_FORMAT_ACTIVE:
                return &mt9v113->crop;
        default:
                return NULL;
        }
}

static int mt9v113_get_format(struct v4l2_subdev *sd,
                                struct v4l2_subdev_fh *fh,
                                struct v4l2_subdev_format *fmt)
{
        struct mt9v113_priv *mt9v113 = container_of(sd,
                                        struct mt9v113_priv, subdev);

        fmt->format = *__mt9v113_get_pad_format(mt9v113, fh, fmt->pad,
                                                fmt->which);

        return 0;
}

static int mt9v113_set_format(struct v4l2_subdev *sd,
                                struct v4l2_subdev_fh *fh,
                                struct v4l2_subdev_format *format)
{
        struct mt9v113_priv *mt9v113 = container_of(sd,
                                        struct mt9v113_priv, subdev);
        struct mt9v113_frame_size size;
        struct v4l2_mbus_framefmt *__format;
        struct v4l2_rect *__crop;
        unsigned int wratio;
        unsigned int hratio;

        /* Clamp the width and height to avoid dividing by zero. */
        size.width = clamp_t(u16, ALIGN(format->format.width, 2),
                        MT9V113_WINDOW_WIDTH_MIN,
                        MT9V113_WINDOW_WIDTH_MAX);
        size.height = clamp_t(u16, ALIGN(format->format.height, 2),
                        MT9V113_WINDOW_HEIGHT_MIN,
                        MT9V113_WINDOW_HEIGHT_MAX);

        __crop = __mt9v113_get_pad_crop(mt9v113, fh, format->pad,
                format->which);

        wratio = DIV_ROUND_CLOSEST(__crop->width, size.width);
        hratio = DIV_ROUND_CLOSEST(__crop->height, size.height);

        __format = __mt9v113_get_pad_format(mt9v113, fh, format->pad,
                                                format->which);
        __format->width = __crop->width / wratio;
        __format->height = __crop->height / hratio;

        format->format = *__format;
        mt9v113->format.width        = format->format.width;
        mt9v113->format.height        = format->format.height;
        mt9v113->format.code        = V4L2_MBUS_FMT_UYVY8_1X16;

        return 0;
}

static int mt9v113_get_crop(struct v4l2_subdev *sd,
                        struct v4l2_subdev_fh *fh,
                        struct v4l2_subdev_crop *crop)
{
        struct mt9v113_priv *mt9v113 = container_of(sd,
                                        struct mt9v113_priv, subdev);

        crop->rect = *__mt9v113_get_pad_crop(mt9v113, fh, crop->pad, crop->which);

        return 0;
}

static int mt9v113_set_crop(struct v4l2_subdev *sd,
                        struct v4l2_subdev_fh *fh,
                        struct v4l2_subdev_crop *crop)
{
        struct mt9v113_priv *mt9v113 = container_of(sd,
                                        struct mt9v113_priv, subdev);
        struct v4l2_mbus_framefmt *__format;
        struct v4l2_rect *__crop;
        struct v4l2_rect rect;

        /* Clamp the crop rectangle boundaries and align them to a multiple of 2
        * pixels to ensure a GRBG Bayer pattern.
        */
        rect.left = clamp(ALIGN(crop->rect.left, 2), MT9V113_COLUMN_START_MIN,
                        MT9V113_COLUMN_START_MAX);
        rect.top = clamp(ALIGN(crop->rect.top, 2), MT9V113_ROW_START_MIN,
                        MT9V113_ROW_START_MAX);
        rect.width = clamp(ALIGN(crop->rect.width, 2),
                        MT9V113_WINDOW_WIDTH_MIN,
                        MT9V113_WINDOW_WIDTH_MAX);
        rect.height = clamp(ALIGN(crop->rect.height, 2),
                        MT9V113_WINDOW_HEIGHT_MIN,
                        MT9V113_WINDOW_HEIGHT_MAX);

        rect.width = min(rect.width, MT9V113_PIXEL_ARRAY_WIDTH - rect.left);
        rect.height = min(rect.height, MT9V113_PIXEL_ARRAY_HEIGHT - rect.top);

        __crop = __mt9v113_get_pad_crop(mt9v113, fh, crop->pad, crop->which);

        /* Reset the output image size if the crop rectangle size has
        * been modified.
        */
        if (rect.width != __crop->width || rect.height != __crop->height) {
                __format = __mt9v113_get_pad_format(mt9v113, fh, crop->pad,
                                                                crop->which);
                __format->width = rect.width;
                __format->height = rect.height;
        }

        *__crop = rect;
        crop->rect = rect;

        mt9v113->crop.left        = crop->rect.left;
        mt9v113->crop.top        = crop->rect.top;
        mt9v113->crop.width        = crop->rect.width;
        mt9v113->crop.height        = crop->rect.height;

        return 0;
}

/***********************************************************
        V4L2 subdev internal operations
************************************************************/
static int mt9v113_registered(struct v4l2_subdev *sd)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        struct mt9v113_priv *mt9v113 = to_mt9v113(client);
        s32 data;
        int count = 0;

        mt9v113_power_on(mt9v113);

        /* Read out the chip version register */
        data = mt9v113_read(client, MT9V113_CHIP_ID_REG);
        if (data != MT9V113_CHIP_ID) {
                while(count++ < 5){
                        data = mt9v113_read(client, MT9V113_CHIP_ID_REG);
                        msleep(5);
                }
                dev_err(&client->dev, "MT9V113 not detected, chip ID read:0x%4.4x\n",
                                data);
                return -ENODEV;
        }
        dev_info(&client->dev, "MT9V113 detected at address 0x%02x:chip ID = 0x%4.4x\n",
                        client->addr, MT9V113_CHIP_ID);

        mt9v113_power_off(mt9v113);

        return 0;
}

static int mt9v113_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
        return mt9v113_s_power(sd, 1);
}

static int mt9v113_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
        return mt9v113_s_power(sd, 0);
}

/***************************************************
                v4l2_subdev_ops
****************************************************/
static struct v4l2_subdev_core_ops mt9v113_subdev_core_ops = {
#ifdef CONFIG_VIDEO_ADV_DEBUG
        .g_register        = mt9v113_g_reg,
        .s_register        = mt9v113_s_reg,
#endif
        .s_power        = mt9v113_s_power,
};

static struct v4l2_subdev_video_ops mt9v113_subdev_video_ops = {
        .s_stream        = mt9v113_s_stream,
};

static struct v4l2_subdev_pad_ops mt9v113_subdev_pad_ops = {
        .enum_mbus_code         = mt9v113_enum_mbus_code,
        .enum_frame_size = mt9v113_enum_frame_size,
        .get_fmt         = mt9v113_get_format,
        .set_fmt         = mt9v113_set_format,
        .get_crop         = mt9v113_get_crop,
        .set_crop         = mt9v113_set_crop,
};

static struct v4l2_subdev_ops mt9v113_subdev_ops = {
        .core        = &mt9v113_subdev_core_ops,
        .video        = &mt9v113_subdev_video_ops,
        .pad        = &mt9v113_subdev_pad_ops,
};

/*
 * Internal ops. Never call this from drivers, only the v4l2 framework can call
 * these ops.
 */
static const struct v4l2_subdev_internal_ops mt9v113_subdev_internal_ops = {
        .registered        = mt9v113_registered,
        .open                = mt9v113_open,
        .close                = mt9v113_close,
};

/***************************************************
                I2C driver
****************************************************/
static int mt9v113_probe(struct i2c_client *client,
                        const struct i2c_device_id *did)
{
        struct mt9v113_platform_data *pdata = client->dev.platform_data;
        struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
        struct mt9v113_priv *mt9v113;
        int ret;
        int i;

        if (pdata == NULL) {
                dev_err(&client->dev, "No platform data\n");
                return -EINVAL;
        }

        if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_WORD_DATA)) {
                dev_warn(&client->dev, "I2C-Adapter doesn't support I2C_FUNC_SMBUS_WORD\n");
                return -EIO;
        }

        mt9v113 = devm_kzalloc(&client->dev, sizeof(struct mt9v113_priv),
                                GFP_KERNEL);
        if (mt9v113 == NULL)
                return -ENOMEM;

        mt9v113->pdata = pdata;

        v4l2_ctrl_handler_init(&mt9v113->ctrls,ARRAY_SIZE(mt9v113_ctrls) + 2);

        v4l2_ctrl_new_std(&mt9v113->ctrls, &mt9v113_ctrl_ops,
                                V4L2_CID_HFLIP, 0, 1, 1, 0);
        v4l2_ctrl_new_std(&mt9v113->ctrls, &mt9v113_ctrl_ops,
                                V4L2_CID_VFLIP, 0, 1, 1, 0);
        for (i = 0; i < ARRAY_SIZE(mt9v113_ctrls); i++){
                v4l2_ctrl_new_custom(&mt9v113->ctrls, &mt9v113_ctrls[i], NULL);
        }
        mt9v113->subdev.ctrl_handler = &mt9v113->ctrls;

        if (mt9v113->ctrls.error) {
                ret = mt9v113->ctrls.error;
                dev_err(&client->dev, "Control initialization error: %d\n",
                        ret);
                goto done;
        }

        mutex_init(&mt9v113->power_lock);
        v4l2_i2c_subdev_init(&mt9v113->subdev, client, &mt9v113_subdev_ops);
        mt9v113->subdev.internal_ops = &mt9v113_subdev_internal_ops;
        mt9v113->subdev.ctrl_handler = &mt9v113->ctrls;

        mt9v113->pad.flags = MEDIA_PAD_FL_SOURCE;
        ret = media_entity_init(&mt9v113->subdev.entity, 1, &mt9v113->pad, 0);
        if (ret < 0)
                goto done;

        mt9v113->subdev.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

        mt9v113->crop.width        = MT9V113_WINDOW_WIDTH_DEF;
        mt9v113->crop.height        = MT9V113_WINDOW_HEIGHT_DEF;
        mt9v113->crop.left        = MT9V113_COLUMN_START_DEF;
        mt9v113->crop.top        = MT9V113_ROW_START_DEF;

        mt9v113->format.code                = V4L2_MBUS_FMT_UYVY8_1X16;
        mt9v113->format.width                = MT9V113_WINDOW_WIDTH_DEF;
        mt9v113->format.height                = MT9V113_WINDOW_HEIGHT_DEF;
        mt9v113->format.field                = V4L2_FIELD_NONE;
        mt9v113->format.colorspace        = V4L2_COLORSPACE_SRGB;

done:
        if (ret < 0) {
                v4l2_ctrl_handler_free(&mt9v113->ctrls);
                media_entity_cleanup(&mt9v113->subdev.entity);
                dev_err(&client->dev, "Probe failed\n");
        }

        return ret;
}

static int mt9v113_remove(struct i2c_client *client)
{
        struct v4l2_subdev *subdev = i2c_get_clientdata(client);
        struct mt9v113_priv *mt9v113 = to_mt9v113(client);

        v4l2_ctrl_handler_free(&mt9v113->ctrls);
        v4l2_device_unregister_subdev(subdev);
        media_entity_cleanup(&subdev->entity);

        return 0;
}

static const struct i2c_device_id mt9v113_id[] = {
        { "mt9v113", 0 },
        { }
};
MODULE_DEVICE_TABLE(i2c, mt9v113_id);

static struct i2c_driver mt9v113_i2c_driver = {
        .driver = {
                 .name = "mt9v113",
        },
        .probe    = mt9v113_probe,
        .remove   = mt9v113_remove,
        .id_table = mt9v113_id,
};

/* module_i2c_driver(mt9v113_i2c_driver); */

static int __init mt9v113_module_init(void)
{
        return i2c_add_driver(&mt9v113_i2c_driver);
}

static void __exit mt9v113_module_exit(void)
{
        i2c_del_driver(&mt9v113_i2c_driver);
}
module_init(mt9v113_module_init);
module_exit(mt9v113_module_exit);

MODULE_DESCRIPTION("Aptina MT9V113 Camera driver");
MODULE_AUTHOR("Aptina Imaging <drivers@aptina.com>");
MODULE_LICENSE("GPL v2");
