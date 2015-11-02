/*
 * sound/soc/sunxi\sun8i-codec.c
 * (C) Copyright 2010-2015
 * reuuimllatech Technology Co., Ltd. <www.reuuimllatech.com>
 * huangxin <huangxin@reuuimllatech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#define DEBUG
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/control.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>
#include <linux/clk.h>
#include <linux/timer.h>
#include <linux/io.h>
#include <linux/reset.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <sound/dmaengine_pcm.h>

#define SUNXI_DAC_DPC		(0x00)		//the same as a10
#define SUNXI_DAC_DPC_EN_DA			(31)
#define SUNXI_DAC_DPC_DVOL			(12)
#define SUNXI_DAC_FIFOC		(0x04)		//the same as a10
#define SUNXI_DAC_FIFOC_DAC_FS			(29)
#define SUNXI_DAC_FIFOC_FIR_VERSION		(28)
#define SUNXI_DAC_FIFOC_SEND_LASAT		(26)
#define SUNXI_DAC_FIFOC_TX_FIFO_MODE		(24)
#define SUNXI_DAC_FIFOC_DRQ_CLR_CNT		(21)
#define SUNXI_DAC_FIFOC_TX_TRIG_LEVEL		(8)
#define SUNXI_DAC_FIFOC_MONO_EN			(6)
#define SUNXI_DAC_FIFOC_TX_SAMPLE_BITS		(5)
#define SUNXI_DAC_FIFOC_DAC_DRQ_EN		(4)
#define SUNXI_DAC_FIFOC_FIFO_FLUSH		(0)
#define SUNXI_DAC_FIFOS		(0x08)		//the same as a10
#define SUNXI_DAC_TXDATA	(0x0c)		//the same as a10
#define SUN6I_ADC_FIFOC		(0x10)		//33 ADC FIFO Control Register, different with a10
#define SUNXI_ADC_FIFOC_DAC_FS			(29)
#define SUNXI_ADC_FIFOC_EN_AD			(28)
#define SUN6I_ADC_DIG_MIC_EN			(27)
#define SUNXI_ADC_FIFOC_RX_FIFO_MODE		(24)
#define SUN6I_ADC_DFEN				(16)
#define SUNXI_ADC_FIFOC_RX_TRIG_LEVEL		(8)
#define SUNXI_ADC_FIFOC_MONO_EN			(7)
#define SUNXI_ADC_FIFOC_RX_SAMPLE_BITS		(6)
#define SUNXI_ADC_FIFOC_ADC_DRQ_EN		(4)
#define SUNXI_ADC_FIFOC_FIFO_FLUSH		(0)

#define SUN6I_ADC_FIFOS		(0x14)		//33 ADC FIFO Status Register, different with a10
#define SUN6I_ADC_RXDATA	(0x18)		//33 ADC RX Data Register
#define SUN6I_HMIC_CTRL		(0x1c)		//Earpiece MIC bias detect register,new func
#define SUN8I_DAC_TXDATA	(0x20)

#define SUN6I_PA_CTRL		(0x24)		//new func
#define SUN6I_PA_CTRL_HPPAEN			(31)
#define SUN6I_PA_CTRL_HPCOM_CTL			(29)
#define SUN6I_PA_CTRL_HPCOM_PRO			(28)
#define SUN6I_PA_CTRL_PA_ANTI_POP_CTL		(26)
#define SUN6I_PA_CTRL_LTRNMUTE			(25)
#define SUN6I_PA_CTRL_RTLNMUTE			(24)
#define SUN6I_PA_CTRL_MIC1G			(15)
#define SUN6I_PA_CTRL_MIG2G			(12)
#define SUN6I_PA_CTRL_LINEING			(9)
#define SUN6I_PA_CTRL_PHONEG			(6)
#define SUN6I_PA_CTRL_PHONEPG			(3)
#define SUN6I_PA_CTRL_PHONENG			(0)

#define SUN6I_MIC_CTRL		(0x28)		//Microphone,Lineout and Phoneout Control Register,new func
#define SUN6I_MIC_CTRL_HBIASEN			(31)
#define SUN6I_MIC_CTRL_MBIASEN			(30)
#define SUN6I_MIC_CTRL_HBIASADCEN		(29)
#define SUN6I_MIC_CTRL_MIC1AMPEN		(28)
#define SUN6I_MIC_CTRL_MIC1BOOST		(25)
#define SUN6I_MIC_CTRL_MIC2AMPEN		(24)
#define SUN6I_MIC_CTRL_MIC2BOOST		(21)
#define SUN6I_MIC_CTRL_MIC2_SEL			(20)
#define SUN6I_MIC_CTRL_LINEOUTL_EN		(19)
#define SUN6I_MIC_CTRL_LINEOUTR_EN		(18)
#define SUN6I_MIC_CTRL_LINEOUTL_SRC_SEL		(17)
#define SUN6I_MIC_CTRL_LINEOUTR_SRC_SEL		(16)
#define SUN6I_MIC_CTRL_LINEOUT_VOL		(11)
#define SUN6I_MIC_CTRL_PHONEPREG		(8)
#define SUN6I_MIC_CTRL_PHONEOUTG		(5)
#define SUN6I_MIC_CTRL_PHONEOUT_EN		(4)
#define SUN6I_MIC_CTRL_PHONEOUTS0		(3)
#define SUN6I_MIC_CTRL_PHONEOUTS1		(2)
#define SUN6I_MIC_CTRL_PHONEOUTS2		(1)
#define SUN6I_MIC_CTRL_PHONEOUTS3		(0)

#define SUN6I_ADC_ACTL		(0x2c)		//diff with a10, ADC Analog Control Register
#define SUNXI_ADC_ACTL_ADCREN			(31)
#define SUNXI_ADC_ACTL_ADCLEN			(30)
#define SUNXI_ADC_ACTL_PREG1EN			(29)
#define SUNXI_ADC_ACTL_PREG2EN			(28)
#define SUNXI_ADC_ACTL_VMICEN			(27)
#define SUN6I_ADC_ACTL_ADCRG			(27)
#define SUN6I_ADC_ACTL_ADCLG			(24)
#define SUNXI_ADC_ACTL_VADCG			(20)
#define SUNXI_ADC_ACTL_ADCIS			(17)
#define SUN6I_ADC_ACTL_RADCMIXMUTEMIC1BOOST	(13)
#define SUN6I_ADC_ACTL_RADCMIXMUTEMIC2BOOST	(12)
#define SUN6I_ADC_ACTL_RADCMIXMUTEPHONEPN	(11)
#define SUN6I_ADC_ACTL_RADCMIXMUTEPHONEP	(10)
#define SUN6I_ADC_ACTL_RADCMIXMUTELINEINR	(9)
#define SUN6I_ADC_ACTL_RADCMIXMUTEROUTPUT	(8)
#define SUN6I_ADC_ACTL_RADCMIXMUTELOUTPUT	(7)
#define SUN6I_ADC_ACTL_LADCMIXMUTEMIC1BOOST	(6)
#define SUN6I_ADC_ACTL_LADCMIXMUTEMIC2BOOST	(5)
#define SUNXI_ADC_ACTL_PA_EN			(4)
#define SUN6I_ADC_ACTL_LADCMIXMUTEPHONEPN	(4)
#define SUNXI_ADC_ACTL_DDE			(3)
#define SUN6I_ADC_ACTL_LADCMIXMUTEPHONEP	(3)
#define SUN6I_ADC_ACTL_LADCMIXMUTELINEINL	(2)
#define SUN6I_ADC_ACTL_LADCMIXMUTELOUTPUT	(1)
#define SUN6I_ADC_ACTL_LADCMIXMUTEROUTPUT	(0)

#define SUN6I_ADDAC_TUNE	(0x30)
#define SUN6I_ADDAC_TUNE_PA_SLOPE_SECECT	(30)
#define SUN6I_ADDAC_TUNE_DITHER			(25)
#define SUN6I_ADDAC_TUNE_ZERO_CROSS_EN		(22)
#define SUN6I_BIAS_CRT		(0x34)
#define SUN6I_BIAS_CRT_OPMIC_BIAS_CUR		(30)
#define SUN6I_BIAS_CRT_BIASCALIVERIFY		(29)
#define SUN6I_BIAS_CRT_BIASVERIFY		(23)
#define SUN6I_BIAS_CRT_BIASCALI			(17)
#define SUN6I_BIAS_CRT_DA16CALIVERIFY		(16)
#define SUN6I_BIAS_CRT_DA16VERIFY		(8)
#define SUN6I_BIAS_CRT_DA16CALI			(0)

#define SUN6I_DAC_TXCNT		(0x40)
#define SUN6I_ADC_RXCNT		(0x44)
#define SUN6I_DAC_DEBUG		(0x48)
#define SUN6I_ADC_DEBUG		(0x4c)
#define SUN6I_HMIC_CTL		(0x50)		//new func
#define SUN6I_HMIC_DATA		(0x54)		//new func
#define SUN6I_DAC_DAP_CTL	(0x60)		//DAC DAP control Register
#define SUN6I_DAC_DAP_VOL	(0x64)		//DAC DAP volume Register
#define SUN6I_DAC_DAP_COF	(0x68)		//DAC DAP Coefficient Register
#define SUN6I_DAC_DAP_OPT	(0x6c)		//DAC DAP Optimum Register
#define SUN6I_ADC_DAP_CTL	(0x70)		//ADC DAP Control Register
#define SUN6I_ADC_DAP_VOL	(0x74)		//ADC DAP volume Register
#define SUN6I_ADC_DAP_LCTL	(0x78)		//ADC DAP Left Control Register
#define SUN6I_ADC_DAP_RCTL	(0x7c)		//ADC DAP Right Control Register
#define SUN6I_ADC_DAP_PARA	(0x80)		//ADC DAP Parameter Control Register
#define SUN6I_ADC_DAP_LAC	(0x84)		//ADC DAP Left Average Coefficient Register
#define SUN6I_ADC_DAP_LDAT	(0x88)		//ADC DAP Left Decay&Attack Time Register
#define SUN6I_ADC_DAP_RAC	(0x8c)		//ADC DAP Right Average Coefficient Register
#define SUN6I_ADC_DAP_RDAC	(0x90)		//ADC DAP Right Decay&Attack time Register
#define SUN6I_ADC_DAP_HPFC	(0x94)		//ADC DAP HPF Coefficient Register


#define RADCMIXMUTE				  (7)
#define LADCMIXMUTE				  (0)

/*DAC Debug Register
* codecbase+0x48
*/
#define DAC_SWP					  (6)

