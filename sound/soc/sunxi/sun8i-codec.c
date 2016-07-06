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
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <sound/dmaengine_pcm.h>

#define SUNXI_DAC_DPC		(0x00)		//the same as a10
	#define SUNXI_DAC_DPC_EN_DA		BIT(31)
	#define SUNXI_DAC_DPC_DVOL		BIT(12)

#define SUNXI_DAC_FIFOC		(0x04)		//the same as a10
	#define SUNXI_DAC_FIFOC_DAC_FS		BIT(29)
	#define SUNXI_DAC_FIFOC_DAC_FS_MASK	GENMASK(31,29)
	#define SUNXI_DAC_FIFOC_DAC_FS_RATE(v)	((v) << 29)
	#define SUNXI_DAC_FIFOC_FIR_VERSION	BIT(28)
	#define SUNXI_DAC_FIFOC_SEND_LASAT	BIT(26)
	#define SUNXI_DAC_FIFOC_TX_FIFO_MODE	BIT(24)
	#define SUNXI_DAC_FIFOC_DRQ_CLR_CNT(v)	((v) << 21)
	#define SUNXI_DAC_FIFOC_DRQ_CLR_CNT_MASK	GENMASK(22,21)
	#define SUNXI_DAC_FIFOC_TX_TRIG_LEVEL(v)	((v) << 8)
	#define SUNXI_DAC_FIFOC_TX_TRIG_LEVEL_MASK	GENMASK(14,8)
	#define SUNXI_DAC_FIFOC_MONO_EN		BIT(6)
	#define SUNXI_DAC_FIFOC_TX_SAMPLE_BITS	BIT(5)
	#define SUNXI_DAC_FIFOC_DAC_DRQ_EN	BIT(4)
	#define SUNXI_DAC_FIFOC_FIFO_FLUSH	BIT(0)

#define SUNXI_DAC_FIFOS		(0x08)		//the same as a10

#define SUNXI_DAC_TXDATA	(0x0c)		//the same as a10

#define SUN8I_ADC_FIFOC		(0x10)		//33 ADC FIFO Control Register, different with a10
	#define SUNXI_ADC_FIFOC_DAC_FS		IT(29)
	#define SUNXI_ADC_FIFOC_DAC_FS_MASK	GENMASK(31,29)
	#define SUNXI_ADC_FIFOC_DAC_FS_RATE(v)	((v) << 29)
	#define SUNXI_ADC_FIFOC_EN_AD		BIT(28)
	#define SUN8I_ADC_DIG_MIC_EN		BIT(27)
	#define SUNXI_ADC_FIFOC_RX_FIFO_MODE	BIT(24)
	#define SUN8I_ADC_DFEN			BIT(16)
	#define SUNXI_ADC_FIFOC_RX_TRIG_LEVEL_MASK	GENMASK(12,8)
	#define SUNXI_ADC_FIFOC_RX_TRIG_LEVEL(v)	((v) << 8)
	#define SUNXI_ADC_FIFOC_MONO_EN		BIT(7)
	#define SUNXI_ADC_FIFOC_RX_SAMPLE_BITS	BIT(6)
	#define SUNXI_ADC_FIFOC_ADC_DRQ_EN	BIT(4)
	#define SUNXI_ADC_FIFOC_FIFO_FLUSH	BIT(0)

#define SUN8I_ADC_FIFOS		(0x14)		//33 ADC FIFO Status Register, different with a10
#define SUN8I_ADC_RXDATA	(0x18)		//33 ADC RX Data Register
#define SUN8I_DAC_TXDATA	(0x20)
#define SUN8I_DAC_TXCNT		(0x40)
#define SUN8I_ADC_RXCNT		(0x44)
#define SUN8I_DAC_DEBUG		(0x48)
	#define SUN8I_DAC_DEBUGDAC_SWP		BIT(6)


#define SUN8I_ADC_DEBUG		(0x4c)

