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

static const char *i8042_kbd_phys_desc;
static const char *i8042_aux_phys_desc;
static const char *i8042_mux_phys_desc;
#define I8042_KBD_PHYS_DESC i8042_kbd_phys_desc
#define I8042_AUX_PHYS_DESC i8042_aux_phys_desc
#define I8042_MUX_PHYS_DESC i8042_mux_phys_desc

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
	int status;

	i8042_base = of_iomap(np, 0);
	if (!i8042_base)
		return -ENOMEM;

	status = of_property_read_u32(np, "command-reg", &i8042_command_reg);
	if (status)
		return status;

	status = of_property_read_u32(np, "status-reg", &i8042_status_reg);
	if (status)
		return status;

	status = of_property_read_u32(np, "data-reg", &i8042_data_reg);
	if (status)
		return status;

	i8042_kbd_irq = irq_of_parse_and_map(np, 0);
	i8042_aux_irq = irq_of_parse_and_map(np, 1);

	status = of_property_read_string(np, "linux,kbd_phys_desc",
						&i8042_kbd_phys_desc);
	if (status)
		i8042_kbd_phys_desc = "i8042/serio0";

	status = of_property_read_string(np, "linux,aux_phys_desc",
						&i8042_aux_phys_desc);
	if (status)
		i8042_aux_phys_desc = "i8042/serio1";

	status = of_property_read_string(np, "linux,mux_phys_desc",
						&i8042_mux_phys_desc);
	if (status)
		i8042_mux_phys_desc = "i8042/serio%d";

	if (of_get_property(np, "init-reset", NULL))
		i8042_reset = true;

	return 0;
}

static inline void i8042_platform_exit(void)
{
	if (i8042_base)
		iounmap(i8042_base);
}

#endif
