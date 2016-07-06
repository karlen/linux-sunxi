/*
 * sound/soc/sunxi\sun8i-codec-analog.c
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
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>

#define SUN8I_LINEOUT_PA_GAT	(0x00)
	#define PA_CLK_GC		(7)
#define SUN8I_LOMIXSC		(0x01)
	#define LMIXMUTE		(0)
	#define LMIXMUTEDACR		(0)
	#define LMIXMUTEDACL		(1)
	#define LMIXMUTELINEINL		(2)
	#define LMIXMUTEMIC2BOOST	(5)
	#define LMIXMUTEMIC1BOOST	(6)
#define SUN8I_ROMIXSC		(0x02)
	#define RMIXMUTE		(0)
	#define RMIXMUTEDACL		(0)
	#define RMIXMUTEDACR		(1)
	#define RMIXMUTELINEINR		(2)
	#define RMIXMUTEMIC2BOOST	(5)
	#define RMIXMUTEMIC1BOOST	(6)
#define SUN8I_DAC_PA_SRC	(0x03)
	#define DACAREN			(7)
	#define DACALEN			(6)
	#define RMIXEN			(5)
	#define LMIXEN			(4)
#define SUN8I_LINEIN_GCTR	(0x05)
	#define LINEING			(4)
#define SUN8I_MIC_GCTR		(0x06)
	#define MIC1G			(4)
	#define MIC2G			(0)
#define SUN8I_PAEN_CTR		(0x07)
	#define LINEOUTEN		(7)
	#define PA_ANTI_POP_CTRL	(2)
#define SUN8I_LINEOUT_VOLC	(0x09)
	#define LINEOUTVOL		(3)
#define SUN8I_MIC2G_LINEOUT_CTR	(0x0A)
	#define MIC2AMPEN		(7)
	#define MIC2BOOST		(4)
	#define LINEOUTL_EN		(3)
	#define LINEOUTR_EN		(2)
	#define LINEOUTL_SS		(1)
	#define LINEOUTR_SS		(0)
#define SUN8I_MIC1G_MICBIAS_CTR	(0x0B)
	#define MMICBIASEN		(6)
	#define MIC1AMPEN		(3)
	#define MIC1BOOST		(0)
#define SUN8I_LADCMIXSC		(0x0C)
	#define LADCMIXMUTE		(0)
	#define LADCMIXMUTEMIC1BOOST	(6)
	#define LADCMIXMUTEMIC2BOOST	(5)
	#define LADCMIXMUTELINEINL	(2)
	#define LADCMIXMUTELOUTPUT	(1)
	#define LADCMIXMUTEROUTPUT	(0)
#define SUN8I_RADCMIXSC		(0x0D)
	#define RADCMIXMUTE		(0)
	#define RADCMIXMUTEMIC1BOOST	(6)
	#define RADCMIXMUTEMIC2BOOST	(5)
	#define RADCMIXMUTEPHONEPN	(4)
	#define RADCMIXMUTEPHONEP	(3)
	#define RADCMIXMUTELINEINR	(2)
	#define RADCMIXMUTEROUTPUT	(1)
	#define RADCMIXMUTELOUTPUT	(0)
#define SUN8I_ADC_AP_EN		(0x0F)
	#define ADCREN			(7)
	#define ADCLEN			(6)
	#define ADCG			(0)
#define SUN8I_ADDA_APT0		(0x10)
	#define OPDRV_OPCOM_CUR		(6)
	#define OPADC1_BIAS_CUR		(4)
	#define OPADC2_BIAS_CUR		(2)
	#define OPAAF_BIAS_CUR		(0)
#define SUN8I_ADDA_APT1		(0x11)
	#define OPMIC_BIAS_CUR		(6)
	#define OPDAC_BIAS_CUR		(2)
	#define OPMIX_BIAS_CUR		(0)
#define SUN8I_ADDA_APT2		(0x12)
	#define ZERO_CROSS_EN 	  	(7)
	#define TIMEOUT_ZERO_CROSS 	(6)
	#define PTDBS			(4)
	#define PA_SLOPE_SELECT	  	(3)
	#define USB_BIAS_CUR		(0)
#define SUN8I_BIAS_DA16_CTR0	(0x13)
	#define MMIC_BIAS_CHOP_EN	(7)
	#define MMIC_BIAS_CLK_SEL	(5)
	#define DITHER			(4)
	#define DITHER_CLK_SELECT	(2)
	#define BIHE_CTRL		(0)
#define SUN8I_BIAS_DA16_CTR1	(0x14)
	#define PA_SPEED_SEL		(7)
	#define CURRENT_TEST_SEL	(6)
	#define BIAS_DA17_CAL_CLK_SEL	(4)
	#define BIAS_CAL_MODE_SEL	(3)
	#define BIAS_DA16_CAL_CTRL	(2)
	#define BIASCALIVERIFY		(1)
	#define DA16CALIVERIFY		(0)
#define SUN8I_DA16CAL		(0x15)
	#define DA16CALI_DATA		(0)		
#define SUN8I_DA16VERIFY	(0x16)
	#define BIASCALI_DATA		(0)
#define SUN8I_BIASCALI		(0x17)
#define SUN8I_BIASVERIFY	(0x18)
	#define BIASVERIFY_DATA		(0)

struct sun8i_priv {
	struct device		*dev;

	bool			linein_enabled;
	bool			lineout_enabled;
	bool			linein_capture;
	bool			addaloop_enabled;
	bool			addadrc_enabled;
	bool			playing;

	struct regmap		*prcm_regmap;

	int 			lineout_vol;
	int 			cap_vol;
	int			codec_cap_mode;
};

//tidy up this later
static int codec_wr_prcm_control(struct sun8i_priv *sun8i, u32 reg, u32 mask, u32 shift, u32 val);
static unsigned int read_prcm_wvalue(struct sun8i_priv *sun8i, unsigned int addr);

static int sun8i_codec_pa_play_open(struct sun8i_priv *sun8i)
{
	int l_vol = 0;
	/*enable dac digital*/
	codec_wr_prcm_control(sun8i, SUN8I_LINEOUT_PA_GAT, 0x1, PA_CLK_GC, 0x0);
	codec_wr_prcm_control(sun8i, SUN8I_PAEN_CTR, 0x3, PA_ANTI_POP_CTRL, 0x3);

	codec_wr_prcm_control(sun8i, SUN8I_DAC_PA_SRC, 0x1, DACALEN, 0x1);
	codec_wr_prcm_control(sun8i, SUN8I_DAC_PA_SRC, 0x1, DACAREN, 0x1);

	codec_wr_prcm_control(sun8i, SUN8I_DAC_PA_SRC, 0x1, LMIXEN, 0x1);
	codec_wr_prcm_control(sun8i, SUN8I_DAC_PA_SRC, 0x1, RMIXEN, 0x1);
	msleep(10);

	codec_wr_prcm_control(sun8i, SUN8I_ROMIXSC, 0x1, RMIXMUTEDACR, 0x1);
	codec_wr_prcm_control(sun8i, SUN8I_LOMIXSC, 0x1, LMIXMUTEDACL, 0x1);
	/*while adjust volume from app interface, so read the hardware vol first*/
	l_vol = read_prcm_wvalue(sun8i, SUN8I_LINEOUT_VOLC);
	l_vol = l_vol>>3;
	codec_wr_prcm_control(sun8i, SUN8I_LINEOUT_VOLC, 0x1f, LINEOUTVOL, l_vol);

	return 0;
}

