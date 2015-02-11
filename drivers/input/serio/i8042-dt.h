#ifndef _I8042_DT_H
#define _I8042_DT_H

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

static void __iomem *i8042_base;
static unsigned int i8042_command_reg;
static unsigned int i8042_status_reg;
static unsigned int i8042_data_reg;
#define I8042_COMMAND_REG i8042_command_reg
#define I8042_STATUS_REG i8042_status_reg
#define I8042_DATA_REG i8042_data_reg

/*
 * Names.
 */

#define I8042_KBD_PHYS_DESC "i8042/serio0"
#define I8042_AUX_PHYS_DESC "i8042/serio1"
#define I8042_MUX_PHYS_DESC "i8042/serio%d"

/*
 * IRQs.
 */
static int i8042_kbd_irq;
static int i8042_aux_irq;
#define I8042_KBD_IRQ i8042_kbd_irq
#define I8042_AUX_IRQ i8042_aux_irq

static inline int i8042_read_data(void)
{
	return readb(i8042_base + i8042_data_reg);
}

static inline int i8042_read_status(void)
{
	return readb(i8042_base + i8042_status_reg);
}

static inline void i8042_write_data(int val)
{
	writeb(val, i8042_base + i8042_data_reg);
}

static inline void i8042_write_command(int val)
{
	writeb(val, i8042_base + i8042_command_reg);
}

static inline int i8042_platform_init(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	const __be32 *regbase_p;
	u64 regsize;
	int status;

	regbase_p = of_get_address(np, 0, &regsize, NULL);
	if (!regbase_p)
		return -EINVAL;

	status = of_property_read_u32(np, "command-reg", &i8042_command_reg);
	if (status)
		return status;

	status = of_property_read_u32(np, "status-reg", &i8042_status_reg);
	if (status)
		return status;

	status = of_property_read_u32(np, "data-reg", &i8042_data_reg);
	if (status)
		return status;

	if ((i8042_command_reg >= regsize) || (i8042_status_reg >= regsize) ||
			(i8042_data_reg >= regsize))
		return -EINVAL;

	i8042_kbd_irq = platform_get_irq_byname(pdev, "kbd");
	i8042_aux_irq = platform_get_irq_byname(pdev, "aux");

	i8042_base = ioremap((unsigned long)of_translate_address(np, regbase_p),
			(unsigned long)regsize);
	if (!i8042_base)
		return -ENOMEM;

	i8042_reset = true;

	return 0;
}

static inline void i8042_platform_exit(void)
{
	if (i8042_base)
		iounmap(i8042_base);
}

#endif
