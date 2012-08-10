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
#define IRQ_ACTIVE_LO			(0 << 0)
#define IRQ_ACTIVE_HI			(1 << 0)
#define IRQ_AUTO_CLEAR			(1 << 1)
#define NOA3301_PS_LED_CURRENT          0x0f
#define NOA3301_PS_TH_UP_MSB            0x10
#define NOA3301_PS_TH_UP_LSB            0x11
#define NOA3301_PS_TH_LO_MSB            0x12
#define NOA3301_PS_TH_LO_LSB            0x13
#define NOA3301_PS_FILTER_CONFIG        0x14
#define PS_FILTER_N_MASK		0xf
#define PS_FILTER_N_SHIFT		4
#define PS_FILTER_M_MASK		0xf
#define PS_FILTER_M_SHIFT		0
#define NOA3301_PS_CONFIG               0x15
#define PS_HYST_ENABLE			(1 << 5)
#define PS_HYST_DISABLE			(1 << 4)
#define NOA3301_PS_INTERVAL             0x16
#define NOA3301_PS_CONTROL              0x17
#define PS_REPEAT			(1 << 1)
#define PS_ONESHOT			(1 << 0)
#define NOA3301_ALS_TH_UP_MSB           0x20
#define NOA3301_ALS_TH_UP_LSB           0x21
#define NOA3301_ALS_TH_LO_MSB           0x22
#define NOA3301_ALS_TH_LO_LSB           0x23
#define NOA3301_ALS_CONFIG              0x25
#define ALS_HYST_ENABLE			(1 << 5)
#define ALS_HYST_TRIGGER		(1 << 4)
#define NOA3301_ALS_INTERVAL            0x26
#define NOA3301_ALS_CONTROL             0x27
#define ALS_REPEAT			(1 << 1)
#define ALS_ONESHOT			(1 << 0)
#define NOA3301_INTERRUPT               0x40
#define IRQ_ASSERTED			(1 << 4)
#define IRQ_ALS_HI			(1 << 3)
#define IRQ_ALS_LO			(1 << 2)
#define IRQ_PS_HI			(1 << 1)
#define IRQ_PS_LO			(1 << 0)
#define NOA3301_PS_DATA_MSB             0x41
#define NOA3301_PS_DATA_LSB             0x42
#define NOA3301_ALS_DATA_MSB            0x43
#define NOA3301_ALS_DATA_LSB            0x44

static int noa3301_reset(struct i2c_client *client)
{
	return i2c_smbus_write_byte_data(client, NOA3301_RESET, 1);
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
		return ret;
	value = ret << 8;
	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_ALS_TH_UP_LSB);
	if (ret < 0)
		return ret;
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

	ret = i2c_smbus_write_byte_data(chip->client,
					NOA3301_ALS_TH_UP_MSB,
					(value >> 8) & 0xff);
	if (ret < 0)
		return ret;
	ret = i2c_smbus_write_byte_data(chip->client,
					NOA3301_ALS_TH_UP_LSB, value & 0xff);
	if (ret < 0)
		return ret;

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
		return ret;
	value = ret << 8;
	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_ALS_TH_LO_LSB);
	if (ret < 0)
		return ret;
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

	ret = i2c_smbus_write_byte_data(chip->client,
					NOA3301_ALS_TH_LO_MSB,
					(value >> 8) & 0xff);
	if (ret < 0)
		return ret;
	ret = i2c_smbus_write_byte_data(chip->client,
					NOA3301_ALS_TH_LO_LSB, value & 0xff);
	if (ret < 0)
		return ret;

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
		return ret;
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
	if (strict_strtoul(buf, 0, &value) || value > (0x3f * 50))
		return -EINVAL;

	ret = i2c_smbus_write_byte_data(chip->client,
					NOA3301_ALS_INTERVAL, value / 50);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t noa3301_als_read(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
	int ret;

	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_ALS_DATA_MSB);
	if (ret < 0)
		return ret;
	value = ret << 8;
	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_ALS_DATA_LSB);
	if (ret < 0)
		return ret;
	value |= ret;
	return sprintf(buf, "%ld\n", value);
}