static int sun8i_codec_capture_open(struct sun8i_priv *sun8i)
{
	/*disable Right linein Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_RADCMIXSC, 0x1, RADCMIXMUTELINEINR, 0x0);
	/*disable Left linein Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_LADCMIXSC, 0x1, LADCMIXMUTELINEINL, 0x0);

	/*enable mic1 pa*/
	codec_wr_prcm_control(sun8i, SUN8I_MIC1G_MICBIAS_CTR, 0x1, MIC1AMPEN, 0x1);
	/*mic1 gain 36dB,if capture volume is too small, enlarge the mic1boost*/
	codec_wr_prcm_control(sun8i, SUN8I_MIC1G_MICBIAS_CTR, 0x7, MIC1BOOST, sun8i->cap_vol);
	/*enable Master microphone bias*/
	codec_wr_prcm_control(sun8i, SUN8I_MIC1G_MICBIAS_CTR, 0x1, MMICBIASEN, 0x1);

	if (sun8i->addaloop_enabled) {
		/*enable Left output Boost stage*/
		codec_wr_prcm_control(sun8i, SUN8I_LADCMIXSC, 0x1, LADCMIXMUTELOUTPUT, 0x1);
		/*enable Right output Boost stage*/
		codec_wr_prcm_control(sun8i, SUN8I_RADCMIXSC, 0x1, RADCMIXMUTEROUTPUT, 0x1);
	} else {
		/*enable Left MIC1 Boost stage*/
		codec_wr_prcm_control(sun8i, SUN8I_LADCMIXSC, 0x1, LADCMIXMUTEMIC1BOOST, 0x1);
		/*enable Right MIC1 Boost stage*/
		codec_wr_prcm_control(sun8i, SUN8I_RADCMIXSC, 0x1, RADCMIXMUTEMIC1BOOST, 0x1);
	}
#if 0 //Moved to calling function
	/*enable adc_r adc_l analog*/
	codec_wr_prcm_control(sun8i, ADC_AP_EN, 0x1,  ADCREN, 0x1);
	codec_wr_prcm_control(sun8i, ADC_AP_EN, 0x1,  ADCLEN, 0x1);

	/*set RX FIFO mode*/
	codec_wr_control(sun8i, SUN8I_ADC_FIFOC, 0x1, SUNXI_ADC_FIFOC_RX_FIFO_MODE, 0x1);
	/*set RX FIFO rec drq level*/
	codec_wr_control(sun8i, SUN8I_ADC_FIFOC, 0x1f, SUNXI_ADC_FIFOC_RX_TRIG_LEVEL, 0xf);
	/*enable adc digital part*/
	codec_wr_control(sun8i, SUN8I_ADC_FIFOC, 0x1, SUNXI_ADC_FIFOC_EN_AD, 0x1);
	/*enable adc drq*/
	codec_wr_control(sun8i, SUN8I_ADC_FIFOC ,0x1, SUNXI_ADC_FIFOC_ADC_DRQ_EN, 0x1);
	/*hardware fifo delay*/
	msleep(200);
#endif
	return 0;
}