#define SUN8I_DAC_DAP_CTL	(0x60)		//DAC DAP control Register
	#define SUN8I_DAC_DAP_CTL_DAC_DAP_EN	BIT(31)
	#define SUN8I_DAC_DAP_CTL_DAC_DRC_EN	BIT(15)
	#define SUN8I_DAC_DAP_CTL_DAC_HPF_EN	BIT(14)

#define SUN8I_ADC_DAP_CTL	(0x70)		//ADC DAP Control Register
	#define SUN8I_ADC_DAP_CTL_ADC_DAP_EN	BIT(31)
	#define SUN8I_ADC_DAP_CTL_ADC_DAP_START		BIT(30)
	#define SUN8I_ADC_DAP_CTL_ENADC_DRC	BIT(26)
	#define SUN8I_ADC_DAP_CTL_ADC_DRC_EN	BIT(25)
	#define SUN8I_ADC_DAP_CTL_ADC_DRC_HPF_EN	BIT(24)
	#define SUN8I_ADC_DAP_CTL_AGC_LEFT_SATURATION_FLAG	BIT(21)
	#define SUN8I_ADC_DAP_CTL_AGC_LEFT_NOISE_THRES_FLAG	BIT(20)
	#define SUN8I_ADC_DAP_CTL_AGC_LEFT_GAIN_APP_MASK	GENMASK(19,12)
	#define SUN8I_ADC_DAP_CTL_AGC_LEFT_GAIN_APP(v)		(v <<(12))
	#define SUN8I_ADC_DAP_CTL_AGC_RIGHT_SATURATION_FLAG	BIT(9)
	#define SUN8I_ADC_DAP_CTL_AGC_RIGHT_NOISE_THRES_FLAG	BIT(8)
	#define SUN8I_ADC_DAP_CTL_AGC_RIGHT_GAIN_APP_MASK	GENMASK(7,0)
	#define SUN8I_ADC_DAP_CTL_AGC_RIGHT_GAIN_APP(v)		(v)

#define SUN8I_ADC_DAP_LCTL	(0x74)		//ADC DAP Left Control Register
	#define SUN8I_ADC_LEFT_CHAN_NOISE_THRES_SET	BIT(16)
	#define SUN8I_ADC_LEFT_AGC_EN			BIT(14)
	#define SUN8I_ADC_LEFT_HPF_EN			BIT(13)
	#define SUN8I_ADC_LEFT_NOISE_DET_EN		BIT(12)
	#define SUN8I_ADC_LEFT_HYS_SET			BIT(8)
	#define SUN8I_ADC_LEFT_NOISE_DEBOURCE_TIME	BIT(4)
	#define SUN8I_ADC_LEFT_SIGNAL_DEBOUNCE_TIME	BIT(0)

#define SUN8I_ADC_DAP_RCTL	(0x78)		//ADC DAP Right Control Register
	#define SUN8I_ADC_RIGHT_CHAN_NOISE_THRES_SET	BIT(16)
	#define SUN8I_ADC_RIGHT_AGC_EN			BIT(14)
	#define SUN8I_ADC_RIGHT_HPF_EN			BIT(13)
	#define SUN8I_ADC_RIGHT_NOISE_DET_EN		BIT(12)
	#define SUN8I_ADC_RIGHT_HYS_SET			BIT(8)
	#define SUN8I_ADC_RIGHT_NOISE_DEBOURCE_TIME	BIT(4)
	#define SUN8I_ADC_RIGHT_SIGNAL_DEBOUNCE_TIME	BIT(0)

#define SUN8I_ADC_DAP_PARA	(0x7c)		//ADC DAP Parameter Control Register
	#define SUN8I_ADC_LEFT_CHAN_TARG_LEVEL_SET	BIT(24)
	#define SUN8I_ADC_RIGHT_CHAN_TARG_LEVEL_SET	BIT(16)
	#define SUN8I_ADC_LEFT_CHAN_MAX_GAIN_SET	BIT(8)
	#define SUN8I_ADC_RIGHT_CHAN_MAX_GAIN_SET	BIT(0)

