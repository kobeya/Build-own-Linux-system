/*前面一节描述了alsa的规范以及通过snd_card对硬件的封装，
	这节我们就探索一下asoc的框架，asoc是怎么实现soc与codec的通讯
	以及它们之间的数据是怎么传输的，驱动是怎么实现的。
*/



ASOC框架分为3部分

1，platform
		用来描述芯片的DAI接口、负责数据的传输
		1）DAI
		snd_soc_dai_driver
		用来表示支持哪些格式的数据
		提供设置格式的函数
		启动数据的传输，比如有trriger函数的实现

		2）数据的传输
		snd_soc_platform_driver
		比如前面DAI提供了启动数据传输的函数，这里使用这些函数来传输数据，比如DMA传输等

2，codec
		用来描述音频编解码芯片，包含DAI、控制接口
		1）DAI
		snd_soc_dai_driver
		用来表示支持哪些格式数据、频率范围、通道数等
			(摘取RK3399 SDK中ak4396的codec驱动DAI结构体)
			static struct snd_soc_dai_driver ak4396_dai = {
				.name = "AK4396 HiFi",
				.playback = {
					.stream_name = "Playback",
					.channels_min = 2,
					.channels_max = 2,
					.rates = SNDRV_PCM_RATE_8000_192000,
					.formats = SNDRV_PCM_FMTBIT_S16_LE  |
						   SNDRV_PCM_FMTBIT_S24_3LE |
						   SNDRV_PCM_FMTBIT_S24_LE  |
						   SNDRV_PCM_FMTBIT_S32_LE
				},
				.ops = &ak4396_dai_ops,
			};

			static struct snd_soc_dai_ops ak4396_dai_ops = {
				.hw_params = ak4396_hw_params,
				.set_fmt = ak4396_set_dai_fmt,
				.set_sysclk = ak4396_set_dai_sysclk,
			};


		提供修改格式、参数的接口

		2）控制接口
		snd_soc_codec_driver,用于读取芯片的寄存器操作

3，machine
		
		snd_soc_card, snd_soc_dai_link
		用来确定使用哪一个paltform，哪一个codec芯片
		一系列操作后会注册一个名为"soc_audio"的平台设备，
		soc-core.c中的soc_probe函数被调用
		最后，回到我们基本的alsa规范中的snd_card结构体的分配、设置、注册。