static int sun8i_codec_play_start(struct sun8i_priv *sun8i)
{
}

static int sun8i_codec_play_stop(struct sun8i_priv *sun8i)
{
	codec_wr_prcm_control(sun8i, SUN8I_ROMIXSC, 0x1, RMIXMUTEDACR, 0x0);
	codec_wr_prcm_control(sun8i, SUN8I_LOMIXSC, 0x1, LMIXMUTEDACL, 0x0);

	return 0;
}

static int sun8i_codec_capture_start(struct sun8i_priv *sun8i)
{
	return 0;
}

static int sun8i_codec_capture_stop(struct sun8i_priv *sun8i)
{
	/*disable mic1 pa*/
	codec_wr_prcm_control(sun8i, SUN8I_MIC1G_MICBIAS_CTR, 0x1, MIC1AMPEN, 0x0);
	/*disable Master microphone bias*/
	codec_wr_prcm_control(sun8i, SUN8I_MIC1G_MICBIAS_CTR, 0x1, MMICBIASEN, 0x0);
	codec_wr_prcm_control(sun8i, SUN8I_MIC2G_LINEOUT_CTR, 0x1, MIC2AMPEN, 0x0);
	
	/*disable Right linein Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_RADCMIXSC, 0x1, RADCMIXMUTELINEINR, 0x0);
	/*disable Left linein Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_LADCMIXSC, 0x1, LADCMIXMUTELINEINL, 0x0);

	/*disable Left MIC1 Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_LADCMIXSC, 0x1, LADCMIXMUTEMIC1BOOST, 0x0);
	/*disable Right MIC1 Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_RADCMIXSC, 0x1, RADCMIXMUTEMIC1BOOST, 0x0);

	/*disable Left output Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_LADCMIXSC, 0x1, LADCMIXMUTELOUTPUT, 0x0);
	/*disable Right output Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_RADCMIXSC, 0x1, RADCMIXMUTEROUTPUT, 0x0);

	/*disable Left MIC2 Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_LADCMIXSC, 0x1, LADCMIXMUTEMIC2BOOST, 0x0);
	/*disable Right MIC2 Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_RADCMIXSC, 0x1, RADCMIXMUTEMIC2BOOST, 0x0);

	/*disable adc_r adc_l analog*/
	codec_wr_prcm_control(sun8i, SUN8I_ADC_AP_EN, 0x1,  ADCREN, 0x0);
	codec_wr_prcm_control(sun8i, SUN8I_ADC_AP_EN, 0x1,  ADCLEN, 0x0);

	return 0;
}

/*
*	use for the line_in record
*/
int sun8i_codec_linein_capture_open(struct sun8i_priv *sun8i)
{
	/*disable mic1 pa*/
	codec_wr_prcm_control(sun8i, SUN8I_MIC1G_MICBIAS_CTR, 0x1, MIC1AMPEN, 0x0);
	/*disable Master microphone bias*/
	codec_wr_prcm_control(sun8i, SUN8I_MIC1G_MICBIAS_CTR, 0x1, MMICBIASEN, 0x0);

	codec_wr_prcm_control(sun8i, SUN8I_MIC2G_LINEOUT_CTR, 0x1, MIC2AMPEN, 0x0);
	/*disable Right MIC2 Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_RADCMIXSC, 0x1, RADCMIXMUTEMIC2BOOST, 0x0);
	/*disable Left MIC1 Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_LADCMIXSC, 0x1, LADCMIXMUTEMIC1BOOST, 0x0);

	/*enable Right linein Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_RADCMIXSC, 0x1, RADCMIXMUTELINEINR, 0x1);
	/*enable Left linein Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_LADCMIXSC, 0x1, LADCMIXMUTELINEINL, 0x1);

#if 0 //Moved to calling function
	/*enable adc_r adc_l analog*/
	codec_wr_prcm_control(sun8i, ADC_AP_EN, 0x1,  ADCREN, 0x1);
	codec_wr_prcm_control(sun8i, ADC_AP_EN, 0x1,  ADCLEN, 0x1);

	/*set RX FIFO mode*/
	codec_wr_control(sun8i, SUN8I_ADC_FIFOC, 0x1, SUNXI_ADC_FIFOC_RX_FIFO_MODE, 0x1);
	/*set RX FIFO rec drq level*/
	codec_wr_control(sun8i, SUN8I_ADC_FIFOC, 0x1f, SUNXI_ADC_FIFOC_RX_TRIG_LEVEL, 0xf);
	/*enable adc digital part*/
	codec_wr_control(sun8i, SUN8I_ADC_FIFOC, 0x1, SUNXI_ADC_FIFOC_EN_AD, 0x1);
	/*enable adc drq*/
	codec_wr_control(sun8i, SUN8I_ADC_FIFOC ,0x1, SUNXI_ADC_FIFOC_ADC_DRQ_EN, 0x1);
	/*hardware fifo delay*/
	msleep(200);
#endif
	return 0;
}