/*HMIC Control Register
*codecbase+0x50
*/
#define HMIC_M					  (28)
#define HMIC_N					  (24)
#define HMIC_DIRQ				  (23)
#define HMIC_TH1_HYS			  (21)
#define	HMIC_EARPHONE_OUT_IRQ_EN  (20)
#define HMIC_EARPHONE_IN_IRQ_EN	  (19)
#define HMIC_KEY_UP_IRQ_EN		  (18)
#define HMIC_KEY_DOWN_IRQ_EN	  (17)
#define HMIC_DATA_IRQ_EN		  (16)
#define HMIC_DS_SAMP			  (14)
#define HMIC_TH2_HYS			  (13)
#define HMIC_TH2_KEY		      (8)
#define HMIC_SF_SMOOTH_FIL		  (6)
#define KEY_UP_IRQ_PEND			  (5)
#define HMIC_TH1_EARPHONE		  (0)

/*HMIC Data Register
* codecbase+0x54
*/
#define HMIC_EARPHONE_OUT_IRQ_PEND  (20)
#define HMIC_EARPHONE_IN_IRQ_PEND   (19)
#define HMIC_KEY_UP_IRQ_PEND 	    (18)
#define HMIC_KEY_DOWN_IRQ_PEND 		(17)
#define HMIC_DATA_IRQ_PEND			(16)
#define HMIC_ADC_DATA				(0)

