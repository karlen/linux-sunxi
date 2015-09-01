/*
 * ALSA SoC I2S Audio Layer
 *
 * (c) 2015 Andrea Venturi <be17068@iperbole.bo.it>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#define DEBUG
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/regmap.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <sound/dmaengine_pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#define SUNXI_I2SCTL		(0x00)
	#define SUNXI_I2SCTL_SDO3EN		BIT(11)
	#define SUNXI_I2SCTL_SDO2EN		BIT(10)
	#define SUNXI_I2SCTL_SDO1EN		BIT(9)
	#define SUNXI_I2SCTL_SDO0EN		BIT(8)
	#define SUNXI_I2SCTL_ASS		BIT(6)
	#define SUNXI_I2SCTL_MS			BIT(5)
	#define SUNXI_I2SCTL_PCM		BIT(4)
	#define SUNXI_I2SCTL_LOOP		BIT(3)
	#define SUNXI_I2SCTL_TXEN		BIT(2)
	#define SUNXI_I2SCTL_RXEN		BIT(1)
	#define SUNXI_I2SCTL_GEN		BIT(0)

#define SUNXI_I2SFAT0		(0x04)
	#define SUNXI_I2SFAT0_LRCP		BIT(7)
	#define SUNXI_I2SFAT0_BCP		BIT(6)
	#define SUNXI_I2SFAT0_SR_RVD		(3 << 4)
	#define SUNXI_I2SFAT0_SR_16BIT		(0 << 4)
	#define	SUNXI_I2SFAT0_SR_20BIT		(1 << 4)
	#define SUNXI_I2SFAT0_SR_24BIT		(2 << 4)
	#define SUNXI_I2SFAT0_WSS_16BCLK	(0 << 2)
	#define SUNXI_I2SFAT0_WSS_20BCLK	(1 << 2)
	#define SUNXI_I2SFAT0_WSS_24BCLK	(2 << 2)
	#define SUNXI_I2SFAT0_WSS_32BCLK	(3 << 2)
	#define SUNXI_I2SFAT0_FMT_I2S		(0 << 0)
	#define SUNXI_I2SFAT0_FMT_LFT		(1 << 0)
	#define SUNXI_I2SFAT0_FMT_RGT		(2 << 0)
	#define SUNXI_I2SFAT0_FMT_RVD		(3 << 0)

#define SUNXI_I2SFAT1		(0x08)
	#define SUNXI_I2SFAT1_SYNCLEN_16BCLK	(0 << 12)
	#define SUNXI_I2SFAT1_SYNCLEN_32BCLK	(1 << 12)
	#define SUNXI_I2SFAT1_SYNCLEN_64BCLK	(2 << 12)
	#define SUNXI_I2SFAT1_SYNCLEN_128BCLK	(3 << 12)
	#define SUNXI_I2SFAT1_SYNCLEN_256BCLK	(4 << 12)
	#define SUNXI_I2SFAT1_SYNCOUTEN		BIT(11)
	#define SUNXI_I2SFAT1_OUTMUTE		BIT(10)
	#define SUNXI_I2SFAT1_MLS		BIT(9)
	#define SUNXI_I2SFAT1_SEXT		BIT(8)
	#define SUNXI_I2SFAT1_SI_1ST		(0 << 6)
	#define SUNXI_I2SFAT1_SI_2ND		(1 << 6)
	#define SUNXI_I2SFAT1_SI_3RD		(2 << 6)
	#define SUNXI_I2SFAT1_SI_4TH		(3 << 6)
	#define SUNXI_I2SFAT1_SW		BIT(5)
	#define SUNXI_I2SFAT1_SSYNC		BIT(4)
	#define SUNXI_I2SFAT1_RXPDM_16PCM	(0 << 2)
	#define SUNXI_I2SFAT1_RXPDM_8PCM	(1 << 2)
	#define SUNXI_I2SFAT1_RXPDM_8ULAW	(2 << 2)
	#define SUNXI_I2SFAT1_RXPDM_8ALAW	(3 << 2)
	#define SUNXI_I2SFAT1_TXPDM_16PCM	(0 << 0)
	#define SUNXI_I2SFAT1_TXPDM_8PCM	(1 << 0)
	#define SUNXI_I2SFAT1_TXPDM_8ULAW	(2 << 0)
	#define SUNXI_I2SFAT1_TXPDM_8ALAW	(3 << 0)

#define SUNXI_I2STXFIFO		(0x0C)

#define SUNXI_I2SRXFIFO		(0x10)

#define SUNXI_I2SFCTL		(0x14)
	#define SUNXI_I2SFCTL_FIFOSRC		BIT(31)
	#define SUNXI_I2SFCTL_FTX		BIT(25)
	#define SUNXI_I2SFCTL_FRX		BIT(24)
	#define SUNXI_I2SFCTL_TXTL(v)		((v) << 12)
	#define SUNXI_I2SFCTL_RXTL(v)		((v) << 4)
	#define SUNXI_I2SFCTL_TXIM_MOD0		(0 << 2)
	#define SUNXI_I2SFCTL_TXIM_MOD1		(1 << 2)
	#define SUNXI_I2SFCTL_RXOM_MOD0		(0 << 0)
	#define SUNXI_I2SFCTL_RXOM_MOD1		(1 << 0)
	#define SUNXI_I2SFCTL_RXOM_MOD2		(2 << 0)
	#define SUNXI_I2SFCTL_RXOM_MOD3		(3 << 0)

#define SUNXI_I2SFSTA		(0x18)
	#define SUNXI_I2SFSTA_TXE		BIT(28)
	#define SUNXI_I2SFSTA_TXECNT(v)		((v) << 16)
	#define SUNXI_I2SFSTA_RXA		BIT(8)
	#define SUNXI_I2SFSTA_RXACNT(v)		((v) << 0)

#define SUNXI_I2SINT		(0x1C)
	#define SUNXI_I2SINT_TXDRQEN		BIT(7)
	#define SUNXI_I2SINT_TXUIEN		BIT(6)
	#define SUNXI_I2SINT_TXOIEN		BIT(5)
	#define SUNXI_I2SINT_TXEIEN		BIT(4)
	#define SUNXI_I2SINT_RXDRQEN		BIT(2)
	#define SUNXI_I2SINT_RXOIEN		BIT(1)
	#define SUNXI_I2SINT_RXAIEN		BIT(0)

#define SUNXI_I2SISTA		(0x20)
	#define SUNXI_I2SISTA_TXUISTA		BIT(6)
	#define SUNXI_I2SISTA_TXOISTA		BIT(5)
	#define SUNXI_I2SISTA_TXEISTA		BIT(4)
	#define SUNXI_I2SISTA_RXOISTA		BIT(1)
	#define SUNXI_I2SISTA_RXAISTA		BIT(0)

#define SUNXI_I2SCLKD		(0x24)
	#define SUNXI_I2SCLKD_MCLKOEN		BIT(7)
	#define SUNXI_I2SCLKD_BCLKDIV_2		(0 << 4)
	#define SUNXI_I2SCLKD_BCLKDIV_4		(1 << 4)
	#define SUNXI_I2SCLKD_BCLKDIV_6		(2 << 4)
	#define SUNXI_I2SCLKD_BCLKDIV_8		(3 << 4)
	#define SUNXI_I2SCLKD_BCLKDIV_12	(4 << 4)
	#define SUNXI_I2SCLKD_BCLKDIV_16	(5 << 4)
	#define SUNXI_I2SCLKD_BCLKDIV_32	(6 << 4)
	#define SUNXI_I2SCLKD_BCLKDIV_64	(7 << 4)
	#define SUNXI_I2SCLKD_MCLKDIV_1		(0 << 0)
	#define SUNXI_I2SCLKD_MCLKDIV_2		(1 << 0)
	#define SUNXI_I2SCLKD_MCLKDIV_4		(2 << 0)
	#define SUNXI_I2SCLKD_MCLKDIV_6		(3 << 0)
	#define SUNXI_I2SCLKD_MCLKDIV_8		(4 << 0)
	#define SUNXI_I2SCLKD_MCLKDIV_12	(5 << 0)
	#define SUNXI_I2SCLKD_MCLKDIV_16	(6 << 0)
	#define SUNXI_I2SCLKD_MCLKDIV_24	(7 << 0)
	#define SUNXI_I2SCLKD_MCLKDIV_3		(8 << 0)
	#define SUNXI_I2SCLKD_MCLKDIV_48	(9 << 0)
	#define SUNXI_I2SCLKD_MCLKDIV_64	(10 << 0)
	#define SUNXI_I2SCLKD_MCLKDIV		(0)
	#define SUNXI_I2SCLKD_BCLKDIV		(4)

#define SUNXI_I2STXCNT		(0x28)

#define SUNXI_I2SRXCNT		(0x2C)

#define SUNXI_TXCHSEL		(0x30)
	#define SUNXI_TXCHSEL_CHNUM(v)		(((v)-1) << 0)

#define SUNXI_TXCHMAP		(0x34)
	#define SUNXI_TXCHMAP_CH7(v)		(((v)-1) << 28)
	#define SUNXI_TXCHMAP_CH6(v)		(((v)-1) << 24)
	#define SUNXI_TXCHMAP_CH5(v)		(((v)-1) << 20)
	#define SUNXI_TXCHMAP_CH4(v)		(((v)-1) << 16)
	#define SUNXI_TXCHMAP_CH3(v)		(((v)-1) << 12)
	#define SUNXI_TXCHMAP_CH2(v)		(((v)-1) << 8)
	#define SUNXI_TXCHMAP_CH1(v)		(((v)-1) << 4)
	#define SUNXI_TXCHMAP_CH0(v)		(((v)-1) << 0)

#define SUNXI_RXCHSEL		(0x38)
	#define SUNXI_RXCHSEL_CHNUM(v)		(((v)-1) << 0)

#define SUNXI_RXCHMAP		(0x3C)
	#define SUNXI_RXCHMAP_CH3(v)		(((v)-1) << 12)
	#define SUNXI_RXCHMAP_CH2(v)		(((v)-1) << 8)
	#define SUNXI_RXCHMAP_CH1(v)		(((v)-1) << 4)
	#define SUNXI_RXCHMAP_CH0(v)		(((v)-1) << 0)
	#define SUNXI_RXCHMAP_CH(v)		(v<<(v<<2))

/* Clock dividers */
#define SUNXI_DIV_MCLK		0
#define SUNXI_DIV_BCLK		1
#define SUNXI_DIV_EXTCLK	2

/* Clock set cases*/
#define SUNXI_SET_MCLK		0
#define SUNXI_MCLKO_EN		1
#define SUNXI_SAMPLING_FREQ	2

/*
 * SND_SOC_DAIFMT extension from legacy linux-sunxi I2S driver
 * Format enumerations for completing the aSoC defines.
 */