#define SUN8I_ADC_DAP_LAC	(0x80)		//ADC DAP Left Average Coefficient Register
#define SUN8I_ADC_DAP_LDAT	(0x84)		//ADC DAP Left Decay&Attack Time Register
	#define SUN8I_ADC_LEFT_ATTACK_TIME_COEFF_SET(v)	((v) << 16)
	#define SUN8I_ADC_LEFT_DECAY_TIME_COEFF_SET(v)	((v) << 0)

#define SUN8I_ADC_DAP_RAC	(0x88)		//ADC DAP Right Average Coefficient Register
#define SUN8I_ADC_DAP_RDAC	(0x8c)		//ADC DAP Right Decay&Attack time Register
	#define SUN8I_ADC_RIGHT_ATTACK_TIME_COEFF_SET(v)	((v) << 16)
	#define SUN8I_ADC_RIGHT_DECAY_TIME_COEFF_SET(v)		((v) << 0)
#define SUN8I_ADC_DAP_HPFC	(0x90)		//ADC DAP HPF Coefficient Register
#define SUN8I_ADC_DAP_LINAC	(0x94)
#define SUN8I_ADC_DAP_RINAC	(0x98)
#define SUN8I_ADC_DAP_ORT	(0x9C)
#define SUN8I_DAC_DRC_HHPFC	(0X100)
#define SUN8I_DAC_DRC_LHPFC	(0X104)
#define SUN8I_DAC_DRC_CTRL	(0X108)
#define SUN8I_DAC_DRC_LPFHAT	(0X10C)
#define SUN8I_DAC_DRC_LPFLAT	(0X110)
#define SUN8I_DAC_DRC_RPFHAT	(0X114)
#define SUN8I_DAC_DRC_RPFLAT	(0X118)
#define SUN8I_DAC_DRC_LPFHRT	(0X11C)
#define SUN8I_DAC_DRC_LPFLRT	(0X120)
#define SUN8I_DAC_DRC_RPFHRT	(0X124)
#define SUN8I_DAC_DRC_RPFLRT	(0X128)
#define SUN8I_DAC_DRC_LRMSHAT	(0X12C)
#define SUN8I_DAC_DRC_LRMSLAT	(0X130)
#define SUN8I_DAC_DRC_RRMSHAT	(0X134)
#define SUN8I_DAC_DRC_RRMSLAT	(0X138)
#define SUN8I_DAC_DRC_HCT	(0X13C)
#define SUN8I_DAC_DRC_LCT	(0X140)
#define SUN8I_DAC_DRC_HKC	(0X144)
#define SUN8I_DAC_DRC_LKC	(0X148)
#define SUN8I_DAC_DRC_HOPC	(0X14C)
#define SUN8I_DAC_DRC_LOPC	(0X150)
#define SUN8I_DAC_DRC_HLT	(0X154)
#define SUN8I_DAC_DRC_LLT	(0X158)
#define SUN8I_DAC_DRC_HKI	(0X15C)
#define SUN8I_DAC_DRC_LKI	(0X160)
#define SUN8I_DAC_DRC_HOPL	(0X164)
#define SUN8I_DAC_DRC_LOPL	(0X168)
#define SUN8I_DAC_DRC_HET	(0X16C)
#define	SUN8I_DAC_DRC_LET	(0X170)
#define SUN8I_DAC_DRC_HKE	(0X174)
#define SUN8I_DAC_DRC_LKE	(0X178)
#define SUN8I_DAC_DRC_HOPE	(0X17C)
#define SUN8I_DAC_DRC_LOPE	(0X180)
#define SUN8I_DAC_DRC_HKN	(0X184)
#define SUN8I_DAC_DRC_LKN	(0X188)
#define SUN8I_DAC_DRC_SFHAT	(0X18C)
#define SUN8I_DAC_DRC_SFLAT	(0X190)
#define SUN8I_DAC_DRC_SFHRT	(0X194)
#define	SUN8I_DAC_DRC_SFLRT	(0X198)
#define	SUN8I_DAC_DRC_MXGHS	(0X19C)
#define SUN8I_DAC_DRC_MXGLS	(0X1A0)
#define SUN8I_DAC_DRC_MNGHS	(0X1A4)
#define SUN8I_DAC_DRC_MNGLS	(0X1A8)
#define SUN8I_DAC_DRC_EPSHC	(0X1AC)
#define SUN8I_DAC_DRC_EPSLC	(0X1B0)
#define SUN8I_DAC_DRC_OPT	(0X1B4)
#define SUN8I_DAC_HPF_HG	(0x1B8)
#define SUN8I_DAC_HPF_LG	(0x1BC)

