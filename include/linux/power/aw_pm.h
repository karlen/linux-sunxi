/*
 * include/linux/power/aw_pm.h
 *
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : pm.h
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-27 14:08
* Descript: power manager
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __AW_PM_H__
#define __AW_PM_H__


/**max device number of pmu*/
#define PMU_MAX_DEVS        2
/**start address for function run in sram*/
#define SRAM_FUNC_START     SW_VA_SRAM_BASE

/**
*@name PMU command
*@{
*/
#define AW_PMU_SET          0x10
#define AW_PMU_VALID        0x20
/**
*@}
*/

/*
* define event source for wakeup system when suspended
*/
#define SUSPEND_WAKEUP_SRC_EXINT    (1<<0)  /* external interrupt, pmu event for ex.    */
#define SUSPEND_WAKEUP_SRC_USB      (1<<1)  /* usb connection event */
#define SUSPEND_WAKEUP_SRC_KEY      (1<<2)  /* key event    */
#define SUSPEND_WAKEUP_SRC_IR       (1<<3)  /* ir event */
#define SUSPEND_WAKEUP_SRC_ALARM    (1<<4)  /* alarm event  */
#define SUSPEND_WAKEUP_SRC_TIMEOFF  (1<<5)  /* set time to power off event  */
#define SUSPEND_WAKEUP_SRC_PIO      (1<<6)  /* gpio event  */

#ifdef CONFIG_ARCH_SUN6I
/*
* the wakeup source of main cpu: cpu0
*/
#define CPU0_WAKEUP_MSGBOX		(1<<0)  /* external interrupt, pmu event for ex.    */
#define CPU0_WAKEUP_KEY			(1<<1)  /* key event    */

//the wakeup source of assistant cpu: cpus
#define CPUS_WAKEUP_LOWBATT    	(1<<2 )
#define CPUS_WAKEUP_USB        	(1<<3 )
#define CPUS_WAKEUP_AC         	(1<<4 )
#define CPUS_WAKEUP_ASCEND     	(1<<5 )
#define CPUS_WAKEUP_DESCEND    	(1<<6 )
#define CPUS_WAKEUP_SHORT_KEY  	(1<<7 )
#define CPUS_WAKEUP_LONG_KEY   	(1<<8 )
#define CPUS_WAKEUP_IR     		(1<<9 )
#define CPUS_WAKEUP_ALM0  		(1<<10)
#define CPUS_WAKEUP_ALM1  		(1<<11)
#define CPUS_WAKEUP_TIMEOUT		(1<<12)
#define CPUS_WAKEUP_GPIO		(1<<13)
#define CPUS_WAKEUP_USBMOUSE    (1<<14)
#define CPUS_WAKEUP_LRADC       (1<<15)
#define CPUS_WAKEUP_CODEC		(1<<16)
#define CPUS_WAKEUP_KEY        	(CPUS_WAKEUP_SHORT_KEY | CPUS_WAKEUP_LONG_KEY)

#define WAKEUP_GPIO_PL(num)     (1 << (num))
#define WAKEUP_GPIO_PM(num)     (1 << (num + 12))
#define WAKEUP_GPIO_AXP(num)    (1 << (num + 24))
#define WAKEUP_GPIO_GROUP(group)  (1 << (group - 'A'))
#define PLL_NUM (11)
#define BUS_NUM (6)

typedef struct pll_para{
	int n;
	int k;
	int m;
	int p;
}pll_para_t;

typedef struct bus_para{
	int src;
	int pre_div;
	int div_ratio;
	int n;
	int m;
}bus_para_t;

typedef struct extended_standby{
	/*
	 * id of extended standby
	 */
	unsigned long id;
	/*
	 * clk tree para description as follow:
	 * avcc : vcc_wifi : vcc_dram: vdd_sys : vdd_cpux : vdd_gpu : vcc_io : vdd_cpus
	 */
	int pwr_dm_en;	//bitx = 1, mean power on when sys is in standby state. otherwise, vice verse.

	/*
	 * Hosc: losc: ldo: ldo1
	 */
	int osc_en;

	/*
	 * pll_10: pll_9: pll_mipi: pll_8: pll_7: pll_6: pll_5: pll_4: pll_3: pll_2: pll_1
	 */
	int init_pll_dis;

	/*
	 * pll_10: pll_9: pll_mipi: pll_8: pll_7: pll_6: pll_5: pll_4: pll_3: pll_2: pll_1
	 */
	int exit_pll_en;

	/*
	 * set corresponding bit if it's pll factors need to be set some value.
	 * pll_10: pll_9: pll_mipi: pll_8: pll_7: pll_6: pll_5: pll_4: pll_3: pll_2: pll_1
	 */
	int pll_change;

	/*
	 * fill in the enabled pll freq factor sequently. unit is khz pll6: 0x90041811
	 * factor n/m/k/p already do the pretreatment of the minus one
	 */
	pll_para_t pll_factor[PLL_NUM];

	/*
	 * bus_en: cpu:axi:atb/apb:ahb1:apb1:apb2,
	 * normally, only clk src need be cared.
	 * so, at a31, only cpu:ahb1:apb2 need be cared.
	 * pll1->cpu -> axi
	 *	     -> atb/apb
	 * ahb1 -> apb1
	 * apb2
	 */
	int bus_change;

	/*
	 * bus_src: ahb1, apb2 src;
	 * option: losc, hosc, pllx
	 */
	bus_para_t bus_factor[BUS_NUM];
}extended_standby_t;


