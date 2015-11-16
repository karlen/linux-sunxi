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

	bool			linein_enabled;
	bool			lineout_enabled;
	bool			linein_capture;
	bool			addaloop_enabled;
	bool			addadrc_enabled;
	bool			playing;

	struct regmap		*regmap;
	struct regmap		*prcm_regmap;
	void __iomem		*analog_part;
	enum sun8i_soc_family revision;

	struct snd_dmaengine_dai_dma_data	playback_dma_data;
	struct snd_dmaengine_dai_dma_data	capture_dma_data;

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
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_DPC, SUNXI_DAC_DPC_EN_DA, SUNXI_DAC_DPC_EN_DA);
	codec_wr_prcm_control(sun8i, SUN8I_LINEOUT_PA_GAT, 0x1, PA_CLK_GC, 0x0);
	/*set TX FIFO send drq level*/
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_TX_TRIG_LEVEL_MASK, SUNXI_DAC_FIFOC_TX_TRIG_LEVEL(0xf));
	/*set TX FIFO MODE*/
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_TX_FIFO_MODE, SUNXI_DAC_FIFOC_TX_FIFO_MODE);

	//send last sample when dac fifo under run
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_SEND_LASAT, 0x0);
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
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_TX_TRIG_LEVEL_MASK, SUNXI_DAC_FIFOC_TX_TRIG_LEVEL(0x40));
	/*enable dac drq*/
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_DAC_DRQ_EN, SUNXI_DAC_FIFOC_DAC_DRQ_EN);
	/*DAC FIFO Flush,Write '1' to flush TX FIFO, self clear to '0'*/
	regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_FIFO_FLUSH, SUNXI_DAC_FIFOC_FIFO_FLUSH);

	return 0;
}

static int sun8i_codec_play_stop(struct sun8i_priv *sun8i)
{
	codec_wr_prcm_control(sun8i, SUN8I_ROMIXSC, 0x1, RMIXMUTEDACR, 0x0);
	codec_wr_prcm_control(sun8i, SUN8I_LOMIXSC, 0x1, LMIXMUTEDACL, 0x0);

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
		/*set TX FIFO send drq level*/
		regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_TX_TRIG_LEVEL_MASK, SUNXI_DAC_FIFOC_TX_TRIG_LEVEL(0xf));

		/*set TX FIFO MODE*/
		regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_TX_FIFO_MODE, SUNXI_DAC_FIFOC_TX_FIFO_MODE);
		//send last sample when dac fifo under run
		regmap_update_bits(sun8i->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_SEND_LASAT, 0x0);
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
	/* COOPS HACK */
	sun8i->lineout_vol = 0x1f;	
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

	clk_prepare_enable(priv->clk_module);

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
		/*disable dac drq*/
		regmap_update_bits(priv->regmap, SUNXI_DAC_FIFOC, SUNXI_DAC_FIFOC_DAC_DRQ_EN, 0x0);
		codec_wr_prcm_control(priv, SUN8I_DAC_PA_SRC, 0x1, DACALEN, 0x0);
		codec_wr_prcm_control(priv, SUN8I_DAC_PA_SRC, 0x1, DACAREN, 0x0);
		/*disable dac digital*/
		regmap_update_bits(priv->regmap, SUNXI_DAC_DPC, SUNXI_DAC_DPC_EN_DA, 0x0);
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

	priv->prcm_regmap = syscon_regmap_lookup_by_compatible(
					"allwinner,sun8i-h3-prcm");
	if (IS_ERR(priv->prcm_regmap))
		return PTR_ERR(priv->prcm_regmap);
	dev_dbg(dev, "COOPS Analog Part mapped");

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
