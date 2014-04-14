/*
 * sound\soc\sunxi\audiocodec\sunxi_sndcodec.c
 * (C) Copyright 2010-2016
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * huangxin <huangxin@Reuuimllatech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/mutex.h>

#include <linux/mfd/core.h>

#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/soc-dapm.h>
#include <linux/io.h>

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
		.name		= "sun6i-audio",
		.stream_name	= "sun6i-test",

		.cpu_dai_name	= "sun6i-audio-pcm.0",
		.codec_dai_name = "sun6i-audio-codec-pcm",

		.platform_name	= "sun6i-audio-pcm.0",
		.codec_name	= "sun6i-audio-codec.0",

		.init		= sun6i_dai_init,
	},
};

static struct snd_soc_card sun6i_codec_card = {
	.name	= "sun6i-audio-card",
	.owner	= THIS_MODULE,

	.dai_link = sun6i_card_dai,
	.num_links = ARRAY_SIZE(sun6i_card_dai),

	.dapm_widgets = sun6i_card_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(sun6i_card_dapm_widgets),
	.dapm_routes = sun6i_card_route,
	.num_dapm_routes = ARRAY_SIZE(sun6i_card_route),
};

static const struct resource sun6i_audio_pcm_res[] = {
	{
		.flags = IORESOURCE_MEM,
	},
};

static const struct resource sun6i_audio_codec_res[] = {
	{
		.flags = IORESOURCE_MEM,
	},
};

static const struct mfd_cell sun6i_audio_subdevs[] = {
	{
		.name		= "sun6i-audio-pcm",
		.of_compatible	= "allwinner,sun6i-a31-audio-pcm",
		.resources	= sun6i_audio_pcm_res,
		.num_resources	= ARRAY_SIZE(sun6i_audio_pcm_res),
	},
	{
		.name		= "sun6i-audio-codec",
		.of_compatible	= "allwinner,sun6i-a31-audio-codec-for-real",
		.resources	= sun6i_audio_codec_res,
		.num_resources	= ARRAY_SIZE(sun6i_audio_codec_res),
	},
};

static int sun6i_audio_card_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &sun6i_codec_card;
	struct resource *res;
	int ret;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	ret = mfd_add_devices(&pdev->dev, 0, sun6i_audio_subdevs,
			      ARRAY_SIZE(sun6i_audio_subdevs),
			      res, -1, NULL);
	if (ret)
		return ret;

	card->dev = &pdev->dev;
	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "Couldn't register ASoC card\n");
		goto err_unregister_mfd;
	}

	return 0;

err_unregister_mfd:
	mfd_remove_devices(&pdev->dev);
	return ret;
}

static int sun6i_audio_card_remove(struct platform_device *pdev)
{
	snd_soc_unregister_card(&sun6i_codec_card);

	mfd_remove_devices(&pdev->dev);

	return 0;
};

static const struct of_device_id sun6i_audio_card_dt_ids[] = {
	{ .compatible = "allwinner,sun6i-a31-audio-codec", },
	{}
};
MODULE_DEVICE_TABLE(of, sun6i_codec_match);

static struct platform_driver sun6i_audio_card_driver = {
	.probe		= sun6i_audio_card_probe,
	.remove		= sun6i_audio_card_remove,
	.driver		= {
		.name	= "sun6i-audio-card",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(sun6i_audio_card_dt_ids),
	},
};
module_platform_driver(sun6i_audio_card_driver);
MODULE_AUTHOR("huangxin");
MODULE_DESCRIPTION("SUNXI_sndpcm ALSA SoC audio driver");
MODULE_LICENSE("GPL");