static ssize_t noa3301_ps_thres_up_read(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
	int ret;

	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_PS_TH_UP_MSB);
	if (ret < 0)
		return ret;
	value = ret << 8;
	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_PS_TH_UP_LSB);
	if (ret < 0)
		return ret;
	value |= ret;
	return sprintf(buf, "%ld\n", value);
}

static ssize_t noa3301_ps_thres_up_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
	int ret;

	if (strict_strtoul(buf, 0, &value) || value > 0xffff)
		return -EINVAL;

	ret = i2c_smbus_write_byte_data(chip->client,
					NOA3301_PS_TH_UP_MSB,
					(value >> 8) & 0xff);
	if (ret < 0)
		return ret;
	ret = i2c_smbus_write_byte_data(chip->client,
					NOA3301_PS_TH_UP_LSB, value & 0xff);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t noa3301_ps_thres_lo_read(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
	int ret;

	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_PS_TH_LO_MSB);
	if (ret < 0)
		return ret;
	value = ret << 8;
	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_PS_TH_LO_LSB);
	if (ret < 0)
		return ret;
	value |= ret;
	return sprintf(buf, "%ld\n", value);
}

static ssize_t noa3301_ps_thres_lo_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
	int ret;

	if (strict_strtoul(buf, 0, &value))
		return -EINVAL;

	ret = i2c_smbus_write_byte_data(chip->client,
					NOA3301_PS_TH_LO_MSB,
					(value >> 8) & 0xff);
	if (ret < 0)
		return ret;
	ret = i2c_smbus_write_byte_data(chip->client,
					NOA3301_PS_TH_LO_LSB, value & 0xff);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t noa3301_ps_interval_read(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	int ret;

	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_PS_INTERVAL);
	if (ret < 0)
		return ret;
	/* interval is 8 bits in units of 5ms */
	return sprintf(buf, "%d\n", ret * 5);
}

static ssize_t noa3301_ps_interval_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
	int ret;

	/* interval is 8 bits in units of 5ms */
	if (strict_strtoul(buf, 0, &value) || value > (0xff * 5))
		return -EINVAL;

	ret = i2c_smbus_write_byte_data(chip->client,
					NOA3301_PS_INTERVAL, value / 5);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t noa3301_ps_led_current_read(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	int ret;

	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_PS_LED_CURRENT);
	if (ret < 0)
		return ret;
	/* led current is 5 bits in units of 5mA with a 5mA bias */
	return sprintf(buf, "%d\n", 5 + ret*5);
}

static ssize_t noa3301_ps_led_current_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
	int ret;

	/* led current is 5 bits in units of 5mA with a 5mA bias */
	if (strict_strtoul(buf, 0, &value) || value < 5 ||
	    value > (5 + 0x1f * 5))
		return -EINVAL;

	ret = i2c_smbus_write_byte_data(chip->client,
					NOA3301_PS_LED_CURRENT, value/5 - 1);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t noa3301_ps_filter_m_read(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	int ret, m;

	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_PS_FILTER_CONFIG);
	if (ret < 0)
		return ret;
	m = (ret >> PS_FILTER_M_SHIFT) & PS_FILTER_M_MASK;

	return sprintf(buf, "%d\n", m);
}

static ssize_t noa3301_ps_filter_m_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
	int ret, m;

	if (strict_strtoul(buf, 0, &value) || value < 1 || value > 0xf)
		return -EINVAL;

	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_PS_FILTER_CONFIG);
	if (ret < 0)
		return ret;
	m = ret & (~PS_FILTER_M_MASK << PS_FILTER_M_SHIFT);
	m |= (value & PS_FILTER_M_MASK) << PS_FILTER_M_SHIFT;
	ret = i2c_smbus_write_byte_data(chip->client,
					NOA3301_PS_FILTER_CONFIG, m);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t noa3301_ps_filter_n_read(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	int ret, n;

	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_PS_FILTER_CONFIG);
	if (ret < 0)
		return ret;
	n = (ret >> PS_FILTER_N_SHIFT) & PS_FILTER_N_MASK;

	return sprintf(buf, "%d\n", n);
}