/*DAC DAP Control Register
* codecbase+0x60
*/
#define DAC_DAP_EN						(31)
#define DAC_DAP_CTL						(30)
#define DAC_DAP_STATE					(29)
#define DAC_BQ_EN						(16)
#define DAC_DRC_EN						(15)
#define DAC_HPF_EN						(14)
#define DAC_DE_CTL						(12)
#define DAC_RAM_ADDR					(0)

/* DAC DAP volume register
*	codecbase+0x64
*/
#define DAC_LEFT_CHAN_SOFT_MUTE_CTL		(30)
#define DAC_RIGHT_CHAN_SOFT_MUTE_CTL	(29)
#define DAC_MASTER_SOFT_MUTE_CTL		(28)
#define DAC_SKEW_TIME_VOL_CTL			(24)
#define DAC_MASTER_VOL					(16)
#define DAC_LEFT_CHAN_VOL				(8)
#define DAC_RIGHT_CHAN_VOL				(0)

/*ADC DAP Control Register
* codecbase+0x70
*/
#define ADC_DAP_EN					(31)
#define ADC_DAP_START				(30)
#define AGC_LEFT_SATURATION_FLAG	(21)
#define AGC_LEFT_NOISE_THRES_FLAG	(20)
#define AGC_LEFT_GAIN_APP			(12)
#define AGC_RIGHT_SATURATION_FLAG	(9)
#define AGC_RIGHT_NOISE_THRES_FLAG	(8)
#define AGC_RIGHT_GAIN_APP			(0)

/*ADC DAP Volume Register
* codecbase+0x74
*/
#define ADC_LEFT_CHAN_VOL_MUTE		(18)
#define ADC_RIGHT_CHAN_VOL_MUTE		(17)
#define ADC_SKEW_TIME_VOL			(16)
#define ADC_LEFT_CHAN_VOL			(8)
#define ADC_RIGHT_CHAN_VOL			(0)

/*ADC DAP Left control register
* codecbase+0x78
*/
#define ADC_LEFT_CHAN_NOISE_THRES_SET	(16)
#define ADC_LEFT_AGC_EN					(14)
#define ADC_LEFT_HPF_EN					(13)
#define ADC_LEFT_NOISE_DET_EN			(12)
#define ADC_LEFT_HYS_SET				(8)
#define ADC_LEFT_NOISE_DEBOURCE_TIME	(4)
#define ADC_LEFT_SIGNAL_DEBOUNCE_TIME	(0)

/*ADC DAP Right Control Register
* codecbase+0x7c
*/
#define ADC_RIGHT_CHAN_NOISE_THRES_SET	(16)
#define ADC_RIGHT_AGC_EN				(14)
#define ADC_RIGHT_HPF_EN				(13)
#define ADC_RIGHT_NOISE_DET_EN			(12)
#define ADC_RIGHT_HYS_SET				(8)
#define ADC_RIGHT_NOISE_DEBOURCE_TIME	(4)
#define ADC_RIGHT_SIGNAL_DEBOUNCE_TIME	(0)

/*ADC DAP Parameter Register
* codecbase+0x80
*/
#define ADC_LEFT_CHAN_TARG_LEVEL_SET	(24)
#define ADC_RIGHT_CHAN_TARG_LEVEL_SET	(16)
#define ADC_LEFT_CHAN_MAX_GAIN_SET		(8)
#define ADC_RIGHT_CHAN_MAX_GAIN_SET		(0)

/*ADC DAP Left Decay&Attack time register
* codecbase+0x88
*/
#define ADC_LEFT_ATTACK_TIME_COEFF_SET	(16)
#define ADC_LEFT_DECAY_TIME_COEFF_SET	(0)

/*ADC DAP Right decay&Attack Time register
* codecbae+0x90
*/
#define ADC_RIGHT_ATTACK_TIME_COEFF_SET	(16)
#define ADC_RIGHT_DECAY_TIME_COEFF_SET	(0)


/* Supported SoC families - used for quirks */
enum sun8i_soc_family {
	SUN8I,	/* H3 SoC */
};

struct sun8i_priv {
	void __iomem		*base;

	struct clk		*clk_apb;
	struct clk		*clk_module;
	struct reset_control	*rstc;

	bool			linein_playback;
	bool			linein_capture;
	bool			headphone_playback;
	bool			earpiece_playback;
	bool			speaker_playback;
	bool			speaker_active;

	struct regmap		*regmap;

	enum sun8i_soc_family revision;

	struct snd_dmaengine_dai_dma_data	playback_dma_data;
	struct snd_dmaengine_dai_dma_data	capture_dma_data;
};

//struct sun8i_priv *sun8i;

void codec_wr_control(struct sun8i_priv *sun8i, u32 reg, u32 mask, u32 shift, u32 val)
{
	regmap_update_bits(sun8i->regmap, reg, mask << shift, val << shift);
}

