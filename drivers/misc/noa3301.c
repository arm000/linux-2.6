/* This file is part of the NOA3301 sensor driver.
 * Chip is combined proximity and ambient light sensor.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/ctype.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/delay.h>
#include "noa3301.h"
#include <linux/noa3301.h>

static const char reg_vcc[] = "Vcc";
static const char reg_vleds[] = "Vleds";


/* registers */
#define NOA3301_PART_ID                 0x00
#define NOA3301_RESET                   0x01
#define NOA3301_INT_CONFIG              0x02
#define NOA3301_PS_LED_CURRENT          0x0f
#define NOA3301_PS_TH_UP_MSB            0x10
#define NOA3301_PS_TH_UP_LSB            0x11
#define NOA3301_PS_TH_LO_MSB            0x12
#define NOA3301_PS_TH_LO_LSB            0x13
#define NOA3301_PS_FILTER_CONFIG        0x14
#define NOA3301_PS_CONFIG               0x15
#define NOA3301_PS_INTERVAL             0x16
#define NOA3301_PS_CONTROL              0x17
#define NOA3301_ALS_TH_UP_MSB           0x20
#define NOA3301_ALS_TH_UP_LSB           0x21
#define NOA3301_ALS_TH_LO_MSB           0x22
#define NOA3301_ALS_TH_LO_LSB           0x23
#define NOA3301_ALS_CONFIG              0x25
#define NOA3301_ALS_INTERVAL            0x26
#define NOA3301_ALS_CONTROL             0x27
#define NOA3301_INTERRUPT               0x40
#define NOA3301_PS_DATA_MSB             0x41
#define NOA3301_PS_DATA_LSB             0x42
#define NOA3301_ALS_DATA_MSB            0x43
#define NOA3301_ALS_DATA_LSB            0x44

static int noa3301_reset(struct noa3301_chip *chip,
			    int reset)
{
	return i2c_smbus_write_byte_data(chip->client,
                                         NOA3301_RESET,
                                         reset);
}

static ssize_t noa3301_als_thres_up_read(struct device *dev,
                                         struct device_attribute *attr,
                                         char *buf)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
        int ret;

        ret = i2c_smbus_read_byte_data(chip->client, NOA3301_ALS_TH_UP_MSB);
        if (ret < 0)
                return -EIO;
        value = ret << 8;
        ret = i2c_smbus_read_byte_data(chip->client, NOA3301_ALS_TH_UP_LSB);
        if (ret < 0)
                return -EIO;
        value |= ret;
	return sprintf(buf, "%ld\n", value);
}

static ssize_t noa3301_als_thres_up_store(struct device *dev,
					     struct device_attribute *attr,
					     const char *buf, size_t count)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
        int ret;

	if (strict_strtoul(buf, 0, &value) || value > 0xffff)
		return -EINVAL;
        pr_warn("%s: setting threshold_up to 0x%lx", __func__, value);

        ret = i2c_smbus_write_byte_data(chip->client,
                                        NOA3301_ALS_TH_UP_MSB,
                                        (value >> 8) & 0xff);
        if (ret < 0) {
                pr_warn("%s: error writing NOA3301_ALS_TH_UP_MSB", __func__);
                return -EIO;
        }
        ret = i2c_smbus_write_byte_data(chip->client,
                                        NOA3301_ALS_TH_UP_LSB,
                                        value & 0xff);
        if (ret < 0) {
                pr_warn("%s: error writing NOA3301_ALS_TH_UP_LSB", __func__);
                return -EIO;
        }

	return count;

}

static ssize_t noa3301_als_thres_lo_read(struct device *dev,
                                         struct device_attribute *attr,
                                         char *buf)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
        int ret;

        ret = i2c_smbus_read_byte_data(chip->client, NOA3301_ALS_TH_LO_MSB);
        if (ret < 0)
                return -EIO;
        value = ret << 8;
        ret = i2c_smbus_read_byte_data(chip->client, NOA3301_ALS_TH_LO_LSB);
        if (ret < 0)
                return -EIO;
        value |= ret;
	return sprintf(buf, "%ld\n", value);
}

