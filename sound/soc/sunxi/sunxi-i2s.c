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
	#define SUNXI_I2SCLKD_BCLK_MASK	GENMASK	(6, 4)
	#define SUNXI_I2SCLKD_BCLK(bclk)	((bclk) << 4)
	#define SUNXI_I2SCLKD_MCLK_MASK	GENMASK	(3, 0)
	#define SUNXI_I2SCLKD_MCLK(mclk)	((mclk) << 0)

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
		SUN4IA,	/* A10 SoC - revision A */
		SUN4I,	/* A10 SoC - later revisions */
		SUN5I,	/* A10S/A13 SoCs */
		SUN7I,	/* A20 SoC */
};

struct sunxi_i2s_info {
	struct platform_device			*pdev;
	struct regmap				*regmap;
	void __iomem				*reg_base; //not used!
	enum sunxi_soc_family			revision;
	struct snd_dmaengine_dai_dma_data	playback_dma_data;
	struct snd_dmaengine_dai_dma_data	capture_dma_data;
	struct clk				*clk_apb;
	struct clk				*clk_pll2;
	struct clk				*clk_module;
	struct clk				*dai_clk; //not used!
	int					master; //not used!
	resource_size_t				mapbase;//not used!
	int					mclk_rate;//set but not used!
	int					ws_size;//ditto
	int					lrc_pol;//ditto
	int					bclk_pol;//ditto
	int					pcm_datamode;//set but not used
	int					pcm_ch_num;//ditto
	int					pcm_txtype;//never used
	int					pcm_rxtype;//never used
	int					pcm_sync_type;//never set
	int					pcm_sw;//ditto
	int					pcm_start_slot;//ditto
	int					pcm_lsb_first;//ditto
	int					pcm_sync_period;//never used
	int					samp_fs;
	int					samp_res;
	int					samp_format;
	int					slave;
};

#define DRV_NAME	"sunxi-i2s"

#define SUNXI_I2S_FORMATS \
		(SNDRV_PCM_FMTBIT_S16_LE | \
			SNDRV_PCM_FMTBIT_S24_LE)

/* for suspend/resume feature */
static int regsave[8];

static irqreturn_t sunxi_dai_isr(int irq, void *devid)
{
	struct sunxi_i2s_info *dai = (struct sunxi_i2s_info *)devid;
	struct device *dev = &dai->pdev->dev;
	bool irq_none = true;

	dev_dbg(dev, "isr: got an IRQ, need to manage\n");

	if (irq_none)
		return IRQ_NONE;
	else
		return IRQ_HANDLED;
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
	printk("DEB %s: DA CTL register: 0x%08x\n", __func__, tmp);

	/* flush TX FIFO */
	regmap_update_bits(priv->regmap, SUNXI_I2SFCTL, SUNXI_I2SFCTL_FTX, SUNXI_I2SFCTL_FTX);

	/* clear TX counter */
	regmap_update_bits(priv->regmap, SUNXI_I2STXCNT, 0xffffffff, 0x0);

	regmap_read(priv->regmap, SUNXI_I2STXCNT, &tmp);
	printk("DEB %s: sample counter: 0x%08x\n", __func__, tmp);

	/* enable DA_CTL TXEN */
	regmap_update_bits(priv->regmap, SUNXI_I2SCTL, SUNXI_I2SCTL_TXEN, SUNXI_I2SCTL_TXEN);

	regmap_read(priv->regmap, SUNXI_I2SCTL, &tmp);
	printk("DEB %s: DA CTL registers after cfg: 0x%08x\n", __func__, tmp);
	regmap_read(priv->regmap, SUNXI_I2SCLKD, &tmp);
	printk("DEB %s: DA CLKD registers: 0x%08x\n", __func__, tmp);
	regmap_read(priv->regmap, SUNXI_I2SFAT0, &tmp);
	printk("DEB %s: DA FAT0 registers: 0x%08x\n", __func__, tmp);

	/* enable DA TX_DRQ */
	regmap_update_bits(priv->regmap, SUNXI_I2SINT, SUNXI_I2SINT_TXDRQEN, SUNXI_I2SINT_TXDRQEN);
	regmap_read(priv->regmap, SUNXI_I2SINT, &tmp);
	printk("DEB %s: DA INT registers: 0x%08x\n", __func__, tmp);

}
static void sunxi_i2s_play_stop(struct sunxi_i2s_info *priv)
{
	unsigned int tx_counter;
	/* TODO: see if we need to drive PA GPIO low */

	regmap_read(priv->regmap, SUNXI_I2STXCNT, &tx_counter);
	printk("DEB %s: sample counter: 0x%08x\n", __func__, tx_counter);

	/* disable DA_CTL TXEN */
	regmap_update_bits(priv->regmap, SUNXI_I2SCTL, SUNXI_I2SCTL_TXEN, 0x0 );

	/* disable DA TX_DRQ */
	regmap_update_bits(priv->regmap, SUNXI_I2SINT, SUNXI_I2SINT_TXDRQEN, 0x0);

}

