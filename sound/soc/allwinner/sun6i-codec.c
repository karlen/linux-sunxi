/*
 * sound\soc\sun6i\sun6i-codec.c
 * (C) Copyright 2010-2016
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
#ifndef CONFIG_PM
#define CONFIG_PM
#endif
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
#ifdef CONFIG_PM
#include <linux/pm.h>
#endif
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
#include <linux/of_gpio.h>
#include <sound/dmaengine_pcm.h>

#include "sun6i-codec.h"

struct sun6i_priv {
	void __iomem		*base;

	struct clk		*apb_clk;
	struct clk		*mod_clk;
	struct reset_control	*rstc;

	bool			linein_playback;
	bool			linein_capture;
	bool			headphone_playback;
	bool			earpiece_playback;
	bool			speaker_playback;
	bool			speaker_active;

	struct regmap		*regmap;
};

struct sun6i_priv *sun6i;

void codec_wr_control(struct sun6i_priv *sun6i, u32 reg, u32 mask, u32 shift, u32 val)
{
	regmap_update_bits(sun6i->regmap, reg, mask << shift, val << shift);
}

static void sun6i_codec_hp_chan_mute(struct sun6i_priv *sun6i, bool left, bool right)
{
	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, LHPPA_MUTE, left ? 0 : 1);
	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, RHPPA_MUTE, right ? 0 : 1);
}

#define SUN6I_DAC_DIGITAL_CTRL_REG	0x00
#define SUN6I_DAC_FIFO_CTRL_REG		0x04
#define SUN6I_DAC_FIFO_STATUS_REG	0x08
#define SUN6I_DAC_FIFO_REG		0x0c
#define SUN6I_ADC_FIFO_CTRL_REG		0x10
#define SUN6I_ADC_FIFO_STATUS_REG	0x14
#define SUN6I_ADC_FIFO_REG		0x18
#define SUN6I_DAC_ANALOG_CTRL_REG	0x20
#define SUN6I_POWER_AMPLIFIER_CTRL_REG	0x24
#define SUN6I_ANALOG_PERF_TUNING_REG	0x30


static const char *sun6i_zero_crossover_time[] = {"32ms", "64ms"};
static const char *sun6i_fir_length[] = {"64 bits", "32 bits"};
static const char *sun6i_left_hp_mux[] = {"Left DAC", "Left Output Mixer"};
static const char *sun6i_right_hp_mux[] = {"Right DAC", "Right Output Mixer"};

static const struct soc_enum sun6i_zero_crossover_time_enum =
	SOC_ENUM_SINGLE(SUN6I_ANALOG_PERF_TUNING_REG, 21, 2, sun6i_zero_crossover_time);

static const struct soc_enum sun6i_fir_length_enum =
	SOC_ENUM_SINGLE(SUN6I_DAC_FIFO_CTRL_REG, 28, 2, sun6i_fir_length);

static const struct soc_enum sun6i_left_hp_mux_enum =
	SOC_ENUM_SINGLE(SUN6I_DAC_ANALOG_CTRL_REG, 8, 2, sun6i_left_hp_mux);

static const struct soc_enum sun6i_right_hp_mux_enum =
	SOC_ENUM_SINGLE(SUN6I_DAC_ANALOG_CTRL_REG, 9, 2, sun6i_right_hp_mux);

static const struct snd_kcontrol_new sun6i_snd_controls[] = {
	/* This is actually an attenuation by 64 steps of -1.16dB */
	SOC_SINGLE("DAC Playback Volume",
		   SUN6I_DAC_DIGITAL_CTRL_REG, 12, 0x1f, 1),
	SOC_SINGLE("DAC High Pass Filter Switch",
		   SUN6I_DAC_DIGITAL_CTRL_REG, 18, 1, 0),

	SOC_SINGLE("Headphone Volume",
		   SUN6I_DAC_ANALOG_CTRL_REG, 0, 0x1f, 0),

	SOC_SINGLE("Zero-crossover Switch",
		   SUN6I_ANALOG_PERF_TUNING_REG, 22, 1, 0),

	SOC_ENUM("Zero-crossover Time", sun6i_zero_crossover_time_enum),
	SOC_ENUM("FIR Length", sun6i_fir_length_enum),

};

static const struct snd_kcontrol_new sun6i_left_output_mixer_controls[] = {
	SOC_DAPM_SINGLE("Right DAC Switch", SUN6I_DAC_ANALOG_CTRL_REG, 10, 1, 0),
	SOC_DAPM_SINGLE("Left DAC Switch", SUN6I_DAC_ANALOG_CTRL_REG, 11, 1, 0),
	SOC_DAPM_SINGLE("Left LineIn Switch", SUN6I_DAC_ANALOG_CTRL_REG, 12, 1, 0),
	SOC_DAPM_SINGLE("Microphone 2 Boost Switch", SUN6I_DAC_ANALOG_CTRL_REG, 15, 1, 0),
	SOC_DAPM_SINGLE("Microphone 1 Boost Switch", SUN6I_DAC_ANALOG_CTRL_REG, 16, 1, 0),
};

static const struct snd_kcontrol_new sun6i_right_output_mixer_controls[] = {
	SOC_DAPM_SINGLE("Left DAC Switch", SUN6I_DAC_ANALOG_CTRL_REG, 17, 1, 0),
	SOC_DAPM_SINGLE("Right DAC Switch", SUN6I_DAC_ANALOG_CTRL_REG, 18, 1, 0),
	SOC_DAPM_SINGLE("Right LineIn Switch", SUN6I_DAC_ANALOG_CTRL_REG, 19, 1, 0),
	SOC_DAPM_SINGLE("Microphone 2 Boost Switch", SUN6I_DAC_ANALOG_CTRL_REG, 22, 1, 0),
	SOC_DAPM_SINGLE("Microphone 1 Boost Switch", SUN6I_DAC_ANALOG_CTRL_REG, 23, 1, 0),
};

static const struct snd_kcontrol_new sun6i_left_hp_mux_controls =
	SOC_DAPM_ENUM("Left Headphone Amplifier Select", sun6i_left_hp_mux_enum);

static const struct snd_kcontrol_new sun6i_right_hp_mux_controls =
	SOC_DAPM_ENUM("Right Headphone Amplifier Select", sun6i_right_hp_mux_enum);