static ssize_t noa3301_als_thres_lo_store(struct device *dev,
					     struct device_attribute *attr,
					     const char *buf, size_t count)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
        int ret;

	if (strict_strtoul(buf, 0, &value))
		return -EINVAL;
        pr_warn("%s: setting threshold_lo to 0x%lx", __func__, value);

        ret = i2c_smbus_write_byte_data(chip->client,
                                        NOA3301_ALS_TH_LO_MSB,
                                        (value >> 8) & 0xff);
        if (ret < 0) {
                pr_warn("%s: error writing NOA3301_ALS_TH_LO_MSB", __func__);
                return -EIO;
        }
        ret = i2c_smbus_write_byte_data(chip->client,
                                        NOA3301_ALS_TH_LO_LSB,
                                        value & 0xff);
        if (ret < 0) {
                pr_warn("%s: error writing NOA3301_ALS_TH_LO_LSB", __func__);
                return -EIO;
        }

	return count;

}

static ssize_t noa3301_als_interval_read(struct device *dev,
                                         struct device_attribute *attr,
                                         char *buf)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
        int ret;

        ret = i2c_smbus_read_byte_data(chip->client, NOA3301_ALS_INTERVAL);
        if (ret < 0)
                return -EIO;
        /* interval is 6 bits in units of 50ms */
	return sprintf(buf, "%d\n", ret * 50);
}

static ssize_t noa3301_als_interval_store(struct device *dev,
                                          struct device_attribute *attr,
                                          const char *buf, size_t count)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
        int ret;

        /* interval is 6 bits in units of 50ms */
	if (strict_strtoul(buf, 0, &value) || value > (0x1f * 50))
		return -EINVAL;

        ret = i2c_smbus_write_byte_data(chip->client,
                                        NOA3301_ALS_INTERVAL,
                                        value / 50);
        if (ret < 0)
                return -EIO;

	return count;
}

static struct device_attribute attributes[] = {
	__ATTR(als_threshold_up, S_IWUSR | S_IRUGO,
               noa3301_als_thres_up_read, noa3301_als_thres_up_store),
	__ATTR(als_threshold_lo, S_IWUSR | S_IRUGO,
               noa3301_als_thres_lo_read, noa3301_als_thres_lo_store),
	__ATTR(als_interval, S_IWUSR | S_IRUGO,
               noa3301_als_interval_read, noa3301_als_interval_store),
#if 0
	__ATTR(als_read, S_IRUGO, noa3301_als_read, NULL),
	__ATTR(ps_threshold_up, S_IWUSR, NULL, noa3301_ps_thres_up_store),
	__ATTR(ps_threshold_lo, S_IWUSR, NULL, noa3301_ps_thres_lo_store),
	__ATTR(ps_interval, S_IWUSR, NULL, noa3301_set_ps_interval),
	__ATTR(ps_read, S_IRUGO, noa3301_ps_read, NULL),
#endif
};

static int create_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		if (device_create_file(dev, attributes + i))
			goto error;
	return 0;
error:
	for (; i >= 0 ; i--)
		device_remove_file(dev, attributes + i);
	dev_err(dev, "%s: Unable to create interface\n", __func__);
	return -ENODEV;
}


static void remove_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		device_remove_file(dev, attributes + i);
}



