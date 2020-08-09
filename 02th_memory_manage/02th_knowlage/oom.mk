Linux内核OOM机制的详细分析
https://blog.csdn.net/liukuan73/article/details/43238623












Linux内核根据应用程序的要求分配内存，通常来说应用程序分配了内存但是并没有实际全部使用，为了提高性能，这部分没用的内存可以留作它用，这部分内存是属于每个进程的，内核直接回收利用的话比较麻烦，所以内核采用一种过度分配内存（over-commit memory）的办法来间接利用这部分“空闲”的内存，提高整体内存的使用效率。一般来说这样做没有问题，但当大多数应用程序都消耗完自己的内存的时候麻烦就来了，因为这些应用程序的内存需求加起来超出了物理内存（包括swap）的容量，内核（OOM killer）必须杀掉一些进程才能腾出空间保障系统正常运行。用银行的例子来讲可能更容易懂一些，部分人取钱的时候银行不怕，银行有足够的存款应付，当全国人民（或者绝大多数）都取钱而且每个人都想把自己钱取完的时候银行的麻烦就来了，银行实际上是没有这么多钱给大家取的。

       比如某天一台机器突然ssh远程登录不了，但能ping通，说明不是网络的故障，原因是sshd进程被OOM killer杀掉了。重启机器后查看系统日志/var/log/messages会发现Out of Memory:Killprocess 1865（sshd）类似的错误信息。又比如有时VPS的MySQL总是无缘无故挂掉，或者VPS 经常死机，登陆到终端发现都是常见的 Out of memory 问题。这通常是因为某时刻应用程序大量请求内存导致系统内存不足造成的，这时会触发 Linux 内核里的 Out of Memory (OOM) killer，OOM killer 会杀掉某个进程以腾出内存留给系统用，不致于让系统立刻崩溃。如果检查相关的日志文件（/var/log/messages）就会看到下面类似的Out of memory:Kill process 信息：

  ...

  Out of memory: Kill process 9682(mysqld) score 9 or sacrifice child

  Killed process 9682, UID 27,(mysqld) total-vm:47388kB, anon-rss:3744kB, file-rss:80kB

  httpd invoked oom-killer:gfp_mask=0x201da, order=0, oom_adj=0, oom_score_adj=0

  httpd cpuset=/ mems_allowed=0

  Pid: 8911, comm: httpd Not tainted2.6.32-279.1.1.el6.i686 #1

  ...

  21556 total pagecache pages

  21049 pages in swap cache

  Swap cache stats: add 12819103,delete 12798054, find 3188096/4634617

  Free swap  = 0kB

  Total swap = 524280kB

  131071 pages RAM

  0 pages HighMem

  3673 pages reserved

   67960 pages shared

  124940 pages non-shared

 

        Linux内核有个机制叫OOM killer（Out-Of-Memory killer），该机制会监控那些占用内存过大，尤其是瞬间很快消耗大量内存的进程，为了防止内存耗尽内核会把该进程杀掉。

        内核检测到系统内存不足、挑选并杀掉某个进程的过程可以参考内核源代码 linux/mm/oom_kill.c，当系统内存不足的时候，out_of_memory()被触发，然后调用 select_bad_process() 选择一个“bad”进程杀掉，判断和选择一个“bad”进程的过程由 oom_badness()决定，最 bad 的那个进程就是那个最占用内存的进程。

 

/**

 * oom_badness -heuristic function to determine which candidate task to kill

 * @p: taskstruct of which task we should calculate

 * @totalpages:total present RAM allowed for page allocation

 *

 * The heuristicfor determining which task to kill is made to be as simple and

 * predictableas possible.  The goal is to return thehighest value for the

 * task consumingthe most memory to avoid subsequent oom failures.

 */

unsigned long oom_badness(struct task_struct *p,struct mem_cgroup *memcg,

                                  const nodemask_t *nodemask, unsigned longtotalpages)