static ssize_t noa3301_ps_filter_n_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
	int ret, n;

	if (strict_strtoul(buf, 0, &value) || value < 1 || value > 0xf)
		return -EINVAL;

	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_PS_FILTER_CONFIG);
	if (ret < 0)
		return ret;
	n = ret &(~PS_FILTER_N_MASK << PS_FILTER_N_SHIFT);
	n |= (value & PS_FILTER_N_MASK) << PS_FILTER_N_SHIFT;
	ret = i2c_smbus_write_byte_data(chip->client,
					NOA3301_PS_FILTER_CONFIG, n);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t noa3301_ps_read(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	struct noa3301_chip *chip = dev_get_drvdata(dev);
	unsigned long value;
	int ret;

	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_PS_DATA_MSB);
	if (ret < 0)
		return ret;
	value = ret << 8;
	ret = i2c_smbus_read_byte_data(chip->client, NOA3301_PS_DATA_LSB);
	if (ret < 0)
		return ret;
	value |= ret;
	return sprintf(buf, "%ld\n", value);
}

static struct device_attribute attributes[] = {
	__ATTR(als_threshold_up, S_IWUSR | S_IRUGO,
	       noa3301_als_thres_up_read, noa3301_als_thres_up_store),
	__ATTR(als_threshold_lo, S_IWUSR | S_IRUGO,
	       noa3301_als_thres_lo_read, noa3301_als_thres_lo_store),
	__ATTR(als_interval, S_IWUSR | S_IRUGO,
	       noa3301_als_interval_read, noa3301_als_interval_store),
	__ATTR(als_read, S_IRUGO, noa3301_als_read, NULL),
	__ATTR(ps_threshold_up, S_IWUSR | S_IRUGO,
	       noa3301_ps_thres_up_read, noa3301_ps_thres_up_store),
	__ATTR(ps_threshold_lo, S_IWUSR | S_IRUGO,
	       noa3301_ps_thres_lo_read, noa3301_ps_thres_lo_store),
	__ATTR(ps_interval, S_IWUSR | S_IRUGO,
	       noa3301_ps_interval_read, noa3301_ps_interval_store),
	__ATTR(ps_led_current, S_IWUSR | S_IRUGO,
	       noa3301_ps_led_current_read, noa3301_ps_led_current_store),
	__ATTR(ps_filter_numerator, S_IWUSR | S_IRUGO,
	       noa3301_ps_filter_m_read, noa3301_ps_filter_m_store),
	__ATTR(ps_filter_denomerator, S_IWUSR | S_IRUGO,
	       noa3301_ps_filter_n_read, noa3301_ps_filter_n_store),
	__ATTR(ps_read, S_IRUGO, noa3301_ps_read, NULL),
};

static int create_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		if (device_create_file(dev, attributes + i))
			goto error;
	return 0;
 error:
	for (; i >= 0; i--)
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

	ret = i2c_smbus_read_byte_data(client, NOA3301_PART_ID);
	if (ret < 0) {
		pr_warn("%s: failed to read part id\n", __func__);
		goto error;
	}
	part = (u8) ret;
	chip->revision = (part & NOACHIP_REV_MASK);
	if ((part & NOACHIP_PART_MASK) == NOACHIP_PART) {
		snprintf(chip->chipname, sizeof(chip->chipname), "NOA3301");
		return 0;
	}
	ret = -ENODEV;
 error:
	dev_dbg(&client->dev, "NOA3301 not found\n");

	return ret;
}

static irqreturn_t noa3301_hardirq(int irq, void *cookie)
{
	return IRQ_WAKE_THREAD;
}