static const struct snd_soc_dapm_widget sun6i_dapm_widgets[] = {
	SND_SOC_DAPM_DAC("DAC", "Playback", SUN6I_DAC_DIGITAL_CTRL_REG, 31, 0),

	SND_SOC_DAPM_DAC("Left DAC", "Playback", SUN6I_DAC_ANALOG_CTRL_REG, 30, 0),
	SND_SOC_DAPM_DAC("Right DAC", "Playback", SUN6I_DAC_ANALOG_CTRL_REG, 31, 0),

	/* Power up of the Headphone amplifier */
	SND_SOC_DAPM_PGA("Headphone Amplifier",
			 SUN6I_POWER_AMPLIFIER_CTRL_REG, 31, 0, NULL, 0),

	SND_SOC_DAPM_PGA("Left Headphone Amplifier",
			 SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("Right Headphone Amplifier",
			 SND_SOC_NOPM, 0, 0, NULL, 0),

	/* Mutes of both channels coming to the headphone amplifier */
	SND_SOC_DAPM_SWITCH("Left Headphone Switch",
			    SUN6I_DAC_ANALOG_CTRL_REG, 6, 1, 0),
	SND_SOC_DAPM_SWITCH("Right Headphone Switch",
			    SUN6I_DAC_ANALOG_CTRL_REG, 7, 1, 0),

	SND_SOC_DAPM_MIXER("Left Output Mixer", SUN6I_DAC_ANALOG_CTRL_REG, 28, 0,
			   sun6i_left_output_mixer_controls,
			   ARRAY_SIZE(sun6i_left_output_mixer_controls)),
	SND_SOC_DAPM_MIXER("Right Output Mixer", SUN6I_DAC_ANALOG_CTRL_REG, 29, 0,
			   sun6i_right_output_mixer_controls,
			   ARRAY_SIZE(sun6i_right_output_mixer_controls)),

	SND_SOC_DAPM_MUX("Left Headphone Amplifier Mux", SND_SOC_NOPM, 0, 0,
			 &sun6i_left_hp_mux_controls),
	SND_SOC_DAPM_MUX("Right Headphone Amplifier Mux", SND_SOC_NOPM, 0, 0,
			 &sun6i_right_hp_mux_controls),

	SND_SOC_DAPM_OUTPUT("HPL"),
	SND_SOC_DAPM_OUTPUT("HPR"),
};

static const struct snd_soc_dapm_route sun6i_dapm_routes[] = {
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
	{ "Left Headphone Amplifier", NULL, "Headphone Amplifier" },
	{ "Left Headphone Amplifier", "Left Headphone Switch", "Left Headphone Amplifier Mux" },

	/* Right HP Amplifier */
	{ "Right Headphone Amplifier", NULL, "Headphone Amplifier" },
	{ "Right Headphone Amplifier", "Right Headphone Switch", "Right Headphone Amplifier Mux" },

	/* Headphone outputs */
	{ "HPL", NULL, "Left Headphone Amplifier" },
	{ "HPR", NULL, "Right Headphone Amplifier" },
};

static int codec_pa_play_open(struct sun6i_priv *sun6i)
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

	/*mute l_pa and r_pa*/
	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, LHPPA_MUTE, 0x0);
	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, RHPPA_MUTE, 0x0);

	/*enable dac digital*/
	codec_wr_control(sun6i, SUN6I_DAC_DPC, 0x1, DAC_EN, 0x1);
	/*set TX FIFO send drq level*/
	codec_wr_control(sun6i, SUN6I_DAC_FIFOC ,0x7f, TX_TRI_LEVEL, 0xf);
	/*set TX FIFO MODE*/
	codec_wr_control(sun6i, SUN6I_DAC_FIFOC ,0x1, TX_FIFO_MODE, 0x1);

	//send last sample when dac fifo under run
	codec_wr_control(sun6i, SUN6I_DAC_FIFOC ,0x1, LAST_SE, 0x0);

	/*enable dac_l and dac_r*/
	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, DACALEN, 0x1);
	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, DACAREN, 0x1);

	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTR_EN, 0x1);
	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTL_EN, 0x1);

	/* TODO: This used to be retrieved by FEX */
	/* if (!pa_double_used) { */
		codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTL_SRC_SEL, 0x1);
		codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTR_SRC_SEL, 0x1);
	/* } else { */
	/* 	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTL_SRC_SEL, 0x0); */
	/* 	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTR_SRC_SEL, 0x0); */
	/* } */

	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, LHPIS, 0x1);
	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, RHPIS, 0x1);

	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x7f, RMIXMUTE, 0x2);
	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x7f, LMIXMUTE, 0x2);

	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, LMIXEN, 0x1);
	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, RMIXEN, 0x1);
	
	/*
	 * TODO: This used to be retrieved by FEX.
	 * The script has nice comments explaining what values mean in term of dB output
	 */
	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1f, LINEOUT_VOL, 0x19);

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

static int codec_capture_open(struct sun6i_priv *sun6i)
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
	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, MIC1AMPEN, 0x1);
	/*mic1 gain 36dB,if capture volume is too small, enlarge the mic1boost*/
	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x7,MIC1BOOST,cap_vol);//36db
	/*enable Master microphone bias*/
	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, MBIASEN, 0x1);

	/*enable Right MIC1 Boost stage*/
	codec_wr_control(sun6i, SUN6I_ADC_ACTL, 0x1, RADCMIXMUTEMIC1BOOST, 0x1);
	/*enable Left MIC1 Boost stage*/
	codec_wr_control(sun6i, SUN6I_ADC_ACTL, 0x1, LADCMIXMUTEMIC1BOOST, 0x1);
	/*enable adc_r adc_l analog*/
	codec_wr_control(sun6i, SUN6I_ADC_ACTL, 0x1,  ADCREN, 0x1);
	codec_wr_control(sun6i, SUN6I_ADC_ACTL, 0x1,  ADCLEN, 0x1);
	/*set RX FIFO mode*/
	codec_wr_control(sun6i, SUN6I_ADC_FIFOC, 0x1, RX_FIFO_MODE, 0x1);
	/*set RX FIFO rec drq level*/
	codec_wr_control(sun6i, SUN6I_ADC_FIFOC, 0x1f, RX_TRI_LEVEL, 0xf);
	/*enable adc digital part*/
	codec_wr_control(sun6i, SUN6I_ADC_FIFOC, 0x1,ADC_EN, 0x1);

	return 0;
}

static int codec_play_start(struct sun6i_priv *sun6i)
{
	/*enable dac drq*/
	codec_wr_control(sun6i, SUN6I_DAC_FIFOC ,0x1, DAC_DRQ, 0x1);
	/*DAC FIFO Flush,Write '1' to flush TX FIFO, self clear to '0'*/
	codec_wr_control(sun6i, SUN6I_DAC_FIFOC ,0x1, DAC_FIFO_FLUSH, 0x1);

	return 0;
}

static int codec_play_stop(struct sun6i_priv *sun6i)
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

	codec_wr_control(sun6i, SUN6I_ADDAC_TUNE, 0x1, ZERO_CROSS_EN, 0x0);
	for (i = 0; i < headphone_vol; i++) {
		/*set HPVOL volume*/
		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x3f, VOLUME, headphone_vol);
		headphone_vol = headphone_vol - i;
		mdelay(1);
		i++;
		if (i > headphone_vol-1) {
			codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x3f, VOLUME, 0x0);
			break;
		}
	}
	/*mute l_pa and r_pa*/
	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, LHPPA_MUTE, 0x0);
	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, RHPPA_MUTE, 0x0);

	/*disable dac drq*/
	codec_wr_control(sun6i, SUN6I_DAC_FIFOC ,0x1, DAC_DRQ, 0x0);

	/*disable dac_l and dac_r*/
	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, DACALEN, 0x0);
	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, DACAREN, 0x0);

	/*disable dac digital*/
	codec_wr_control(sun6i, SUN6I_DAC_DPC ,  0x1, DAC_EN, 0x0);

	/* TODO: Configure the GPIO */
	/* item.gpio.data = 0; */
	/* /\*config gpio info of audio_pa_ctrl open*\/ */
	/* if (0 != sw_gpio_setall_range(&item.gpio, 1)) { */
	/* 	printk("sw_gpio_setall_range failed\n"); */
	/* } */

	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTR_EN, 0x0);
	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTL_EN, 0x0);

	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTL_SRC_SEL, 0x0);
	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTR_SRC_SEL, 0x0);

	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, LHPIS, 0x0);
	codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, RHPIS, 0x0);
	
	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, PHONEOUTS2, 0x0);
	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, PHONEOUTS3, 0x0);
	return 0;
}

static int codec_capture_start(struct sun6i_priv *sun6i)
{
	/*enable adc drq*/
	codec_wr_control(sun6i, SUN6I_ADC_FIFOC ,0x1, ADC_DRQ, 0x1);
	return 0;
}