#define SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_MASK		(3 << 16)
#define SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_16BCLK		(0 << 16)
#define SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_20BCLK		(1 << 16)
#define SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_24BCLK		(2 << 16)
#define SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_32BCLK		(3 << 16)

#define SUNXI_I2S_RATES \
	(SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 | SNDRV_PCM_RATE_16000 | \
	 SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | \
	 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000| \
	 SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_192000)

/* Supported SoC families - used for quirks */
enum sunxi_soc_family {
        SUN4IA, /* A10 SoC - revision A */
        SUN4I,  /* A10 SoC - later revisions */
        SUN5I,  /* A10S/A13 SoCs */
        SUN7I,  /* A20 SoC */
};

struct sunxi_i2s_info {
	struct platform_device			*pdev;
        struct regmap				*regmap;
	void __iomem				*reg_base;
        enum sunxi_soc_family			revision;
	struct snd_dmaengine_dai_dma_data	playback_dma_data;
	struct snd_dmaengine_dai_dma_data	capture_dma_data;
	struct clk				*clk_apb;
	struct clk				*clk_pll2;
	struct clk				*clk_module;
	struct clk				*dai_clk;
	int					master;
	resource_size_t				mapbase;
	int					mclk_rate;
	int					ws_size;
	int					lrc_pol;
	int					bclk_pol;
	int					pcm_datamode;
	int					pcm_ch_num;
	int					pcm_txtype;
	int					pcm_rxtype;
	int					pcm_sync_type;
	int					pcm_sw;
	int					pcm_start_slot;
	int					pcm_lsb_first;
	int					pcm_sync_period;
	int					samp_fs;
	int					samp_res;
	int					samp_format;
	int					slave;
};
#if 0
static void sunxi_i2s_clock(struct sunxi_i2s_info *host, u32 core_freq,
		u32 rate)
{
	u32 divider;

	clk_set_rate(host->clk_module, core_freq);
	divider = DIV_ROUND_CLOSEST(clk_get_rate(host->clk_module), (rate * 128));
}

typedef struct __MCLK_SET_INF {
	__u32	samp_rate;	/* sample rate */
	__u16	mult_fs;	/* multiply of sample rate */

	__u8	clk_div;	/* mpll division */
	__u8	mpll;		/* select mpll, 0:24.576 Mhz, 1:22.5792 Mhz */
} __mclk_set_inf;


typedef struct __BCLK_SET_INF {
	__u8	bitpersamp;	/* bits per sample */
	__u8	clk_div;	/* clock division */
	__u16	mult_fs;	/* multiplay of sample rate */
} __bclk_set_inf;


static __bclk_set_inf BCLK_INF[] = {
	/* 16bits per sample */
	{16, 4, 128}, {16, 6, 192}, {16, 8, 256},
	{16, 12, 384}, {16, 16, 512},

	/* 24 bits per sample */
	{24, 4, 192}, {24, 8, 384}, {24, 16, 768},

	/* 32 bits per sample */
	{32, 2, 128}, {32, 4, 256}, {32, 6, 384},
	{32, 8, 512}, {32, 12, 768},

	/* end flag */
	{0xff, 0, 0},
};

/* TX RATIO value */
static __mclk_set_inf MCLK_INF[] = {
	/* 88.2k bitrate //2 */
	{ 88200, 128, 2, 1}, { 88200, 256, 2, 1},

	/* 22.05k bitrate //8 */
	{ 22050, 128, 8, 1}, { 22050, 256, 8, 1},
	{ 22050, 512, 8, 1},

	/* 24k bitrate //8 */
	{ 24000, 128, 8, 0}, { 24000, 256, 8, 0}, { 24000, 512, 8, 0},

	/* 32k bitrate //2.048MHz 24/4 = 6 */
	{ 32000, 128, 6, 0}, { 32000, 192, 6, 0}, { 32000, 384, 6, 0},
	{ 32000, 768, 6, 0},

	/* 48K bitrate 3.072Mbit/s 16/4 = 4 */
	{ 48000, 128, 4, 0}, { 48000, 256, 4, 0}, { 48000, 512, 4, 0},

	/* 96k bitrate 6.144MHZ 8/4 = 2 */
	{ 96000, 128 , 2, 0}, { 96000, 256, 2, 0},

	/* 192k bitrate 12.288MHZ 4/4 = 1 */
	{192000, 128, 1, 0},

	/* 44.1k bitrate 2.8224MHz 16/4 = 4 */
	{ 44100, 128, 4, 1}, { 44100, 256, 4, 1}, { 44100, 512, 4, 1},

	/* 176.4k bitrate 11.2896MHZ 4/4 = 1 */
	{176400, 128, 1, 1},

	/* end flag 0xffffffff */
	{0xffffffff, 0, 0, 0},
};

static s32 get_clock_divider(u32 sample_rate, u32 sample_width, u32 *mclk_div,
					u32 *mpll, u32 *bclk_div, u32 *mult_fs)
{
	u32 i, j, ret = -EINVAL;

	for (i = 0; i < 100; i++) {
		if ((MCLK_INF[i].samp_rate == sample_rate) &&
					((MCLK_INF[i].mult_fs == 256) ||
					(MCLK_INF[i].mult_fs == 128))) {
			for (j = 0; j < ARRAY_SIZE(BCLK_INF); j++) {
				if ((BCLK_INF[j].bitpersamp == sample_width) &&
					(BCLK_INF[j].mult_fs ==
							MCLK_INF[i].mult_fs)) {
					*mclk_div = MCLK_INF[i].clk_div;
					*mpll = MCLK_INF[i].mpll;
					*bclk_div = BCLK_INF[j].clk_div;
					*mult_fs = MCLK_INF[i].mult_fs;
					ret = 0;
					break;
				}
			}
		} else if (MCLK_INF[i].samp_rate == 0xffffffff)
			break;
	}

	return ret;
}

/* freq: 1: 22.5792MHz 0: 24.576MHz SLAVE MODE NOT SUPPORTED YET */
static int sunxi_i2s_set_sysclk(struct snd_soc_dai *cpu_dai, int clk_id,
						unsigned int freq, int dir)
{
	struct snd_soc_card *card = snd_soc_dai_get_drvdata(cpu_dai);
	struct sunxi_i2s_info *i2s = snd_soc_card_get_drvdata(card);

	if (!freq)
		freq = 24576000;
	else
		freq = 22579200;

	if (clk_set_rate(i2s->clk_pll2, freq))
		printk("%s try to set the pll2 clock rate to %d failed!\n", __func__, freq);
	if (clk_set_rate(i2s->clk_module, freq))
		printk("%s try to set the i2s clock rate to %d failed!\n", __func__, freq);
	return 0;
}

/* COOPS CHECK THIS OUT AS IT'S MIXING STYLES AND WHERE COPIED FROM */
static int sunxi_i2s_set_clkdiv(struct snd_soc_dai *cpu_dai, int div_id, int value)
{
	u32 reg_bk, ret;
	u32 mclk = 0;
	u32 mclk_div = 0;
	u32 bclk_div = 0;
	struct snd_soc_card *card = snd_soc_dai_get_drvdata(cpu_dai);
	struct sunxi_i2s_info *i2s = snd_soc_card_get_drvdata(card);

	// Here i should know the sample rate and the FS multiple.

	printk("[I2S]Entered %s\n", __func__);

	switch (div_id) {
	case SUNXI_DIV_MCLK:
		regmap_update_bits(i2s->reg_base, SUNXI_I2SCLKD, 0xf << 0, (value & 0xf) << 0);
		break;
	case SUNXI_DIV_BCLK:
		regmap_update_bits(i2s->reg_base, SUNXI_I2SCLKD, 0x7 << 4, (value & 0x7) << 4);
		break;
/*
	case SUNXI_SAMPLING_FREQ:
		if(!i2s->slave) {
			reg_bk = i2s->samp_fs;
			i2s->samp_fs = (u32)value;
			ret = sunxi_i2s_divisor_values(&mclk_div, &bclk_div, &mclk);
			if (ret != 0) {
				printk("[I2S]Sampling rate frequency not supported.");
				i2s->samp_fs = reg_bk;
				return ret;
			} else {
				i2s->samp_fs = (u32)value;
				sunxi_i2s_set_sysclk(cpu_dai, SUNXI_SET_MCLK, mclk, 0);
				regmap_update_bits(i2s->reg_base, SUNXI_I2SCLKD, (0xf << 0)|(0x7 << 4),
				((mclk_div & 0xf) << 0)|((bclk_div & 0x7) << 4));
			}
		} else
			i2s->samp_fs = (u32)value;
		break;
*/
	default:
		printk("[I2S]div_id %n not supported in %s\n", div_id, __func__);
		return -EINVAL;
	}
	return 0;
}


static void sunxi_i2s_tx_en(void __iomem *base, bool on)
{
	unsigned long val;

	val = readl_relaxed(base + SUNXI_I2SCTL);
	if (on)
		val |= SUNXI_I2SCTL_TXEN | SUNXI_I2SCTL_GEN;
	else
		val &= ~(SUNXI_I2SCTL_TXEN | SUNXI_I2SCTL_GEN);
	writel_relaxed(val, base + SUNXI_I2SCTL);
}

static void sunxi_i2s_rx_en(void __iomem *base, bool on)
{
	unsigned long val;

	val = readl_relaxed(base + SUNXI_I2SCTL);
	if (on)
		val |= SUNXI_I2SCTL_RXEN | SUNXI_I2SCTL_GEN;
	else
		val &= ~(SUNXI_I2SCTL_RXEN | SUNXI_I2SCTL_GEN);
	writel_relaxed(val, base + SUNXI_I2SCTL);
}

static void sunxi_i2s_tx_dma_en(void __iomem *base, bool on)
{
	unsigned long val;

	/* Flush TX FIFO */
	val = readl_relaxed(base + SUNXI_I2SFCTL);
	val |= SUNXI_I2SFCTL_FTX;
	writel_relaxed(val, base + SUNXI_I2SFCTL);

	val = readl_relaxed(base + SUNXI_I2SINT);
	if (on)
		val |= SUNXI_I2SINT_TXDRQEN;
	else
		val &= ~SUNXI_I2SINT_TXDRQEN;
	writel_relaxed(val, base + SUNXI_I2SINT);
}

static void sunxi_i2s_rx_dma_en(void __iomem *base, bool on)
{
	unsigned long val;

	/* Flush RX FIFO */
	val = readl_relaxed(base + SUNXI_I2SFCTL);
	val |= SUNXI_I2SFCTL_FRX;
	writel_relaxed(val, base + SUNXI_I2SFCTL);

	val = readl_relaxed(base + SUNXI_I2SINT);
	if (on)
		val |= SUNXI_I2SINT_RXDRQEN;
	else
		val &= ~SUNXI_I2SINT_RXDRQEN;
	writel_relaxed(val, base + SUNXI_I2SINT);
}

