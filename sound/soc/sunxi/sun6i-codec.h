/*
 * sound\soc\sun6i\sun6i-codec.h
 * (C) Copyright 2010-2016
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
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
#ifndef _SUN6I_CODEC_H
#define _SUN6I_CODEC_H

/*Codec Register*/
#define CODEC_BASSADDRESS	(0x01c22c00)
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
#define SUN6I_ADCDFEN				(16)
#define SUNXI_ADC_FIFOC_RX_TRIG_LEVEL		(8)
#define SUNXI_ADC_FIFOC_MONO_EN			(7)
#define SUNXI_ADC_FIFOC_RX_SAMPLE_BITS		(6)
#define SUNXI_ADC_FIFOC_ADC_DRQ_EN		(4)
#define SUNXI_ADC_FIFOC_FIFO_FLUSH		(0)

#define SUN6I_ADC_FIFOS		(0x14)		//33 ADC FIFO Status Register, different with a10
#define SUN6I_ADC_RXDATA	(0x18)		//33 ADC RX Data Register
#define SUN6I_HMIC_CTRL		(0x1c)		//Earpiece MIC bias detect register,new func
#define SUN6I_DAC_ACTL		(0x20)		//Output Mixer & DAC Analog Control Register
#define SUNXI_DAC_ACTL_DACAENR			(31)
#define SUNXI_DAC_ACTL_DACAENL			(30)
#define SUNXI_DAC_ACTL_MIXEN			(29)
#define SUN6I_DAC_ACTL_MIXENR			(29)
#define SUN6I_DAC_ACTL_MIXENL			(28)
#define SUN6I_DAC_ACTL_MIXMUTER			(17)
#define SUNXI_DAC_ACTL_LDACLMIXS		(15)
#define SUNXI_DAC_ACTL_RDACRMIXS		(14)
#define SUNXI_DAC_ACTL_LDACRMIXS		(13)
#define SUN6I_DAC_ACTL_MIXMUTEL			(10)
#define SUN6I_DAC_ACTL_HPISR			(9)
#define SUN6I_DAC_ACTL_HPISL			(8)
#define SUNXI_DAC_ACTL_DACPAS			(8)
#define SUNXI_DAC_ACTL_MIXPAS			(7)
#define SUN6I_DAC_ACTL_HPPAMUTER		(7)
#define SUN6I_DAC_ACTL_HPPAMUTEL		(6)
#define SUNXI_DAC_ACTL_PA_MUTE			(6)
#define SUNXI_DAC_ACTL_PA_VOL			(0)

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


#define AUDIO_RATE_DEFAULT	44100
#define ST_RUNNING		(1<<0)
#define ST_OPENED		(1<<1)
 
struct sun6i_pcm_dma_params {
	char *name;		
	dma_addr_t dma_addr;	
};

/*
* Convenience kcontrol builders
*/
#define CODEC_SINGLE_VALUE(xreg, xshift, xmax,	xinvert)\
		((unsigned long)&(struct codec_mixer_control)\
		{.reg	=	xreg,	.shift	=	xshift,	.rshift	=	xshift,	.max	=	xmax,\
   	.invert	=	xinvert})

#define CODEC_SINGLE(xname,	reg,	shift,	max,	invert)\
{	.iface	= SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname,\
	.info	= snd_codec_info_volsw,	.get = snd_codec_get_volsw,\
	.put	= snd_codec_put_volsw,\
	.private_value	= CODEC_SINGLE_VALUE(reg, shift, max, invert)}

/*	mixer control*/	
struct	codec_mixer_control{
	int	min;
	int     max;
	int     where;
	unsigned int mask;
	unsigned int reg;
	unsigned int rreg;
	unsigned int shift;
	unsigned int rshift;
	unsigned int invert;
	unsigned int value;
};

#endif