static int codec_capture_stop(struct sun6i_priv *sun6i)
{
	/*disable adc digital part*/
	codec_wr_control(sun6i, SUN6I_ADC_FIFOC, 0x1,ADC_EN, 0x0);
	/*disable adc drq*/
	codec_wr_control(sun6i, SUN6I_ADC_FIFOC ,0x1, ADC_DRQ, 0x0);	
	/*disable mic1 pa*/
	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, MIC1AMPEN, 0x0);
	/*disable Master microphone bias*/
	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, MBIASEN, 0x0);
	/*disable adc_r adc_l analog*/
	codec_wr_control(sun6i, SUN6I_ADC_ACTL, 0x1, ADCREN, 0x0);
	codec_wr_control(sun6i, SUN6I_ADC_ACTL, 0x1, ADCLEN, 0x0);

	return 0;
}

/* /\* */
/*  *	codec_lineinin_enabled == 1, open the linein in. */
/*  *	codec_lineinin_enabled == 0, close the linein in. */
/*  *\/ */
/* static int codec_set_lineinin(struct snd_kcontrol *kcontrol, */
/* 			      struct snd_ctl_elem_value *ucontrol) */
/* { */
/* 	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol); */
/* 	struct sun6i_priv *sun6i = snd_soc_codec_get_drvdata(codec); */

/* 	sun6i->linein_playback = ucontrol->value.integer.value[0]; */

/* 	if (sun6i->linein_playback) { */
/* 		/\* Enable the DAC mixers *\/ */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, LMIXEN, 0x1); */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, RMIXEN, 0x1); */
/* 		/\* Unmute the line in in the DAC mixers *\/ */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x7f, RMIXMUTE, 0x4); */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x7f, LMIXMUTE, 0x4); */
/* 	} else { */
/* 		/\* Mute the linein in the DAC mixers *\/ */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x7f, RMIXMUTE, 0x0); */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x7f, LMIXMUTE, 0x0); */
/* 		/\* Disable the DAC mixers *\/ */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, LMIXEN, 0x0); */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, RMIXEN, 0x0); */
/* 	} */

/* 	return 0; */
/* } */

/* static int codec_get_lineinin(struct snd_kcontrol *kcontrol, */
/* 			      struct snd_ctl_elem_value *ucontrol) */
/* { */
/* 	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol); */
/* 	struct sun6i_priv *sun6i = snd_soc_codec_get_drvdata(codec); */

/* 	ucontrol->value.integer.value[0] = sun6i->linein_playback; */
/* 	return 0; */
/* } */

/* static int codec_set_lineincap(struct snd_kcontrol *kcontrol, */
/* 			       struct snd_ctl_elem_value *ucontrol) */
/* { */
/* 	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol); */
/* 	struct sun6i_priv *sun6i = snd_soc_codec_get_drvdata(codec); */

/* 	sun6i->linein_capture = ucontrol->value.integer.value[0]; */

/* 	if (sun6i->linein_capture) { */
/* 		/\*enable LINEINR ADC*\/ */
/* 		codec_wr_control(sun6i, SUN6I_ADC_ACTL, 0x1, RADCMIXMUTELINEINR, 0x1); */
/* 		/\*enable LINEINL ADC*\/ */
/* 		codec_wr_control(sun6i, SUN6I_ADC_ACTL, 0x1, LADCMIXMUTELINEINL, 0x1); */
/* 	} else { */
/* 		/\*disable LINEINR ADC*\/ */
/* 		codec_wr_control(sun6i, SUN6I_ADC_ACTL, 0x1, RADCMIXMUTELINEINR, 0x0); */
/* 		/\*disable LINEINL ADC*\/ */
/* 		codec_wr_control(sun6i, SUN6I_ADC_ACTL, 0x1, LADCMIXMUTELINEINL, 0x0); */
/* 	} */
/* 	return 0; */
/* } */

/* static int codec_get_lineincap(struct snd_kcontrol *kcontrol, */
/* 			       struct snd_ctl_elem_value *ucontrol) */
/* { */
/* 	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol); */
/* 	struct sun6i_priv *sun6i = snd_soc_codec_get_drvdata(codec); */

/* 	ucontrol->value.integer.value[0] = sun6i->linein_capture; */
/* 	return 0; */
/* } */

/* /\* */
/*  *	codec_speakerout_enabled == 1, open the speaker. */
/*  *	codec_speakerout_enabled == 0, close the speaker. */
/*  *\/ */
/* static int codec_set_speakerout(struct snd_kcontrol *kcontrol, */
/* 				struct snd_ctl_elem_value *ucontrol) */
/* { */
/* 	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol); */
/* 	struct sun6i_priv *sun6i = snd_soc_codec_get_drvdata(codec); */
/* 	int pa_vol = 0x19; */

/* 	sun6i->speaker_playback = ucontrol->value.integer.value[0]; */

/* 	if (sun6i->speaker_playback) { */
/* 		/\* Enable Lineout *\/ */
/* 		codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTR_EN, 0x1); */
/* 		codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTL_EN, 0x1); */

/* 		/\* */
/* 		 * Set the left lineout source to both mixers (left + */
/* 		 * right) */
/* 		 *\/ */
/* 		codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTL_SRC_SEL, 0x1); */

/* 		/\* */
/* 		 * Set the right lineout source to left lineout (for */
/* 		 * differential output) */
/* 		 *\/ */
/* 		codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTR_SRC_SEL, 0x1); */

/* 		/\* Set default volume *\/ */
/* 		codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1f, LINEOUT_VOL, pa_vol); */

/* 		/\* Set the speaker amplifier source to their associated mixers *\/ */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, LHPIS, 0x1); */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, RHPIS, 0x1); */
/* 	} else { */
/* 		/\* Disable lineout *\/ */
/* 		codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTR_EN, 0x0); */
/* 		codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTL_EN, 0x0); */

/* 		/\* Set the lineout source to the associated mixers *\/ */
/* 		codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTL_SRC_SEL, 0x0); */
/* 		codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTR_SRC_SEL, 0x0); */

/* 		/\* Set the speaker amplifier source to the DACs *\/ */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, LHPIS, 0x0); */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, RHPIS, 0x0); */
/* 	} */

/* 	return 0; */
/* } */

/* static int codec_get_speakerout(struct snd_kcontrol *kcontrol, */
/* 				struct snd_ctl_elem_value *ucontrol) */
/* { */
/* 	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol); */
/* 	struct sun6i_priv *sun6i = snd_soc_codec_get_drvdata(codec); */

/* 	ucontrol->value.integer.value[0] = sun6i->speaker_playback; */
/* 	return 0; */
/* } */

/* /\* */
/*  *	codec_earpieceout_enabled == 1, open the earpiece. */
/*  *	codec_earpieceout_enabled == 0, close the earpiece. */
/*  *\/ */
/* static int codec_set_earpieceout(struct snd_kcontrol *kcontrol, */
/* 				 struct snd_ctl_elem_value *ucontrol) */
/* { */
/* 	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol); */
/* 	struct sun6i_priv *sun6i = snd_soc_codec_get_drvdata(codec); */
/* 	int headphone_vol = 0x3b; */

/* 	sun6i->earpiece_playback = ucontrol->value.integer.value[0]; */

/* 	if (sun6i->earpiece_playback) { */
/* 		sun6i_codec_hp_chan_mute(sun6i, 0, 0); */

/* 		/\* Select the associated analog mixers *\/ */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, LHPIS, 0x1); */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, RHPIS, 0x1); */

/* 		/\*select HPL inverting output *\/ */
/* 		codec_wr_control(sun6i, SUN6I_PA_CTRL, 0x3, HPCOM_CTL, 0x1); */

/* 		/\* 64ms zero crossover *\/ */
/* 		codec_wr_control(sun6i, SUN6I_ADDAC_TUNE, 0x1, ZERO_CROSS_EN, 0x1); */