#define SUNXI_I2S_FMTBIT \
	(SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE)

static int sunxi_i2s_dai_probe(struct snd_soc_dai *dai)
{
	struct sunxi_i2s_info *sunxi_i2s = dev_get_drvdata(dai->dev);

	snd_soc_dai_set_drvdata(dai, sunxi_i2s);
	sunxi_i2s->dma_playback.addr = sunxi_i2s->mapbase + SUNXI_I2STXFIFO;
	sunxi_i2s->dma_playback.maxburst = 16;
	sunxi_i2s->dma_capture.addr = sunxi_i2s->mapbase + SUNXI_I2SRXFIFO;
	sunxi_i2s->dma_capture.maxburst = 16;
	snd_soc_dai_init_dma_data(dai, &sunxi_i2s->dma_playback,
						&sunxi_i2s->dma_capture);
	return 0;
}

static int sunxi_i2s_set_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
	struct sunxi_i2s_info *i2s = snd_soc_dai_get_drvdata(cpu_dai);
	unsigned long val, clk_val;

	val = readl_relaxed(i2s->reg_base + SUNXI_I2SFAT0);
	clk_val = readl_relaxed(i2s->reg_base + SUNXI_I2SCTL);

	/* SDO ON (setup for sun4i and sun7i will mod for others */
	clk_val |= (SUNXI_I2SCTL_SDO0EN | SUNXI_I2SCTL_SDO1EN |
			SUNXI_I2SCTL_SDO2EN | SUNXI_I2SCTL_SDO3EN);

	val &= ~SUNXI_I2SFAT0_FMT_RVD;
	clk_val &= ~SUNXI_I2SCTL_PCM;
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		val |= SUNXI_I2SFAT0_FMT_I2S;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		val |= SUNXI_I2SFAT0_FMT_LFT;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		val |= SUNXI_I2SFAT0_FMT_RGT;
		break;
	default:
		dev_err(cpu_dai->dev, "Unknown i2s timing\n");
		return -EINVAL;
	}

	/* master or slave selection */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		i2s->master = 1;
		clk_val |= SUNXI_I2SCTL_MS;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		i2s->master = 0;
		clk_val &= ~SUNXI_I2SCTL_MS;
		break;
	default:
		dev_err(cpu_dai->dev, "Unknown master/slave format\n");
		return -EINVAL;
	}
	writel_relaxed(clk_val, i2s->reg_base + SUNXI_I2SCTL);

	/* DAI signal inversions */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:	/* normal bit clock + frame */
		val &= ~SUNXI_I2SFAT0_LRCP;
		val &= ~SUNXI_I2SFAT0_BCP;
		printk("[I2S-0] %s: normal bit clock + frame\n", __func__);
		break;
	case SND_SOC_DAIFMT_NB_IF:	/* normal bclk + inv frm */
		val |= SUNXI_I2SFAT0_LRCP;
		val &= ~SUNXI_I2SFAT0_BCP;
		printk("[I2S-0] %s: normal bclk + inv frm\n", __func__);
		break;
	case SND_SOC_DAIFMT_IB_NF:	/* invert bclk + nor frm */
		val &= ~SUNXI_I2SFAT0_LRCP;
		val |= SUNXI_I2SFAT0_BCP;
		printk("[I2S-0] %s: invert bclk + nor frm\n", __func__);
		break;
	case SND_SOC_DAIFMT_IB_IF:	/* invert bclk + frm */
		val |= SUNXI_I2SFAT0_LRCP;
		val |= SUNXI_I2SFAT0_BCP;
		printk("[I2S-0] %s: invert bclk + frm\n", __func__);
		break;
	}

	/* word select size DEFAULTS TO 32BITS*/
	val &= ~SUNXI_I2SFAT0_WSS_32BCLK;
	if (i2s->ws_size == 16)
		val |= SUNXI_I2SFAT0_WSS_16BCLK;
	else if (i2s->ws_size == 20)
		val |= SUNXI_I2SFAT0_WSS_20BCLK;
	else if (i2s->ws_size == 24)
		val |= SUNXI_I2SFAT0_WSS_24BCLK;
	else
		val |= SUNXI_I2SFAT0_WSS_32BCLK;
	writel_relaxed(val, i2s->reg_base + SUNXI_I2SFAT0);

	/* PCM REGISTER setup DEFAULTS TO I2S*/
	val = i2s->pcm_txtype & 0x3;
	val |= i2s->pcm_rxtype << 2;

	if (i2s->pcm_sync_type) {
		val |= SUNXI_I2SFAT1_SSYNC;	/* short sync */
		printk("[IIS-0] %s: pcm_sync_type = short sync\n", __func__);
	}
	if (i2s->pcm_sw == 16) {
		val |= SUNXI_I2SFAT1_SW;
		printk("[IIS-0] %s: pcm_sw == 16\n", __func__);
	}

	/* start slot index */
	val |= ((i2s->pcm_start_slot - 1) & 0x3) << 6;

	val |= i2s->pcm_lsb_first << 9; /* MSB or LSB first */

	if (i2s->pcm_sync_period == 256)
		val |= 0x4 << 12;
	else if (i2s->pcm_sync_period == 128)
		val |= 0x3 << 12;
	else if (i2s->pcm_sync_period == 64)
		val |= 0x2 << 12;
	else if (i2s->pcm_sync_period == 32)
		val |= 0x1 << 12;
	writel_relaxed(val, i2s->reg_base + SUNXI_I2SFAT1);

	/* set FIFO control register */
	val = SUNXI_I2SFCTL_RXOM_MOD0;
	val |= SUNXI_I2SFCTL_TXIM_MOD1;
	val |= SUNXI_I2SFCTL_RXTL(0xf);		/* RX FIFO trigger level */
	val |= SUNXI_I2SFCTL_TXTL(0x40);	/* TX FIFO trigger level */
	writel_relaxed(val, i2s->reg_base + SUNXI_I2SFCTL);
	return 0;
}

static int sunxi_i2s_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *socdai)
{
	struct sunxi_i2s_info *i2s = snd_soc_dai_get_drvdata(socdai);
	struct snd_dmaengine_dai_dma_data *dma_data;
	unsigned int lane, ch_num, len, ret = 0;
	unsigned long val, format;
	unsigned long chn_cfg;
	u32 mclk_div = 0, mpll = 0, bclk_div = 0, mult_fs = 0;
	unsigned long rate = params_rate(params);
	get_clock_divider(rate, 32, &mclk_div, &mpll, &bclk_div, &mult_fs);

	printk("COOPS %s Host %s regmap 0x%x\n", __func__, "i2s", (unsigned int)i2s->reg_base);
	printk("COOPS %s Host %s rate %d mclk_div %d blck_div %d mpll %d mult_fs %d\n", __func__, "i2s", rate, mclk_div, bclk_div, mpll, mult_fs);
	dma_data = snd_soc_dai_get_dma_data(socdai, substream);
	dma_data->addr_width = DMA_SLAVE_BUSWIDTH_32_BYTES;//params_width(params) >> 3;

	val = readl_relaxed(i2s->reg_base + SUNXI_I2SFAT0);

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		val |= SUNXI_I2SFAT0_SR_16BIT;
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		val |= SUNXI_I2SFAT0_SR_20BIT;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		val |= SUNXI_I2SFAT0_SR_24BIT;
		break;
	default:
		dev_err(socdai->dev, "Unknown data format\n");
		return -EINVAL;
	}

	writel_relaxed(val, i2s->reg_base + SUNXI_I2SFAT0);

	ret = sunxi_i2s_set_clkdiv(socdai, SUNXI_DIV_MCLK, mclk_div);
	if (ret < 0)
		return ret;
	printk("COOPS %s line %d\n", __func__, __LINE__);
	ret = sunxi_i2s_set_clkdiv(socdai, SUNXI_DIV_BCLK, bclk_div);
	if (ret < 0)
		return ret;
	printk("COOPS %s line %d\n", __func__, __LINE__);
/*	snd_soc_dai_set_dma_drvdata(socdai, substream, dma_data);//MAY NOT BE REQUIRED*/
	return ret;
}

static int sunxi_i2s_trigger(struct snd_pcm_substream *substream, int cmd,
							struct snd_soc_dai *dai)
{
	struct sunxi_i2s_info *sunxi_i2s = dev_get_drvdata(dai->dev);
	int capture = (substream->stream == SNDRV_PCM_STREAM_CAPTURE);
	int ret = 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		if (capture)
			sunxi_i2s_rx_dma_en(sunxi_i2s->reg_base, true);
		else
			sunxi_i2s_tx_dma_en(sunxi_i2s->reg_base, true);
	/* fall thru */
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (capture)
			sunxi_i2s_rx_en(sunxi_i2s->reg_base, true);
		else
			sunxi_i2s_tx_en(sunxi_i2s->reg_base, true);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
		if (capture)
			sunxi_i2s_rx_dma_en(sunxi_i2s->reg_base, false);
		else
			sunxi_i2s_tx_dma_en(sunxi_i2s->reg_base, false);
	/* fall thru */
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (capture)
			sunxi_i2s_rx_en(sunxi_i2s->reg_base, false);
		else
			sunxi_i2s_tx_en(sunxi_i2s->reg_base, false);
		break;

	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int sunxi_i2s_startup(struct snd_pcm_substream *substream,
						struct snd_soc_dai *dai)
{
	struct sunxi_i2s_info *sunxi_i2s = dev_get_drvdata(dai->dev);

	return clk_prepare_enable(sunxi_i2s->dai_clk);
}

static void sunxi_i2s_shutdown(struct snd_pcm_substream *substream,
						struct snd_soc_dai *dai)
{
	struct sunxi_i2s_info *sunxi_i2s = dev_get_drvdata(dai->dev);

	clk_disable_unprepare(sunxi_i2s->dai_clk);
}

static struct snd_soc_dai_ops sunxi_i2s_dai_ops = {
	.trigger	= sunxi_i2s_trigger,
	.hw_params	= sunxi_i2s_hw_params,
	.set_fmt	= sunxi_i2s_set_fmt,
	.startup	= sunxi_i2s_startup,
	.shutdown	= sunxi_i2s_shutdown,
};

static const struct snd_soc_component_driver sunxi_i2s_component = {
	.name			= "sunxi-i2s",
};

struct snd_soc_dai_driver sunxi_i2s_dai = {
	.name	= "sunxi-i2s-dai",
	.id	= 0,
	.probe	= sunxi_i2s_dai_probe,
	.playback	= {
		.channels_min	= 1,
		.channels_max	= 8,
		.rates = SUNXI_I2S_RATES,
		.formats = SUNXI_I2S_FMTBIT,
	},
	.capture	= {
		.channels_min	= 1,
		.channels_max	= 2,
		.rates = SUNXI_I2S_RATES,
		.formats = SUNXI_I2S_FMTBIT,
	},
	.ops	= &sunxi_i2s_dai_ops,
};

static const struct of_device_id sunxi_i2s_dt_ids[] = {
	{ .compatible = "allwinner,sunxi-a10-i2s", },
	{}
};
MODULE_DEVICE_TABLE(of, sunxi_i2s_dt_ids);

static int sunxi_i2s_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct sunxi_i2s_info *sunxi_i2s;
	int ret;
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	const struct of_device_id *of_id;