static int sun8i_codec_pa_play_open(struct sun8i_priv *sun8i)
{
	/* int pa_vol = 0; */
	/* script_item_u val; */
	/* script_item_value_type_e  type; */
	/* int pa_double_used = 0; */

	/* type = script_get_item("audio_para", "pa_double_used", &val); */
	/* if (SCIRPT_ITEM_VALUE_TYPE_INT != type) { */
	/* 	printk("[audiocodec] type err!\n"); */
	/* } */
	/* pa_double_used = val.val; */
	/* if (!pa_double_used) { */
	/* 	type = script_get_item("audio_para", "pa_single_vol", &val); */
	/* 	if (SCIRPT_ITEM_VALUE_TYPE_INT != type) { */
	/* 		printk("[audiocodec] type err!\n"); */
	/* 	} */
	/* 	pa_vol = val.val; */
	/* } else { */
	/* 	type = script_get_item("audio_para", "pa_double_vol", &val); */
	/* 	if (SCIRPT_ITEM_VALUE_TYPE_INT != type) { */
	/* 		printk("[audiocodec] type err!\n"); */
	/* 	} */
	/* 	pa_vol = val.val; */
	/* } */

	/*enable dac digital*/
	codec_wr_control(sun8i, SUNXI_DAC_DPC, 0x1, SUNXI_DAC_DPC_EN_DA, 0x1);
	/*set TX FIFO send drq level*/
	codec_wr_control(sun8i, SUNXI_DAC_FIFOC ,0x7f, SUNXI_DAC_FIFOC_TX_TRIG_LEVEL, 0xf);
	/*set TX FIFO MODE*/
	codec_wr_control(sun8i, SUNXI_DAC_FIFOC ,0x1, SUNXI_DAC_FIFOC_TX_FIFO_MODE, 0x1);

	//send last sample when dac fifo under run
	codec_wr_control(sun8i, SUNXI_DAC_FIFOC ,0x1, SUNXI_DAC_FIFOC_SEND_LASAT, 0x0);

	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, SUN6I_MIC_CTRL_LINEOUTL_EN, 0x1);
	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, SUN6I_MIC_CTRL_LINEOUTR_EN, 0x1);

	/* TODO: This used to be retrieved by FEX */
	/* if (!pa_double_used) { */
		codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, SUN6I_MIC_CTRL_LINEOUTR_SRC_SEL, 0x1);
		codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, SUN6I_MIC_CTRL_LINEOUTL_SRC_SEL, 0x1);
	/* } else { */
	/* 	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, LINEOUTL_SRC_SEL, 0x0); */
	/* 	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, LINEOUTR_SRC_SEL, 0x0); */
	/* } */
	
	/*
	 * TODO: This used to be retrieved by FEX.
	 * The script has nice comments explaining what values mean in term of dB output
	 */
	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1f, SUN6I_MIC_CTRL_LINEOUT_VOL, 0x19);

	/* TODO: Configure the GPIO using gpiolib */
	/* mdelay(3); */
	/* item.gpio.data = 1; */
	/* /\*config gpio info of audio_pa_ctrl open*\/ */
	/* if (0 != sw_gpio_setall_range(&item.gpio, 1)) { */
	/* 	printk("sw_gpio_setall_range failed\n"); */
	/* } */
	/* mdelay(62); */

	return 0;
}

static int sun8i_codec_capture_open(struct sun8i_priv *sun8i)
{
	int cap_vol = 0;
	/* script_item_u val; */
	/* script_item_value_type_e  type; */

	/* type = script_get_item("audio_para", "cap_vol", &val); */
	/* if (SCIRPT_ITEM_VALUE_TYPE_INT != type) { */
	/* 	printk("[audiocodec] type err!\n"); */
	/* } */
	/* cap_vol = val.val; */

	/* TODO: This used to be retrieved by FEX */
	cap_vol = 5;

	/*enable mic1 pa*/
	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, SUN6I_MIC_CTRL_MIC1AMPEN, 0x1);
	/*mic1 gain 36dB,if capture volume is too small, enlarge the mic1boost*/
	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x7, SUN6I_MIC_CTRL_MIC1BOOST,cap_vol);//36db
	/*enable Master microphone bias*/
	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, SUN6I_MIC_CTRL_MBIASEN, 0x1);

	/*enable Right MIC1 Boost stage*/
	codec_wr_control(sun8i, SUN6I_ADC_ACTL, 0x1, SUN6I_ADC_ACTL_RADCMIXMUTEMIC1BOOST, 0x1);
	/*enable Left MIC1 Boost stage*/
	codec_wr_control(sun8i, SUN6I_ADC_ACTL, 0x1, SUN6I_ADC_ACTL_LADCMIXMUTEMIC1BOOST, 0x1);
	/*enable adc_r adc_l analog*/
	codec_wr_control(sun8i, SUN6I_ADC_ACTL, 0x1, SUNXI_ADC_ACTL_ADCREN, 0x1);
	codec_wr_control(sun8i, SUN6I_ADC_ACTL, 0x1, SUNXI_ADC_ACTL_ADCLEN, 0x1);
	/*set RX FIFO mode*/
	codec_wr_control(sun8i, SUN6I_ADC_FIFOC, 0x1, SUNXI_ADC_FIFOC_RX_FIFO_MODE, 0x1);
	/*set RX FIFO rec drq level*/
	codec_wr_control(sun8i, SUN6I_ADC_FIFOC, 0x1f, SUNXI_ADC_FIFOC_RX_TRIG_LEVEL, 0xf);
	/*enable adc digital part*/
	codec_wr_control(sun8i, SUN6I_ADC_FIFOC, 0x1, SUNXI_ADC_FIFOC_EN_AD, 0x1);

	return 0;
}

