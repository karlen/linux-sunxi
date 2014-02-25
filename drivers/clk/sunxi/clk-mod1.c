/*
 * Allwinner SoCs module 1 clocks support.
 *
 * Copyright (C) 2014 Maxime Ripard
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#define SUNXI_MOD1_PARENTS	4

static DEFINE_SPINLOCK(mod1_lock);

static void __init sun6i_mod1_clk_setup(struct device_node *node)
{
	const char *parents[SUNXI_MOD1_PARENTS];
	const char *clk_name = node->name;
	struct clk_gate *gate;
	struct clk_mux *mux;
	struct resource res;
	void __iomem *reg;
	struct clk *clk;
	int i, ret;

	printk("Prout\n");

	ret = of_address_to_resource(node, 0, &res);
	
	if (!request_mem_region(res.start, resource_size(&res), node->name))
		return;

	reg = ioremap(res.start, resource_size(&res));

	for (i = 0; i < SUNXI_MOD1_PARENTS; i++)
		parents[i] = of_clk_get_parent_name(node, i);

	of_property_read_string(node, "clock-output-names", &clk_name);

	gate = kzalloc(sizeof(struct clk_gate), GFP_KERNEL);
	if (!gate)
		return;

	/* set up gate properties */
	gate->reg = reg;
	gate->bit_idx = 31;
	gate->lock = &mod1_lock;

	mux = kzalloc(sizeof(struct clk_mux), GFP_KERNEL);
	if (!mux) {
		kfree(gate);
		return;
	}

	/* set up mux properties */
	mux->reg = reg;
	mux->shift = 16;
	mux->mask = 0x3;
	mux->lock = &mod1_lock;

	clk = clk_register_composite(NULL, clk_name,
				     parents, SUNXI_MOD1_PARENTS,
				     &mux->hw, &clk_mux_ops,
				     NULL, NULL,
				     &gate->hw, &clk_gate_ops,
				     0);
	if (IS_ERR(clk)) {
		kfree(mux);
		kfree(gate);
		return;
	}

	of_clk_add_provider(node, of_clk_src_simple_get, clk);
	clk_register_clkdev(clk, clk_name, NULL);
}
CLK_OF_DECLARE(sun6i_mod1, "allwinner,sun6i-a31-mod1-clk", sun6i_mod1_clk_setup);