static int sunxi_i2s_trigger(struct snd_pcm_substream *substream, int cmd,
				struct snd_soc_dai *dai)
{
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

/*
* TODO: Function Description
* Saved in snd_soc_dai_ops sunxi_i2s_dai_ops.
* Function called internally. The Machine Driver doesn't need to call this function because it is called whenever sunxi_i2s_set_clkdiv is called.
* The master clock in Allwinner SoM depends on the sampling frequency.
*/
/* From Maxime's DAI code */
struct sun4i_dai_clk_div {
	u8	div;
	u8	val;
};

static const struct sun4i_dai_clk_div sun4i_dai_bclk_div[] = {
	{ .div = 2, .val = 0 },
	{ .div = 4, .val = 1 },
	{ .div = 6, .val = 2 },
	{ .div = 8, .val = 3 },
	{ .div = 12, .val = 4 },
	{ .div = 16, .val = 5 },
	{ /* Sentinel */ },
};

static const struct sun4i_dai_clk_div sun4i_dai_mclk_div[] = {
	{ .div = 1, .val = 0 },
	{ .div = 2, .val = 1 },
	{ .div = 4, .val = 2 },
	{ .div = 6, .val = 3 },
	{ .div = 8, .val = 4 },
	{ .div = 12, .val = 5 },
	{ .div = 16, .val = 6 },
	{ .div = 24, .val = 7 },
	{ /* Sentinel */ },
};

static int sun4i_dai_get_bclk_div(unsigned int oversample_rate,
					unsigned int word_size)
{
	int div = oversample_rate / word_size / 2;
	int i;

	for (i = 0; sun4i_dai_bclk_div[i].div; i++) {
		const struct sun4i_dai_clk_div *bdiv = sun4i_dai_bclk_div + i;

		if (bdiv->div == div)
			return bdiv->val;
	}

	return -EINVAL;
}

static int sun4i_dai_get_mclk_div(unsigned int oversample_rate,
					unsigned int module_rate,
					unsigned int sampling_rate)
{
	int div = module_rate / sampling_rate / oversample_rate;
	int i;

	for (i = 0; sun4i_dai_mclk_div[i].div; i++) {
		const struct sun4i_dai_clk_div *mdiv = sun4i_dai_mclk_div + i;

		if (mdiv->div == div)
			return mdiv->val;
	}

	return -EINVAL;
}

static int sun4i_dai_oversample_rates[] = { 128, 192, 256, 384, 512, 768 };

static int sun4i_dai_set_clk_rate(struct sunxi_i2s_info *priv,
					unsigned int rate,
					unsigned int word_size)
{
	unsigned int clk_rate;
	int bclk_div, mclk_div;
	int i;

	switch (rate) {
	case 176400:
	case 88200:
	case 44100:
	case 22050:
	case 11025:
		clk_rate = 22579200;
		break;

	case 192000:
	case 128000:
	case 96000:
	case 64000:
	case 48000:
	case 32000:
	case 24000:
	case 16000:
	case 12000:
	case 8000:
		clk_rate = 24576000;
		break;

	default:
		return -EINVAL;
	}

	clk_set_rate(priv->clk_module, 24576000);

	/* Always favor the highest oversampling rate */
	for (i = (ARRAY_SIZE(sun4i_dai_oversample_rates) - 1); i >= 0; i--) {
		unsigned int oversample_rate = sun4i_dai_oversample_rates[i];

		bclk_div = sun4i_dai_get_bclk_div(oversample_rate, word_size);
		mclk_div = sun4i_dai_get_mclk_div(oversample_rate, clk_rate,
									rate);

		if (bclk_div > 0 || mclk_div > 0)
			break;
	}

	if (bclk_div <= 0 || mclk_div <= 0)
		return -EINVAL;

	regmap_update_bits(priv->regmap, SUNXI_I2SCLKD,
				SUNXI_I2SCLKD_BCLK_MASK |
				SUNXI_I2SCLKD_MCLK_MASK,
				SUNXI_I2SCLKD_BCLK(bclk_div) |
				SUNXI_I2SCLKD_MCLK(mclk_div));

	return 0;
}

/*
* TODO: Function description.
* TODO: Refactor function because the configuration is with wrong scheme. Use a 4bit mask with the configuration option and then the value?
* TODO: Include TX and RX FIFO trigger levels.
* Saved in snd_soc_dai_ops sunxi_i2s_dai_ops.
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
	case SND_SOC_DAIFMT_I2S:	/* I2S mode */
		reg_val1 &= ~SUNXI_I2SCTL_PCM;
		reg_val2 &= ~SUNXI_I2SFAT0_FMT_RVD;/* Clear FMT(Bit 1:0) */
		reg_val2 |= SUNXI_I2SFAT0_FMT_I2S;
		printk("[I2S]%s: Set I2S mode\n", __func__);
		priv->samp_format = SND_SOC_DAIFMT_I2S;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:	/* Right Justified mode */
		reg_val1 &= ~SUNXI_I2SCTL_PCM;
		reg_val2 &= ~SUNXI_I2SFAT0_FMT_RVD;/* Clear FMT(Bit 1:0) */
		reg_val2 |= SUNXI_I2SFAT0_FMT_RGT;
		printk("[I2S]%s: Set Right Justified mode\n", __func__);
		priv->samp_format = SND_SOC_DAIFMT_RIGHT_J;
		break;
	case SND_SOC_DAIFMT_LEFT_J:	/* Left Justified mode */
		reg_val1 &= ~SUNXI_I2SCTL_PCM;
		reg_val2 &= ~SUNXI_I2SFAT0_FMT_RVD;/* Clear FMT(Bit 1:0) */
		reg_val2 |= SUNXI_I2SFAT0_FMT_LFT;
		printk("[I2S]%s: Set Left Justified mode\n", __func__);
		priv->samp_format = SND_SOC_DAIFMT_LEFT_J;
		break;
	case SND_SOC_DAIFMT_DSP_A:	/* L data msb after FRM LRC */
		reg_val1 &= ~SUNXI_I2SCTL_PCM;
		reg_val2 &= ~SUNXI_I2SFAT0_FMT_RVD;/* Clear FMT(Bit 1:0) */
		reg_val2 |= SUNXI_I2SFAT0_FMT_LFT;
		priv->samp_format = SND_SOC_DAIFMT_DSP_A;
		printk("[I2S]%s: Set L data msb after FRM LRC mode\n", __func__);
		break;
	case SND_SOC_DAIFMT_DSP_B:	/* L data msb during FRM LRC */
		reg_val1 |= SUNXI_I2SCTL_PCM;
		reg_val2 &= ~SUNXI_I2SFAT0_FMT_RVD;/* Clear FMT(Bit 1:0) */
		reg_val2 |= SUNXI_I2SFAT0_LRCP;
		priv->samp_format = SND_SOC_DAIFMT_DSP_B;
		printk("[I2S]%s: Set L data msb during FRM LRC mode\n", __func__);
		break;
	default:
		printk("[I2S]%s: Unknown mode\n", __func__);
		return -EINVAL;
	}
	regmap_write(priv->regmap, SUNXI_I2SCTL, reg_val1 );
	regmap_write(priv->regmap, SUNXI_I2SFAT0, reg_val2 );

	/* Word select Size */
	regmap_read(priv->regmap, SUNXI_I2SFAT0, &reg_val1 );
	reg_val1 &= ~SUNXI_I2SFAT0_WSS_32BCLK; /* clear word select size */
	switch(fmt & SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_MASK)/* TODO: Refactor, wrong configuration scheme.*/ {
	case SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_16BCLK:
		reg_val1 |= SUNXI_I2SFAT0_WSS_16BCLK;
		priv->ws_size = 16;
		printk("[I2S]%s: Set word select size = 16.\n", __func__);
		break;
	case SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_20BCLK:
		reg_val1 |= SUNXI_I2SFAT0_WSS_20BCLK;
		priv->ws_size = 20;
		printk("[I2S]%s: Set word select size = 20.\n", __func__);
		break;
	case SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_24BCLK:
		reg_val1 |= SUNXI_I2SFAT0_WSS_24BCLK;
		priv->ws_size = 24;
		printk("[I2S]%s: Set word select size = 24.\n", __func__);
		break;
	case SND_SOC_DAIFMT_SUNXI_I2SFAT0_WSS_32BCLK:
		reg_val1 |= SUNXI_I2SFAT0_WSS_32BCLK;
		priv->ws_size = 32;
		printk("[I2S]%s: Set word select size = 32.\n", __func__);
		break;
	default:
		printk("[I2S]%s: Unknown mode.\n", __func__);
		break;
	}
	regmap_write(priv->regmap, SUNXI_I2SFAT0, reg_val1 );

	/* Signal Inversion */
	regmap_read(priv->regmap, SUNXI_I2SFAT0, &reg_val1 );
	switch(fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:	/* normal bit clock + frame */
		reg_val1 &= ~SUNXI_I2SFAT0_LRCP;
		reg_val1 &= ~SUNXI_I2SFAT0_BCP;
		priv->bclk_pol = 0;
		priv->lrc_pol = 0;
		printk("[I2S]%s: Normal bit clock + frame\n", __func__);
		break;
	case SND_SOC_DAIFMT_NB_IF:	/* normal bclk + inverted frame */
		reg_val1 |= SUNXI_I2SFAT0_LRCP;
		reg_val1 &= ~SUNXI_I2SFAT0_BCP;
		priv->bclk_pol = 0;
		priv->lrc_pol = 1;
		printk("[I2S]%s: Normal bclk + inverted frame\n", __func__);
		break;
	case SND_SOC_DAIFMT_IB_NF:	/* inverted bclk + normal frame */
		reg_val1 &= ~SUNXI_I2SFAT0_LRCP;
		reg_val1 |= SUNXI_I2SFAT0_BCP;
		priv->bclk_pol = 1;
		priv->lrc_pol = 0;
		printk("[I2S]%s: Inverted bclk + normal frame\n", __func__);
		break;
	case SND_SOC_DAIFMT_IB_IF:	/* inverted bclk + frame */
		reg_val1 |= SUNXI_I2SFAT0_LRCP;
		reg_val1 |= SUNXI_I2SFAT0_BCP;
		priv->bclk_pol = 1;
		priv->lrc_pol = 1;
		printk("[I2S]%s: Inverted bclk + frame\n", __func__);
		break;
	default:
		printk("[I2S]%s: Unknown mode\n", __func__);
		return -EINVAL;
	}
	regmap_write(priv->regmap, SUNXI_I2SFAT0, reg_val1 );

	return 0;
}


