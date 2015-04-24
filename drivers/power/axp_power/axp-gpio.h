/*
 * gpio.h  --  GPIO Driver for Krosspower axp199 PMIC
 *
 * Copyright 2011 Krosspower Microelectronics PLC
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#ifndef _LINUX_AXP_GPIO_H_
#define _LINUX_AXP_GPIO_H_

/*
 * GPIO Registers.
 */
/*    AXP19   */
#define AXP19_GPIO0_CFG                   (POWER19_GPIO0_CTL)
#define AXP19_GPIO1_CFG                   (POWER19_GPIO1_CTL)
#define AXP19_GPIO2_CFG                   (POWER19_GPIO2_CTL)
#define AXP19_GPIO34_CFG                  (POWER19_SENSE_CTL)
#define AXP19_GPIO5_CFG                   (POWER19_RSTO_CTL)
#define AXP19_GPIO67_CFG0                 (POWER19_GPIO67_CFG)
#define AXP19_GPIO67_CFG1                 (POWER19_GPIO67_CTL)

#define AXP19_GPIO012_STATE               (POWER19_GPIO012_SIGNAL)
#define AXP19_GPIO34_STATE                (POWER19_SENSE_SIGNAL)
#define AXP19_GPIO5_STATE                 (POWER19_RSTO_CTL)
#define AXP19_GPIO67_STATE                (POWER19_GPIO67_CTL)


/*    AXP20   */
#define AXP20_GPIO0_CFG                   (POWER20_GPIO0_CTL)
#define AXP20_GPIO1_CFG                   (POWER20_GPIO1_CTL)
#define AXP20_GPIO2_CFG                   (POWER20_GPIO2_CTL)
#define AXP20_GPIO3_CFG                   (POWER20_GPIO3_CTL)

#define AXP20_GPIO012_STATE               (POWER20_GPIO012_SIGNAL)

/*    AXP22   
* GPIO0<-->GPIO0/LDO(PIN 37)
* GPIO1<-->GPIO1/LDO(PIN 31)
* GPIO2<-->DC1SW(PIN 22)
* GPIO3<-->CHGLED/MOTODRV(PIN 52)
* GPIO4<-->N_VBUSEN(PIN 11)
*/
#define AXP22_GPIO0_CFG                   (AXP22_GPIO0_CTL)//0x90
#define AXP22_GPIO1_CFG                   (AXP22_GPIO1_CTL)//0x92
#define AXP22_GPIO2_CFG                   (AXP22_LDO_DC_EN2)//0x12
#define AXP22_GPIO3_CFG                   (AXP22_OFF_CTL)//0x32
#define AXP22_GPIO4_CFG                   (AXP22_HOTOVER_CTL)//0x8f
#define AXP22_GPIO4_STA                   (AXP22_IPS_SET)//0x30

#define AXP22_GPIO01_STATE               (AXP22_GPIO01_SIGNAL)

extern int axp_gpio_set_io(int gpio, int io_state);
extern int axp_gpio_get_io(int gpio, int *io_state);
extern int axp_gpio_set_value(int gpio, int value);
extern int axp_gpio_get_value(int gpio, int *value);
#endif