#define SUN8I_ADC_DRC_HHPFC	(0X200)
#define SUN8I_ADC_DRC_LHPFC	(0X204)
#define SUN8I_ADC_DRC_CTRL	(0X208)
#define SUN8I_ADC_DRC_LPFHAT	(0X20C)
#define SUN8I_ADC_DRC_LPFLAT	(0X210)
#define SUN8I_ADC_DRC_RPFHAT	(0X214)
#define SUN8I_ADC_DRC_RPFLAT	(0X218)
#define SUN8I_ADC_DRC_LPFHRT	(0X21C)
#define SUN8I_ADC_DRC_LPFLRT	(0X220)
#define SUN8I_ADC_DRC_RPFHRT	(0X224)
#define SUN8I_ADC_DRC_RPFLRT	(0X228)
#define SUN8I_ADC_DRC_LRMSHAT	(0X22C)
#define SUN8I_ADC_DRC_LRMSLAT	(0X230)
#define SUN8I_ADC_DRC_RRMSHAT	(0X234)
#define SUN8I_ADC_DRC_RRMSLAT	(0X238)
#define SUN8I_ADC_DRC_HCT	(0X23C)
#define SUN8I_ADC_DRC_LCT	(0X240)
#define SUN8I_ADC_DRC_HKC	(0X244)
#define SUN8I_ADC_DRC_LKC	(0X248)
#define SUN8I_ADC_DRC_HOPC	(0X24C)
#define SUN8I_ADC_DRC_LOPC	(0X250)
#define SUN8I_ADC_DRC_HLT	(0X254)
#define SUN8I_ADC_DRC_LLT	(0X258)
#define SUN8I_ADC_DRC_HKI	(0X25C)
#define SUN8I_ADC_DRC_LKI	(0X260)
#define SUN8I_ADC_DRC_HOPL	(0X264)
#define SUN8I_ADC_DRC_LOPL	(0X268)
#define SUN8I_ADC_DRC_HET	(0X26C)
#define SUN8I_ADC_DRC_LET	(0X270)
#define	SUN8I_ADC_DRC_HKE	(0X274)
#define SUN8I_ADC_DRC_LKE	(0X278)
#define SUN8I_ADC_DRC_HOPE	(0X27C)
#define SUN8I_ADC_DRC_LOPE	(0X280)
#define SUN8I_ADC_DRC_HKN	(0X284)
#define SUN8I_ADC_DRC_LKN	(0X288)
#define SUN8I_ADC_DRC_SFHAT	(0X28C)
#define SUN8I_ADC_DRC_SFLAT	(0X290)
#define SUN8I_ADC_DRC_SFHRT	(0X294)
#define SUN8I_ADC_DRC_SFLRT	(0X298)
#define SUN8I_ADC_DRC_MXGHS	(0X29C)
#define SUN8I_ADC_DRC_MXGLS	(0X2A0)
#define SUN8I_ADC_DRC_MNGHS	(0X2A4)
#define SUN8I_ADC_DRC_MNGLS	(0X2A8)
#define SUN8I_ADC_DRC_EPSHC	(0X2AC)
#define SUN8I_ADC_DRC_EPSLC	(0X2B0)
#define SUN8I_ADC_DRC_OPT	(0X2B4)
#define SUN8I_ADC_HPF_HG	(0x2B8)
#define SUN8I_ADC_HPF_LG	(0x2BC)

