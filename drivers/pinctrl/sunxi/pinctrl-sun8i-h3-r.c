/*
 * Allwinner H3 SoCs special pins pinctrl driver.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/reset.h>

#include "pinctrl-sunxi.h"

static const struct sunxi_desc_pin sun8i_h3_r_pins[] = {
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 0),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "s_i2c"),		/* CLK */
		  SUNXI_FUNCTION_IRQ_BANK(0x4, 1, 0)),	/* S_PL_EINT0 */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 1),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "s_i2c"),		/* SDA */
		  SUNXI_FUNCTION_IRQ_BANK(0x4, 1, 1)),	/* S_PL_EINT1 */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 2),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "s_uart"),	/* TX */
		  SUNXI_FUNCTION_IRQ_BANK(0x4, 1, 2)),	/* S_PL_EINT2 */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 3),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "s_uart"),	/* RX */
		  SUNXI_FUNCTION_IRQ_BANK(0x4, 1, 3)),	/* S_PL_EINT3 */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 4),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "s_jtag"),	/* MS */
		  SUNXI_FUNCTION_IRQ_BANK(0x4, 1, 4)),	/* S_PL_EINT4 */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 5),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "s_jtag"),	/* CK */
		  SUNXI_FUNCTION_IRQ_BANK(0x4, 1, 5)),	/* S_PL_EINT5 */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 6),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "s_jtag"),	/* DO */
		  SUNXI_FUNCTION_IRQ_BANK(0x4, 1, 6)),	/* S_PL_EINT6 */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 7),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "s_jtag"),	/* DI */
		  SUNXI_FUNCTION_IRQ_BANK(0x4, 1, 7)),	/* S_PL_EINT7 */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 8),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION_IRQ_BANK(0x4, 1, 8)),	/* S_PL_EINT8 */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 9),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION_IRQ_BANK(0x4, 1, 9)),	/* S_PL_EINT9 */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 10),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "s_pwm"),		/* PWM */
		  SUNXI_FUNCTION_IRQ_BANK(0x4, 1, 10)),	/* S_PL_EINT10 */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 11),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "s_ir"),		/* RX */
		  SUNXI_FUNCTION_IRQ_BANK(0x4, 1, 11)),	/* S_PL0_EINT12 */
};

static const struct sunxi_pinctrl_desc sun8i_h3_r_pinctrl_data = {
	.pins = sun8i_h3_r_pins,
	.npins = ARRAY_SIZE(sun8i_h3_r_pins),
	.pin_base = PL_BASE,
	.irq_banks = 2,
};

static int sun8i_h3_r_pinctrl_probe(struct platform_device *pdev)
{
	struct reset_control *rstc;
	int ret;

	rstc = devm_reset_control_get(&pdev->dev, NULL);
	if (IS_ERR(rstc)) {
		dev_err(&pdev->dev, "Reset controller missing\n");
		return PTR_ERR(rstc);
	}

	ret = reset_control_deassert(rstc);
	if (ret)
		return ret;

	ret = sunxi_pinctrl_init(pdev,
				 &sun8i_h3_r_pinctrl_data);

	if (ret)
		reset_control_assert(rstc);

	return ret;
}

static const struct of_device_id sun8i_h3_r_pinctrl_match[] = {
	{ .compatible = "allwinner,sun8i-h3-r-pinctrl", },
	{}
};
MODULE_DEVICE_TABLE(of, sun8i_h3_r_pinctrl_match);

static struct platform_driver sun8i_h3_r_pinctrl_driver = {
	.probe	= sun8i_h3_r_pinctrl_probe,
	.driver	= {
		.name		= "sun8i-h3-r-pinctrl",
		.of_match_table	= sun8i_h3_r_pinctrl_match,
	},
};
module_platform_driver(sun8i_h3_r_pinctrl_driver);

MODULE_AUTHOR("Marcus Cooper <codekipper@gmail.com");
MODULE_DESCRIPTION("Allwinner H3 R_PIO pinctrl driver");
MODULE_LICENSE("GPL");