static int sun8i_codec_play_start(struct sun8i_priv *sun8i)
{
	/*enable dac drq*/
	codec_wr_control(sun8i, SUNXI_DAC_FIFOC ,0x1, SUNXI_DAC_FIFOC_DAC_DRQ_EN, 0x1);
	/*DAC FIFO Flush,Write '1' to flush TX FIFO, self clear to '0'*/
	codec_wr_control(sun8i, SUNXI_DAC_FIFOC ,0x1, SUNXI_DAC_FIFOC_FIFO_FLUSH, 0x1);

	return 0;
}

static int sun8i_codec_play_stop(struct sun8i_priv *sun8i)
{
	int i = 0;
	int headphone_vol = 0;
	/* script_item_u val; */
	/* script_item_value_type_e  type; */

	/* type = script_get_item("audio_para", "headphone_vol", &val); */
	/* if (SCIRPT_ITEM_VALUE_TYPE_INT != type) { */
	/* 	printk("[audiocodec] type err!\n"); */
	/* } */
	/* headphone_vol = val.val; */

	/* TODO: This used to be retrieved by FEX*/
	headphone_vol = 0x3b;

	codec_wr_control(sun8i, SUN6I_ADDAC_TUNE, 0x1, SUN6I_ADDAC_TUNE_ZERO_CROSS_EN, 0x0);

	/*disable dac drq*/
	codec_wr_control(sun8i, SUNXI_DAC_FIFOC ,0x1, SUNXI_DAC_FIFOC_DAC_DRQ_EN, 0x0);

	/*disable dac digital*/
	codec_wr_control(sun8i, SUNXI_DAC_DPC ,  0x1, SUNXI_DAC_DPC_EN_DA, 0x0);

	/* TODO: Configure the GPIO */
	/* item.gpio.data = 0; */
	/* /\*config gpio info of audio_pa_ctrl open*\/ */
	/* if (0 != sw_gpio_setall_range(&item.gpio, 1)) { */
	/* 	printk("sw_gpio_setall_range failed\n"); */
	/* } */

	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, SUN6I_MIC_CTRL_LINEOUTR_EN, 0x0);
	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, SUN6I_MIC_CTRL_LINEOUTL_EN, 0x0);

	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, SUN6I_MIC_CTRL_LINEOUTL_SRC_SEL, 0x0);
	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, SUN6I_MIC_CTRL_LINEOUTR_SRC_SEL, 0x0);
	
	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, SUN6I_MIC_CTRL_PHONEOUTS2, 0x0);
	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, SUN6I_MIC_CTRL_PHONEOUTS3, 0x0);
	return 0;
}

static int sun8i_codec_capture_start(struct sun8i_priv *sun8i)
{
	/*enable adc drq*/
	codec_wr_control(sun8i, SUN6I_ADC_FIFOC ,0x1, SUNXI_ADC_FIFOC_ADC_DRQ_EN, 0x1);
	return 0;
}

static int sun8i_codec_capture_stop(struct sun8i_priv *sun8i)
{
	/*disable adc digital part*/
	codec_wr_control(sun8i, SUN6I_ADC_FIFOC, 0x1, SUNXI_ADC_FIFOC_EN_AD, 0x0);
	/*disable adc drq*/
	codec_wr_control(sun8i, SUN6I_ADC_FIFOC ,0x1, SUNXI_ADC_FIFOC_ADC_DRQ_EN, 0x0);
	/*disable mic1 pa*/
	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, SUN6I_MIC_CTRL_MIC1AMPEN, 0x0);
	/*disable Master microphone bias*/
	codec_wr_control(sun8i, SUN6I_MIC_CTRL, 0x1, SUN6I_MIC_CTRL_MBIASEN, 0x0);
	/*disable adc_r adc_l analog*/
	codec_wr_control(sun8i, SUN6I_ADC_ACTL, 0x1, SUNXI_ADC_ACTL_ADCREN, 0x0);
	codec_wr_control(sun8i, SUN6I_ADC_ACTL, 0x1, SUNXI_ADC_ACTL_ADCLEN, 0x0);

	return 0;
}

static int sun8i_codec_prepare(struct snd_pcm_substream *substream,
			 struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct sun8i_priv *sun8i = snd_soc_card_get_drvdata(rtd->card);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (sun8i->speaker_active) {
			/* return codec_pa_play_open(sun8i); */
		} else {
			/*set TX FIFO send drq level*/
			codec_wr_control(sun8i, SUNXI_DAC_FIFOC ,0x7f, SUNXI_DAC_FIFOC_TX_TRIG_LEVEL, 0xf);

			/*set TX FIFO MODE*/
			codec_wr_control(sun8i, SUNXI_DAC_FIFOC ,0x1, SUNXI_DAC_FIFOC_TX_FIFO_MODE, 0x1);

			//send last sample when dac fifo under run
			codec_wr_control(sun8i, SUNXI_DAC_FIFOC ,0x1, SUNXI_DAC_FIFOC_SEND_LASAT, 0x0);
		}
	} else {
		/* return codec_capture_open(sun8i); */
	}

	return 0;
}

static int sun8i_codec_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	return 0;
}

static int sun8i_codec_digital_mute(struct snd_soc_dai *dai, int mute)
{
	return 0;
}