/* Supported SoC families - used for quirks */
enum sun8i_soc_family {
	SUN8I,	/* H3 SoC */
};

struct sun8i_priv {
	struct device		*dev;
	void __iomem		*base;

	struct clk		*clk_apb;
	struct clk		*clk_module;
	struct reset_control	*rstc;

	struct regmap		*regmap;
	enum sun8i_soc_family revision;

	struct snd_dmaengine_dai_dma_data	playback_dma_data;
	struct snd_dmaengine_dai_dma_data	capture_dma_data;
};

static int sun8i_codec_pa_play_open(struct sun8i_priv *sun8i)
{
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_DPC, SUNXI_DAC_DPC_EN_DA, SUNXI_DAC_DPC_EN_DA);

	return 0;
}

static int sun8i_codec_capture_open(struct sun8i_priv *sun8i)
{
	return 0;
}

static int sun8i_codec_play_start(struct sun8i_priv *sun8i)
{
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_TX_TRIG_LEVEL_MASK, SUNXI_DAC_FIFOC_TX_TRIG_LEVEL(0x40));
	/*enable dac drq*/
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_DAC_DRQ_EN, SUNXI_DAC_FIFOC_DAC_DRQ_EN);
	/*DAC FIFO Flush,Write '1' to flush TX FIFO, self clear to '0'*/
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_FIFO_FLUSH, SUNXI_DAC_FIFOC_FIFO_FLUSH);

	return 0;
}

static int sun8i_codec_play_stop(struct sun8i_priv *sun8i)
{
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_FIFO_FLUSH, SUNXI_DAC_FIFOC_FIFO_FLUSH);

	return 0;
}

static int sun8i_codec_capture_start(struct sun8i_priv *sun8i)
{
	/*enable adc drq*/
	regmap_update_bits(sun8i->regmap, SUN8I_ADC_FIFOC, SUNXI_ADC_FIFOC_ADC_DRQ_EN, SUNXI_ADC_FIFOC_ADC_DRQ_EN);
	return 0;
}

static int sun8i_codec_capture_stop(struct sun8i_priv *sun8i)
{
	/*disable adc digital part*/
	regmap_update_bits(sun8i->regmap, SUN8I_ADC_FIFOC, SUNXI_ADC_FIFOC_EN_AD, 0x0);
	/*disable adc drq*/
	regmap_update_bits(sun8i->regmap, SUN8I_ADC_FIFOC, SUNXI_ADC_FIFOC_ADC_DRQ_EN, 0x0);
	regmap_update_bits(sun8i->regmap, SUN8I_ADC_FIFOC, SUNXI_ADC_FIFOC_FIFO_FLUSH, SUNXI_ADC_FIFOC_FIFO_FLUSH);

	return 0;
}