/* 		/\* Set default HPVOL volume *\/ */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x3f, VOLUME, headphone_vol); */
/* 	} else { */
/* 		sun6i_codec_hp_chan_mute(sun6i, 1, 1); */

/* 		/\* Select the DAC for the headphone power amplifiers *\/ */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, LHPIS, 0x0); */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, RHPIS, 0x0); */

/* 		/\* Disable HPCOM *\/ */
/* 		codec_wr_control(sun6i, SUN6I_PA_CTRL, 0x3, HPCOM_CTL, 0x0); */

/* 		/\* 32ms zero crossover *\/ */
/* 		codec_wr_control(sun6i, SUN6I_ADDAC_TUNE, 0x1, ZERO_CROSS_EN, 0x0); */

/* 		/\* Mute headphones *\/ */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x3f, VOLUME, 0x0); */
/* 	} */

/* 	return 0; */
/* } */

/* static int codec_get_earpieceout(struct snd_kcontrol *kcontrol, */
/* 				 struct snd_ctl_elem_value *ucontrol) */
/* { */
/* 	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol); */
/* 	struct sun6i_priv *sun6i = snd_soc_codec_get_drvdata(codec); */

/* 	ucontrol->value.integer.value[0] = sun6i->earpiece_playback; */
/* 	return 0; */
/* } */

/* /\* */
/*  *	codec_speaker_enabled == 1, speaker is open, headphone is close. */
/*  *	codec_speaker_enabled == 0, speaker is closed, headphone is open. */
/*  *	this function just used for the system voice(such as music and moive voice and so on), */
/*  *	no the phone call. */
/*  *\/ */
/* static int codec_set_spk(struct snd_kcontrol *kcontrol, */
/* 			 struct snd_ctl_elem_value *ucontrol) */
/* { */
/* 	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol); */
/* 	struct sun6i_priv *sun6i = snd_soc_codec_get_drvdata(codec); */

/* 	int ret = 0; */
/* 	int headphone_vol = 0x3b; */
/* 	/\* script_item_u val; *\/ */
/* 	/\* script_item_value_type_e  type; *\/ */
/* 	/\* enum sw_ic_ver  codec_chip_ver; *\/ */

/* 	/\* TODO: Retrieved by FEX, plus some revisions mangling *\/ */
/* 	/\* codec_chip_ver = sw_get_ic_ver(); *\/ */
/* 	/\* type = script_get_item("audio_para", "headphone_direct_used", &val); *\/ */
/* 	/\* if (SCIRPT_ITEM_VALUE_TYPE_INT != type) { *\/ */
/* 	/\* 	printk("[audiocodec] type err!\n"); *\/ */
/* 	/\* } *\/ */
/* 	/\* headphone_direct_used = val.val; *\/ */

/* 	/\* if (headphone_direct_used && (codec_chip_ver != MAGIC_VER_A31A)) { *\/ */
/* 	/\* 	codec_wr_control(sun6i, SUN6I_PA_CTRL, 0x3, HPCOM_CTL, 0x3); *\/ */
/* 	/\* 	codec_wr_control(sun6i, SUN6I_PA_CTRL, 0x1, HPCOM_PRO, 0x1); *\/ */
/* 	/\* } else { *\/ */
/* 		codec_wr_control(sun6i, SUN6I_PA_CTRL, 0x3, HPCOM_CTL, 0x0); */
/* 		codec_wr_control(sun6i, SUN6I_PA_CTRL, 0x1, HPCOM_PRO, 0x0); */
/* 	/\* } *\/ */

/* 	sun6i->speaker_active = ucontrol->value.integer.value[0]; */
/* 	if (sun6i->speaker_active) { */
/* 		ret = codec_pa_play_open(sun6i); */
/* 	} else { */
/* 		/\* item.gpio.data = 0; *\/ */
/* 		codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1f, LINEOUT_VOL, 0x0); */
/* 		codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTL_EN, 0x0); */
/* 		codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTR_EN, 0x0); */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, LHPIS, 0x0); */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, RHPIS, 0x0); */

/* 		/\* TODO: GPIOlib *\/ */
/* 		/\* /\\*config gpio info of audio_pa_ctrl close*\\/ *\/ */
/* 		/\* if (0 != sw_gpio_setall_range(&item.gpio, 1)) { *\/ */
/* 		/\* 	printk("sw_gpio_setall_range failed\n"); *\/ */
/* 		/\* } *\/ */

/* 		/\*unmute l_pa and r_pa*\/ */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, LHPPA_MUTE, 0x1); */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x1, RHPPA_MUTE, 0x1); */
/* 		codec_wr_control(sun6i, SUN6I_ADDAC_TUNE, 0x1, ZERO_CROSS_EN, 0x1); */

/* 		/\* TODO: Retrieved by FEX *\/ */
/* 		/\* type = script_get_item("audio_para", "headphone_vol", &val); *\/ */
/* 		/\* if (SCIRPT_ITEM_VALUE_TYPE_INT != type) { *\/ */
/* 		/\* 	printk("[audiocodec] type err!\n"); *\/ */
/* 		/\* } *\/ */
/* 		/\* headphone_vol = val.val; *\/ */

/* 		/\*set HPVOL volume*\/ */
/* 		codec_wr_control(sun6i, SUN6I_DAC_ACTL, 0x3f, VOLUME, headphone_vol); */
/* 	} */
/* 	return 0; */
/* } */

/* static int codec_get_spk(struct snd_kcontrol *kcontrol, */
/* 			 struct snd_ctl_elem_value *ucontrol) */
/* { */
/* 	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol); */
/* 	struct sun6i_priv *sun6i = snd_soc_codec_get_drvdata(codec); */

/* 	ucontrol->value.integer.value[0] = sun6i->speaker_active; */
/* 	return 0; */
/* } */

/* /\* */
/*  * 	.info = snd_codec_info_volsw, .get = snd_codec_get_volsw,\.put = snd_codec_put_volsw,  */
/*  *\/ */
/* static const struct snd_kcontrol_new codec_snd_controls[] = { */
/* 	/\*SUN6I_DAC_ACTL = 0x20,PAVOL*\/ */
/* 	SOC_SINGLE("Master Playback Volume", SUN6I_DAC_ACTL,0,0x3f,0),			/\*0*\/ */
/* 	/\*total output switch PAMUTE, if set this bit to 0, the voice is mute*\/ */
/* 	SOC_SINGLE("Playback LPAMUTE SWITCH", SUN6I_DAC_ACTL,6,0x1,0),			/\*1*\/ */
/* 	SOC_SINGLE("Playback RPAMUTE SWITCH", SUN6I_DAC_ACTL,7,0x1,0),			/\*2*\/ */
/* 	SOC_SINGLE("Left Headphone PA input src select", SUN6I_DAC_ACTL,8,0x1,0),	/\*3*\/ */
/* 	SOC_SINGLE("Right Headphone PA input src select", SUN6I_DAC_ACTL,9,0x1,0),/\*4*\/ */
/* 	SOC_SINGLE("Left output mixer mute control", SUN6I_DAC_ACTL,10,0x7f,0),	/\*5*\/ */
/* 	SOC_SINGLE("Right output mixer mute control", SUN6I_DAC_ACTL,17,0x7f,0),	/\*6*\/ */
/* 	SOC_SINGLE("Left analog output mixer en", SUN6I_DAC_ACTL,28,0x1,0),		/\*7*\/ */
/* 	SOC_SINGLE("Right analog output mixer en", SUN6I_DAC_ACTL,29,0x1,0),		/\*8*\/ */
/* 	SOC_SINGLE("Inter DAC analog left channel en", SUN6I_DAC_ACTL,30,0x1,0),	/\*9*\/ */
/* 	SOC_SINGLE("Inter DAC analog right channel en", SUN6I_DAC_ACTL,31,0x1,0),	/\*10*\/ */