/*
*	use for the phone noise reduced while in phone model.
*	use the mic1 and mic2 to reduced the noise from record
*/
static int sun8i_codec_mic1_2_capture_open(struct sun8i_priv *sun8i)
{
	/*disable Right linein Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_RADCMIXSC, 0x1, RADCMIXMUTELINEINR, 0x0);
	/*disable Left linein Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_LADCMIXSC, 0x1, LADCMIXMUTELINEINL, 0x0);
	/*disable Right MIC1 Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_RADCMIXSC, 0x1, RADCMIXMUTEMIC1BOOST, 0x0);

	/*enable mic1 pa*/
	codec_wr_prcm_control(sun8i, SUN8I_MIC1G_MICBIAS_CTR, 0x1, MIC1AMPEN, 0x1);
	/*enable Master microphone bias*/
	codec_wr_prcm_control(sun8i, SUN8I_MIC1G_MICBIAS_CTR, 0x1, MMICBIASEN, 0x1);

	codec_wr_prcm_control(sun8i, SUN8I_MIC2G_LINEOUT_CTR, 0x1, MIC2AMPEN, 0x1);

	/*enable Right MIC2 Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_RADCMIXSC, 0x1, RADCMIXMUTEMIC2BOOST, 0x1);
	/*enable Left MIC1 Boost stage*/
	codec_wr_prcm_control(sun8i, SUN8I_LADCMIXSC, 0x1, LADCMIXMUTEMIC1BOOST, 0x1);
	
#if 0 //Moved to calling function
	/*enable adc_r adc_l analog*/
	codec_wr_prcm_control(sun8i, ADC_AP_EN, 0x1,  ADCREN, 0x1);
	codec_wr_prcm_control(sun8i, ADC_AP_EN, 0x1,  ADCLEN, 0x1);

	/*set RX FIFO mode*/
	codec_wr_control(sun8i, SUN8I_ADC_FIFOC, 0x1, SUNXI_ADC_FIFOC_RX_FIFO_MODE, 0x1);
	/*set RX FIFO rec drq level*/
	codec_wr_control(sun8i, SUN8I_ADC_FIFOC, 0x1f, SUNXI_ADC_FIFOC_RX_TRIG_LEVEL, 0xf);
	/*enable adc digital part*/
	codec_wr_control(sun8i, SUN8I_ADC_FIFOC, 0x1, SUNXI_ADC_FIFOC_EN_AD, 0x1);
	/*enable adc drq*/
	codec_wr_control(sun8i, SUN8I_ADC_FIFOC ,0x1, SUNXI_ADC_FIFOC_ADC_DRQ_EN, 0x1);
	/*hardware fifo delay*/
	msleep(200);
#endif
	return 0;
}

static int sun8i_codec_prepare(struct snd_pcm_substream *substream,
			 struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct sun8i_priv *sun8i = snd_soc_card_get_drvdata(rtd->card);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		sun8i_codec_pa_play_open(sun8i);
	} else {
		if (sun8i->codec_cap_mode == 1) {
			sun8i_codec_mic1_2_capture_open(sun8i);
		} else if (sun8i->codec_cap_mode == 2) {
			sun8i_codec_linein_capture_open(sun8i);
		} else {
			sun8i_codec_capture_open(sun8i);
		}
		/*enable adc_r adc_l analog*/
		codec_wr_prcm_control(sun8i, SUN8I_ADC_AP_EN, 0x1,  ADCREN, 0x1);
		codec_wr_prcm_control(sun8i, SUN8I_ADC_AP_EN, 0x1,  ADCLEN, 0x1);
	}

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
	/* COOPS HACK */
	sun8i->lineout_vol = 0x1f;	

	/* /\* Use a 32 bits FIR *\/ */
	/* codec_wr_control(sun8i, SUNXI_DAC_FIFOC, 0x1, FIR_VERSION, 0x1); */
	codec_wr_prcm_control(sun8i, SUN8I_LINEOUT_VOLC, 0x20, LINEOUTVOL, sun8i->lineout_vol);

	codec_wr_prcm_control(sun8i, SUN8I_PAEN_CTR, 0x1, LINEOUTEN, 0x1);
	codec_wr_prcm_control(sun8i, SUN8I_MIC2G_LINEOUT_CTR, 0x1, LINEOUTL_EN, 0x1);
	codec_wr_prcm_control(sun8i, SUN8I_MIC2G_LINEOUT_CTR, 0x1, LINEOUTR_EN, 0x1);
	msleep(1250);