static int noa3301_detect_device(struct noa3301_chip *chip)
{
	struct i2c_client *client = chip->client;
	s32 ret;
	u8 part;

	ret = i2c_smbus_read_byte_data(client, NOACHIP_PART_ID);
	if (ret < 0) {
                pr_warn("%s: failed to read part id", __func__);
		goto error;
        }
	part = (u8)ret;
        pr_warn("%s: part number is 0x%x", __func__, ret);
	chip->revision = (part & NOACHIP_REV_MASK);
	if ((part & NOACHIP_PART_MASK) == NOACHIP_PART) {
                pr_warn("%s: part recognized as 0x%x", __func__, NOACHIP_PART);
		snprintf(chip->chipname, sizeof(chip->chipname), "NOA3301");
		chip->active = 1;
		return 0;
	}
        pr_warn("%s: unknown part", __func__);
	ret = -ENODEV;
error:
	dev_dbg(&client->dev, "NOA3301 not found\n");

	return ret;
}


static int __devinit noa3301_probe(struct i2c_client *client,
				       const struct i2c_device_id *id)
{
	struct noa3301_chip *chip;
	struct noa3301_platform_data *pdata = client->dev.platform_data;
	int err;

        dev_warn(&client->dev, "%s", __func__);

	if (!pdata || !pdata->gpio_setup || !pdata->hw_config) {
		dev_err(&client->dev, "%s: platform data is not complete.\n",
			__func__);
		return -ENODEV;
	}

	err = pdata->gpio_setup(1);
	if (err) {
		dev_err(&client->dev, "%s: gpio_setup failed\n", __func__);
		goto err_gpio_setup_failed;
	}
	chip = kzalloc(sizeof *chip, GFP_KERNEL);
	if (!chip) {
                dev_err(&client->dev, "%s: kzalloc failed\n", __func__);
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}


	mutex_init(&chip->mutex);
	chip->pdata = client->dev.platform_data;
	chip->client  = client;

	pdata->hw_config(1, 1, 0, 0);
	err = noa3301_detect_device(chip);
	pdata->hw_config(1, 1, 0, 0);
	if (err) {
		dev_err(&client->dev, "%s: device not responding"
			" error = %d\n", __func__, err);
		err = -ENODEV;
		goto err_not_responding;
	}
        pr_warn("%s: part was identified", __func__);
	i2c_set_clientdata(client, chip);

	err = create_sysfs_interfaces(&client->dev);
	if (err)
		goto err_create_interfaces_failed;

        dev_warn(&client->dev, "%s success", __func__);
        return 0;

err_create_interfaces_failed:
err_not_responding:
	pdata->gpio_setup(0);
exit_alloc_data_failed:
	kfree(chip);
	return err;
err_gpio_setup_failed:
	dev_err(&client->dev, "%s: device create failed.\n", __func__);
	return err;
}

static int noa3301_remove(struct i2c_client *client)
{
	struct noa3301_chip *chip = i2c_get_clientdata(client);

	remove_sysfs_interfaces(&client->dev);
	chip->pdata->hw_config(0, 0, 0, 0);
	kfree(chip);
	i2c_set_clientdata(client, NULL);
	return 0;
}


static int __devinit noa3301_detect(struct i2c_client *client,
					struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
	dev_dbg(&client->dev, "%s\n", __func__);
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -ENODEV;
	return 0;
}


static const struct i2c_device_id noa3301_id[] = {
	{NOA3301_NAME, 0 },
	{}
};

MODULE_DEVICE_TABLE(i2c, noa3301_id);

static struct i2c_driver noa3301_driver = {
	.driver	 = {
		.name  = NOA3301_NAME,
		.owner	= THIS_MODULE,
	},
	.class  = I2C_CLASS_HWMON,
	.probe  = noa3301_probe,
	.detect = noa3301_detect,
	.remove	= noa3301_remove,
	.id_table = noa3301_id,
};

static int __init noa3301_init(void)
{
        pr_warn("%s", __func__);
	return i2c_add_driver(&noa3301_driver);
}

static void __exit noa3301_exit(void)
{
	i2c_del_driver(&noa3301_driver);
}

MODULE_DESCRIPTION("NOA3301 combined ALS and proximity sensor");
MODULE_AUTHOR("ME");
MODULE_LICENSE("GPL v2");

module_init(noa3301_init);
module_exit(noa3301_exit);
