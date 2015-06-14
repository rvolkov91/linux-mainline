#ifndef _I8042_OF_H
#define _I8042_OF_H

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
static void __iomem *i8042_data;
static void __iomem *i8042_ctrl;
#define I8042_DATA_REG			i8042_data
#define I8042_COMMAND_REG		i8042_ctrl
#define I8042_STATUS_REG		i8042_ctrl

/*
 * Names.
 */
#define I8042_KBD_PHYS_DESC		"i8042/serio0"
#define I8042_AUX_PHYS_DESC		"i8042/serio1"
#define I8042_MUX_PHYS_DESC		"i8042/serio%d"

#define OBP_PS2KBD_NAME1		"kb_ps2"
#define OBP_PS2KBD_NAME2		"keyboard"
#define OBP_PS2MS_NAME1			"kdmouse"
#define OBP_PS2MS_NAME2			"mouse"

/*
 * IRQs.
 */
static int i8042_kbd_irq;
static int i8042_aux_irq;
#define I8042_KBD_IRQ			i8042_kbd_irq
#define I8042_AUX_IRQ			i8042_aux_irq

static inline int i8042_read_data(void)
{
	return readb(I8042_DATA_REG);
}

static inline void i8042_write_data(int val)
{
	writeb(val, I8042_DATA_REG);
}

static inline int i8042_read_status(void)
{
	return readb(I8042_STATUS_REG);
}

static inline void i8042_write_command(int val)
{
	writeb(val, I8042_COMMAND_REG);
}

/*
 * Device Tree/platform code
 */
static int __init i8042_platform_probe_subdevice(struct platform_device *pdev,
	struct device_node *psub)
{
	if (!pdev || !psub)
		return -EINVAL;

	if (of_device_is_compatible(psub, "pnpPNP,303") ||
	    !strcmp(psub->name, OBP_PS2KBD_NAME1) ||
	    !strcmp(psub->name, OBP_PS2KBD_NAME2)) {
		i8042_kbd_irq = irq_of_parse_and_map(psub, 0);
		if (i8042_kbd_irq <= 0)
			i8042_kbd_irq = platform_get_irq_byname(pdev, "kbd");
		if (i8042_kbd_irq <= 0)
			i8042_kbd_irq = platform_get_irq(pdev, 0);
		return 0;
	}

	if (of_device_is_compatible(psub, "pnpPNP,f03") ||
	    !strcmp(psub->name, OBP_PS2MS_NAME1) ||
	    !strcmp(psub->name, OBP_PS2MS_NAME2)) {
		i8042_aux_irq = irq_of_parse_and_map(psub, 0);
		if (i8042_aux_irq <= 0)
			i8042_aux_irq = platform_get_irq_byname(pdev, "aux");
		if (i8042_aux_irq <= 0)
			i8042_aux_irq = platform_get_irq(pdev, 1);
		return 0;
	}

	return -ENODEV;
}

static int __init i8042_platform_probe(struct platform_device *pdev)
{
	struct device_node *node;
	struct resource *res;

	if (i8042_data || i8042_ctrl)
		return -EBUSY;

	node = pdev->dev.of_node;
	if (!node)
		return -EINVAL;

	node = node->child;
	while (node) {
		i8042_platform_probe_subdevice(pdev, node);
		node = node->sibling;
	}

	if (i8042_kbd_irq <= 0)
		i8042_nokbd = true;
	if (i8042_aux_irq <= 0)
		i8042_noaux = true;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	i8042_data = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(i8042_data))
		return PTR_ERR(i8042_data);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	i8042_ctrl = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(i8042_ctrl)) {
		devm_iounmap(&pdev->dev, i8042_data);
		return PTR_ERR(i8042_ctrl);
	}

	return 0;
}

static int i8042_platform_remove(struct platform_device *pdev)
{
	if (i8042_data) {
		if (!IS_ERR(i8042_data))
			devm_iounmap(&pdev->dev, i8042_data);
		i8042_data = NULL;
	}

	if (i8042_ctrl) {
		if (!IS_ERR(i8042_ctrl))
			devm_iounmap(&pdev->dev, i8042_ctrl);
		i8042_ctrl = NULL;
	}

	return 0;
}

static int __init i8042_platform_init(void)
{
	i8042_data = NULL;
	i8042_ctrl = NULL;
	i8042_reset = true;

	return 0;
}

static inline void i8042_platform_exit(void)
{
}

static const struct of_device_id i8042_of_match[] = {
	{ .compatible = "intel,8042", },
	{ },
};
MODULE_DEVICE_TABLE(of, i8042_of_match);

#endif