/*
	gpio_set_value(item.gpio.gpio, 1);
	if (adcagc_used) {
		adcagc_config();
	}
	if (adcdrc_used) {
		adcdrc_config();
	}
	if (adchpf_used) {
		adchpf_config();
	}
	if (dacdrc_used) {
		dacdrc_config();
	}
	if (dachpf_used) {
		dachpf_config();
	}
*/
}

static int sun8i_codec_startup(struct snd_pcm_substream *substream,
				   struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct sun8i_priv *priv = snd_soc_card_get_drvdata(rtd->card);

	sun8i_codec_init(priv);

	return 0;
}

static void sun8i_codec_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct sun8i_priv *priv = snd_soc_card_get_drvdata(rtd->card);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		codec_wr_prcm_control(priv, SUN8I_ADDA_APT2, 0x1, ZERO_CROSS_EN, 0x0);
		if (priv->lineout_enabled) {
			codec_wr_prcm_control(priv, SUN8I_DAC_PA_SRC, 0x1, LMIXEN, 0x0);
			codec_wr_prcm_control(priv, SUN8I_DAC_PA_SRC, 0x1, RMIXEN, 0x0);
		}
		codec_wr_prcm_control(priv, SUN8I_DAC_PA_SRC, 0x1, DACALEN, 0x0);
		codec_wr_prcm_control(priv, SUN8I_DAC_PA_SRC, 0x1, DACAREN, 0x0);
		codec_wr_prcm_control(priv, SUN8I_LINEOUT_PA_GAT, 0x1, PA_CLK_GC, 0x0);
		priv->playing = false;
/*		if (dacdrc_used) {
			dacdrc_enable(0);
		}
		if (dachpf_used) {
			dachpf_enable(0);
		}
	} else {
	    if (adcdrc_used) {
			adcdrc_enable(0);
		}
		if (adcagc_used) {
			adcagc_enable(0);
		}
		if (adchpf_used) {
			adchpf_enable(0);
		}*/
	}
}

/*** Codec DAI ***/

static const struct snd_soc_dai_ops sun8i_codec_dai_ops = {
	.startup	= sun8i_codec_startup,
	.shutdown	= sun8i_codec_shutdown,
	.prepare	= sun8i_codec_prepare,
	.trigger	= sun8i_codec_trigger,
};

#define SUN8I_RATES	SNDRV_PCM_RATE_8000_192000
#define SUN8I_FORMATS	(SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | \
			SNDRV_PCM_FMTBIT_S24_LE)
static struct snd_soc_dai_driver sun8i_codec_dai = {
	.name = "Codec",
	.playback = {
		.stream_name = "Codec Playback",
		.channels_min	= 1,
		.channels_max	= 2,
		.rate_min	= 8000,
		.rate_max	= 192000,
		.rates		= SUN8I_RATES | SNDRV_PCM_RATE_KNOT,
		.formats	= SUN8I_FORMATS,
	},

	.capture = {
		.stream_name = "Codec Capture",
		.channels_min	= 1,
		.channels_max	= 2,
		.rate_min	= 8000,
		.rate_max	= 192000,
		.rates		= SUN8I_RATES | SNDRV_PCM_RATE_KNOT,
		.formats	= SUN8I_FORMATS,
	},
	.ops = &sun8i_codec_dai_ops,
};

/*** Codec ***/
static	bool adcdrc_used       = false;
static	bool dacdrc_used       = false;
static	bool adchpf_used       = false;

static unsigned int read_prcm_wvalue(struct sun8i_priv *sun8i, unsigned int addr)
{
	unsigned int reg;
	(void)regmap_read(sun8i->prcm_regmap, 0, &reg);
	reg |= (0x1<<28);
	(void)regmap_write(sun8i->prcm_regmap, 0, reg);

	(void)regmap_read(sun8i->prcm_regmap, 0, &reg);
	reg &= ~(0x1<<24);
	(void)regmap_write(sun8i->prcm_regmap, 0, reg);

	(void)regmap_read(sun8i->prcm_regmap, 0, &reg);
	reg &= ~(0x1f<<16);
	reg |= (addr<<16);
	(void)regmap_write(sun8i->prcm_regmap, 0, reg);

	(void)regmap_read(sun8i->prcm_regmap, 0, &reg);
	reg &= (0xff<<0);
	dev_dbg (sun8i->dev, "COOPS - Analog block read 0x%x from addr %x", reg, addr);
	return reg;
}