{

           long points;

           long adj;

           if (oom_unkillable_task(p, memcg, nodemask))

                     return 0;

           p = find_lock_task_mm(p);

           if (!p)

                     return 0;

           adj = (long)p->signal->oom_score_adj;

           if (adj == OOM_SCORE_ADJ_MIN) {

                     task_unlock(p);

                     return 0;

           }

 

           /*

            * The baseline for thebadness score is the proportion of RAM that each

            * task's rss, pagetable and swap space use.

            */

           points = get_mm_rss(p->mm) + atomic_long_read(&p->mm->nr_ptes)+

                      get_mm_counter(p->mm, MM_SWAPENTS);

           task_unlock(p);

           /*

            * Root processes get 3% bonus, just like the__vm_enough_memory()

            * implementation used by LSMs.

            */

           if (has_capability_noaudit(p,CAP_SYS_ADMIN))

                     adj -= (points * 3) / 100;

           /*Normalize to oom_score_adj units */

           adj *= totalpages / 1000;

           points += adj;

           /*

            * Never return 0 for an eligible taskregardless of the root bonus and

            * oom_score_adj (oom_score_adj can't beOOM_SCORE_ADJ_MIN here).

            */

           returnpoints > 0 ? points : 1;

}

      从上面的 oom_kill.c 代码里可以看到 oom_badness() 给每个进程打分，根据 points 的高低来决定杀哪个进程，这个 points 可以根据 adj 调节，root 权限的进程通常被认为很重要，不应该被轻易杀掉，所以打分的时候可以得到 3% 的优惠（分数越低越不容易被杀掉）。我们可以在用户空间通过操作每个进程的 oom_adj 内核参数来决定哪些进程不这么容易被 OOM killer 选中杀掉。比如，如果不想 MySQL 进程被轻易杀掉的话可以找到 MySQL 运行的进程号后，调整 /proc/PID/oom_score_adj 为 -15（注意 points越小越不容易被杀）防止重要的系统进程触发(OOM)机制而被杀死，内核会通过特定的算法给每个进程计算一个分数来决定杀哪个进程，每个进程的oom分数可以在/proc/PID/oom_score中找到。每个进程都有一个oom_score的属性，oom killer会杀死oom_score较大的进程，当oom_score为0时禁止内核杀死该进程。设置/proc/PID/oom_adj可以改变oom_score，oom_adj的范围为【-17，15】，其中15最大-16最小，-17为禁止使用OOM，至于为什么用-17而不用其他数值（默认值为0），这个是由linux内核定义的，查看内核源码可知：路径为linux-xxxxx/include /uapi/linux/oom.h。

 
       oom_score为2的n次方计算出来的，其中n就是进程的oom_adj值，oom_score的分数越高就越会被内核优先杀掉。当oom_adj=-17时，oom_score将变为0，所以可以设置参数/proc/PID/oom_adj为-17禁止内核杀死该进程。

       上面的那个MySQL例子可以如下解决来降低mysql的points，降低被杀掉的可能：
              # ps aux | grep mysqld
              mysql 2196 1.6 2.1 623800 44876 ? Ssl 09:42 0:00 /usr/sbin/mysqld
              # cat /proc/2196/oom_score_adj
              0
              # echo -15 > /proc/2196/oom_score_adj

       当然了，保证某个进程不被内核杀掉可以这样操作：
               echo -17 > /proc/$PID/oom_adj
       例如防止sshd被杀，可以这样操作：
               pgrep -f "/usr/sbin/sshd" | while read PID;do echo -17 > /proc/$PID/oom_adj;done
       为了验证OOM机制的效果，我们不妨做个测试。
       首先看看我系统现有内存大小，没错96G多，物理上还要比查看的值大一些。



 
       再看看目前进程最大的有哪些，top查看，我目前只跑了两个java程序的进程，分别4.6G，再往后redis进程吃了21m，iscsi服务占了32m，gdm占了25m，其它的进程都是几M而已。






       现在我自己用C写一个叫bigmem程序，我指定该程序分配内存85G，呵呵，效果明显，然后执行后再用top查看，排在第一位的是我的bigmem，RES是物理内存，已经吃满了85G。
 
       继续观察，当bigmem稳定保持在85G一会后，内核会自动将其进程kill掉，增长的过程中没有被杀，如果不希望被杀可以执行
       pgrep -f "bigmem" | while read PID; do echo -17 > /proc/$PID/oom_adj;done
       执行以上命令前后，明显会对比出效果，就可以体会到内核OOM机制的实际作用了。
       注意，由任意调整的进程衍生的任意进程将继承该进程的 oom_score。例如：如果 sshd 进程不受 oom_killer 功能影响，所有由 SSH 会话产生的进程都将不受其影响。这可在出现 OOM 时影响 oom_killer 功能救援系统的能力。

       当然还可以通过修改内核参数禁止在内存出现OOM时采取杀掉进程的这种机制，但此时会触发kernel panic。当内存严重不足时，内核有两种选择：1.直接panic 2.杀掉部分进程，释放一些内存。通过/proc/sys/vm/panic_on_oom可以控制，当panic_on_oom为1时，直接panic，当panic_on_oom为0时内核将通过oom killer杀掉部分进程。（默认是为0的）
              # sysctl -w vm.panic_on_oom=1
              vm.panic_on_oom = 1 //1表示关闭，默认为0表示开启OOM killer
              # sysctl –p
       我们可以通过一些内核参数来调整 OOM killer 的行为，避免系统在那里不停的杀进程。比如我们可以在触发 OOM 后立刻触发 kernel panic，kernel panic 10秒后自动重启系统：
              # sysctl -w vm.panic_on_oom=1
              vm.panic_on_oom = 1
              # sysctl -w kernel.panic=10
              kernel.panic = 10
       或者：
              # echo "vm.panic_on_oom=1" >> /etc/sysctl.conf
              # echo "kernel.panic=10" >> /etc/sysctl.conf



       当然，如果需要的话可以完全不允许过度分配内存，此时也就不会出现OOM的问题（不过不推荐这样做）：
              # sysctl -w vm.overcommit_memory=2
              # echo "vm.overcommit_memory=2" >> /etc/sysctl.conf，
       vm.overcommit_memory 表示内核在分配内存时候做检查的方式。这个变量可以取到0,1,2三个值。对取不同的值时的处理方式都定义在内核源码 mm/mmap.c 的 __vm_enough_memory 函数中。
       0：当用户空间请求更多的的内存时，内核尝试估算出剩余可用的内存。此时宏为 OVERCOMMIT_GUESS，内核计算：NR_FILE_PAGES 总量+SWAP总量+slab中可以释放的内存总量，如果申请空间超过此数值，则将此数值与空闲内存总量减掉 totalreserve_pages(?) 的总量相加。如果申请空间依然超过此数值，则分配失败。
       1：当设这个参数值为1时，宏为 OVERCOMMIT_ALWAYS，内核允许超量使用内存直到用完为止，主要用于科学计算。
       2：当设这个参数值为2时，此时宏为 OVERCOMMIT_NEVER，内核会使用一个决不过量使用内存的算法，即系统整个内存地址空间不能超过swap+50%的RAM值，50%参数的设定是在overcommit_ratio中设定，内核计算：内存总量×vm.overcommit_ratio/100＋SWAP 的总量，如果申请空间超过此数值，则分配失败。vm.overcommit_ratio 的默认值为50。
       以上为粗略描述，在实际计算时，如果非root进程，则在计算时候会保留3%的空间，而root进程则没有该限制。详细过程可看源码。