	if (!of_device_is_available(np))
		return -ENODEV;

	of_id = of_match_device(sunxi_i2s_dt_ids, dev);
	if (!of_id)
		return -EINVAL;

	sunxi_i2s = kzalloc(sizeof(*sunxi_i2s), GFP_KERNEL);
	if (!sunxi_i2s)
		return -ENOMEM;

	dev_dbg(dev, "COOPS %s\n", __FILE__);
	/* Get the clocks from the DT */
	sunxi_i2s->clk_apb = devm_clk_get(dev, "apb");
	if (IS_ERR(sunxi_i2s->clk_apb)) {
		dev_err(dev, "failed to get apb clock\n");
		return PTR_ERR(sunxi_i2s->clk_apb);
	}
	sunxi_i2s->clk_pll2 = devm_clk_get(dev, "audio");
	if (IS_ERR(sunxi_i2s->clk_pll2)) {
		dev_err(dev, "failed to get pll2 clock\n");
		return PTR_ERR(sunxi_i2s->clk_pll2);
	}
	sunxi_i2s->clk_module = devm_clk_get(dev, "i2s");
	if (IS_ERR(sunxi_i2s->clk_module)) {
		dev_err(dev, "failed to get codec clock\n");
		return PTR_ERR(sunxi_i2s->clk_module);
	}
	dev_dbg(dev, "COOPS Got to line %d\n", __LINE__);

	/* Enable PLL2 on a basic rate */
	ret = clk_set_rate(sunxi_i2s->clk_pll2, 24576000);
	if (ret) {
		dev_err(dev, "failed to set codec base clock rate\n");
		return ret;
	}
	if (clk_prepare_enable(sunxi_i2s->clk_pll2)) {
		dev_err(dev, "failed to enable pll2 clock\n");
		return -EINVAL;
	}

	/* Enable the bus clock */
	if (clk_prepare_enable(sunxi_i2s->clk_apb)) {
		dev_err(dev, "failed to enable apb clock\n");
		clk_disable_unprepare(sunxi_i2s->clk_pll2);
		return -EINVAL;
	}
	dev_dbg(dev, "COOPS Got to line %d\n", __LINE__);
/*
	sunxi_i2s->dai_clk = devm_clk_get(&pdev->dev, "tx");
	if (IS_ERR(sunxi_i2s->dai_clk)) {
		dev_err(&pdev->dev, "Fail to get clk\n");
		return PTR_ERR(sunxi_i2s->dai_clk);
	}
*/
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	sunxi_i2s->mapbase = res->start;
	sunxi_i2s->reg_base = devm_ioremap_resource(&pdev->dev, res);
	if (!sunxi_i2s->reg_base) {
		dev_err(&pdev->dev, "ioremap failed!\n");
		return -EIO;
	}
	dev_dbg(dev, "COOPS Got to line %d\n", __LINE__);

	writel_relaxed(0, sunxi_i2s->reg_base + SUNXI_I2SFCTL);
	platform_set_drvdata(pdev, sunxi_i2s);

	ret = snd_soc_register_component(&pdev->dev, &sunxi_i2s_component,
							&sunxi_i2s_dai, 1);
	if (ret) {
		dev_err(&pdev->dev, "Register DAI failed: %d\n", ret);
		return ret;
	}
	dev_dbg(dev, "COOPS Got to line %d\n", __LINE__);

	ret = devm_snd_dmaengine_pcm_register(&pdev->dev, NULL, 0);
	if (ret)
		dev_err(&pdev->dev, "Register platform PCM failed: %d\n", ret);

	return ret;
}

static struct platform_driver i2s_driver = {
	.probe = sunxi_i2s_probe,
	.driver = {
		.name = "sunxi-i2s",
		.owner = THIS_MODULE,
		.of_match_table = sunxi_i2s_dt_ids,
	},
};

module_platform_driver(i2s_driver);
MODULE_AUTHOR("Marcus Cooper <codekipper@gmail.com>");
MODULE_DESCRIPTION("Allwinner Sunxi I2S SoC DAI");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sunxi-i2s");
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/mbus.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
//#include <linux/platform_data/asoc-sunxi.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>

#include <sound/dmaengine_pcm.h>


//#include "sunxi.h"
#else

#define DRV_NAME	"sunxi-i2s"

#define SUNXI_I2S_FORMATS \
		(SNDRV_PCM_FMTBIT_S16_LE | \
			SNDRV_PCM_FMTBIT_S24_LE)

/* for suspend/resume feature */
static int regsave[8];

/* TODO: Initialize structure on probe, after register default configuration */
/* NOT USED NOW */
static struct sunxi_i2s_info sunxi_iis = {
	.slave = 0,
	.samp_fs = 48000,
	.samp_res = 24,
	.samp_format = 0,
	.ws_size = 32,
	.mclk_rate = 512,
	.lrc_pol = 0,
	.bclk_pol = 0,
	.pcm_datamode = 0,
	.pcm_sw = 0,
	.pcm_sync_period = 0,
	.pcm_sync_type = 0,
	.pcm_start_slot = 0,
	.pcm_lsb_first = 0,
	.pcm_ch_num = 1,
};

typedef struct __BCLK_SET_INF
{
    __u8	bitpersamp;	/* bits per sample - Word Sizes */
    __u8	clk_div;	/* clock division */
    __u16	mult_fs;	/* multiplay of sample rate */

} __bclk_set_inf;

typedef struct __MCLK_SET_INF
{
    __u32	samp_rate;	/* sample rate */
    __u16	mult_fs;	/* multiply of sample rate */

    __u8	clk_div;	/* mpll division */
    __u32	mclk;		/* select mpll, 24.576MHz/22.5792Mhz */

} __mclk_set_inf;

static __bclk_set_inf BCLK_INF[] =
{
    // 16bits per sample
    {16,  4, 128}, {16,  6, 192}, {16,  8, 256},
    {16, 12, 384}, {16, 16, 512},

    //24 bits per sample
    {24,  4, 192}, {24,  8, 384}, {24, 16, 768},

    //32 bits per sample
    {32,  2, 128}, {32,  4, 256}, {32,  6, 384},
    {32,  8, 512}, {32, 12, 768},

    //end flag
    {0xff, 0, 0},
};

static __mclk_set_inf  MCLK_INF[] =
{
    // 8k bitrate
    {  8000, 128, 24, 24576000}, {  8000, 192, 16, 24576000}, {  8000, 256, 12, 24576000},
    {  8000, 384,  8, 24576000}, {  8000, 512,  6, 24576000}, {  8000, 768,  4, 24576000},

    // 16k bitrate
    { 16000, 128, 12, 24576000}, { 16000, 192,  8, 24576000}, { 16000, 256,  6, 24576000},
    { 16000, 384,  4, 24576000}, { 16000, 768,  2, 24576000},

    // 32k bitrate
    { 32000, 128,  6, 24576000}, { 32000, 192,  4, 24576000}, { 32000, 384,  2, 24576000},
    { 32000, 768,  1, 24576000},

    // 64k bitrate
    { 64000, 192,  2, 24576000}, { 64000, 384,  1, 24576000},

    //128k bitrate
    {128000, 192,  1, 24576000},

    // 12k bitrate
    { 12000, 128, 16, 24576000}, { 12000, 256, 8, 24576000}, { 12000, 512, 4, 24576000},

    // 24k bitrate
    { 24000, 128,  8, 24576000}, { 24000, 256, 4, 24576000}, { 24000, 512, 2, 24576000},

    // 48K bitrate
    { 48000, 128,  4, 24576000}, { 48000, 256,  2, 24576000}, { 48000, 512, 1, 24576000},

    // 96k bitrate
    { 96000, 128 , 2, 24576000}, { 96000, 256,  1, 24576000},

    //192k bitrate
    {192000, 128,  1, 24576000},

    //11.025k bitrate
    { 11025, 128, 16, 22579200}, { 11205, 256,  8, 22579200}, { 11205, 512,  4, 22579200},

    //22.05k bitrate
    { 22050, 128,  8, 22579200}, { 22050, 256,  4, 22579200},
    { 22050, 512,  2, 22579200},

    //44.1k bitrate
    { 44100, 128,  4, 22579200}, { 44100, 256,  2, 22579200}, { 44100, 512,  1, 22579200},

    //88.2k bitrate
    { 88200, 128,  2, 22579200}, { 88200, 256,  1, 22579200},

    //176.4k bitrate
    {176400, 128, 1, 22579200},

    //end flag 0xffffffff
    {0xffffffff, 0, 0, 24576000},
};

static irqreturn_t sunxi_dai_isr(int irq, void *devid)
{
	struct sunxi_i2s_info *dai = (struct sunxi_i2s_info *)devid;
	struct device *dev = &dai->pdev->dev;
	u32 flags, xcsr, mask;
	bool irq_none = true;

	dev_dbg(dev, "isr: got an IRQ, need to manage\n");

	/*
	 * TODO: manage the IRQ from DAI I2S!
	 *	look for hint in sound/soc/fsl/fsl_sai.c
	 */

out:
	if (irq_none)
		return IRQ_NONE;
	else
		return IRQ_HANDLED;
}


/*
* TODO: Function description.
*/
//static s32 get_clock_divder(u32 sample_rate, u32 sample_width, u32 * mclk_div, u32* mpll, u32* bclk_div, u32* mult_fs)
static s32 sunxi_i2s_divisor_values(struct sunxi_i2s_info *i2s_info, u32 * mclk_div, u32* bclk_div, u32* mclk)
{
	u32 i, j, ret = -EINVAL;

	printk("[I2S]Entered %s\n", __func__);
	printk("[I2S]priv->samp_fs = %d\n", i2s_info->samp_fs);
	printk("[I2S]priv->samp_res = %d\n", i2s_info->samp_res);

	for(i=0; i< ARRAY_SIZE(MCLK_INF); i++) {
        if((MCLK_INF[i].samp_rate == i2s_info->samp_fs) && ((MCLK_INF[i].mult_fs == 256) || (MCLK_INF[i].mult_fs == 128))) {
			for(j=0; j<ARRAY_SIZE(BCLK_INF); j++) {
                if((BCLK_INF[j].bitpersamp == i2s_info->samp_res) && (BCLK_INF[j].mult_fs == MCLK_INF[i].mult_fs)) {
					//set mclk and bclk division
					*mclk_div = MCLK_INF[i].clk_div;
					*mclk = MCLK_INF[i].mclk;
					*bclk_div = BCLK_INF[j].clk_div;
					i2s_info->mclk_rate = MCLK_INF[i].mult_fs;
					ret = 0;
					break;
				}
			}
		} else if(MCLK_INF[i].samp_rate == 0xffffffff)
			break;
	}
	return ret;
}