static void write_prcm_wvalue(struct sun8i_priv *sun8i, unsigned int addr, unsigned int val)
{
	unsigned int reg;
	(void)regmap_read(sun8i->prcm_regmap, 0, &reg);
	reg |= (0x1<<28);
	(void)regmap_write(sun8i->prcm_regmap, 0, reg);

	(void)regmap_read(sun8i->prcm_regmap, 0, &reg);
	reg &= ~(0x1f<<16);
	reg |= (addr<<16);
	(void)regmap_write(sun8i->prcm_regmap, 0, reg);

	(void)regmap_read(sun8i->prcm_regmap, 0, &reg);
	reg &= ~(0xff<<8);
	reg |= (val<<8);
	(void)regmap_write(sun8i->prcm_regmap, 0, reg);

	(void)regmap_read(sun8i->prcm_regmap, 0, &reg);
	reg |= (0x1<<24);
	(void)regmap_write(sun8i->prcm_regmap, 0, reg);

	(void)regmap_read(sun8i->prcm_regmap, 0, &reg);
	reg &= ~(0x1<<24);
	(void)regmap_write(sun8i->prcm_regmap, 0, reg);
	dev_dbg (sun8i->dev, "COOPS - Analog block wrote 0x%x to addr %x", val, addr);
}

static int codec_wrreg_prcm_bits(struct sun8i_priv *sun8i, unsigned short reg, unsigned int mask, unsigned int value)
{
	unsigned int old, new;
		
	old	=	read_prcm_wvalue(sun8i, reg);
	new	=	(old & ~mask) | value;
	write_prcm_wvalue(sun8i, reg, new);

	return 0;
}

static int codec_wr_prcm_control(struct sun8i_priv *sun8i, u32 reg, u32 mask, u32 shift, u32 val)
{
	u32 reg_val;
	reg_val = val << shift;
	mask = mask << shift;
	codec_wrreg_prcm_bits(sun8i, reg, mask, reg_val);
	return 0;
}

/*
*	linein_enabled == 1, open the linein in.
*	linein_enabled == 0, close the linein in.
*/
static int codec_set_lineinin(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_platform *platform = snd_soc_kcontrol_platform(kcontrol);
	struct sun8i_priv *sun8i = snd_soc_platform_get_drvdata(platform);
	sun8i->linein_enabled = ucontrol->value.integer.value[0];

	/*close LINEINL*/
	codec_wr_prcm_control(sun8i, SUN8I_LOMIXSC, 0x1, LMIXMUTELINEINL, sun8i->linein_enabled);
	/*close LINEINR*/
	codec_wr_prcm_control(sun8i, SUN8I_ROMIXSC, 0x1, RMIXMUTELINEINR, sun8i->linein_enabled);
	
	return 0;
}

static int codec_get_lineinin(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_platform *platform = snd_soc_kcontrol_platform(kcontrol);
	struct sun8i_priv *sun8i = snd_soc_platform_get_drvdata(platform);

	ucontrol->value.integer.value[0] = sun8i->linein_enabled;
	return 0;
}

static int codec_get_lineout_vol(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	struct snd_soc_platform *platform = snd_soc_kcontrol_platform(kcontrol);
	struct sun8i_priv *sun8i = snd_soc_platform_get_drvdata(platform);

	if (sun8i->lineout_enabled) {
		ret = read_prcm_wvalue(sun8i, SUN8I_LINEOUT_VOLC);
		ret = ret>>3;
	}
	return ret;
}

static int codec_put_lineout_vol(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int lvol = 0;
	struct snd_soc_platform *platform = snd_soc_kcontrol_platform(kcontrol);
	struct sun8i_priv *sun8i = snd_soc_platform_get_drvdata(platform);

	lvol = ucontrol->value.integer.value[0];
	if (sun8i->lineout_enabled) {
		codec_wr_prcm_control(sun8i, SUN8I_LINEOUT_VOLC, 0x1f, LINEOUTVOL, lvol);
	}
	return 0;
}

/*
*	lineout_enabled == 1, open the speaker.
*	lineout_enabled == 0, close the speaker.
*/
static int codec_set_lineout(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_platform *platform = snd_soc_kcontrol_platform(kcontrol);
	struct sun8i_priv *sun8i = snd_soc_platform_get_drvdata(platform);

	sun8i->lineout_enabled = ucontrol->value.integer.value[0];
	if (sun8i->lineout_enabled) {
		codec_wr_prcm_control(sun8i, SUN8I_LOMIXSC, 0x1, LMIXMUTELINEINL, 0x1);
		codec_wr_prcm_control(sun8i, SUN8I_ROMIXSC, 0x1, RMIXMUTELINEINR, 0x1);
		codec_wr_prcm_control(sun8i, SUN8I_PAEN_CTR, 0x1, LINEOUTEN, 0x1);
		codec_wr_prcm_control(sun8i, SUN8I_MIC2G_LINEOUT_CTR, 0x1, LINEOUTL_EN, 0x1);
		codec_wr_prcm_control(sun8i, SUN8I_MIC2G_LINEOUT_CTR, 0x1, LINEOUTR_EN, 0x1);

		codec_wr_prcm_control(sun8i, SUN8I_DAC_PA_SRC, 0x1, LMIXEN, 0x1);
		codec_wr_prcm_control(sun8i, SUN8I_DAC_PA_SRC, 0x1, RMIXEN, 0x1);
		usleep_range(2000, 3000);
		//gpio_set_value(item.gpio.gpio, 1);
		msleep(62);
	} else {
		codec_wr_prcm_control(sun8i, SUN8I_LOMIXSC, 0x1, LMIXMUTELINEINL, 0x0);
		codec_wr_prcm_control(sun8i, SUN8I_ROMIXSC, 0x1, RMIXMUTELINEINR, 0x0);
		if (!sun8i->playing) {
			codec_wr_prcm_control(sun8i, SUN8I_PAEN_CTR, 0x1, LINEOUTEN, 0x0);
			codec_wr_prcm_control(sun8i, SUN8I_MIC2G_LINEOUT_CTR, 0x1, LINEOUTL_EN, 0x0);
			codec_wr_prcm_control(sun8i, SUN8I_MIC2G_LINEOUT_CTR, 0x1, LINEOUTR_EN, 0x0);

			codec_wr_prcm_control(sun8i, SUN8I_DAC_PA_SRC, 0x1, LMIXEN, 0x0);
			codec_wr_prcm_control(sun8i, SUN8I_DAC_PA_SRC, 0x1, RMIXEN, 0x0);
		//	gpio_set_value(item.gpio.gpio, 0);
		}
	}

	return 0;
}