/* 	/\*SUN6I_PA_CTRL = 0x24*\/ */
/* 	SOC_SINGLE("r_and_l Headphone Power amplifier en", SUN6I_PA_CTRL,29,0x3,0),		/\*11*\/ */
/* 	SOC_SINGLE("HPCOM output protection en", SUN6I_PA_CTRL,28,0x1,0),					/\*12*\/ */
/* 	SOC_SINGLE("L_to_R Headphone apmplifier output mute", SUN6I_PA_CTRL,25,0x1,0),	/\*13*\/ */
/* 	SOC_SINGLE("R_to_L Headphone apmplifier output mute", SUN6I_PA_CTRL,24,0x1,0),	/\*14*\/ */
/* 	SOC_SINGLE("MIC1_G boost stage output mixer control", SUN6I_PA_CTRL,15,0x7,0),	/\*15*\/ */
/* 	SOC_SINGLE("MIC2_G boost stage output mixer control", SUN6I_PA_CTRL,12,0x7,0),	/\*16*\/ */
/* 	SOC_SINGLE("LINEIN_G boost stage output mixer control", SUN6I_PA_CTRL,9,0x7,0),	/\*17*\/ */
/* 	SOC_SINGLE("PHONE_G boost stage output mixer control", SUN6I_PA_CTRL,6,0x7,0),	/\*18*\/ */
/* 	SOC_SINGLE("PHONE_PG boost stage output mixer control", SUN6I_PA_CTRL,3,0x7,0),	/\*19*\/ */
/* 	SOC_SINGLE("PHONE_NG boost stage output mixer control", SUN6I_PA_CTRL,0,0x7,0),	/\*20*\/ */

/* 	/\*SUN6I_MIC_CTRL = 0x28*\/ */
/* 	SOC_SINGLE("Earpiece microphone bias enable", SUN6I_MIC_CTRL,31,0x1,0),			/\*21*\/ */
/* 	SOC_SINGLE("Master microphone bias enable", SUN6I_MIC_CTRL,30,0x1,0),				/\*22*\/ */
/* 	SOC_SINGLE("Earpiece MIC bias_cur_sen and ADC enable", SUN6I_MIC_CTRL,29,0x1,0),	/\*23*\/ */
/* 	SOC_SINGLE("MIC1 boost AMP enable", SUN6I_MIC_CTRL,28,0x1,0),						/\*24*\/ */
/* 	SOC_SINGLE("MIC1 boost AMP gain control", SUN6I_MIC_CTRL,25,0x7,0),				/\*25*\/ */
/* 	SOC_SINGLE("MIC2 boost AMP enable", SUN6I_MIC_CTRL,24,0x1,0),						/\*26*\/ */
/* 	SOC_SINGLE("MIC2 boost AMP gain control", SUN6I_MIC_CTRL,21,0x7,0),				/\*27*\/ */
/* 	SOC_SINGLE("MIC2 source select", SUN6I_MIC_CTRL,20,0x1,0),						/\*28*\/ */
/* 	SOC_SINGLE("Lineout left enable", SUN6I_MIC_CTRL,19,0x1,0),						/\*29*\/ */
/* 	SOC_SINGLE("Lineout right enable", SUN6I_MIC_CTRL,18,0x1,0),						/\*30*\/ */
/* 	SOC_SINGLE("Left lineout source select", SUN6I_MIC_CTRL,17,0x1,0),				/\*31*\/ */
/* 	SOC_SINGLE("Right lineout source select", SUN6I_MIC_CTRL,16,0x1,0),				/\*32*\/ */
/* 	SOC_SINGLE("Lineout volume control", SUN6I_MIC_CTRL,11,0x1f,0),					/\*33*\/ */
/* 	SOC_SINGLE("PHONEP-PHONEN pre-amp gain control", SUN6I_MIC_CTRL,8,0x7,0),			/\*34*\/ */
/* 	SOC_SINGLE("Phoneout gain control", SUN6I_MIC_CTRL,5,0x7,0),						/\*35*\/ */
/* 	SOC_SINGLE("PHONEOUT en", SUN6I_MIC_CTRL,4,0x1,0),								/\*36*\/ */
/* 	SOC_SINGLE("MIC1 boost stage to phone out mute", SUN6I_MIC_CTRL,3,0x1,0),			/\*37*\/ */
/* 	SOC_SINGLE("MIC2 boost stage to phone out mute", SUN6I_MIC_CTRL,2,0x1,0),			/\*38*\/ */
/* 	SOC_SINGLE("Right output mixer to phone out mute", SUN6I_MIC_CTRL,1,0x1,0),		/\*39*\/ */
/* 	SOC_SINGLE("Left output mixer to phone out mute", SUN6I_MIC_CTRL,1,0x1,0),		/\*40*\/ */

/* 	/\*SUN6I_ADC_ACTL = 0x2c*\/ */
/* 	SOC_SINGLE("ADC Right channel en", SUN6I_ADC_ACTL,31,0x1,0),						/\*41*\/ */
/* 	SOC_SINGLE("ADC Left channel en", SUN6I_ADC_ACTL,30,0x1,0),						/\*42*\/ */
/* 	SOC_SINGLE("ADC input gain ctrl", SUN6I_ADC_ACTL,27,0x7,0),						/\*43*\/ */
/* 	SOC_SINGLE("Right ADC mixer mute ctrl", SUN6I_ADC_ACTL,7,0x7f,0),					/\*44*\/ */
/* 	SOC_SINGLE("Left ADC mixer mute ctrl", SUN6I_ADC_ACTL,0,0x7f,0),					/\*45*\/ */
/* 	/\*SUN6I_ADDAC_TUNE = 0x30*\/		 */
/* 	SOC_SINGLE("ADC dither on_off ctrl", SUN6I_ADDAC_TUNE,25,0x7f,0),					/\*46*\/ */
	
/* 	/\*SUN6I_HMIC_CTL = 0x50 */
/* 	 * warning: */
/* 	 * the key and headphone should be check in the switch driver, */
/* 	 * can't be used in this mixer control. */
/* 	 * you should be careful while use the key and headphone check in the mixer control */
/* 	 * it may be confilcted with the key and headphone switch driver. */
/* 	 *\/ */
/* 	SOC_SINGLE("Hmic_M debounce key down_up", SUN6I_HMIC_CTL,28,0xf,0),				/\*47*\/ */
/* 	SOC_SINGLE("Hmic_N debounce earphone plug in_out", SUN6I_HMIC_CTL,24,0xf,0),		/\*48*\/ */
	
/* 	/\*SUN6I_DAC_DAP_CTL = 0x60 */
/* 	 * warning:the DAP should be realize in a DAP driver? */
/* 	 * it may be strange using the mixer control to realize the DAP function. */
/* 	 *\/ */
/* 	SOC_SINGLE("DAP enable", SUN6I_DAC_DAP_CTL,31,0x1,0),								/\*49*\/ */
/* 	SOC_SINGLE("DAP start control", SUN6I_DAC_DAP_CTL,30,0x1,0),						/\*50*\/ */
/* 	SOC_SINGLE("DAP state", SUN6I_DAC_DAP_CTL,29,0x1,0),								/\*51*\/ */
/* 	SOC_SINGLE("BQ enable control", SUN6I_DAC_DAP_CTL,16,0x1,0),						/\*52*\/ */
/* 	SOC_SINGLE("DRC enable control", SUN6I_DAC_DAP_CTL,15,0x1,0),						/\*53*\/ */
/* 	SOC_SINGLE("HPF enable control", SUN6I_DAC_DAP_CTL,14,0x1,0),						/\*54*\/ */
/* 	SOC_SINGLE("DE function control", SUN6I_DAC_DAP_CTL,12,0x3,0),					/\*55*\/ */
/* 	SOC_SINGLE("Ram address", SUN6I_DAC_DAP_CTL,0,0x7f,0),							/\*56*\/ */