static int sun8i_codec_trigger(struct snd_pcm_substream *substream, int cmd,
			 struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct sun8i_priv *priv = snd_soc_card_get_drvdata(rtd->card);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			sun8i_codec_play_start(priv);
		} else {
			sun8i_codec_capture_start(priv);

			/*hardware fifo delay*/
			mdelay(200);
			codec_wr_control(priv, SUN6I_ADC_FIFOC, 0x1, SUNXI_ADC_FIFOC_FIFO_FLUSH, 0x1);
		}
		break;

	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
			sun8i_codec_play_stop(priv);
		else
			sun8i_codec_capture_stop(priv);
		break;
	default:
		dev_err(dai->dev, "Unsupported trigger operation\n");
		return -EINVAL;

	}
	return 0;
}

static int sun8i_codec_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params,
				 struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct sun8i_priv *priv = snd_soc_card_get_drvdata(rtd->card);
	int is_mono = !!(params_channels(params) == 1);
	int is_24bit = !!(hw_param_interval(params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS)->min == 32);
	unsigned int rate = params_rate(params);
	unsigned int hwrate;

	switch (rate) {
	case 192000:
	case 96000:
	case 48000:
	case 32000:
	case 24000:
	case 16000:
	case 12000:
	case 8000:
		clk_set_rate(priv->clk_module, 24576000);
		break;
	case 176400:
	case 88200:
	case 44100:
	case 33075:
	case 22050:
	case 14700:
	case 11025:
	case 7350:
	default:
		clk_set_rate(priv->clk_module, 22579200);
		break;
	}

	switch (rate) {
	case 192000:
	case 176400:
		hwrate = 6;
		break;
	case 96000:
	case 88200:
		hwrate = 7;
		break;
	default:
	case 48000:
	case 44100:
		hwrate = 0;
		break;
	case 32000:
	case 33075:
		hwrate = 1;
		break;
	case 24000:
	case 22050:
		hwrate = 2;
		break;
	case 16000:
	case 14700:
		hwrate = 3;
		break;
	case 12000:
	case 11025:
		hwrate = 4;
		break;
	case 8000:
	case 7350:
		hwrate = 5;
		break;
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		regmap_update_bits(priv->regmap, SUNXI_DAC_FIFOC, 7 << SUNXI_DAC_FIFOC_DAC_FS, hwrate << SUNXI_DAC_FIFOC_DAC_FS);
		regmap_update_bits(priv->regmap, SUNXI_DAC_FIFOC, 1 << SUNXI_DAC_FIFOC_MONO_EN, is_mono << SUNXI_DAC_FIFOC_MONO_EN);
		regmap_update_bits(priv->regmap, SUNXI_DAC_FIFOC, 1 << SUNXI_DAC_FIFOC_TX_SAMPLE_BITS, is_24bit << SUNXI_DAC_FIFOC_TX_SAMPLE_BITS);
		regmap_update_bits(priv->regmap, SUNXI_DAC_FIFOC, 1 << SUNXI_DAC_FIFOC_TX_FIFO_MODE, !is_24bit << SUNXI_DAC_FIFOC_TX_FIFO_MODE);
		if (is_24bit)
			priv->playback_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		else
			priv->playback_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	} else  {
		regmap_update_bits(priv->regmap, SUN6I_ADC_FIFOC, 7 << SUNXI_ADC_FIFOC_DAC_FS, hwrate << SUNXI_ADC_FIFOC_DAC_FS);
		regmap_update_bits(priv->regmap, SUN6I_ADC_FIFOC, 1 << SUNXI_ADC_FIFOC_MONO_EN, is_mono << SUNXI_ADC_FIFOC_MONO_EN);
	}

	return 0;
}

static int sun8i_codec_dai_probe(struct snd_soc_dai *dai)
{
	struct snd_soc_card *card = snd_soc_dai_get_drvdata(dai);
	struct sun8i_priv *priv = snd_soc_card_get_drvdata(card);

	snd_soc_dai_init_dma_data(dai,
				  &priv->playback_dma_data,
				  &priv->capture_dma_data);
	return 0;
}

static void sun8i_codec_init(struct sun8i_priv *sun8i)
{
	/*
	 * Stop doing DMA requests whenever there's only 16 samples
	 * left available in the TX FIFO.
	 */
	codec_wr_control(sun8i, SUNXI_DAC_FIFOC, 0x3, SUNXI_DAC_FIFOC_DRQ_CLR_CNT,0x3);

	/* Flush TX FIFO */
	codec_wr_control(sun8i, SUNXI_DAC_FIFOC, 0x1, SUNXI_DAC_FIFOC_FIFO_FLUSH, 0x1);

	/* Flush RX FIFO */
	codec_wr_control(sun8i, SUN6I_ADC_FIFOC, 0x1, SUNXI_ADC_FIFOC_FIFO_FLUSH, 0x1);

	/* Do DRQ whenever the FIFO is empty */
	codec_wr_control(sun8i, SUNXI_DAC_FIFOC, 0x1, SUNXI_DAC_FIFOC_DAC_DRQ_EN, 0x1);

	/* /\* Use a 32 bits FIR *\/ */
	/* codec_wr_control(sun8i, SUNXI_DAC_FIFOC, 0x1, FIR_VERSION, 0x1); */

}

static int sun8i_codec_startup(struct snd_pcm_substream *substream,
				   struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct sun8i_priv *priv = snd_soc_card_get_drvdata(rtd->card);

	sun8i_codec_init(priv);

	clk_prepare_enable(priv->clk_module);

	return 0;
}

static void sun8i_codec_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct sun8i_priv *priv = snd_soc_card_get_drvdata(rtd->card);

	clk_disable_unprepare(priv->clk_module);
}

/*** Codec DAI ***/