static int sunxi_i2s_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	bool tx = substream->stream == SNDRV_PCM_STREAM_PLAYBACK;

	struct sunxi_i2s_info *priv2 = snd_soc_card_get_drvdata(rtd->card); /* AV now just a check if the data structure is filled..*/
	struct sunxi_i2s_info *priv = snd_soc_dai_get_drvdata(dai);

	if (!priv2)
		printk("[I2S]DEB Entered %s BUT missing runtime private data for DMA\n", __func__);

	printk("[I2S]Entered %s: hw is %s, stream is: %s\n", __func__, substream->name, tx?"playback":"capture");

	return clk_prepare_enable(priv->clk_module);
}

static void sunxi_i2s_shutdown(struct snd_pcm_substream *substream,
						struct snd_soc_dai *cpu_dai)
{
	struct sunxi_i2s_info *priv = snd_soc_dai_get_drvdata(cpu_dai);

	printk("[I2S]Entered %s, need to clear some HW regs?\n", __func__);

	clk_disable_unprepare(priv->clk_module);
}

static void sunxi_i2s_capture_start(struct sunxi_i2s_info *priv)
{
	printk("[I2S]Entered %s\n", __func__);
	/* flush RXFIFO */
	regmap_update_bits(priv->regmap, SUNXI_I2SFCTL, SUNXI_I2SFCTL_FRX, SUNXI_I2SFCTL_FRX);

	/* clear RX counter */
	regmap_update_bits(priv->regmap, SUNXI_I2SRXCNT, 0xffffffff, 0x0);

	/* enable DA_CTL RXEN */
	regmap_update_bits(priv->regmap, SUNXI_I2SCTL, SUNXI_I2SCTL_RXEN, SUNXI_I2SCTL_RXEN);

	/* enable DA_INT RX_DRQ */
	regmap_update_bits(priv->regmap, SUNXI_I2SINT, SUNXI_I2SINT_RXDRQEN, SUNXI_I2SINT_RXDRQEN);

}

static void sunxi_i2s_capture_stop(struct sunxi_i2s_info *priv)
{
	unsigned int rx_counter;

	/* disable DA RX_DRQ */
	regmap_update_bits(priv->regmap, SUNXI_I2SINT, SUNXI_I2SINT_RXDRQEN, 0x0);
	//regmap_update_bits(priv->regmap, SUNXI_I2SRXCNT, 0xffffffff << SUNXI_I2SRXCNT_RX_CNT, 0x0 << SUNXI_I2SRXCNT_RX_CNT);

	regmap_read(priv->regmap, SUNXI_I2SRXCNT, &rx_counter);
	printk("DEB: stop I2S rec: sample counter: 0x%08x\n", rx_counter);

	/* disable DA_CTL RXEN */
	regmap_update_bits(priv->regmap, SUNXI_I2SCTL, SUNXI_I2SCTL_RXEN, 0x0 );

	/* flush RXFIFO */
	regmap_update_bits(priv->regmap, SUNXI_I2SFCTL, SUNXI_I2SFCTL_FRX, 0x0);

}
static void sunxi_i2s_play_start(struct sunxi_i2s_info *priv)
{
	unsigned int tmp;
	printk("[I2S]Entered %s\n", __func__);

	regmap_read(priv->regmap, SUNXI_I2SCTL, &tmp);
	printk("DEB %s: DA CTL register: 0x%08x\n",  __func__, tmp);

	/* flush TX FIFO */
	regmap_update_bits(priv->regmap, SUNXI_I2SFCTL, SUNXI_I2SFCTL_FTX, SUNXI_I2SFCTL_FTX);

	/* clear TX counter */
	regmap_update_bits(priv->regmap, SUNXI_I2STXCNT, 0xffffffff, 0x0);

	regmap_read(priv->regmap, SUNXI_I2STXCNT, &tmp);
	printk("DEB %s: sample counter: 0x%08x\n",  __func__, tmp);

	/* enable DA_CTL TXEN */
	regmap_update_bits(priv->regmap, SUNXI_I2SCTL, SUNXI_I2SCTL_TXEN, SUNXI_I2SCTL_TXEN);

	regmap_read(priv->regmap, SUNXI_I2SCTL, &tmp);
	printk("DEB %s: DA CTL registers after cfg: 0x%08x\n",  __func__, tmp);
	regmap_read(priv->regmap, SUNXI_I2SCLKD, &tmp);
	printk("DEB %s: DA CLKD registers: 0x%08x\n",  __func__, tmp);
	regmap_read(priv->regmap, SUNXI_I2SFAT0, &tmp);
	printk("DEB %s: DA FAT0 registers: 0x%08x\n",  __func__, tmp);

	/* enable DA TX_DRQ */
	regmap_update_bits(priv->regmap, SUNXI_I2SINT, SUNXI_I2SINT_TXDRQEN, SUNXI_I2SINT_TXDRQEN);
	regmap_read(priv->regmap, SUNXI_I2SINT, &tmp);
	printk("DEB %s: DA INT registers: 0x%08x\n",  __func__, tmp);

}
static void sunxi_i2s_play_stop(struct sunxi_i2s_info *priv)
{
	unsigned int tx_counter;
	/* TODO: see if we need to drive PA GPIO low */

	regmap_read(priv->regmap, SUNXI_I2STXCNT, &tx_counter);
	printk("DEB %s: sample counter: 0x%08x\n",  __func__, tx_counter);

	/* disable DA_CTL TXEN */
	regmap_update_bits(priv->regmap, SUNXI_I2SCTL, SUNXI_I2SCTL_TXEN, 0x0 );

	/* disable DA TX_DRQ */
	regmap_update_bits(priv->regmap, SUNXI_I2SINT, SUNXI_I2SINT_TXDRQEN, 0x0);

}

static int sunxi_i2s_trigger(struct snd_pcm_substream *substream, int cmd,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct sunxi_i2s_info *priv = snd_soc_dai_get_drvdata(dai);

	printk("DEB: %s, copy from same fmt on i2s 3.4 legacy sunxi-i2s.c\n", __func__);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			sunxi_i2s_capture_start(priv);
		else
			sunxi_i2s_play_start(priv);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			sunxi_i2s_capture_stop(priv);
		else
			sunxi_i2s_play_stop(priv);
		break;
	default:
		return -EINVAL;
	}

	return 0;

}

static int sunxi_i2s_init(struct sunxi_i2s_info *priv)
{
	printk("[I2S]Entered %s\n", __func__);
	/*
	 * was used for parsing FEX file, and, is suffesful:
	 * - setting slave or master
	 * - requesting GPIO of I2S ctrl
	 * - registering platform driver
	 */

	return 0;

}

/*
* TODO: Function Description
* Saved in snd_soc_dai_ops sunxi_iis_dai_ops.
* Function called internally. The Machine Driver doesn't need to call this function because it is called whenever sunxi_i2s_set_clkdiv is called.
* The master clock in Allwinner SoM depends on the sampling frequency.
*/
static int sunxi_i2s_set_sysclk(struct snd_soc_dai *cpu_dai, int clk_id, unsigned int freq, int dir)
{
	u32 reg_val;
	struct sunxi_i2s_info *priv = snd_soc_dai_get_drvdata(cpu_dai);

	printk("[I2S]Entered %s\n", __func__);
	if(!priv->slave) {
		switch(clk_id) {
			case SUNXI_SET_MCLK:
				/* TODO - Check if the master clock is needed when slave mode is selected. */
				if (clk_set_rate(priv->clk_pll2, freq)) {
					pr_err("Try to set the i2s pll2 clock failed!\n");
					return -EINVAL;
				}
				break;
			case SUNXI_MCLKO_EN:    /* Master clock output */
				if(dir == 1)    /* Enable */
					regmap_update_bits(priv->regmap, SUNXI_I2SCLKD, SUNXI_I2SCLKD_MCLKOEN, SUNXI_I2SCLKD_MCLKOEN);
				if(dir == 0)    /* Disable */
					regmap_update_bits(priv->regmap, SUNXI_I2SCLKD, SUNXI_I2SCLKD_MCLKOEN, 0x0);
				break;
			default:
				pr_err("Try to set unknown clkid: %d\n", clk_id);
				return -EINVAL;

		}
		regmap_read(priv->regmap, SUNXI_I2SCLKD, &reg_val);
		printk("DEB: DA I2S CLKD: 0x%08x\n", reg_val);
	}
	return 0;
}