/* 	/\*SUN6I_DAC_DAP_VOL = 0x64*\/ */
/* 	SOC_SINGLE("DAP DAC left chan soft mute ctrl", SUN6I_DAC_DAP_VOL,30,0x1,0),		/\*57*\/ */
/* 	SOC_SINGLE("DAP DAC right chan soft mute ctrl", SUN6I_DAC_DAP_VOL,29,0x1,0),		/\*58*\/ */
/* 	SOC_SINGLE("DAP DAC master soft mute ctrl", SUN6I_DAC_DAP_VOL,28,0x1,0),			/\*59*\/ */
/* 	SOC_SINGLE("DAP DAC vol skew time ctrl", SUN6I_DAC_DAP_VOL,24,0x3,0),				/\*60*\/ */
/* 	SOC_SINGLE("DAP DAC master volume", SUN6I_DAC_DAP_VOL,16,0xff,0),					/\*61*\/ */
/* 	SOC_SINGLE("DAP DAC left chan volume", SUN6I_DAC_DAP_VOL,8,0xff,0),				/\*62*\/ */
/* 	SOC_SINGLE("DAP DAC right chan volume", SUN6I_DAC_DAP_VOL,0,0xff,0),				/\*63*\/ */
	
/* 	/\*SUN6I_ADC_DAP_CTL = 0x70*\/ */
/* 	SOC_SINGLE("DAP for ADC en", SUN6I_ADC_DAP_CTL,31,0x1,0),							/\*64*\/ */
/* 	SOC_SINGLE("DAP for ADC start up", SUN6I_ADC_DAP_CTL,30,0x1,0),					/\*65*\/ */
/* 	SOC_SINGLE("DAP left AGC saturation flag", SUN6I_ADC_DAP_CTL,21,0x1,0),			/\*66*\/ */
/* 	SOC_SINGLE("DAP left AGC noise-threshold flag", SUN6I_ADC_DAP_CTL,20,0x1,0),		/\*67*\/ */
/* 	SOC_SINGLE("DAP left gain applied by AGC", SUN6I_ADC_DAP_CTL,12,0xff,0),			/\*68*\/ */
/* 	SOC_SINGLE("DAP right AGC saturation flag", SUN6I_ADC_DAP_CTL,9,0x1,0),			/\*69*\/ */
/* 	SOC_SINGLE("DAP right AGC noise-threshold flag", SUN6I_ADC_DAP_CTL,8,0x1,0),		/\*70*\/ */
/* 	SOC_SINGLE("DAP right gain applied by AGC", SUN6I_ADC_DAP_CTL,0,0xff,0),			/\*71*\/ */

/* 	/\*SUN6I_ADC_DAP_VOL = 0x74*\/ */
/* 	SOC_SINGLE("DAP ADC left chan vol mute", SUN6I_ADC_DAP_VOL,18,0x1,0),				/\*72*\/ */
/* 	SOC_SINGLE("DAP ADC right chan vol mute", SUN6I_ADC_DAP_VOL,17,0x1,0),			/\*73*\/ */
/* 	SOC_SINGLE("DAP ADC volume skew mute", SUN6I_ADC_DAP_VOL,16,0x1,0),				/\*74*\/ */
/* 	SOC_SINGLE("DAP ADC left chan vol set", SUN6I_ADC_DAP_VOL,8,0x3f,0),				/\*75*\/ */
/* 	SOC_SINGLE("DAP ADC right chan vol set", SUN6I_ADC_DAP_VOL,0,0x3f,0),				/\*76*\/ */
	
/* 	/\*SUN6I_ADC_DAP_LCTL = 0x78*\/ */
/* 	SOC_SINGLE("DAP ADC Left chan noise-threshold set", SUN6I_ADC_DAP_VOL,16,0xff,0),	/\*77*\/ */
/* 	SOC_SINGLE("DAP Left AGC en", SUN6I_ADC_DAP_VOL,14,0x1,0),						/\*78*\/ */
/* 	SOC_SINGLE("DAP Left HPF en", SUN6I_ADC_DAP_VOL,13,0x1,0),						/\*79*\/ */
/* 	SOC_SINGLE("DAP Left noise-detect en", SUN6I_ADC_DAP_VOL,12,0x1,0),				/\*80*\/ */
/* 	SOC_SINGLE("DAP Left hysteresis setting", SUN6I_ADC_DAP_VOL,8,0x3,0),				/\*81*\/ */
/* 	SOC_SINGLE("DAP Left noise-debounce time", SUN6I_ADC_DAP_VOL,4,0xf,0),			/\*82*\/ */
/* 	SOC_SINGLE("DAP Left signal-debounce time", SUN6I_ADC_DAP_VOL,0,0xf,0),			/\*83*\/ */

/* 	/\*SUN6I_ADC_DAP_RCTL = 0x7c*\/ */
/* 	SOC_SINGLE("DAP ADC right chan noise-threshold set", SUN6I_ADC_DAP_RCTL,0,0xff,0), 	 	/\*84*\/ */
/* 	SOC_SINGLE("DAP Right AGC en", SUN6I_ADC_DAP_VOL,14,0x1,0),						 	 	/\*85*\/ */
/* 	SOC_SINGLE("DAP Right HPF en", SUN6I_ADC_DAP_VOL,13,0x1,0),						 	 	/\*86*\/ */
/* 	SOC_SINGLE("DAP Right noise-detect en", SUN6I_ADC_DAP_VOL,12,0x1,0),				 	 	/\*87*\/ */
/* 	SOC_SINGLE("DAP Right hysteresis setting", SUN6I_ADC_DAP_VOL,8,0x3,0),			 	 	/\*88*\/ */
/* 	SOC_SINGLE("DAP Right noise-debounce time", SUN6I_ADC_DAP_VOL,4,0xf,0),			 	 	/\*89*\/ */
/* 	SOC_SINGLE("DAP Right signal-debounce time", SUN6I_ADC_DAP_VOL,0,0xf,0),			 	 	/\*90*\/ */

/* 	SOC_SINGLE_BOOL_EXT("Audio Spk Switch", 0, codec_get_spk, codec_set_spk),			     	/\*91*\/ */
/* 	SOC_SINGLE_BOOL_EXT("Audio earpiece out", 0, codec_get_earpieceout, codec_set_earpieceout), 	/\*95*\/ */
/* 	SOC_SINGLE_BOOL_EXT("Audio headphone out", 0, codec_get_headphoneout, codec_set_headphoneout), /\*96*\/ */
/* 	SOC_SINGLE_BOOL_EXT("Audio speaker out", 0, codec_get_speakerout, codec_set_speakerout), 		/\*97*\/ */
	
/* 	SOC_SINGLE_BOOL_EXT("Audio linein record", 0, codec_get_lineincap, codec_set_lineincap), 		/\*100*\/ */
/* 	SOC_SINGLE_BOOL_EXT("Audio linein in", 0, codec_get_lineinin, codec_set_lineinin),    			/\*101*\/ */
/* }; */