static int codec_get_lineout(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_platform *platform = snd_soc_kcontrol_platform(kcontrol);
	struct sun8i_priv *sun8i = snd_soc_platform_get_drvdata(platform);

	ucontrol->value.integer.value[0] = sun8i->lineout_enabled;
	return 0;
}

static int codec_set_addadrc(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_platform *platform = snd_soc_kcontrol_platform(kcontrol);
	struct sun8i_priv *sun8i = snd_soc_platform_get_drvdata(platform);

	sun8i->addadrc_enabled = ucontrol->value.integer.value[0];

	if (sun8i->addadrc_enabled) {
		adcdrc_used       		= 1;
		dacdrc_used       		= 1;
		adchpf_used       		= 1;
		//sun8i_codec_init();
	} else {
		adcdrc_used       		= 0;
		dacdrc_used       		= 0;
		adchpf_used       		= 0;
	}

	return 0;
}

static int codec_get_addadrc(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_platform *platform = snd_soc_kcontrol_platform(kcontrol);
	struct sun8i_priv *sun8i = snd_soc_platform_get_drvdata(platform);

	ucontrol->value.integer.value[0] = sun8i->addadrc_enabled;
	return 0;
}
static int codec_set_addaloop(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_platform *platform = snd_soc_kcontrol_platform(kcontrol);
	struct sun8i_priv *sun8i = snd_soc_platform_get_drvdata(platform);

	sun8i->addaloop_enabled = ucontrol->value.integer.value[0];

	return 0;
}

static int codec_get_addaloop(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_platform *platform = snd_soc_kcontrol_platform(kcontrol);
	struct sun8i_priv *sun8i = snd_soc_platform_get_drvdata(platform);

	ucontrol->value.integer.value[0] = sun8i->addaloop_enabled;
	return 0;
}
static int codec_set_audio_capture_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	//codec_cap_mode = ucontrol->value.integer.value[0];

	return 0;
}

static int codec_get_audio_capture_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	//ucontrol->value.integer.value[0] = codec_cap_mode;
	return 0;
}

static const char *audio_capture_function[] = {"main mic", "mic1_2", "linein"};
static const struct soc_enum audio_capture_enum[] = {
        SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(audio_capture_function), audio_capture_function),
};
static DECLARE_TLV_DB_SCALE(sun8i_codec_pa_volume_scale, -4800, 100, 1);
#if 0
/* Codec */
static const struct snd_kcontrol_new sun8i_codec_pa_mute =
	SOC_DAPM_SINGLE("Switch", SUNXI_CODEC_DAC_ACTL,
			SUN4I_CODEC_DAC_ACTL_PA_MUTE, 1, 0);