static int sun8i_codec_prepare(struct snd_pcm_substream *substream,
			 struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct sun8i_priv *sun8i = snd_soc_card_get_drvdata(rtd->card);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		sun8i_codec_pa_play_open(sun8i);
		/*set TX FIFO send drq level*/
		regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_TX_TRIG_LEVEL_MASK, SUNXI_DAC_FIFOC_TX_TRIG_LEVEL(0xf));

		/*set TX FIFO MODE*/
		regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_TX_FIFO_MODE, SUNXI_DAC_FIFOC_TX_FIFO_MODE);
		//send last sample when dac fifo under run
		regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_SEND_LASAT, 0x0);
	} else {
		sun8i_codec_capture_open(sun8i);

		/*set RX FIFO mode*/
		regmap_update_bits(sun8i->regmap, SUN8I_ADC_FIFOC, SUNXI_ADC_FIFOC_RX_FIFO_MODE, SUNXI_ADC_FIFOC_RX_FIFO_MODE);
		/*set RX FIFO rec drq level*/
		regmap_update_bits(sun8i->regmap, SUN8I_ADC_FIFOC, SUNXI_ADC_FIFOC_RX_TRIG_LEVEL_MASK, SUNXI_ADC_FIFOC_RX_TRIG_LEVEL(0xf));
		/*enable adc digital part*/
		regmap_update_bits(sun8i->regmap, SUN8I_ADC_FIFOC, SUNXI_ADC_FIFOC_EN_AD, SUNXI_ADC_FIFOC_EN_AD);
		/*enable adc drq*/
		regmap_update_bits(sun8i->regmap, SUN8I_ADC_FIFOC, SUNXI_ADC_FIFOC_ADC_DRQ_EN, SUNXI_ADC_FIFOC_ADC_DRQ_EN);
		/*hardware fifo delay*/
		msleep(200);
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
			regmap_update_bits(priv->regmap, SUN8I_ADC_FIFOC, SUNXI_ADC_FIFOC_FIFO_FLUSH, SUNXI_ADC_FIFOC_FIFO_FLUSH);
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
		regmap_update_bits(priv->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_DAC_FS_MASK, SUNXI_DAC_FIFOC_DAC_FS_RATE(hwrate));
		regmap_update_bits(priv->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_MONO_EN, is_mono ? SUNXI_DAC_FIFOC_MONO_EN:0);
		regmap_update_bits(priv->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_TX_SAMPLE_BITS, is_24bit ? SUNXI_DAC_FIFOC_TX_SAMPLE_BITS:0);
		regmap_update_bits(priv->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_TX_FIFO_MODE, !is_24bit ? SUNXI_DAC_FIFOC_TX_FIFO_MODE : 0);
		if (is_24bit)
			priv->playback_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		else
			priv->playback_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	} else  {
		regmap_update_bits(priv->regmap, SUN8I_ADC_FIFOC, SUNXI_ADC_FIFOC_DAC_FS_MASK, SUNXI_ADC_FIFOC_DAC_FS_RATE(hwrate));
		regmap_update_bits(priv->regmap, SUN8I_ADC_FIFOC, SUNXI_ADC_FIFOC_MONO_EN, is_mono ? SUNXI_ADC_FIFOC_MONO_EN : 0);
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
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_DRQ_CLR_CNT_MASK, SUNXI_DAC_FIFOC_DRQ_CLR_CNT(0x3));

	/* Flush TX FIFO */
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_FIFO_FLUSH, SUNXI_DAC_FIFOC_FIFO_FLUSH);

	/* Flush RX FIFO */
	regmap_update_bits(sun8i->regmap, SUN8I_ADC_FIFOC, SUNXI_ADC_FIFOC_FIFO_FLUSH, SUNXI_ADC_FIFOC_FIFO_FLUSH);

	/* Do DRQ whenever the FIFO is empty */
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_DAC_DRQ_EN, SUNXI_DAC_FIFOC_DAC_DRQ_EN);
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

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/*disable dac drq*/
		regmap_update_bits(priv->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_DAC_DRQ_EN, 0x0);
		/*disable dac digital*/
		regmap_update_bits(priv->regmap, SUNXI_DAC_DPC, SUNXI_DAC_DPC_EN_DA, 0x0);
		priv->playing = false;
	}

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

	priv->revision = (enum sun8i_soc_family)of_id->data;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base))
		return PTR_ERR(base);

	priv->regmap = devm_regmap_init_mmio(&pdev->dev, base,
					     &sun8i_codec_regmap_config);
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);
	dev_dbg(dev, "COOPS registers mapped");

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
	priv->capture_dma_data.addr = res->start + SUN8I_ADC_RXDATA;
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