static int sunxi_i2s_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	struct sunxi_i2s_info *priv = snd_soc_dai_get_drvdata(dai);
	int is_24bit = !!(hw_param_interval(params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS)->min == 32);
	unsigned int rate = params_rate(params);

	printk("[I2S]Entered %s\n", __func__);
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

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		printk("[I2S]%s: SNDRV_PCM_STREAM_PLAYBACK. chan: %d\n", __func__, params_channels(params));
		switch (params_channels(params)) { /* Enables the outputs and sets the map of the samples, on crescent order. */
		/* FIXME: always 2 channels, for draft */
		default:
			printk("[I2S]%s: channels selected different then 2 but not implemented\n", __func__);
		case 2:
			regmap_update_bits(priv->regmap, SUNXI_I2SCTL, SUNXI_I2SCTL_SDO0EN|SUNXI_I2SCTL_SDO1EN|SUNXI_I2SCTL_SDO2EN|SUNXI_I2SCTL_SDO3EN, SUNXI_I2SCTL_SDO0EN);
			printk("[I2S]%s: SDO0 enabled, 2 channels selected.\n", __func__);
			break;
		}
		if (is_24bit) /* FIXME need supporting also 20 bit properly, here it's two bytes only */
			priv->playback_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		else
			priv->playback_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	} else {
		printk("[I2S]%s: SNDRV_PCM_STREAM_CAPTURE.\n", __func__);
		switch (params_channels(params)) { /* Enables the outputs and sets the map of the samples, on crescent order.*/
		/* FIXME: always 2 channels, for draft */
		default:
			printk("[I2S]%s: channels selected different then 2 but...\n", __func__);
		case 2:
			regmap_update_bits(priv->regmap, SUNXI_RXCHSEL, 7, SUNXI_RXCHSEL_CHNUM(2)); /* mask 3 lsbs */
			regmap_update_bits(priv->regmap, SUNXI_RXCHMAP, 0x3f, SUNXI_RXCHMAP_CH(1)|SUNXI_RXCHMAP_CH(2)); /* FIXME: ugly masks!*/
			printk("[I2S]%s: SDO0 enabled, 2 channels selected.\n", __func__);
			break;
		}
		if (is_24bit) /* FIXME need supporting also 20 bit properly, here it's two bytes only */
			priv->playback_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		else
			priv->playback_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;

	}

	sun4i_dai_set_clk_rate(priv, params_rate(params), params_format(params));

	return 0;
}