static int sun6i_prepare(struct snd_pcm_substream *substream,
			 struct snd_soc_dai *dai)
{
	u32 freq, reg_val;
	u8 rate;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		switch (substream->runtime->rate) {
		case 44100:
			freq = 22579200;
			rate = 0;
			break;

		case 22050:
			freq = 22579200;
			rate = 2;
			break;

		case 11025:
			freq = 22579200;
			rate = 4;
			break;

		case 96000:
			freq = 24576000;
			rate = 7;
			break;

		case 192000:
			freq = 24576000;
			rate = 6;
			break;

		case 32000:
			freq = 24576000;
			rate = 1;
			break;

		case 24000:
			freq = 24576000;
			rate = 2;
			break;

		case 16000:
			freq = 24576000;
			rate = 3;
			break;

		case 12000:
			freq = 24576000;
			rate = 4;
			break;

		case 8000:
			freq = 24576000;
			rate = 5;
			break;

		case 48000:
		default:
			freq = 24576000;
			rate = 0;
			break;
		}

		if (clk_set_rate(sun6i->mod_clk, freq)) {
			dev_err(dai->dev, "Couldn't set mod clock rate to %d\n", freq);
			return -EINVAL;
		}

		reg_val = readl(sun6i->base + SUN6I_DAC_FIFOC);
		reg_val &= ~(7 << 29);
		reg_val |= rate << 29;
		writel(reg_val, sun6i->base + SUN6I_DAC_FIFOC);

		switch (substream->runtime->channels) {
		case 1:
			reg_val = readl(sun6i->base + SUN6I_DAC_FIFOC);
			reg_val |= BIT(6);
			writel(reg_val, sun6i->base + SUN6I_DAC_FIFOC);			
			break;
		case 2:
		default:
			reg_val = readl(sun6i->base + SUN6I_DAC_FIFOC);
			reg_val &= ~BIT(6);
			writel(reg_val, sun6i->base + SUN6I_DAC_FIFOC);
			break;
		}
	} else {
		switch (substream->runtime->rate) {
		case 44100:
			freq = 22579200;
			rate = 0;
			break;

		case 22050:
			freq = 22579200;
			rate = 2;
			break;

		case 11025:
			freq = 22579200;
			rate = 4;
			break;

		case 32000:
			freq = 24576000;
			rate = 1;
			break;

		case 24000:
			freq = 24576000;
			rate = 2;
			break;

		case 16000:
			freq = 24576000;
			rate = 3;
			break;

		case 12000:
			freq = 24576000;
			rate = 4;
			break;

		case 8000:
			freq = 24576000;
			rate = 5;
			break;

		case 48000:
		default:
			freq = 24576000;
			rate = 4;
			break;
		}

		switch (substream->runtime->channels) {
		case 1:
			reg_val = readl(sun6i->base + SUN6I_ADC_FIFOC);
			reg_val |= BIT(7);
			writel(reg_val, sun6i->base + SUN6I_ADC_FIFOC);			
			break;

		case 2:
		default:
			reg_val = readl(sun6i->base + SUN6I_ADC_FIFOC);
			reg_val &= ~BIT(7);
			writel(reg_val, sun6i->base + SUN6I_ADC_FIFOC);
			break;
		}
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (sun6i->speaker_active) {
			/* return codec_pa_play_open(sun6i); */
		} else {
			/*set TX FIFO send drq level*/
			codec_wr_control(sun6i, SUN6I_DAC_FIFOC ,0x7f, TX_TRI_LEVEL, 0xf);

			/*set TX FIFO MODE*/
			codec_wr_control(sun6i, SUN6I_DAC_FIFOC ,0x1, TX_FIFO_MODE, 0x1);

			//send last sample when dac fifo under run
			codec_wr_control(sun6i, SUN6I_DAC_FIFOC ,0x1, LAST_SE, 0x0);
		}
	} else {
		/* return codec_capture_open(sun6i); */
	}

	return 0;
}

// TODO: Should be removed
// Trigger should be only about dma transfers. hw ops are supposed to
// be either in prepare or hw_params (most likely prepare).
static int sun6i_trigger(struct snd_pcm_substream *substream, int cmd,
			 struct snd_soc_dai *dai)
{
	if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
		switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			codec_play_start(sun6i);

			if (!sun6i->speaker_active)
				/* set the default output is HPOUTL/R for pad headphone */
				sun6i_codec_hp_chan_mute(sun6i, 0, 0);

			break;

		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_STOP:
			codec_play_stop(sun6i);
			break;

		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:							
			break;

		default:
			dev_err(dai->dev, "Unsupported trigger operation\n");
			return -EINVAL;
		}
	} else {
		switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			codec_capture_start(sun6i);

			/*hardware fifo delay*/
			mdelay(200);
			codec_wr_control(sun6i, SUN6I_ADC_FIFOC, 0x1, ADC_FIFO_FLUSH, 0x1);
			break;

		case SNDRV_PCM_TRIGGER_SUSPEND:
			codec_capture_stop(sun6i);
			break;

		case SNDRV_PCM_TRIGGER_STOP:		 
			codec_capture_stop(sun6i);
			break;

		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:		
			break;

		default:
			dev_err(dai->dev, "Unsupported trigger operation\n");
			return -EINVAL;
		}
	}
	return 0;
}

static int sun6i_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	return 0;
}

static int sun6i_digital_mute(struct snd_soc_dai *dai, int mute)
{
	sun6i_codec_hp_chan_mute(sun6i, mute, mute);

	return 0;
}

static const struct snd_soc_dai_ops sun6i_codec_dai_ops = {
	.set_fmt		= sun6i_set_fmt,
	.digital_mute		= sun6i_digital_mute,
	.prepare		= sun6i_prepare,
	.trigger		= sun6i_trigger,
};

static struct snd_soc_dai_driver sun6i_codec_dai[] = {
	{
		.name = "sun6i-codec",
		.playback = {
			.stream_name = "Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE, },
		.capture = {
			.stream_name = "Capture",
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE, },
		.ops = &sun6i_codec_dai_ops,
	},
};

static int sun6i_soc_probe(struct snd_soc_codec *codec)
{
	printk("codec->dev %p\n", codec->dev);
	printk("dev data %p\n", dev_get_drvdata(codec->dev));
	printk("sun6i %p\n", sun6i);
	printk("sun6i->regmap %p\n", sun6i->regmap);

	/* HPCOMM is off and output is floating (WTF?!) */
	codec_wr_control(sun6i, SUN6I_PA_CTRL, 0x3, HPCOM_CTL, 0x0);
	/* Disable headphone output */
	codec_wr_control(sun6i, SUN6I_PA_CTRL, 0x1, HPCOM_PRO, 0x0);

	/*
	 * Enable Headset MIC Bias Current sensor & ADC
	 * Due to an hardware bug, it seems to be only possible at init
	 */
	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, HBIASADCEN, 0x1);

	/*
	 * Mute Playback Left and Right channels
	 * Also disables the associated mixer and DAC
	 */
	sun6i_codec_hp_chan_mute(sun6i, true, true);

	/* Disable Playback Lineouts */ 
	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTL_EN, 0x0);
	codec_wr_control(sun6i, SUN6I_MIC_CTRL, 0x1, LINEOUTR_EN, 0x0);

	/*
	 * Fix the init blaze noise
	 * Really have to find more details about that
	 */
	codec_wr_control(sun6i, SUN6I_ADDAC_TUNE, 0x1, PA_SLOPE_SECECT, 0x1);

	/* set HPCOM control as direct driver for floating (Redundant?) */
	codec_wr_control(sun6i, SUN6I_PA_CTRL, 0x3, HPCOM_CTL, 0x0);

	/*
	 * Stop doing DMA requests whenever there's only 16 samples
	 * left available in the TX FIFO.
	 */
	codec_wr_control(sun6i, SUN6I_DAC_FIFOC, 0x3, DRA_LEVEL,0x3);

	/* Flush TX FIFO */
	codec_wr_control(sun6i, SUN6I_DAC_FIFOC, 0x1, DAC_FIFO_FLUSH, 0x1);

	/* Flush RX FIFO */
	codec_wr_control(sun6i, SUN6I_ADC_FIFOC, 0x1, ADC_FIFO_FLUSH, 0x1);

	/* Use a 32 bits FIR */
	codec_wr_control(sun6i, SUN6I_DAC_FIFOC, 0x1, FIR_VERSION, 0x1);

	return 0;
}