static const struct snd_soc_dapm_widget sun8i_codec_dapm_widgets[] = {
	/* Digital parts of the DACs */
	SND_SOC_DAPM_SUPPLY("DAC", SUNXI_DAC_DPC,
			    SUNXI_DAC_DPC_EN_DA, 0,
			    NULL, 0),

	/* Analog parts of the DACs */
	SND_SOC_DAPM_DAC("Left DAC", "Codec Playback", SUNXI_DAC_ACTL,
			 SUN4I_CODEC_DAC_ACTL_DACAENL, 0),
	SND_SOC_DAPM_DAC("Right DAC", "Codec Playback", SUN4I_CODEC_DAC_ACTL,
			 SUN4I_CODEC_DAC_ACTL_DACAENR, 0),

	/* Mixers */
	SND_SOC_DAPM_MIXER("Left Mixer", SND_SOC_NOPM, 0, 0,
			   sun4i_codec_left_mixer_controls,
			   ARRAY_SIZE(sun4i_codec_left_mixer_controls)),
	SND_SOC_DAPM_MIXER("Right Mixer", SND_SOC_NOPM, 0, 0,
			   sun4i_codec_right_mixer_controls,
			   ARRAY_SIZE(sun4i_codec_right_mixer_controls)),

	/* Global Mixer Enable */
	SND_SOC_DAPM_SUPPLY("Mixer Enable", SUN4I_CODEC_DAC_ACTL,
			    SUN4I_CODEC_DAC_ACTL_MIXEN, 0, NULL, 0),

	/* Power Amplifier */
	SND_SOC_DAPM_MIXER("Power Amplifier", SUN4I_CODEC_ADC_ACTL,
			   SUN4I_CODEC_ADC_ACTL_PA_EN, 0,
			   sun4i_codec_pa_mixer_controls,
			   ARRAY_SIZE(sun4i_codec_pa_mixer_controls)),
	SND_SOC_DAPM_SWITCH("Power Amplifier Mute", SND_SOC_NOPM, 0, 0,
			    &sun8i_codec_pa_mute),

	SND_SOC_DAPM_OUTPUT("Line Out Right"),
	SND_SOC_DAPM_OUTPUT("Line Out Left"),
};

static const struct snd_soc_dapm_route sun8i_codec_dapm_routes[] = {
	/* Left DAC Routes */
	{ "Left DAC", NULL, "DAC" },

	/* Right DAC Routes */
	{ "Right DAC", NULL, "DAC" },

	/* Right Mixer Routes */
	{ "Right Mixer", NULL, "Mixer Enable" },
	{ "Right Mixer", "Left DAC Playback Switch", "Left DAC" },
	{ "Right Mixer", "Right DAC Playback Switch", "Right DAC" },

	/* Left Mixer Routes */
	{ "Left Mixer", NULL, "Mixer Enable" },
	{ "Left Mixer", "Left DAC Playback Switch", "Left DAC" },

	/* Power Amplifier Routes */
	{ "Power Amplifier", "Mixer Playback Switch", "Left Mixer" },
	{ "Power Amplifier", "Mixer Playback Switch", "Right Mixer" },
	{ "Power Amplifier", "DAC Playback Switch", "Left DAC" },
	{ "Power Amplifier", "DAC Playback Switch", "Right DAC" },

	/* Line Output Routes */
	{ "Power Amplifier Mute", "Switch", "Power Amplifier" },
	{ "Line Out Right", NULL, "Power Amplifier Mute" },
	{ "Line Out Left", NULL, "Power Amplifier Mute" },
};
#endif
static const struct snd_kcontrol_new sun8i_codec_controls[] = {
	SOC_SINGLE_EXT_TLV("Lineout volume control", SUN8I_LINEOUT_VOLC, LINEOUTVOL, 0x1f, 0,
			codec_get_lineout_vol, codec_put_lineout_vol, sun8i_codec_pa_volume_scale),
	SOC_SINGLE_BOOL_EXT("Audio lineout", 	0, codec_get_lineout, 	codec_set_lineout),
};

static struct snd_soc_codec_driver sun8i_codec = {
	.controls		= sun8i_codec_controls,
	.num_controls		= ARRAY_SIZE(sun8i_codec_controls),
/*	.dapm_widgets		= sun8i_codec_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(sun8i_codec_dapm_widgets),
	.dapm_routes		= sun8i_codec_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(sun8i_codec_dapm_routes),*/
};

/*** Board routing ***/
/* TODO: do this with DT */
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
	},
};

static struct snd_soc_card snd_soc_sun8i_codec = {
	.name	= "sun8i-codec",
	.owner	= THIS_MODULE,

	.dai_link = &sun8i_card_dai,
	.num_links = ARRAY_SIZE(sun8i_card_dai),
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
		.rates		= SUN8I_RATES,
		.formats	= SUN8I_FORMATS,
	},

	.capture = {
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= SUN8I_RATES,
		.formats	= SUN8I_FORMATS,
	},
};

static struct regmap_config sun8i_codec_regmap_config = {
	.reg_bits	= 32,
	.reg_stride	= 4,
	.val_bits	= 32,
	.max_register	= SUN8I_ADC_HPF_LG,
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
	priv->dev=&pdev->dev;
	card->dev = &pdev->dev;
	platform_set_drvdata(pdev, card);
	snd_soc_card_set_drvdata(card, priv);

	priv->prcm_regmap = syscon_regmap_lookup_by_compatible(
					"allwinner,sun8i-h3-ac-pr-cfg");
	if (IS_ERR(priv->prcm_regmap))
		return PTR_ERR(priv->prcm_regmap);
	dev_dbg(dev, "COOPS Analog Part mapped");

	ret = snd_soc_register_codec(&pdev->dev, &sun8i_codec, &sun8i_codec_dai, 1);

	ret = devm_snd_soc_register_component(&pdev->dev, &sun8i_codec_component, &dummy_cpu_dai, 1);
	sun8i_codec_init(priv);

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
	return ret;
}

static int sun8i_codec_remove(struct platform_device *pdev)
{
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