/*
* TODO: Function Description
* Saved in snd_soc_dai_ops sunxi_iis_dai_ops.
*/
static int sunxi_i2s_set_clkdiv(struct snd_soc_dai *cpu_dai, int div_id, int value)
{
	u32 reg_bk, reg_val, ret;
	u32 mclk = 0;
	u32 mclk_div = 0;
	u32 bclk_div = 0;
	u32 mclk_divreg = 0;
	u32 bclk_divreg = 0;
	struct sunxi_i2s_info *priv = snd_soc_dai_get_drvdata(cpu_dai);

	/* Here i should know the sample rate and the FS multiple. */

	printk("[I2S]Entered %s, div_id is %d\n", __func__, div_id);

	switch (div_id) {
		case SUNXI_DIV_MCLK:
			regmap_update_bits(priv->regmap, SUNXI_I2SCLKD, 0xf << SUNXI_I2SCLKD_MCLKDIV, (value & 0xf) << SUNXI_I2SCLKD_MCLKDIV);
			break;
		case SUNXI_DIV_BCLK:
			regmap_update_bits(priv->regmap, SUNXI_I2SCLKD, 0x7 << SUNXI_I2SCLKD_BCLKDIV, (value & 0x7) << SUNXI_I2SCLKD_BCLKDIV);
			break;
		case SUNXI_SAMPLING_FREQ:
			if (!priv->slave) {
				reg_bk = priv->samp_fs;
				priv->samp_fs = (u32)value;
				ret = sunxi_i2s_divisor_values(priv, &mclk_div, &bclk_div, &mclk); /* Get the register values */
				printk("[I2S]Sampling rate is %d; selected MCLK: %d, MCLK_DIV: %d, BCLKDIV: %d\n", priv->samp_fs, mclk, mclk_div, bclk_div);
				if(ret != 0) {
					printk("[I2S]Sampling rate %d frequency not supported, turning to backup: %d.", priv->samp_fs, reg_bk);
					priv->samp_fs = reg_bk;
					return ret;
				} else {
					priv->samp_fs = (u32)value;
					sunxi_i2s_set_sysclk(cpu_dai, SUNXI_SET_MCLK, mclk, 0); /* Set the master clock. */
					/* AV for BCLK_DIV and MCLK_DIV, need to find the index from divisor value.. */
					switch (mclk_div) {
						case 1: mclk_divreg=0; break;
						case 2: mclk_divreg=1; break;
						case 4: mclk_divreg=2; break;
						case 6: mclk_divreg=3; break;
						case 8: mclk_divreg=4; break;
						case 12: mclk_divreg=5; break;
						case 16: mclk_divreg=6; break;
						case 24: mclk_divreg=7; break;
						case 32: mclk_divreg=8; break;
						case 48: mclk_divreg=9; break;
						case 64: mclk_divreg=10; break;
						default:
							printk("[I2S] %s: MCLK div unsupported %d, putting %d\n", __func__, mclk_div, mclk_divreg);
					}
					switch (bclk_div) {
						case 2: bclk_divreg=0; break;
						case 4: bclk_divreg=1; break;
						case 6: bclk_divreg=2; break;
						case 8: bclk_divreg=3; break;
						case 12: bclk_divreg=4; break;
						case 16: bclk_divreg=5; break;
						case 32: bclk_divreg=6; break;
						case 64: bclk_divreg=7; break;
						default:
							printk("[I2S] %s: BCLK div unsupported %d, putting %d\n", __func__, bclk_div, bclk_divreg);

					}
					/*regmap_update_bits(priv->regmap, SUNXI_I2SCLKD, (0xf << SUNXI_I2SCLKD_MCLKDIV)|(0x7 << SUNXI_I2SCLKD_BCLKDIV),
							((mclk_div & 0xf) << SUNXI_I2SCLKD_MCLKDIV)|((bclk_div & 0x7) << SUNXI_I2SCLKD_BCLKDIV));*/
					regmap_update_bits(priv->regmap, SUNXI_I2SCLKD, 0xf << SUNXI_I2SCLKD_MCLKDIV, (mclk_divreg & 0xf) << SUNXI_I2SCLKD_MCLKDIV);
					regmap_update_bits(priv->regmap, SUNXI_I2SCLKD, 0x7 << SUNXI_I2SCLKD_BCLKDIV, (bclk_divreg & 0x7) << SUNXI_I2SCLKD_BCLKDIV);
					regmap_read(priv->regmap, SUNXI_I2SCLKD, &reg_val);
					printk("DEB: DA I2S internal CLKD (MCLK / BCLK): 0x%02x\n", reg_val);
				}
			} else {
				priv->samp_fs = (u32)value;
			}
			break;
		default:
				regmap_read(priv->regmap, SUNXI_I2SCLKD, &reg_val);
				printk("ERR: dev_id unknown: %d, DA I2S CLKD (MCLK / BCLK): 0x%02x\n", div_id, reg_val);
			return -EINVAL;

	}
		regmap_read(priv->regmap, SUNXI_I2SCLKD, &reg_val);
		printk("DEB: DA I2S CLKD (MCLK / BCLK): 0x%02x\n", reg_val);
	return 0;
}

/*
* TODO: Function description.
* TODO: Refactor function because the configuration is with wrong scheme. Use a 4bit mask with the configuration option and then the value?
* TODO: Include TX and RX FIFO trigger levels.
* Saved in snd_soc_dai_ops sunxi_iis_dai_ops.
* Configure:
* - Master/Slave.
* - I2S/PCM mode.
* - Signal Inversion.
* - Word Select Size.
* - PCM Registers.
*/
static int sunxi_i2s_set_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
	u32 reg_val1;
	u32 reg_val2;
	struct sunxi_i2s_info *priv = snd_soc_dai_get_drvdata(cpu_dai);

	printk("[I2S]Entered %s, FMT: 0x%08x\n", __func__, fmt);


	/* Master/Slave Definition*/
	regmap_read(priv->regmap, SUNXI_I2SCTL, &reg_val1 );
	printk("[I2S] %s: reg DA_CTL: 0x%08x\n", __func__, reg_val1);
	switch(fmt & SND_SOC_DAIFMT_MASTER_MASK){
		case SND_SOC_DAIFMT_CBS_CFS:		/* clk & frm slave */
			reg_val1 &= ~SUNXI_I2SCTL_MS;	/* 0: I2S Master! */
			printk("[I2S] %s, Master, so codec Slave.\n", __func__);
			break;
		case SND_SOC_DAIFMT_CBM_CFM:		/* clk & frm master */
			reg_val1 |= SUNXI_I2SCTL_MS;	/* 1: I2S Slave! */
			printk("[I2S] %s, Slave, so codec Master.\n", __func__);
			break;
		default:
			printk("[I2S] %s: Master-Slave Select unknown mode: (fmt=%x)\n", __func__, fmt);
			return -EINVAL;
	}
	regmap_write(priv->regmap, SUNXI_I2SCTL, reg_val1 );

	/* I2S or PCM mode.*/
	regmap_read(priv->regmap, SUNXI_I2SCTL, &reg_val1 );
	regmap_read(priv->regmap, SUNXI_I2SFAT0, &reg_val2 );
	printk("[I2S] %s: reg DA_FAT0: 0x%08x\n", __func__, reg_val2);
	switch(fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
		case SND_SOC_DAIFMT_I2S:        /* I2S mode */
			reg_val1 &= ~SUNXI_I2SCTL_PCM;
			reg_val2 &= ~SUNXI_I2SFAT0_FMT_RVD;/* Clear FMT(Bit 1:0) */
			reg_val2 |= SUNXI_I2SFAT0_FMT_I2S;
			printk("[I2S]sunxi_i2s_set_fmt: Set I2S mode\n");
			priv->samp_format = SND_SOC_DAIFMT_I2S;
			break;
		case SND_SOC_DAIFMT_RIGHT_J:    /* Right Justified mode */
			reg_val1 &= ~SUNXI_I2SCTL_PCM;
			reg_val2 &= ~SUNXI_I2SFAT0_FMT_RVD;/* Clear FMT(Bit 1:0) */
			reg_val2 |= SUNXI_I2SFAT0_FMT_RGT;
			printk("[I2S]sunxi_i2s_set_fmt: Set Right Justified mode\n");
			priv->samp_format = SND_SOC_DAIFMT_RIGHT_J;
			break;
		case SND_SOC_DAIFMT_LEFT_J:     /* Left Justified mode */
			reg_val1 &= ~SUNXI_I2SCTL_PCM;
			reg_val2 &= ~SUNXI_I2SFAT0_FMT_RVD;/* Clear FMT(Bit 1:0) */
			reg_val2 |= SUNXI_I2SFAT0_FMT_LFT;
			printk("[I2S]sunxi_i2s_set_fmt: Set Left Justified mode\n");
			priv->samp_format = SND_SOC_DAIFMT_LEFT_J;
			break;
		case SND_SOC_DAIFMT_DSP_A:      /* L data msb after FRM LRC */
			reg_val1 &= ~SUNXI_I2SCTL_PCM;
			reg_val2 &= ~SUNXI_I2SFAT0_FMT_RVD;/* Clear FMT(Bit 1:0) */
			reg_val2 |= SUNXI_I2SFAT0_FMT_LFT;
			priv->samp_format = SND_SOC_DAIFMT_DSP_A;
			printk("[I2S]sunxi_i2s_set_fmt: Set L data msb after FRM LRC mode\n");
			break;
		case SND_SOC_DAIFMT_DSP_B:      /* L data msb during FRM LRC */
			reg_val1 |= SUNXI_I2SCTL_PCM;
			reg_val2 &= ~SUNXI_I2SFAT0_FMT_RVD;/* Clear FMT(Bit 1:0) */
			reg_val2 |= SUNXI_I2SFAT0_LRCP;
			priv->samp_format = SND_SOC_DAIFMT_DSP_B;
			printk("[I2S]sunxi_i2s_set_fmt: Set L data msb during FRM LRC mode\n");
			break;
		default:
			printk("[I2S]sunxi_i2s_set_fmt: Unknown mode\n");
			return -EINVAL;
	}
	regmap_write(priv->regmap, SUNXI_I2SCTL, reg_val1 );
	regmap_write(priv->regmap, SUNXI_I2SFAT0, reg_val2 );

	/* Word select Size */
	regmap_read(priv->regmap, SUNXI_I2SFAT0, &reg_val1 );
	switch(fmt & SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_MASK)/* TODO: Refactor, wrong configuration scheme.*/ {
		case SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_16BCLK:
			reg_val1 &= ~SUNXI_I2SFAT0_WSS_32BCLK; /* clear word select size */
			reg_val1 |= SUNXI_I2SFAT0_WSS_16BCLK;
			priv->ws_size = 16;
			printk("[I2S]sunxi_i2s_set_fmt: Set word select size = 16.\n");
			break;
		case SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_20BCLK:
			reg_val1 &= ~SUNXI_I2SFAT0_WSS_32BCLK; /* clear word select size */
			reg_val1 |= SUNXI_I2SFAT0_WSS_20BCLK;
			priv->ws_size = 20;
			printk("[I2S]sunxi_i2s_set_fmt: Set word select size = 20.\n");
			break;
		case SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_24BCLK:
			reg_val1 &= ~SUNXI_I2SFAT0_WSS_32BCLK; /* clear word select size */
			reg_val1 |= SUNXI_I2SFAT0_WSS_24BCLK;
			priv->ws_size = 24;
			printk("[I2S]sunxi_i2s_set_fmt: Set word select size = 24.\n");
			break;
		case SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_32BCLK:
			reg_val1 &= ~SUNXI_I2SFAT0_WSS_32BCLK; /* clear word select size */
			reg_val1 |= SUNXI_I2SFAT0_WSS_32BCLK;
			priv->ws_size = 32;
			printk("[I2S]sunxi_i2s_set_fmt: Set word select size = 32.\n");
			break;
		default:
			printk("[I2S]sunxi_i2s_set_fmt: Unknown mode.\n");
			break;
	}
	regmap_write(priv->regmap, SUNXI_I2SFAT0, reg_val1 );

	/* Signal Inversion */
	regmap_read(priv->regmap, SUNXI_I2SFAT0, &reg_val1 );
	switch(fmt & SND_SOC_DAIFMT_INV_MASK) {
		case SND_SOC_DAIFMT_NB_NF:     /* normal bit clock + frame */
			reg_val1 &= ~SUNXI_I2SFAT0_LRCP;
			reg_val1 &= ~SUNXI_I2SFAT0_BCP;
			priv->bclk_pol = 0;
			priv->lrc_pol = 0;
			printk("[I2S]sunxi_i2s_set_fmt: Normal bit clock + frame\n");
			break;
		case SND_SOC_DAIFMT_NB_IF:     /* normal bclk + inverted frame */
			reg_val1 |= SUNXI_I2SFAT0_LRCP;
			reg_val1 &= ~SUNXI_I2SFAT0_BCP;
			priv->bclk_pol = 0;
			priv->lrc_pol = 1;
			printk("[I2S]sunxi_i2s_set_fmt: Normal bclk + inverted frame\n");
			break;
		case SND_SOC_DAIFMT_IB_NF:     /* inverted bclk + normal frame */
			reg_val1 &= ~SUNXI_I2SFAT0_LRCP;
			reg_val1 |= SUNXI_I2SFAT0_BCP;
			priv->bclk_pol = 1;
			priv->lrc_pol = 0;
			printk("[I2S]sunxi_i2s_set_fmt: Inverted bclk + normal frame\n");
			break;
		case SND_SOC_DAIFMT_IB_IF:     /* inverted bclk + frame */
			reg_val1 |= SUNXI_I2SFAT0_LRCP;;
			reg_val1 |= SUNXI_I2SFAT0_BCP;
			priv->bclk_pol = 1;
			priv->lrc_pol = 1;
			printk("[I2S]sunxi_i2s_set_fmt: Inverted bclk + frame\n");
			break;
		default:
			printk("[I2S]sunxi_i2s_set_fmt: Unknown mode\n");
			return -EINVAL;
	}
	regmap_write(priv->regmap, SUNXI_I2SFAT0, reg_val1 );

	return 0;
}