找出最有可能被 OOM Killer 杀掉的进程：
我们知道了在用户空间可以通过操作每个进程的 oom_adj 内核参数来调整进程的分数，这个分数也可以通过 oom_score 这个内核参数看到，比如查看进程号为981的 omm_score，这个分数被上面提到的 omm_score_adj 参数调整后（-15），就变成了3：
# cat /proc/981/oom_score
18
# echo -15 > /proc/981/oom_score_adj
# cat /proc/981/oom_score
3
下面这个 bash 脚本可用来打印当前系统上 oom_score 分数最高（最容易被 OOM Killer 杀掉）的进程：

#!/bin/bash
for proc in $(find /proc -maxdepth 1 -regex '/proc/[0-9]+'); do
printf "%2d %5d %s\n" \
"$(cat $proc/oom_score)" \
"$(basename $proc)" \
"$(cat $proc/cmdline | tr '\0' ' ' | head -c 50)"
done 2>/dev/null | sort -nr | head -n 10

# chmod +x oomscore.sh
# ./oomscore.sh
18 981 /usr/sbin/mysqld
4 31359 -bash
4 31056 -bash
1 31358 sshd: root@pts/6
1 31244 sshd: vpsee [priv]
1 31159 -bash
1 31158 sudo -i
1 31055 sshd: root@pts/3
1 30912 sshd: vpsee [priv]
1 29547 /usr/sbin/sshd –D

