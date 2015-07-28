/*
 * sunxi.h
 *
 * (c) 2015 Andrea Venturi <be17068@iperbole.bo.it>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#ifndef _SUNXI_AUDIO_H
#define _SUNXI_AUDIO_H

/* Supported SoC families - used for quirks */
enum sunxi_soc_family {
        SUN4IA, /* A10 SoC - revision A */
        SUN4I,  /* A10 SoC - later revisions */
        SUN5I,  /* A10S/A13 SoCs */
        SUN7I,  /* A20 SoC */
};

struct sunxi_priv {
	struct platform_device *pdev;
        struct regmap *regmap;
        struct clk *clk_apb, *clk_module, *clk_pll2, *clk_pll2x8;

        enum sunxi_soc_family revision;

        struct snd_dmaengine_dai_dma_data playback_dma_data;
        struct snd_dmaengine_dai_dma_data capture_dma_data;


};

/* Clock set cases*/
#define SUNXI_SET_MCLK  0
#define SUNXI_MCLKO_EN  1

/* Clock divider cases*/
#define SUNXI_DIV_MCLK                  0
#define SUNXI_DIV_BCLK                  1
#define SUNXI_SAMPLING_FREQ             2

struct sunxi_i2s_info {
        //legacy void __iomem   *regs;    /* IIS BASE */
        //legacy void __iomem   *ccmregs;  //CCM BASE
        //legacy void __iomem   *ioregs;   //IO BASE

        u32 slave;                      //0: master, 1: slave
        u32 samp_fs;            //audio sample rate (unit in Hz)
        u32 samp_res;           //16 bits, 20 bits , 24 bits, 32 bits)
        u32 samp_format;        //audio sample format (0: standard I2S, 1: left-justified, 2: right-justified, 3: pcm - MSB on 2nd BCLK, 4: pcm - MSB on 1st BCLK)
        u32 ws_size;            //16 BCLK, 20 BCLK, 24 BCLK, 32 BCLK)
        u32 mclk_rate;          //mclk frequency divide by fs (128fs, 192fs, 256fs, 384fs, 512fs, 768fs)
        u32 lrc_pol;            //LRC clock polarity (0: normal ,1: inverted)
        u32 bclk_pol;           //BCLK polarity (0: normal, 1: inverted)
        u32 pcm_datamode;       //PCM transmitter type (0: 16-bits linear mode, 1: 8-bits linear mode, 2: u-law, 3: A-law)
        u32 pcm_sync_type;      //PCM sync symbol size (0: short sync, 1: long sync)
        u32 pcm_sw;                     //PCM slot width (8: 8 bits, 16: 16 bits)
        u32 pcm_start_slot;     //PCM start slot index (1--4)
        u32 pcm_lsb_first;      //0: MSB first, 1: LSB first
        u32 pcm_sync_period;//PCM sync period (16/32/64/128/256)
        u32 pcm_ch_num;         //PCM channel number (1: one channel, 2: two channel)

};


/* I2S register offsets and bit fields */
/* for DA0 and presumibly for DA1 in A20 */
#define SUNXI_DA_CTL			(0x00)
#define SUNXI_DA_CTL_SDO3_EN			(1<<11)
#define SUNXI_DA_CTL_SDO2_EN			(1<<10)
#define SUNXI_DA_CTL_SDO1_EN			(1<<9)
#define SUNXI_DA_CTL_SDO0_EN			(1<<8)
#define SUNXI_DA_CTL_ASS			(1<<6) // Audio Sample Select, 0:zero, 1:last sample
#define SUNXI_DA_CTL_MS				(1<<5) // 0:Master, 1:Slave
#define SUNXI_DA_CTL_PCM			(1<<4) // 0:I2S, 1:PCM mode
#define SUNXI_DA_CTL_TXEN			(1<<2)
#define SUNXI_DA_CTL_RXEN			(1<<1)
#define SUNXI_DA_CTL_GEN			(1<<0)

#define SUNXI_DA_FAT0			(0x04)
#define SUNXI_DA_FAT0_LRCP			(1<<7) // in PCM mode, means MSB on first (1) or second (0) clock after LRCLK rising
#define SUNXI_DA_FAT0_BCP			(1<<6)
#define SUNXI_DA_FAT0_SR(x)			((x)<<4) // Sample Resolution, 00:16, 01:20, 10:24, 11:reserved
#define SUNXI_DA_FAT0_SR_16			SUNXI_DA_FAT0_SR(0)
#define SUNXI_DA_FAT0_SR_20			SUNXI_DA_FAT0_SR(1)
#define SUNXI_DA_FAT0_SR_24			SUNXI_DA_FAT0_SR(2)
#define SUNXI_DA_FAT0_SR_MASK			SUNXI_DA_FAT0_SR(3)
#define SUNXI_DA_FAT0_WSS(x)			((x)<<2) // WordSelectSize, BCLK 00:16, 01:20, 10:24, 11:32
#define SUNXI_DA_FAT0_WSS_16			SUNXI_DA_FAT0_WSS(0)
#define SUNXI_DA_FAT0_WSS_20			SUNXI_DA_FAT0_WSS(1)
#define SUNXI_DA_FAT0_WSS_24			SUNXI_DA_FAT0_WSS(2)
#define SUNXI_DA_FAT0_WSS_32			SUNXI_DA_FAT0_WSS(3)
#define SUNXI_DA_FAT0_FMT(x)			((x)<<0) // I2S format, 00:std, 01:left, 10:right, 11:reserved
#define SUNXI_DA_FAT0_FMT_STD			SUNXI_DA_FAT0_FMT(0)
#define SUNXI_DA_FAT0_FMT_LEFT			SUNXI_DA_FAT0_FMT(1)
#define SUNXI_DA_FAT0_FMT_RIGHT			SUNXI_DA_FAT0_FMT(2)
#define SUNXI_DA_FAT1			(0x08)
#define SUNXI_DA_FAT1_PCM_SYNC_PERIOD(x)	((x)<<0) 