static int sunxi_i2s_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct sunxi_i2s_info *priv = snd_soc_dai_get_drvdata(dai);
	int is_24bit = !!(hw_param_interval(params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS)->min == 32);
	unsigned int rate = params_rate(params);
	unsigned int hwrate;
	unsigned int tmp;
	unsigned int reg_val1;

	printk("[I2S]Entered %s\n", __func__);
	switch (rate) {
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
		printk("[I2S]sunxi_i2s_hw_params: SNDRV_PCM_STREAM_PLAYBACK. chan: %d\n", params_channels(params));
		switch (params_channels(params)) { /* Enables the outputs and sets the map of the samples, on crescent order. */
		/* FIXME: always 2 channels, for draft */
		default:
			printk("[I2S] %s: channels selected different then 2 but not implemented\n", __func__);
		case 2:
			regmap_update_bits(priv->regmap, SUNXI_I2SCTL, SUNXI_I2SCTL_SDO0EN|SUNXI_I2SCTL_SDO1EN|SUNXI_I2SCTL_SDO2EN|SUNXI_I2SCTL_SDO3EN, SUNXI_I2SCTL_SDO0EN);
			printk("[I2S]sunxi_i2s_hw_params: SDO0 enabled, 2 channels selected.\n");
			break;
		}
		if (is_24bit) /* FIXME need supporting also 20 bit properly, here it's two bytes only */
			priv->playback_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		else
			priv->playback_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	} else  {
		printk("[I2S]sunxi_i2s_hw_params: SNDRV_PCM_STREAM_CAPTURE.\n");
		switch (params_channels(params)) { /* Enables the outputs and sets the map of the samples, on crescent order.*/
		/* FIXME: always 2 channels, for draft */
		default:
			printk("[I2S]sunxi_i2s_hw_params: channels selected different then 2 but...\n");
		case 2:
			regmap_update_bits(priv->regmap, SUNXI_RXCHSEL, 7, SUNXI_RXCHSEL_CHNUM(2)); /* mask 3 lsbs */
			regmap_update_bits(priv->regmap, SUNXI_RXCHMAP, 0x3f, SUNXI_RXCHMAP_CH(1)|SUNXI_RXCHMAP_CH(2)); /* FIXME: ugly masks!*/
			printk("[I2S]sunxi_i2s_hw_params: SDO0 enabled, 2 channels selected.\n");
			break;
		}
		if (is_24bit) /* FIXME need supporting also 20 bit properly, here it's two bytes only */
			priv->playback_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		else
			priv->playback_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;

	}

	/* Sample Rate. */
	if(priv->slave == 0) {
		/* Only master has to configure the clock registers for sample rate setting.*/
		priv->samp_fs = params_rate(params);
		sunxi_i2s_set_clkdiv(dai, SUNXI_SAMPLING_FREQ, priv->samp_fs);
	}

	/* Sample Format. */
	/* TODO: Support SNDRV_PCM_FORMAT_S20_3LE and SNDRV_PCM_FMTBIT_S24_3LE formats. Must check the Word Size and change it for 24bits ("3LE").*/
	reg_val1 = 0;	/* Clear sample resolution select size */
	switch (params_format(params)) {
		case SNDRV_PCM_FORMAT_S16_LE:
			reg_val1 |= SUNXI_I2SFAT0_SR_16BIT | SUNXI_I2SFAT0_WSS_16BCLK;
			priv->samp_res = 16;
			printk("[I2S] %s: SNDRV_PCM_FORMAT_S16_LE.\n", __func__);
			break;

		case SNDRV_PCM_FORMAT_S24_3LE:
			reg_val1 |= SUNXI_I2SFAT0_SR_24BIT;
			priv->samp_res = 24;
			if(priv->ws_size != 32) {/* If the Word Size is not equal to 32, sets word size to 32.*/
				reg_val1 |= SUNXI_I2SFAT0_WSS_32BCLK;
				priv->ws_size = 32;
				printk("[I2S] sunxi_i2s_hw_params: Changing word slect size to 32bit.\n");
			}
			printk("[I2S] sunxi_i2s_hw_params: SNDRV_PCM_FORMAT_S24_3LE.\n");
			break;
		case SNDRV_PCM_FORMAT_S24_LE:
			reg_val1 |= SUNXI_I2SFAT0_SR_24BIT;
			priv->samp_res = 24;
			if(priv->ws_size != 32) {/* If the Word Size is not equal to 32, sets word size to 32.*/
				reg_val1 |= SUNXI_I2SFAT0_WSS_32BCLK;
				priv->ws_size = 32;
				printk("[I2S] sunxi_i2s_hw_params: Changing word slect size to 32bit.\n");
			}
			printk("[I2S] sunxi_i2s_hw_params: SNDRV_PCM_FORMAT_S24_LE.\n");
			break;
		default:
			printk("[I2S] sunxi_i2s_hw_params: Unsupported format (%d).\n", (int)params_format(params));
			reg_val1 |= SUNXI_I2SFAT0_SR_24BIT | SUNXI_I2SFAT0_WSS_32BCLK;
			priv->samp_res = 24;
			priv->ws_size = 32;
			printk("[I2S] sunxi_i2s_hw_params: Setting 24 bit format and changing word slect size to 32bit.\n");
			break;
	}
	regmap_update_bits(priv->regmap, SUNXI_I2SFAT0, SUNXI_I2SFAT0_WSS_32BCLK|SUNXI_I2SFAT0_SR_RVD, reg_val1);
	regmap_read(priv->regmap, SUNXI_I2SFAT0, &tmp);
		printk("DEB %s: DA FAT0 register: %x\n",  __func__, tmp);

	return 0;
}


/*
* TODO: Function Description.
* Saved in snd_soc_dai_driver sunxi_iis_dai.
*/
static int sunxi_i2s_dai_probe(struct snd_soc_dai *cpu_dai)
{
	struct sunxi_i2s_info *priv = snd_soc_dai_get_drvdata(cpu_dai);

	snd_soc_dai_init_dma_data(cpu_dai, &priv->playback_dma_data, &priv->capture_dma_data);


	printk("[I2S]Entered %s\n", __func__);

	/* I2S Default Register Configuration */
	priv->slave = 0, /* put as default Master */
	priv->samp_fs = 48000,
	priv->samp_res = 24,
	priv->samp_format = SND_SOC_DAIFMT_I2S,
	priv->ws_size = 32,
	priv->mclk_rate = 512,
	priv->lrc_pol = 0,
	priv->bclk_pol = 0,
	priv->pcm_datamode = 0,
	priv->pcm_sw = 0,
	priv->pcm_sync_period = 0,
	priv->pcm_sync_type = 0,
	priv->pcm_start_slot = 0,
	priv->pcm_lsb_first = 0,
	priv->pcm_ch_num = 1,

	/* Digital Audio Register Default Values */
	/* DIGITAL AUDIO CONTROL REGISTER DEF */
	regmap_update_bits(priv->regmap, SUNXI_I2SCTL, SUNXI_I2SCTL_GEN, SUNXI_I2SCTL_GEN);

	/* DIGITAL AUDIO FORMAT REGISTER 0 */
	regmap_write(priv->regmap, SUNXI_I2SFAT0, SUNXI_I2SFAT0_FMT_I2S|SUNXI_I2SFAT0_SR_24BIT|SUNXI_I2SFAT0_WSS_32BCLK);

	/* FIFO control register. TODO: Understand how to optimize this parameter.*/
	regmap_write(priv->regmap, SUNXI_I2SFCTL, SUNXI_I2SFCTL_RXOM_MOD1|SUNXI_I2SFCTL_TXIM_MOD1|SUNXI_I2SFCTL_RXTL(0xf)|SUNXI_I2SFCTL_TXTL(0x40));

	/* enable MCLK output */
	regmap_update_bits(priv->regmap, SUNXI_I2SCLKD, SUNXI_I2SCLKD_MCLKOEN, SUNXI_I2SCLKD_MCLKOEN);
	printk("[IIS-0] sunxi_i2s_set_clkdiv: enable MCLK\n");

	printk("[I2S]I2S default register configuration complete.\n");

	return 0;
}

/*
* TODO: Function Description.
* Saved in snd_soc_dai_driver sunxi_iis_dai.
*/
static int sunxi_i2s_dai_remove(struct snd_soc_dai *dai)
{
	struct sunxi_i2s_info *priv = snd_soc_dai_get_drvdata(dai);
	printk("[I2S]Entered %s\n", __func__);

	/* DIGITAL AUDIO CONTROL REGISTER */
	regmap_write(priv->regmap, SUNXI_I2SCTL, 0);

	return 0;
}

/*
* TODO: Function description.
*/
static void iisregsave(struct sunxi_i2s_info *priv)
{
	printk("[I2S]Entered %s\n", __func__);

	regmap_read(priv->regmap, SUNXI_I2SCTL,  &regsave[0]);
	regmap_read(priv->regmap, SUNXI_I2SFAT0,  &regsave[1]);
	regmap_read(priv->regmap, SUNXI_I2SFAT1,  &regsave[2]);
	regmap_read(priv->regmap, SUNXI_I2SFCTL,  &regsave[3]); /*| (0x3<<24);*//* TODO: Bit 24- FRX - Write ‘1’ to flush RX FIFO, self clear to ‘0’. Really needed?*/
	regmap_read(priv->regmap, SUNXI_I2SINT,  &regsave[4]);
	regmap_read(priv->regmap, SUNXI_I2SCLKD,  &regsave[5]);
	regmap_read(priv->regmap, SUNXI_TXCHSEL,  &regsave[6]);
	regmap_read(priv->regmap, SUNXI_TXCHMAP,  &regsave[7]);
}