/*
* TODO: Function Description.
* Saved in snd_soc_dai_driver sunxi_i2s_dai.
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
	printk("[I2S] sunxi_i2s_set_clkdiv: enable MCLK\n");

	printk("[I2S]I2S default register configuration complete.\n");

	return 0;
}

/*
* TODO: Function Description.
* Saved in snd_soc_dai_driver sunxi_i2s_dai.
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
static void i2sregsave(struct sunxi_i2s_info *priv)
{
	printk("[I2S]Entered %s\n", __func__);

	regmap_read(priv->regmap, SUNXI_I2SCTL, &regsave[0]);
	regmap_read(priv->regmap, SUNXI_I2SFAT0, &regsave[1]);
	regmap_read(priv->regmap, SUNXI_I2SFAT1, &regsave[2]);
	regmap_read(priv->regmap, SUNXI_I2SFCTL, &regsave[3]); /*| (0x3<<24);*//* TODO: Bit 24- FRX - Write ‘1’ to flush RX FIFO, self clear to ‘0’. Really needed?*/
	regmap_read(priv->regmap, SUNXI_I2SINT, &regsave[4]);
	regmap_read(priv->regmap, SUNXI_I2SCLKD, &regsave[5]);
	regmap_read(priv->regmap, SUNXI_TXCHSEL, &regsave[6]);
	regmap_read(priv->regmap, SUNXI_TXCHMAP, &regsave[7]);
}