static int sun6i_soc_remove(struct snd_soc_codec *codec)
{
	return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_sun6i = {
	.probe			= sun6i_soc_probe,
	.remove			= sun6i_soc_remove,

	.controls		= sun6i_snd_controls,
	.num_controls		= ARRAY_SIZE(sun6i_snd_controls),
	.dapm_widgets		= sun6i_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(sun6i_dapm_widgets),
	.dapm_routes		= sun6i_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(sun6i_dapm_routes),
};

static const struct snd_soc_dapm_widget sun6i_card_dapm_widgets[] = {
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
};

static const struct snd_soc_dapm_route sun6i_card_route[] = {
	{ "Headphone Jack",	NULL,	"HPL" },
	{ "Headphone Jack",	NULL,	"HPR" },
};

static int sun6i_dai_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dapm_context *dapm = &codec->dapm;

	snd_soc_dapm_enable_pin(dapm, "Headphone Jack");

	return 0;
}

static struct snd_soc_dai_link sun6i_card_dai[] = {
	{
		.name		= "sun6i_codec",
		.stream_name	= "sun6i_codec",
		.cpu_dai_name	= "snd-soc-dummy-dai",

		.codec_dai_name = "sun6i-codec",
		.init		= sun6i_dai_init,
	},
};

static struct snd_soc_card sun6i_codec_card = {
	.name	= "sun6i-codec",
	.owner	= THIS_MODULE,

	.dai_link = sun6i_card_dai,
	.num_links = ARRAY_SIZE(sun6i_card_dai),

	.dapm_widgets = sun6i_card_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(sun6i_card_dapm_widgets),
	.dapm_routes = sun6i_card_route,
	.num_dapm_routes = ARRAY_SIZE(sun6i_card_route),
};

static const struct snd_pcm_hardware sun6i_pcm_hardware = {
	.info			= (SNDRV_PCM_INFO_INTERLEAVED |
				   SNDRV_PCM_INFO_BLOCK_TRANSFER |
				   SNDRV_PCM_INFO_MMAP |
				   SNDRV_PCM_INFO_MMAP_VALID |
				   SNDRV_PCM_INFO_PAUSE |
				   SNDRV_PCM_INFO_RESUME),
	.formats		= (SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE),
	.rates			= (SNDRV_PCM_RATE_8000_192000 |
				   SNDRV_PCM_RATE_KNOT),

	.rate_min		= 8000,
	.rate_max		= 192000,
	.channels_min		= 1,
	.channels_max		= 2,
	.buffer_bytes_max	= 128 * 1024,	/* value must be (2^n)Kbyte size */
	.period_bytes_min	= 1024 * 2,
	.period_bytes_max	= 1024 * 32,
	.periods_min		= 2,
	.periods_max		= 8,
	.fifo_size	     	= 32,
};


static int sun6i_configure_dma(struct snd_pcm_substream *substream,
			       struct snd_pcm_hw_params *params,
			       struct dma_slave_config *slave_config)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	int ret;

	ret = snd_hwparams_to_dma_slave_config(substream, params, slave_config);
	if (ret) {
		dev_err(rtd->cpu_dai->dev, "hwparams to dma slave configure failed\n");
		return ret;
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		slave_config->dst_addr = sun6i->base + SUN6I_DAC_FIFO_REG;
		slave_config->dst_maxburst = 1;
	} else {
		slave_config->src_addr = sun6i->base + SUN6I_ADC_FIFO_REG;
		slave_config->src_maxburst = 1;
	}

	return 0;
 } 

static const struct snd_dmaengine_pcm_config sun6i_dmaengine_pcm_config = {
	.pcm_hardware		= &sun6i_pcm_hardware,
	.prealloc_buffer_size	= 32 * 1024,
	.prepare_slave_config	= sun6i_configure_dma,
};

static struct regmap_config sun6i_codec_regmap_config = {
	.reg_bits	= 32,
	.reg_stride	= 4,
	.val_bits	= 32,
	.max_register	= 0x94,
	.fast_io	= true,
};

static int sun6i_codec_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret;

	printk("BWAAAAAAH\n");

	sun6i = devm_kzalloc(&pdev->dev, sizeof(struct sun6i_priv), GFP_KERNEL);
	if (!sun6i)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	sun6i->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(sun6i->base))
		return PTR_ERR(sun6i->base);

	sun6i->apb_clk = devm_clk_get(&pdev->dev, "apb");
	if (IS_ERR(sun6i->apb_clk)) {
		dev_err(&pdev->dev, "Couldn't get the APB clock\n");
		return PTR_ERR(sun6i->apb_clk);
	}
	clk_prepare_enable(sun6i->apb_clk);

	sun6i->mod_clk = devm_clk_get(&pdev->dev, "mod");
	if (IS_ERR(sun6i->mod_clk)) {
		dev_err(&pdev->dev, "Couldn't get the module clock\n");
		return PTR_ERR(sun6i->mod_clk);
	}
	clk_prepare_enable(sun6i->mod_clk);

	sun6i->rstc = devm_reset_control_get(&pdev->dev, NULL);
	if (IS_ERR(sun6i->rstc)) {
		dev_err(&pdev->dev, "Couldn't get the reset controller\n");
		return PTR_ERR(sun6i->rstc);
	}

	sun6i->regmap = devm_regmap_init_mmio(&pdev->dev, sun6i->base,
					      &sun6i_codec_regmap_config);
	if (IS_ERR(sun6i->regmap)) {
		dev_err(&pdev->dev, "Couldn't register MMIO regmap\n");
		return PTR_ERR(sun6i->regmap);
	}

	printk("sun6i %p\n", sun6i);
	printk("sun6i->regmap %p\n", sun6i->regmap);

	ret = devm_snd_dmaengine_pcm_register(&pdev->dev, &sun6i_dmaengine_pcm_config, 0);
	if (ret) {
		dev_err(&pdev->dev, "Couldn't register DMAengine PCM layer\n");
		return ret;
	}

	ret = snd_soc_register_codec(&pdev->dev, &soc_codec_dev_sun6i,
				     sun6i_codec_dai, ARRAY_SIZE(sun6i_codec_dai));
	if (ret) {
		dev_err(&pdev->dev, "Couldn't register the codec\n");
		return ret;
	}

	sun6i_card_dai->codec_of_node = pdev->dev.of_node;
	sun6i_card_dai->platform_of_node = pdev->dev.of_node;

	sun6i_codec_card.dev = &pdev->dev;

	ret = devm_snd_soc_register_card(&pdev->dev, &sun6i_codec_card);
	if (ret) {
		dev_err(&pdev->dev, "Couldn't register the card\n");
		snd_soc_unregister_codec(&pdev->dev);
		return ret;
	}

	return 0;
}

static int sun6i_codec_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);

	return 0;
}

static const struct of_device_id sun6i_codec_match[] = {
	{ .compatible = "allwinner,sun6i-a31-audio-codec", },
	{}
};
MODULE_DEVICE_TABLE(of, sun6i_codec_match);


static struct platform_driver sun6i_codec_driver = {
	.probe		= sun6i_codec_probe,
	.remove		= sun6i_codec_remove,
	.driver		= {
		.name	= "sun6i-codec",
		.of_match_table = sun6i_codec_match,
	},
};

module_platform_driver(sun6i_codec_driver);

MODULE_DESCRIPTION("sun6i CODEC ALSA codec driver");
MODULE_AUTHOR("huangxin");
MODULE_LICENSE("GPL");