static irqreturn_t noa3301_irq(int irq, void *cookie)
{
	struct noa3301_chip *chip = cookie;
	int irq_status;

	/* read irq status register */
	irq_status = i2c_smbus_read_byte_data(chip->client, NOA3301_INTERRUPT);
	pr_err("%s: 0x%x\n", __func__, irq_status);
	if (irq_status & IRQ_ALS_HI)
		pr_err("%s: ALS HI\n", __func__);
	if (irq_status & IRQ_ALS_LO)
		pr_err("%s: ALS LO\n", __func__);
	if (irq_status & IRQ_PS_HI)
		pr_err("%s: PS HI\n", __func__);
	if (irq_status & IRQ_PS_LO)
		pr_err("%s: PS LO\n", __func__);

	return IRQ_HANDLED;
}

static int __devinit noa3301_init_client(struct i2c_client *client)
{
	int ret;

	ret = noa3301_reset(client);
	if (ret)
		goto out;
	ret = i2c_smbus_write_byte_data(client, NOA3301_INT_CONFIG,
					IRQ_ACTIVE_LO);
	if (ret)
		goto out;
	ret = i2c_smbus_write_byte_data(client, NOA3301_ALS_CONTROL,
					ALS_REPEAT);
	if (ret)
		goto out;
	ret = i2c_smbus_write_byte_data(client, NOA3301_PS_CONTROL,
					PS_REPEAT);
	if (ret)
		goto out;

out:
	return ret;
}

static int __devinit noa3301_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct noa3301_chip *chip;
	struct noa3301_platform_data *pdata = client->dev.platform_data;
	int err;

	if (!pdata) {
		dev_err(&client->dev, "%s: platform data is not complete.\n",
			__func__);
		return -ENODEV;
	}

	chip = kzalloc(sizeof *chip, GFP_KERNEL);
	if (!chip) {
		dev_err(&client->dev, "%s: kzalloc failed\n", __func__);
		err = -ENOMEM;
		goto fail;
	}

	chip->pdata = client->dev.platform_data;
	chip->client = client;

	err = noa3301_detect_device(chip);
	if (err) {
		dev_err(&client->dev, "%s: device not responding"
			" error = %d\n", __func__, err);
		err = -ENODEV;
		goto fail;
	}
	err = noa3301_init_client(client);
	if (err) {
		dev_err(&client->dev, "%s: error initializing device"
			" error = %d\n", __func__, err);
		err = -ENODEV;
		goto fail;
	}
	i2c_set_clientdata(client, chip);

	err = request_threaded_irq(chip->pdata->irq,
				   noa3301_hardirq, noa3301_irq,
				   IRQF_TRIGGER_LOW | IRQF_ONESHOT,
				   "noa3301", chip);
	if (err < 0) {
		dev_err(&client->dev, "%s: error requesting irq"
			" error = %d\n", __func__, err);
		err = -ENODEV;
		goto fail;
	}

	err = create_sysfs_interfaces(&client->dev);
	if (err)
		goto fail_sysfs;

	dev_info(&client->dev, "found %s, revision %d\n",
		 chip->chipname, chip->revision);
	return 0;

fail_sysfs:
	free_irq(chip->pdata->irq, chip);
fail:
	kfree(chip);
	return err;
}

static int noa3301_remove(struct i2c_client *client)
{
	struct noa3301_chip *chip = i2c_get_clientdata(client);

	remove_sysfs_interfaces(&client->dev);
	free_irq(chip->pdata->irq, chip);
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
	{NOA3301_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, noa3301_id);

static struct i2c_driver noa3301_driver = {
	.driver = {
		   .name = NOA3301_NAME,
		   .owner = THIS_MODULE,
		   },
	.class = I2C_CLASS_HWMON,
	.probe = noa3301_probe,
	.detect = noa3301_detect,
	.remove = noa3301_remove,
	.id_table = noa3301_id,
};

static int __init noa3301_init(void)
{
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