注意：
1.Kernel-2.6.26之前版本的oomkiller算法不够精确，RHEL6.x版本的2.6.32可以解决这个问题。
2.子进程会继承父进程的oom_adj。
3.OOM不适合于解决内存泄漏(Memory leak)的问题。
4.有时free查看还有充足的内存，但还是会触发OOM，是因为该进程可能占用了特殊的内存地址空间。

参考：http://laoxu.blog.51cto.com/4120547/1267097
http://blog.chinaunix.net/uid-26490154-id-3063309.html
http://www.linuxeye.com/Linux/2119.html
http://www.vpsee.com/2013/10/how-to-configure-the-linux-oom-killer/
https://access.redhat.com/documentation/zh-CN/Red_Hat_Enterprise_Linux/6/html/Performance_Tuning_Guide/s-memory-captun.html
http://blog.sina.com.cn/s/blog_7429b9c801012evk.html
http://skyou.blog.51cto.com/2915693/558461
http://c.biancheng.net/cpp/html/2827.html
http://blog.jobbole.com/45748/



附我的吃内存程序：
#include <stdlib.h>
#include <stdio.h>
int main(){
while(1){
malloc(1);
}
return 0;
}
oom_score为2的n次方计算出来的，其中n就是进程的oom_adj值，oom_score的分数越高就越会被内核优先杀掉。当oom_adj=-17时，oom_score将变为0，所以可以设置参数/proc/PID/oom_adj为-17禁止内核杀死该进程。

 

 

上面的那个MySQL例子可以如下解决来降低mysql的points，降低被杀掉的可能：

# ps aux | grep mysqld

mysql    2196  1.6 2.1 623800 44876 ?        Ssl  09:42  0:00 /usr/sbin/mysqld

# cat /proc/2196/oom_score_adj

0

# echo -15 > /proc/2196/oom_score_adj

 

当然了，保证某个进程不被内核杀掉可以这样操作：

    echo -17> /proc/$PID/oom_adj

例如防止sshd被杀，可以这样操作：

    pgrep-f "/usr/sbin/sshd" | while read PID;do echo -17 > /proc/$PID/oom_adj;done

为了验证OOM机制的效果，我们不妨做个测试。

首先看看我系统现有内存大小，没错96G多，物理上还要比查看的值大一些。

再看看目前进程最大的有哪些，top查看，我目前只跑了两个java程序的进程，分别4.6G，再往后redis进程吃了21m，iscsi服务占了32m，gdm占了25m，其它的进程都是几M而已。



现在我自己用C写一个叫bigmem程序，我指定该程序分配内存85G，呵呵，效果明显，然后执行后再用top查看，排在第一位的是我的bigmem，RES是物理内存，已经吃满了85G。



继续观察，当bigmem稳定保持在85G一会后，内核会自动将其进程kill掉，增长的过程中没有被杀，如果不希望被杀可以执行

pgrep -f "bigmem" | while read PID; do echo -17 > /proc/$PID/oom_adj;done

执行以上命令前后，明显会对比出效果，就可以体会到内核OOM机制的实际作用了。

注意，由任意调整的进程衍生的任意进程将继承该进程的 oom_score。例如：如果 sshd 进程不受 oom_killer 功能影响，所有由 SSH 会话产生的进程都将不受其影响。这可在出现 OOM 时影响 oom_killer 功能救援系统的能力。

 

当然还可以通过修改内核参数禁止在内存出现OOM时采取杀掉进程的这种机制，但此时会触发kernel panic。当内存严重不足时，内核有两种选择：1.直接panic 2.杀掉部分进程，释放一些内存。通过/proc/sys/vm/panic_on_oom可以控制，当panic_on_oom为1时，直接panic，当panic_on_oom为0时内核将通过oomkiller杀掉部分进程。（默认是为0的）

# sysctl -wvm.panic_on_oom=1

vm.panic_on_oom = 1   //1表示关闭，默认为0表示开启OOM killer

# sysctl –p

我们可以通过一些内核参数来调整 OOM killer 的行为，避免系统在那里不停的杀进程。比如我们可以在触发 OOM 后立刻触发 kernel panic，kernel panic 10秒后自动重启系统：

# sysctl -w vm.panic_on_oom=1

vm.panic_on_oom = 1

# sysctl -w kernel.panic=10

kernel.panic = 10

或者：

# echo"vm.panic_on_oom=1" >> /etc/sysctl.conf

# echo "kernel.panic=10">> /etc/sysctl.conf

 

 

 

