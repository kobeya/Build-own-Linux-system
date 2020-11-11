/*alsa整个框架在我理解中可以分成两部分来描述

	1，alsa的基本框架，PC机、嵌入式均需要使用

	2，基于alsa再次封装的嵌入式alsa system on chip，asoc框架


这个章节，大概描述一下alsa的基本框架，因为这部分的内容非常成熟，至少对于目前的我来说
工作中没有遇到需要追踪这部分代码的问题，所以这里暂时只是理解脉络，不深入探索。
*/


alsa音频驱动框架

	1，sound/core/sound.c		实现最顶层的file_operations结构体，只有一个open函数，起中转作用
	
	2，sound/core/control.c		实现控制接口的file_opetations结构体
	
	3，sound/core/pcm_native.c	实现playback、capture的file_operations结构体

	这些结构体，规定了ALSA的接口

	然后再基于这些ALSA的结构体来填充硬件相关的代码
	主要是分配、设置、注册snd_card结构体

	1，snd_card_create	创建控制接口
	2，snd_pcm_new		创建播放、录音接口
	3，snd_card_register	注册


总结来说，就是首先通过sound/core音频核心来规定各种控制、播放、录音等结构体
然后在snd_card中实现与硬件相关的代码。


作为一个引子，这些硬件相关的代码，具体怎么实现呢？下一节asoc中来描述，snd_card是把硬件相关的内容
规范在一起的一个结构体，一般我们也不需要修改这个地方，这是sound的核心代码。