/*
* TODO: Function description.
*/
static void i2sregrestore(struct sunxi_i2s_info *priv)
{
	printk("[I2S]Entered %s\n", __func__);

	regmap_write(priv->regmap, SUNXI_I2SCTL, regsave[0]);
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
* Saved in snd_soc_dai_driver sunxi_i2s_dai.
*/
static int sunxi_i2s_suspend(struct snd_soc_dai *cpu_dai)
{
	struct sunxi_i2s_info *priv = snd_soc_dai_get_drvdata(cpu_dai);

	printk("[I2S]Entered %s\n", __func__);

	/* Global Disable Digital Audio Interface */
	regmap_update_bits(priv->regmap, SUNXI_I2SCTL, SUNXI_I2SCTL_GEN, 0x0 );

	i2sregsave(priv);

	if(!priv->slave) {
		/* release the module clock, only for master mode */
		clk_disable(priv->clk_module);
	}
	clk_disable(priv->clk_apb);

	return 0;
}

/*
* TODO: Function Description.
* Saved in snd_soc_dai_driver sunxi_i2s_dai.
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

	i2sregrestore(priv);

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
	.startup	= sunxi_i2s_startup,
	.shutdown	= sunxi_i2s_shutdown,
	.set_fmt	= sunxi_i2s_set_fmt,
	.hw_params	= sunxi_i2s_hw_params,
	.trigger	= sunxi_i2s_trigger,
};

static struct snd_soc_dai_driver sunxi_i2s_dai = {
	.name		= "sunxi-i2s-dai",
	.probe		= sunxi_i2s_dai_probe,
	.remove		= sunxi_i2s_dai_remove,
	.suspend	= sunxi_i2s_suspend,
	.resume		= sunxi_i2s_resume,
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
	{ .compatible = "allwinner,sun5i-a10s-i2s" },
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
	int irq;

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
		goto err_platform;
	}

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
	.probe	= sunxi_digitalaudio_probe,
	.remove	= sunxi_digitalaudio_remove,
	.driver	= {
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
