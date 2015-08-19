/*
 *  Copyright (C) 2010, Lars-Peter Clausen <lars@metafoo.de>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <linux/clk.h>
#include <linux/delay.h>

#include <linux/dma-mapping.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <sound/dmaengine_pcm.h>

#define SUN6I_DAC_FIFO_REG		0x0c
#define SUN6I_ADC_FIFO_REG		0x18

struct sun6i_audio_pcm {
	dma_addr_t				phys_base;

	struct snd_dmaengine_dai_dma_data	playback_dma_data;
	struct snd_dmaengine_dai_dma_data	capture_dma_data;
};	

static int sun6i_audio_pcm_startup(struct snd_pcm_substream *substream,
				   struct snd_soc_dai *dai)
{
	struct sun6i_audio_pcm *pcm = snd_soc_dai_get_drvdata(dai);

	if (dai->active)
		return 0;

	/* Flush FIFOs */
	/* Start DMA transfers */

	return 0;
}

static void sun6i_audio_pcm_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct sun6i_audio_pcm *pcm = snd_soc_dai_get_drvdata(dai);

	if (dai->active)
		return;

	/* Disable DMA transfers */
}

static int sun6i_audio_pcm_trigger(struct snd_pcm_substream *substream, int cmd,
			      struct snd_soc_dai *dai)
{
	struct sun6i_audio_pcm *pcm = snd_soc_dai_get_drvdata(dai);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		/* Start transfer */
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		/* Stop transfer */
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct snd_soc_dai_ops sun6i_audio_pcm_dai_ops = {
	.startup	= sun6i_audio_pcm_startup,
	.shutdown	= sun6i_audio_pcm_shutdown,
	.trigger	= sun6i_audio_pcm_trigger,
};

static void sun6i_audio_pcm_config(struct sun6i_audio_pcm *pcm)
{
	struct snd_dmaengine_dai_dma_data *dma_data;

	/* Playback */
	dma_data = &pcm->playback_dma_data;
	dma_data->maxburst = 1;
	dma_data->addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	dma_data->addr = pcm->phys_base + SUN6I_DAC_FIFO_REG;

	/* Capture */
	dma_data = &pcm->capture_dma_data;
	dma_data->maxburst = 1;
	dma_data->addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	dma_data->addr = pcm->phys_base + SUN6I_ADC_FIFO_REG;
}

static int sun6i_audio_pcm_dai_probe(struct snd_soc_dai *dai)
{
	struct sun6i_audio_pcm *pcm = snd_soc_dai_get_drvdata(dai);

	sun6i_audio_pcm_config(pcm);
	snd_soc_dai_init_dma_data(dai,
				  &pcm->playback_dma_data,
				  &pcm->capture_dma_data);

	/* clk_enable ? */

	return 0;
}

static int sun6i_audio_pcm_dai_remove(struct snd_soc_dai *dai)
{
	struct sun6i_audio_pcm *pcm = snd_soc_dai_get_drvdata(dai);

	/* clk_disable ? */

	return 0;
}

static struct snd_soc_dai_driver sun6i_audio_pcm_dai = {
	.probe	= sun6i_audio_pcm_dai_probe,
	.remove	= sun6i_audio_pcm_dai_remove,

	.playback = {
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= SNDRV_PCM_RATE_8000_192000 | SNDRV_PCM_RATE_KNOT,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE,
	},

	.capture = {
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= SNDRV_PCM_RATE_8000_192000 | SNDRV_PCM_RATE_KNOT,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE,
	},

	.ops = &sun6i_audio_pcm_dai_ops,
};

static const struct snd_soc_component_driver sun6i_audio_pcm_component = {
	.name		= "sun6i-audio-pcm",
};

static const struct snd_pcm_hardware sun6i_audio_pcm_hardware = {
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

static const struct snd_dmaengine_pcm_config sun6i_audio_pcm_dma_config = {
	.pcm_hardware		= &sun6i_audio_pcm_hardware,
	.prealloc_buffer_size	= 32 * 1024,
};

#include <linux/of.h>

static int sun6i_audio_pcm_dev_probe(struct platform_device *pdev)
{
	struct sun6i_audio_pcm *pcm;
	int ret;

	pcm = devm_kzalloc(&pdev->dev, sizeof(*pcm), GFP_KERNEL);
	if (!pcm)
		return -ENOMEM;

	platform_set_drvdata(pdev, pcm);

	pcm->phys_base = 0x01c22c00;

	ret = devm_snd_soc_register_component(&pdev->dev,
					      &sun6i_audio_pcm_component,
					      &sun6i_audio_pcm_dai,
					      1);
	if (ret)
		return ret;

	return devm_snd_dmaengine_pcm_register(&pdev->dev,
					       &sun6i_audio_pcm_dma_config,
					       0);
}

static struct platform_driver sun6i_audio_pcm_driver = {
	.probe = sun6i_audio_pcm_dev_probe,
	.driver = {
		.name = "sun6i-audio-pcm",
		.owner = THIS_MODULE,
	},
};

module_platform_driver(sun6i_audio_pcm_driver);