static const struct snd_soc_dai_ops sun8i_codec_dai_ops = {
	.startup	= sun8i_codec_startup,
	.shutdown	= sun8i_codec_shutdown,
	.set_fmt	= sun8i_codec_set_fmt,
	.digital_mute	= sun8i_codec_digital_mute,
	.prepare	= sun8i_codec_prepare,
	.hw_params	= sun8i_codec_hw_params,
	.trigger	= sun8i_codec_trigger,
};

#define SUN6I_RATES	SNDRV_PCM_RATE_8000_192000
#define SUN6I_FORMATS	(SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | \
			SNDRV_PCM_FMTBIT_S24_LE)
static struct snd_soc_dai_driver sun8i_codec_dai = {
	.name = "Codec",
	.playback = {
		.stream_name = "Codec Playback",
		.channels_min	= 1,
		.channels_max	= 2,
		.rate_min	= 8000,
		.rate_max	= 192000,
		.rates		= SUN6I_RATES | SNDRV_PCM_RATE_KNOT,
		.formats	= SUN6I_FORMATS,
	},

	.capture = {
		.stream_name = "Codec Capture",
		.channels_min	= 1,
		.channels_max	= 2,
		.rate_min	= 8000,
		.rate_max	= 192000,
		.rates		= SUN6I_RATES | SNDRV_PCM_RATE_KNOT,
		.formats	= SUN6I_FORMATS,
	},
	.ops = &sun8i_codec_dai_ops,
};

/*** Codec ***/

static const char *sun8i_zero_crossover_time[] = {"32ms", "64ms"};
static const struct soc_enum sun8i_zero_crossover_time_enum =
	SOC_ENUM_SINGLE(SUN6I_ADDAC_TUNE, 21, 2, sun8i_zero_crossover_time);

static const char *sun8i_fir_length[] = {"64 bits", "32 bits"};
static const struct soc_enum sun8i_fir_length_enum =
	SOC_ENUM_SINGLE(SUNXI_DAC_FIFOC, 28, 2, sun8i_fir_length);

static const struct snd_kcontrol_new sun8i_snd_controls[] = {
	/* This is actually an attenuation by 64 steps of -1.16dB */
	SOC_SINGLE("DAC Playback Volume",
		   SUNXI_DAC_DPC, 12, 0x1f, 1),
	SOC_SINGLE("DAC High Pass Filter Switch",
		   SUNXI_DAC_DPC, 18, 1, 0),

	SOC_ENUM("Zero-crossover Time", sun8i_zero_crossover_time_enum),
	SOC_ENUM("FIR Length", sun8i_fir_length_enum),

};

static const struct snd_soc_dapm_widget sun8i_dapm_widgets[] = {
	/* Digital controls of the DACs */
	SND_SOC_DAPM_DAC("DAC", "Playback", SUNXI_DAC_DPC, 31, 0),

	/* Power up of the Headphone amplifier */
	SND_SOC_DAPM_PGA("Headphone Amplifier",
			 SUN6I_PA_CTRL, 31, 0, NULL, 0),

	SND_SOC_DAPM_OUTPUT("HPL"),
	SND_SOC_DAPM_OUTPUT("HPR"),
};

static const struct snd_soc_dapm_route sun8i_dapm_routes[] = {
	/* DACs */
	{ "Left DAC", NULL, "DAC" },
	{ "Right DAC", NULL, "DAC" },

	/* Left Mixer */
	{ "Left Output Mixer", NULL, "Left DAC" },
	{ "Left Output Mixer", NULL, "Right DAC" },

	/* Right Mixer */
	{ "Right Output Mixer", NULL, "Left DAC" },
	{ "Right Output Mixer", NULL, "Right DAC" },

	/* Left HP Mux */
	{ "Left Headphone Amplifier Mux", NULL, "Left Output Mixer" },
	{ "Left Headphone Amplifier Mux", NULL, "Left DAC" },

	/* Right HP Mux */
	{ "Right Headphone Amplifier Mux", NULL, "Right Output Mixer" },
	{ "Right Headphone Amplifier Mux", NULL, "Right DAC" },

	/* Left HP Amplifier */
	{ "Headphone Amplifier", NULL, "Left Headphone Amplifier Mux" },

	/* Right HP Amplifier */
	{ "Headphone Amplifier", NULL, "Right Headphone Amplifier Mux" },

	/* Power up the headphone amplifiers */
	{ "Left Headphone Amplifier", NULL, "Headphone Amplifier" },
	{ "Right Headphone Amplifier", NULL, "Headphone Amplifier" },

	/* Headphone outputs */
	{ "HPL", NULL, "Left Headphone Amplifier" },
	{ "HPR", NULL, "Right Headphone Amplifier" },
};

static struct snd_soc_codec_driver sun8i_codec = {

	.controls		= sun8i_snd_controls,
	.num_controls		= ARRAY_SIZE(sun8i_snd_controls),
	.dapm_widgets		= sun8i_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(sun8i_dapm_widgets),
	.dapm_routes		= sun8i_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(sun8i_dapm_routes),
};

/*** Board routing ***/
/* TODO: do this with DT */

static const struct snd_soc_dapm_widget sun8i_board_dapm_widgets[] = {
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
};

static const struct snd_soc_dapm_route sun8i_board_routing[] = {
	{ "Headphone Jack",	NULL,	"HPL" },
	{ "Headphone Jack",	NULL,	"HPR" },
};

static int sun8i_dai_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dapm_context *dapm = snd_soc_codec_get_dapm(codec);

	snd_soc_dapm_enable_pin(dapm, "Headphone Jack");

	return 0;
}

/*** Card and DAI Link ***/