/*
* TODO: Function description.
*/
static void iisregrestore(struct sunxi_i2s_info *priv)
{
	printk("[I2S]Entered %s\n", __func__);

	regmap_write(priv->regmap, SUNXI_I2SCTL,  regsave[0]);
	regmap_write(priv->regmap, SUNXI_I2SFAT0, regsave[1]);
	regmap_write(priv->regmap, SUNXI_I2SFAT1, regsave[2]);
	regmap_write(priv->regmap, SUNXI_I2SFCTL, regsave[3]);
	regmap_write(priv->regmap, SUNXI_I2SINT, regsave[4]);
	regmap_write(priv->regmap, SUNXI_I2SCLKD, regsave[5]);
	regmap_write(priv->regmap, SUNXI_TXCHSEL, regsave[6]);
	regmap_write(priv->regmap, SUNXI_TXCHMAP, regsave[7]);
}

/*
* TODO: Function Description.
* Saved in snd_soc_dai_driver sunxi_iis_dai.
*/
static int sunxi_i2s_suspend(struct snd_soc_dai *cpu_dai)
{
	struct sunxi_i2s_info *priv = snd_soc_dai_get_drvdata(cpu_dai);

	printk("[I2S]Entered %s\n", __func__);

	/* Global Disable Digital Audio Interface */
	regmap_update_bits(priv->regmap, SUNXI_I2SCTL, SUNXI_I2SCTL_GEN, 0x0 );

	iisregsave(priv);

	if(!priv->slave) {
		/* release the module clock, only for master mode */
		clk_disable(priv->clk_module);
	}
	clk_disable(priv->clk_apb);

	return 0;
}

/*
* TODO: Function Description.
* Saved in snd_soc_dai_driver sunxi_iis_dai.
*/
static int sunxi_i2s_resume(struct snd_soc_dai *cpu_dai)
{
	struct sunxi_i2s_info *priv = snd_soc_dai_get_drvdata(cpu_dai);

	printk("[I2S]Entered %s\n", __func__);

	/* enable the module clock */
	clk_enable(priv->clk_apb);

	if (!priv->slave) {
		/* enable the module clock */
		clk_enable(priv->clk_module);
	}

	iisregrestore(priv);

	/* Global Enable Digital Audio Interface */
	regmap_update_bits(priv->regmap, SUNXI_I2SCTL, SUNXI_I2SCTL_GEN, SUNXI_I2SCTL_GEN);

	return 0;
}


static const struct regmap_config sunxi_i2s_regmap_config = {
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32,
	.max_register = SUNXI_RXCHMAP,
};

static const struct snd_soc_component_driver sunxi_i2s_component = {
	.name		= DRV_NAME,
};

static struct snd_soc_dai_ops sunxi_i2s_dai_ops = {
	.startup    = sunxi_i2s_startup,
	.shutdown   = sunxi_i2s_shutdown,
	.set_sysclk = sunxi_i2s_set_sysclk,
	.set_clkdiv = sunxi_i2s_set_clkdiv,
	.set_fmt    = sunxi_i2s_set_fmt,
	.hw_params  = sunxi_i2s_hw_params,
	.trigger    = sunxi_i2s_trigger,
};

static struct snd_soc_dai_driver sunxi_i2s_dai = {
	.name		= "sunxi-i2s-dai",
	.probe		= sunxi_i2s_dai_probe,
/* TO ADD LATER */
/*
	.remove		= sunxi_i2s_dai_remove,
	.suspend	= sunxi_i2s_suspend,
	.resume		= sunxi_i2s_resume,
*/
	.ops		= &sunxi_i2s_dai_ops,
	.capture	= {
		.stream_name = "pcm0c",
		/* TODO: Support SNDRV_PCM_FMTBIT_S20_3LE and SNDRV_PCM_FMTBIT_S24_3LE. */
		.formats = SUNXI_I2S_FORMATS,
		.rates = SUNXI_I2S_RATES,
		.channels_min = 1,
		.channels_max = 2,
	},
	.playback	= {
		.stream_name = "pcm0p",
		/* TODO: Support SNDRV_PCM_FMTBIT_S20_3LE and SNDRV_PCM_FMTBIT_S24_3LE. Implies in changing the word select size in *_set_fmt.*/
		.formats = SUNXI_I2S_FORMATS,
		.rates = SUNXI_I2S_RATES,
		.channels_min = 1,
		.channels_max = 2,
	},
	.symmetric_rates = 1,
};

#ifdef CONFIG_OF
static const struct of_device_id sunxi_i2s_of_match[] = {
	{ .compatible = "allwinner,sun4i-a10-i2s" },
	{ .compatible = "allwinner,sun7i-a20-i2s" },
	{ }
};
MODULE_DEVICE_TABLE(of, sunxi_i2s_of_match);
#endif


static int sunxi_digitalaudio_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct sunxi_i2s_info *priv;
	const struct of_device_id *of_id;
	struct device *dev = &pdev->dev;
	struct resource *res;
	void __iomem *base;
	int irq, i, tmp;

	int ret;

	printk("[I2S]Entered %s\n", __func__);

	if (!of_device_is_available(np))
		return -ENODEV;

	of_id = of_match_device(sunxi_i2s_of_match, dev);
	if (!of_id)
		return -EINVAL;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	printk("[AV]alloc ok %s\n", __func__);

	priv->pdev = pdev;
	priv->revision = (enum sunxi_soc_family)of_id->data;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base))
		return PTR_ERR(base);

	priv->regmap = devm_regmap_init_mmio(&pdev->dev, base,
						&sunxi_i2s_regmap_config);
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);

	/* Get the clocks from the DT */
	priv->clk_apb = devm_clk_get(dev, "apb");
	if (IS_ERR(priv->clk_apb)) {
		dev_err(dev, "failed to get apb clock\n");
		return PTR_ERR(priv->clk_apb);
	}
	/* Enable the bus clock */
	if (clk_prepare_enable(priv->clk_apb)) {
		dev_err(dev, "failed to enable apb clock\n");
		return -EINVAL;
	}

	priv->clk_pll2 = devm_clk_get(dev, "audio");
	if (IS_ERR(priv->clk_pll2)) {
		dev_err(dev, "failed to get audio pll2 clock\n");
		return PTR_ERR(priv->clk_pll2);
	}

	priv->clk_module = devm_clk_get(dev, "i2s");
	if (IS_ERR(priv->clk_module)) {
		dev_err(dev, "failed to get i2s module clock\n");
		return PTR_ERR(priv->clk_module);
	}

	/* Enable the clock on a basic rate */
	ret = clk_set_rate(priv->clk_pll2, 24576000);
	if (ret) {
		dev_err(dev, "failed to set pll2 base clock rate\n");
		return -EINVAL;
	}

	if (clk_prepare_enable(priv->clk_pll2)) {
		dev_err(dev, "failed to enable pll2 clock\n");
		ret = -EINVAL;
		goto exit_clkdisable_apb_clk;
	}

	if (clk_prepare_enable(priv->clk_module)) {
		dev_err(dev, "failed to enable i2s module clock\n");
		ret = -EINVAL;
		goto exit_clkdisable_pll2_apb_clk;
	}

	dev_info(dev, "[AV] set clock and rate on i2s, %s\n", __func__);

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "no irq for node %s\n", pdev->name);
		return irq;
	}

	ret = devm_request_irq(&pdev->dev, irq, sunxi_dai_isr, 0, np->name, priv);
	if (ret) {
		dev_err(&pdev->dev, "failed to claim irq %u\n", irq);
		return ret;
	}
	dev_info(dev, "[AV] got assigned irq: %d\n", irq);

	/* DMA configuration for TX FIFO */
	priv->playback_dma_data.addr = res->start + SUNXI_I2STXFIFO;
	priv->playback_dma_data.maxburst = 4;
	priv->playback_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;

	/* DMA configuration for RX FIFO */
	priv->capture_dma_data.addr = res->start + SUNXI_I2SRXFIFO;
	priv->capture_dma_data.maxburst = 4;
	priv->capture_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;

	platform_set_drvdata(pdev, priv);
	dev_info(dev, "%s, [AV] set sunxi_i2s_info into platform\n", __func__);

	ret = devm_snd_soc_register_component(&pdev->dev, &sunxi_i2s_component, &sunxi_i2s_dai, 1);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_component failed (%d)\n", ret);
		goto err_clk_disable;
	}

	ret = devm_snd_dmaengine_pcm_register(&pdev->dev, NULL, 0);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_dmaengine failed (%d)\n", ret);
		goto err_clk_disable;
	}

	sunxi_i2s_init(priv);
	return 0;

err_platform:
	dev_info(&pdev->dev, "AV snd_soc_register_platform failed\n");
	snd_soc_unregister_component(&pdev->dev);
err_clk_disable:
	dev_info(&pdev->dev, "AV snd_soc_register_* failed\n");
	if (!IS_ERR(priv->clk_module))
		clk_disable_unprepare(priv->clk_module);
	clk_disable_unprepare(priv->clk_apb);
	return ret;

exit_clkdisable_pll2_apb_clk:
	clk_disable_unprepare(priv->clk_pll2);
exit_clkdisable_apb_clk:
	clk_disable_unprepare(priv->clk_apb);
	return ret;

}

static int sunxi_digitalaudio_remove(struct platform_device *pdev)
{
	struct sunxi_i2s_info *priv = dev_get_drvdata(&pdev->dev);

	snd_soc_unregister_component(&pdev->dev);

	if (!IS_ERR(priv->clk_apb))
		clk_disable_unprepare(priv->clk_apb);
	if (!IS_ERR(priv->clk_module))
		clk_disable_unprepare(priv->clk_module);

	return 0;
}

static struct platform_driver sunxi_i2s_driver = {
	.probe  = sunxi_digitalaudio_probe,
	.remove = sunxi_digitalaudio_remove,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = of_match_ptr(sunxi_i2s_of_match),
	},
};

module_platform_driver(sunxi_i2s_driver);

/* Module information */
MODULE_DEVICE_TABLE(of, sunxi_i2s_of_match); /* autoload from: https://lwn.net/Articles/448502/ */
MODULE_AUTHOR("Andrea Venturi, <be17068@iperbole.bo.it>");
MODULE_DESCRIPTION("Sunxi I2S ASoC Interface");
MODULE_LICENSE("GPL");
#endif