当然，如果需要的话可以完全不允许过度分配内存，此时也就不会出现OOM的问题（不过不推荐这样做）：

# sysctl -w vm.overcommit_memory=2

# echo "vm.overcommit_memory=2" >> /etc/sysctl.conf，

vm.overcommit_memory表示内核在分配内存时候做检查的方式。这个变量可以取到0,1,2三个值。对取不同的值时的处理方式都定义在内核源码 mm/mmap.c 的 __vm_enough_memory 函数中。

0：当用户空间请求更多的的内存时，内核尝试估算出剩余可用的内存。此时宏为 OVERCOMMIT_GUESS，内核计算：NR_FILE_PAGES 总量+SWAP总量+slab中可以释放的内存总量，如果申请空间超过此数值，则将此数值与空闲内存总量减掉 totalreserve_pages(?) 的总量相加。如果申请空间依然超过此数值，则分配失败。

1：当设这个参数值为1时，宏为OVERCOMMIT_ALWAYS，内核允许超量使用内存直到用完为止，主要用于科学计算。

2：当设这个参数值为2时，此时宏为OVERCOMMIT_NEVER，内核会使用一个决不过量使用内存的算法，即系统整个内存地址空间不能超过swap+50%的RAM值，50%参数的设定是在overcommit_ratio中设定，内核计算：内存总量×vm.overcommit_ratio/100＋SWAP 的总量，如果申请空间超过此数值，则分配失败。vm.overcommit_ratio的默认值为50。

以上为粗略描述，在实际计算时，如果非root进程，则在计算时候会保留3%的空间，而root进程则没有该限制。详细过程可看源码。

 

 

 

 

 

找出最有可能被OOM Killer 杀掉的进程：

       我们知道了在用户空间可以通过操作每个进程的 oom_adj 内核参数来调整进程的分数，这个分数也可以通过 oom_score 这个内核参数看到，比如查看进程号为981的 omm_score，这个分数被上面提到的 omm_score_adj 参数调整后（-15），就变成了3：

              #cat /proc/981/oom_score

              18

              # echo -15 >/proc/981/oom_score_adj

              # cat /proc/981/oom_score

              3

       下面这个 bash 脚本可用来打印当前系统上 oom_score 分数最高（最容易被 OOM Killer 杀掉）的进程：

 

              #!/bin/bash

              for proc in $(find /proc -maxdepth1 -regex '/proc/[0-9]+'); do

                    printf "%2d %5d %s\n" \

                    "$(cat $proc/oom_score)" \

                     "$(basename $proc)" \

                      "$(cat $proc/cmdline | tr '\0' ' ' | head-c 50)"

              done 2>/dev/null | sort -nr |head -n 10

 

              # chmod +x oomscore.sh

              # ./oomscore.sh

              18  981 /usr/sbin/mysqld

              4 31359 -bash

              4 31056 -bash

              1 31358 sshd: root@pts/6

              1 31244 sshd: vpsee [priv]

              1 31159 -bash

              1 31158 sudo -i

              1 31055 sshd: root@pts/3

              1 30912 sshd: vpsee [priv]

              1 29547 /usr/sbin/sshd –D

 

注意：

1.Kernel-2.6.26之前版本的oomkiller算法不够精确，RHEL6.x版本的2.6.32可以解决这个问题。

2.子进程会继承父进程的oom_adj。

3.OOM不适合于解决内存泄漏(Memory leak)的问题。

4.有时free查看还有充足的内存，但还是会触发OOM，是因为该进程可能占用了特殊的内存地址空间。

 

参考：http://laoxu.blog.51cto.com/4120547/1267097

                  http://blog.chinaunix.net/uid-26490154-id-3063309.html

                   http://www.linuxeye.com/Linux/2119.html

                   http://www.vpsee.com/2013/10/how-to-configure-the-linux-oom-killer/

                   https://access.redhat.com/documentation/zh-CN/Red_Hat_Enterprise_Linux/6/html/Performance_Tuning_Guide/s-memory-captun.html

                   http://blog.sina.com.cn/s/blog_7429b9c801012evk.html

       http://skyou.blog.51cto.com/2915693/558461

 

 

 

 

附一个吃内存程序：

#include <stdlib.h>

#include <stdio.h>

int main(){

           while(1){

                     malloc(1);

           }

           return0;

}