static struct snd_soc_dai_link sun8i_card_dai[] = {
	{
		.name		= "sun8i-audio",
		.stream_name	= "CDC PCM",
		.codec_dai_name	= "Codec",
		.cpu_dai_name	= "1c22c00.codec",
		.codec_name	= "1c22c00.codec",
		.platform_name	= "1c22c00.codec",
		.dai_fmt	= SND_SOC_DAIFMT_I2S,
		.init		= sun8i_dai_init,
	},
};

static struct snd_soc_card snd_soc_sun8i_codec = {
	.name	= "sun8i-codec",
	.owner	= THIS_MODULE,

	.dai_link = &sun8i_card_dai,
	.num_links = ARRAY_SIZE(sun8i_card_dai),

	.dapm_widgets = sun8i_board_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(sun8i_board_dapm_widgets),
	.dapm_routes = sun8i_board_routing,
	.num_dapm_routes = ARRAY_SIZE(sun8i_board_routing),
};

/*** CPU DAI ***/

static const struct snd_soc_component_driver sun8i_codec_component = {
	.name		= "sun8i-codec",
};

static struct snd_soc_dai_driver dummy_cpu_dai = {
	.name = "sun8i-cpu-dai",
	.probe	= sun8i_codec_dai_probe,
	.playback = {
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= SUN6I_RATES,
		.formats	= SUN6I_FORMATS,
	},

	.capture = {
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= SUN6I_RATES,
		.formats	= SUN6I_FORMATS,
	},
};

static struct regmap_config sun8i_codec_regmap_config = {
	.reg_bits	= 32,
	.reg_stride	= 4,
	.val_bits	= 32,
	.max_register	= SUN6I_ADC_DAP_HPFC,
	.fast_io	= true,
};

static const struct of_device_id sun8i_codec_of_match[] = {
	{ .compatible = "allwinner,sun8i-h3-codec", .data = (void *)SUN8I},
	{}
};
MODULE_DEVICE_TABLE(of, sun8i_codec_of_match);

static int sun8i_codec_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct snd_soc_card *card = &snd_soc_sun8i_codec;
	const struct of_device_id *of_id;
	struct device *dev = &pdev->dev;
	struct sun8i_priv *priv;
	struct resource *res;
	void __iomem *base;
	int ret;

	if (!of_device_is_available(np))
		return -ENODEV;

	of_id = of_match_device(sun8i_codec_of_match, dev);
	if (!of_id)
		return -EINVAL;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	card->dev = &pdev->dev;
	platform_set_drvdata(pdev, card);
	snd_soc_card_set_drvdata(card, priv);

	priv->revision = (enum sun8i_soc_family)of_id->data;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base))
		return PTR_ERR(base);

	priv->regmap = devm_regmap_init_mmio(&pdev->dev, base,
					     &sun8i_codec_regmap_config);
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);

	/* Get the clocks from the DT */
	priv->clk_apb = devm_clk_get(dev, "apb");
	if (IS_ERR(priv->clk_apb)) {
		dev_err(dev, "failed to get apb clock\n");
		return PTR_ERR(priv->clk_apb);
	}
	priv->clk_module = devm_clk_get(dev, "codec");
	if (IS_ERR(priv->clk_module)) {
		dev_err(dev, "failed to get codec clock\n");
		return PTR_ERR(priv->clk_module);
	}

	/* Enable the clock on a basic rate */
	ret = clk_set_rate(priv->clk_module, 24576000);
	if (ret) {
		dev_err(dev, "failed to set codec base clock rate\n");
		return ret;
	}

	/* Enable the bus clock */
	if (clk_prepare_enable(priv->clk_apb)) {
		dev_err(dev, "failed to enable apb clock\n");
		clk_disable_unprepare(priv->clk_module);
		return -EINVAL;
	}

	/* DMA configuration for TX FIFO */
	priv->playback_dma_data.addr = res->start + SUN8I_DAC_TXDATA;

	priv->playback_dma_data.maxburst = 4;
	priv->playback_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;

	/* DMA configuration for RX FIFO */
	priv->capture_dma_data.addr = res->start + SUN6I_ADC_RXDATA;
	priv->capture_dma_data.maxburst = 4;
	priv->capture_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;

	ret = snd_soc_register_codec(&pdev->dev, &sun8i_codec, &sun8i_codec_dai, 1);

	ret = devm_snd_soc_register_component(&pdev->dev, &sun8i_codec_component, &dummy_cpu_dai, 1);
	if (ret)
		goto err_clk_disable;

	ret = devm_snd_dmaengine_pcm_register(&pdev->dev, NULL, 0);
	if (ret)
		goto err_clk_disable;

	sun8i_codec_init(priv);
	dev_err(&pdev->dev, "COOPS:regmap 0x%x\n", priv->regmap);

	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card failed (%d)\n", ret);
		goto err_fini_utils;
	}

	ret = snd_soc_of_parse_audio_routing(card, "routing");
	if (ret)
		goto err;

	return 0;

err_fini_utils:
err:
err_clk_disable:
	clk_disable_unprepare(priv->clk_apb);
	return ret;
}

static int sun8i_codec_remove(struct platform_device *pdev)
{
	struct sun8i_priv *priv = platform_get_drvdata(pdev);

	clk_disable_unprepare(priv->clk_apb);
	clk_disable_unprepare(priv->clk_module);

	return 0;
}

static struct platform_driver sun8i_codec_driver = {
	.driver		= {
		.name	= "sun8i-codec",
		.owner = THIS_MODULE,
		.of_match_table = sun8i_codec_of_match,
	},
	.probe		= sun8i_codec_probe,
	.remove		= sun8i_codec_remove,
};
module_platform_driver(sun8i_codec_driver);

MODULE_DESCRIPTION("sun8i CODEC ALSA codec driver");
MODULE_AUTHOR("huangxin");
MODULE_LICENSE("GPL");