typedef	struct super_standby_para
{
	unsigned long event;			//cpus wakeup event types
	unsigned long resume_code_src; 		//cpux resume code src
	unsigned long resume_code_length; 	//cpux resume code length
	unsigned long resume_entry; 		//cpux resume entry
	unsigned long timeout;			//wakeup after timeout seconds
	unsigned long gpio_enable_bitmap;
	unsigned long cpux_gpiog_bitmap;
	extended_standby_t *pextended_standby;
} super_standby_para_t;

typedef	struct normal_standby_para
{
	unsigned long event;		//cpus wakeup event types
	unsigned long timeout;		//wakeup after timeout seconds
	unsigned long gpio_enable_bitmap;
	unsigned long cpux_gpiog_bitmap;
	extended_standby_t *pextended_standby;
} normal_standby_para_t;


//define cpus wakeup src
#define CPUS_MEM_WAKEUP              (CPUS_WAKEUP_LOWBATT | CPUS_WAKEUP_USB | CPUS_WAKEUP_AC | \
						CPUS_WAKEUP_DESCEND | CPUS_WAKEUP_ASCEND | CPUS_WAKEUP_ALM0 | CPUS_WAKEUP_GPIO)
#define CPUS_BOOTFAST_WAKEUP         (CPUS_WAKEUP_LOWBATT | CPUS_WAKEUP_LONG_KEY |CPUS_WAKEUP_USB|CPUS_WAKEUP_AC )

/*used in normal standby*/
#define CPU0_MEM_WAKEUP              (CPU0_WAKEUP_MSGBOX)
#define CPU0_BOOTFAST_WAKEUP         (CPU0_WAKEUP_MSGBOX)

#endif // CONFIG_ARCH_SUN6I

/**
*@brief struct of pmu device arg
*/
struct aw_pmu_arg{
    unsigned int  twi_port;     /**<twi port for pmu chip   */
    unsigned char dev_addr;     /**<address of pmu device   */
};

#ifdef CONFIG_ARCH_SUN7I
typedef struct _boot_dram_para_t
{
	unsigned int	dram_baseaddr;
	unsigned int	dram_clk;
	unsigned int	dram_type;
	unsigned int	dram_rank_num;
	unsigned int	dram_chip_density;
	unsigned int	dram_io_width;
	unsigned int	dram_bus_width;
	unsigned int	dram_cas;
	unsigned int	dram_zq;
	unsigned int	dram_odt_en;
	unsigned int 	dram_size;
	unsigned int	dram_tpr0;
	unsigned int	dram_tpr1;
	unsigned int	dram_tpr2;
	unsigned int	dram_tpr3;
	unsigned int	dram_tpr4;
	unsigned int	dram_tpr5;
	unsigned int 	dram_emr1;
	unsigned int	dram_emr2;
	unsigned int	dram_emr3;
}standy_dram_para_t;
#endif

/**
*@brief struct of standby
*/
struct aw_standby_para{
#ifdef CONFIG_ARCH_SUN7I
	unsigned int event_enable;   /**<event type for system wakeup        */
#endif
    unsigned int event;     /**<event type for system wakeup    */
#ifdef CONFIG_ARCH_SUN7I
	unsigned int axp_src;        /**<axp event type for system wakeup    */
	unsigned int axp_enable;     /**<axp event type for system wakeup    */
#endif
#ifdef CONFIG_ARCH_SUN6I
	unsigned int axp_event;		/**<axp event type for system wakeup    */
	unsigned int debug_mask;	/* debug mask */
	signed int   timeout;		/**<time to power off system from now, based on second */
	unsigned long gpio_enable_bitmap;
#else
    signed int   time_off;  /**<time to power off from now, based on second */
#endif
};


/**
*@brief struct of power management info
*/
struct aw_pm_info{
    struct aw_standby_para  standby_para;   /* standby parameter            */
    struct aw_pmu_arg       pmu_arg;        /**<args used by main function  */
#ifdef CONFIG_ARCH_SUN7I
	standy_dram_para_t	dram_para;
#endif
};


#endif /* __AW_PM_H__ */

