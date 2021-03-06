/* sensor_mcp9808.c - Driver for MCP9808 temperature sensor */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>

#include <nanokernel.h>
#include <i2c.h>
#include <init.h>
#include <misc/byteorder.h>
#include <misc/__assert.h>

#include "sensor_mcp9808.h"


int mcp9808_reg_read(struct mcp9808_data *data, uint8_t reg, uint16_t *val)
{
	struct i2c_msg msgs[2] = {
		{
			.buf = &reg,
			.len = 1,
			.flags = I2C_MSG_WRITE | I2C_MSG_RESTART,
		},
		{
			.buf = (uint8_t *)val,
			.len = 2,
			.flags = I2C_MSG_READ | I2C_MSG_STOP,
		},
	};

	if (i2c_transfer(data->i2c_master, msgs, 2, data->i2c_slave_addr)
			 < 0) {
		return -EIO;
	}

	*val = sys_be16_to_cpu(*val);

	return 0;
}

static int mcp9808_sample_fetch(struct device *dev, enum sensor_channel chan)
{
	struct mcp9808_data *data = dev->driver_data;

	__ASSERT_NO_MSG(chan == SENSOR_CHAN_ALL || chan == SENSOR_CHAN_TEMP);

	return mcp9808_reg_read(data, MCP9808_REG_TEMP_AMB, &data->reg_val);
}

static int mcp9808_channel_get(struct device *dev,
			       enum sensor_channel chan,
			       struct sensor_value *val)
{
	struct mcp9808_data *data = dev->driver_data;

	__ASSERT_NO_MSG(chan == SENSOR_CHAN_TEMP);

	val->type = SENSOR_VALUE_TYPE_INT_PLUS_MICRO;

	val->val1 = (data->reg_val & MCP9808_TEMP_INT_MASK) >>
		     MCP9808_TEMP_INT_SHIFT;
	val->val2 = (data->reg_val & MCP9808_TEMP_FRAC_MASK) * 62500;

	if (data->reg_val & MCP9808_SIGN_BIT) {
		val->val1 -= 256;
	}

	return 0;
}

static struct sensor_driver_api mcp9808_api_funcs = {
	.sample_fetch = mcp9808_sample_fetch,
	.channel_get = mcp9808_channel_get,
	.attr_set = mcp9808_attr_set,
	.trigger_set = mcp9808_trigger_set,
};

int mcp9808_init(struct device *dev)
{
	struct mcp9808_data *data = dev->driver_data;

	data->i2c_master = device_get_binding(CONFIG_MCP9808_I2C_DEV_NAME);
	if (!data->i2c_master) {
		SYS_LOG_DBG("mcp9808: i2c master not found: %s",
		    CONFIG_MCP9808_I2C_DEV_NAME);
		return -EINVAL;
	}

	data->i2c_slave_addr = CONFIG_MCP9808_I2C_ADDR;

	mcp9808_setup_interrupt(dev);

	dev->driver_api = &mcp9808_api_funcs;

	return 0;
}

struct mcp9808_data mcp9808_data;

DEVICE_INIT(mcp9808, CONFIG_MCP9808_DEV_NAME, mcp9808_init, &mcp9808_data,
	    NULL, SECONDARY, CONFIG_SENSOR_INIT_PRIORITY);