#define SUNXI_DA_TXFIFO			(0x0c)
#define SUNXI_DA_RXFIFO			(0x10)
#define SUNXI_DA_FCTL			(0x14)
#define SUNXI_DA_FCTL_FIFOSRC			(31)
#define SUNXI_DA_FCTL_FTX			(25)
#define SUNXI_DA_FCTL_FRX			(24)
#define SUNXI_DA_FCTL_TXTL			(12)
#define SUNXI_DA_FCTL_RXTL			(4)
#define SUNXI_DA_FCTL_TXIM			(2)
#define SUNXI_DA_FCTL_RXOM			(0)

#define SUNXI_DA_FSTA			(0x18) // Fifo Status Register
#define SUNXI_DA_INT			(0x1c)
#define SUNXI_DA_INT_TX_DRQ			(7)
#define SUNXI_DA_INT_RX_DRQ			(3)

#define SUNXI_DA_ISTA			(0x20)  // Interrupt Status Register, could be used for check if TX/RX under/overrun
#define SUNXI_DA_CLKD			(0x24)  // FIXME CLKD, lot's of config to be done here
#define SUNXI_DA_CLKD_MCLKO_EN			(7)  // can be output also if the i2s intf is in slave mode
#define SUNXI_DA_CLKD_BCLKDIV			(4)  // 3 bits for bit clock divider from MCLK
#define SUNXI_DA_CLKD_MCLKDIV			(0)  // 4 bits for bit clock divider from MCLK

#define SUNXI_DA_TXCNT			(0x28)
#define SUNXI_DA_TXCNT_TX_CNT			(0)
#define SUNXI_DA_RXCNT			(0x2c)
#define SUNXI_DA_RXCNT_RX_CNT			(0)

#define SUNXI_DA_TXCHSEL		(0x30)
#define SUNXI_DA_TXCHSEL_CHNUM(x)		(((x)-1)<<0)
#define SUNXI_DA_TXCHMAP		(0x34)
#define SUNXI_DA_TXCHMAP_TX_CH(x)		(x<<(x<<2))
#define SUNXI_DA_RXCHSEL		(0x38)
#define SUNXI_DA_RXCHSEL_CHNUM(x)		(((x)-1)<<0)
#define SUNXI_DA_RXCHMAP		(0x3c)
#define SUNXI_DA_RXCHMAP_RX_CH(x)		(x<<(x<<2))

// SND_SOC_DAIFMT extension from legacy linux-sunxi I2S driver
// Format enumerations for completing the aSoC defines.
#define SND_SOC_DAIFMT_SUNXI_IISFAT0_WSS_MASK           (3<<16)
#define SND_SOC_DAIFMT_SUNXI_IISFAT0_WSS_16BCLK         (0<<16)
#define SND_SOC_DAIFMT_SUNXI_IISFAT0_WSS_20BCLK         (1<<16)
#define SND_SOC_DAIFMT_SUNXI_IISFAT0_WSS_24BCLK         (2<<16)
#define SND_SOC_DAIFMT_SUNXI_IISFAT0_WSS_32BCLK         (3<<16)

#define SUNXI_IISFAT1_MASK              (0xfff)

#define SND_SOC_DAIFMT_SUNXI_IISFAT1_PDM_MASK           (3<<18) // RX and TC PCM Data Mode (PDM) are equal.
#define SND_SOC_DAIFMT_SUNXI_IISFAT1_PDM_16PCM          (0<<18)
#define SND_SOC_DAIFMT_SUNXI_IISFAT1_PDM_8PCM           (1<<18)
#define SND_SOC_DAIFMT_SUNXI_IISFAT1_PDM_8ULAW          (2<<18)
#define SND_SOC_DAIFMT_SUNXI_IISFAT1_PDM_8ALAW          (3<<18)

#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SSYNC                      (1<<20)

#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SW                         (1<<21)

#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SI_MASK            (3<<22)
#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SI_1ST                     (0<<22)
#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SI_2ND                     (1<<22)
#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SI_3RD                     (2<<22)
#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SI_4TH                     (3<<22)

#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SEXT                       (1<<24)

#define SND_SOC_DAIFMT_SUNXI_IISFAT1_MLS                        (1<<25)

#define SND_SOC_DAIFMT_SUNXI_IISFAT1_OUTMUTE            (1<<26)

#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SYNCOUTEN          (1<<27)

#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SYNCLEN_MASK               (7<<28)
#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SYNCLEN_16BCLK             (0<<28)
#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SYNCLEN_32BCLK             (1<<28)
#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SYNCLEN_64BCLK             (2<<28)
#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SYNCLEN_128BCLK    (3<<28)
#define SND_SOC_DAIFMT_SUNXI_IISFAT1_SYNCLEN_256BCLK    (4<<28)

#define SUNXI_I2S_RATES (SNDRV_PCM_RATE_8000_192000 | SNDRV_PCM_RATE_KNOT)
#define SUNXI_I2S_PLAYBACK_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE)
#define SUNXI_I2S_CAPTURE_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE)

// FIXME from kirkwood-i2s, obsoleted by dmaengine, i suppose
struct sunxi_dma_data {
        void __iomem *io;
        struct clk *clk;
        struct clk *extclk;
        uint32_t ctl_play;
        uint32_t ctl_rec;
        struct snd_pcm_substream *substream_play;
        struct snd_pcm_substream *substream_rec;
        int irq;
        int burst;
};

extern struct snd_soc_platform_driver sunxi_soc_platform;

#endif

