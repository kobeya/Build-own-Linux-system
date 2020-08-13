/*
 *	(GNU v3.0) kobey 2020年08月12日09:06:33
 *	【进程间通讯——socket】
 *
 *	【参考文献】
 *		1，https://www.cnblogs.com/wangcq/p/3520400.html
 *		2，https://www.jianshu.com/p/cd801d1b3147
 *		3，https://www.cnblogs.com/zengzy/p/5107516.html
 *		4, https://zhuanlan.zhihu.com/p/180556309(这个帖子暂时没看，有谈到抓包的描述)
 *
 *		收藏一些好文:
 *		关于TCP/IP更好的博客:
 *		1，https://blog.csdn.net/russell_tao/article/details/9111769
 *		上面一篇收藏博主的收费教学，陶辉，关于<系统性能调优>，他居然推荐<性能之巅>
 *		2，https://blog.csdn.net/russell_tao/article/details/105797166
 *		也是关于性能优化的博主，先收藏着吧
 *		3,https://blog.csdn.net/zxcc1314/article/details/85862871
 *
 *	【词汇概念】
 *		1，TCP Transmission Control Protocol传输控制协议，还有一个TCP的改进版本STCP。
 *			TCP是一种面向连接的、可靠的、基于字节流的传输层通信协议。
 *			可靠性机制：
 *			① 通信双方3次握手建立通信，UDP就两次握手(请求，回复)
 *			② 数据分段，MTU
 *			③ 为了保证不丢包，给报文编序，就算收到的报文乱序，TCP也会重新排序
 *			④ 收到报文，会回复一个ACK，如果在往返时延RTT内没有收到确认，重传
 *			⑤ 用校验和函数来检验报文段是否出错，出错就丢包，不回复ACK
 *			⑥ TCP还能提供流量控制：缓冲区（发送缓冲区、接收缓冲区）和滑动窗口
 *			⑦ 在拥塞控制上，采用慢启动和拥塞避免算法
 *
 *		2，IP  Internet Protocol网络间协议
 *			以上是一个工业标准的协议栈，为WANs设计的。
 *		3，UDP User Data Protocol用户数据报协议，与TCP一样做传输，属于TCP/IP协议族的一员。
 *			还有一个DCCP，用来取代UDP的新的非可靠传输协议。
 *			UDP是一种无连接的协议，提供面向事务的简单不可靠信息传送服务，不建立会话，
 *			不对数据分段，不编号不排序，也就是说，当报文发送之后，是无法得知其是否安
 *			全完整到达的。但正因为UDP协议的控制选项较少，在数据传输过程中延迟小、
 *			数据传输效率高，适合对可靠性要求不高的应用程序，
 *			或者可以保障可靠性的应用程序。
 *			常用的UDP端口号有：DNS 53，TFTP 69，SNMP 161
 *		4，OSI七层模型（Open System Interconnect）：
 *			应用层、表示层、会话层、传输层、网络层、数据链路层、物理层
 *		5，TCP滑动窗口机制
 *			https://baijiahao.baidu.com/s?id=1650553452294041970&wfr=spider&for=pc
 *			https://baijiahao.baidu.com/s?id=1658482402006954335&wfr=spider&for=pc
 *			https://www.cnblogs.com/coder-programming/p/10627746.html
 *			暂时没有时间去深入，简单的说，就是TCP传输的数据，就跟滑到窗口一样，
 *			一个一个的传输，像手机滑动窗口一样。
 *
 *
 *
 *	【背景小知识】
 *		1，使用TCP/IP协议的应用程序通常采用应用编程接口：
 *			UNIX System V的TLI（已经被淘汰)；
 *			UNIX BSD的套接字（socket）来实现网络进程之间的通信。	
 *		2，socket词汇的起源：
 *			socket起源于Unix，而Unix/Linux基本哲学之一就是“一切皆文件”，
 *			都可以用“打开open –> 读写write/read –> 关闭close”模式来操作。
 *			目前理解Socket就是该模式的一个实现，socket即是一种特殊的文件，
 *			一些socket函数就是对其进行的操作（读/写IO、打开、关闭）。
 *			|
 *			在组网领域的首次使用是在1970年2月12日发布的文献IETF RFC33中发现的，
 *			撰写者为Stephen Carr、Steve Crocker和Vint Cerf。
 *			根据美国计算机历史博物馆的记载，Croker写道：“命名空间的元素都可
 *			称为套接字接口。一个套接字接口构成一个连接的一端，而一个连接可完全由
 *			一对套接字接口规定。”
 *			计算机历史博物馆补充道：“这比BSD的套接字接口定义早了大约12年。”
 *		
 *		3，网络字节序与主机字节序：
 *			主机字节序，就是我们常说的大小端。
 *			Little-Endian就是低位字节排放在内存的低地址端，高位字节排放在内存的高地址端。
 *			Big-Endian就是高位字节排放在内存的低地址端，低位字节排放在内存的高地址端。
 *
 *			网络字节序，4个字节的32 bit值以下面的次序传输：
 *			首先是0～7bit，
 *			其次8～15bit，
 *			然后16～23bit，
 *			最后是24~31bit。
 *			这种传输次序称作大端字节序。
 *			由于TCP/IP首部中所有的二进制整数在网络中传输时都要求以这种次序，
 *			因此它又称作网络字节序。字节序，顾名思义字节的顺序，
 *			就是大于一个字节类型的数据在内存中的存放顺序，一个字节的数据没有顺序的问题。
 *
 *			所以： 在将一个地址绑定到socket的时候，请先将主机字节序转换成为网络字节序，
 *			而不要假定主机字节序跟网络字节序一样使用的是Big-Endian。
 *			由于 这个问题曾引发过血案！公司项目代码中由于存在这个问题，导致了很多莫名
 *			其妙的问题，所以请谨记对主机字节序不要做任何假定，务必将其转化为网络节序
 *			再赋给socket。
 *	
 *		4，端口分类：
 *			0~1023，管理员才有权限使用，永久地分配给对应的应用端口使用
 *			1024~41951，只有一部分被注册，分配原则上非特别严格
 *			41952+，动态端口或者私有端口
 *			  /proc/sys/net/ipv4/ip_local_port_range
 *			  定义两个数字，表示可以做为临时端口的起始数字和结束数字
 *			  比如：kobey@pc:~$ cat /proc/sys/net/ipv4/ip_local_port_range 
 *			  32768	60999
 *
 *		5，SYN攻击：
 *			在三次握手过程中，Server发送SYN-ACK之后，收到Client的ACK之前的TCP连接称为
 *			半连接（half-open connect），此时Server处于SYN_RCVD状态，当收到ACK后，
 *			Server转入ESTABLISHED状态。SYN攻击就是Client在短时间内伪造大量不存在的
 *			IP地址，并向Server不断地发送SYN包，Server回复确认包，并等待Client的确认，
 *			由于源地址是不存在的，因此，Server需要不断重发直至超时，这些伪造的SYN包
 *			将长时间占用未连接队列，导致正常的SYN请求因为队列满而被丢弃，从而引起网络
 *			堵塞甚至系统瘫痪。SYN攻击时一种典型的DDOS攻击，检测SYN攻击的方式非常简单，
 *			即当Server上有大量半连接状态且源IP地址是随机的，则可以断定遭到SYN攻击了，
 *			使用如下命令可以让之现行：
 *				#netstat -nap | grep SYN_RECV
 *
 *		6，创建socket返回文件描述符的原理：
 *			就跟每个进程都拥有一个task_struct一样，每个socket也有自己的文件描述符，当进程
 *			打开一个文件时，系统就把一个指向这个文件的内部数据结构的指针，
 *			写入由系统维护的一张单独的文件描述符表，并把该表的索引值，返回给调用者。
 *			应用程序只需要记住这个描述符，并在以后操作该文件时使用它。
 *			系统把描述符作为索引访问进程描述符表，通过表中的指针找到保存该文件的所有
 *			信息的数据结构，如下图所示
 *
 *
 *				open()/create()/socket()...
 *					|
 *					V
 *				系统会把打开文件的内部数据结构的指针，保存在一个描述符表，
 *				实际上，这个表系统维护，却也保存在描述符中。
 *					|
 *					V
 *				再返回一个描述符表项的对应的索引值
 *
 *			这样，我们就可以通过这个索引值，找到打开的文件的内部数据结构。
 *
 *			注意事项：在windows上，文件描述符被称呼为文件句柄。
 *
 *	【协议层图演示】
 *
 *
 *			用户A		用户B		用户C		用户D     应用层
 *			  \              |		 |		/
 *			   \		 |		 |	       /
 *			    \		 |		 |	      /
 *			---------------------------------------------------	
 *			|		socket抽象层			  |	
 *			---------------------------------------------------
 *		raw socket  |		|	 |	|
 *		应用程序数据|		TCP	 |	UDP			  运输层
 *		直接到达IP层|		\	 |	/
 *			    |		 \	 |     /
 *			  ICMP<--------->	IP	<--------> IGMP		  网络层
 *						|
 *						|
 *			ARP <--------------->硬件接口<------------->RARP	  链路层
 *						|
 *						V
 *					       媒体
 *
 * 
 *
 *	【socket是什么】
 *		socket是应用层与TCP/IP，也就是应用层与运输层和网络层通信的中间，软件抽象层，一组API。
 *		在设计模式中，socket是一个门面模式，它把复杂的TCP/IP协议族隐藏在socket接口的后面，对
 *		用户来说，所有的访问都通过一组简单的API来实现，然后让这组API去组织数据，以符合指定的
 *		协议要求。
 *
 *	【socket是如何实现进程间通讯】
 *		需要三个标识：
 *			1，网络层的ip地址
 *			2，运输层的协议
 *			3，运输层的端口
 *			比如，访问https://192.168.11.1:8080
 *
 *	【以TCP为运输协议的socket基本API】
 *		1，socket
 *			int socket(int domain, int type, int protocol);
 *			创建一个socket套接字文件，返回一个socket描述符，唯一的标识这个socket。
 *			参数
 *			① domain，协议域、协议族(family)。
 *			常用的协议族有，AF_INET、AF_INET6、AF_LOCAL（或称AF_UNIX，Unix域socket）、
 *			AF_ROUTE等等。协议族决定了socket的地址类型，在通信中必须采用对应的地址，
 *			如AF_INET决定了要用ipv4地址（32位的）与端口号（16位的）的组合、
 *			AF_UNIX决定了要用一个绝对路径名作为地址。
 *		
 *			② type，指定socket类型。
 *			SOCK_STREAM、SOCK_DGRAM、SOCK_RAW、SOCK_PACKET、SOCK_SEQPACKET等等
 *			
 *			③ protocl，指定协议,如果是0，那么就会根据参数type来决定默认的运输协议
 *			IPPROTO_TCP：TCP传输协议
 *			IPPTOTO_UDP：UDP传输协议
 *			IPPROTO_SCTP：STCP传输协议
 *			IPPROTO_TIPC：TIPC传输协议
 *				这里有个细节，既然0默认使用TCP了，这个参数protocol的到底存在的意义是
 *				什么？我翻阅了一些资料，暂时得到的结果是：比如上图描述到的raw socket
 *				不应该TCP/UDP直接到达IP网络层，此刻就需要protocol参数指定了。
 *
 *
 *			注意事项：
 *			① 并不是上面的type和protocol可以随意组合的，如SOCK_STREAM
 *			不可以跟IPPROTO_UDP组合。当protocol为0时，会自动选择type类型对应的默认协议。
 *			
 *			② 当我们调用socket创一个socket时，返回的socket描述字，存在协议族，也就是
 *			domain的空间中，没有具体的地址。我们可以使用bind()函数给他赋一个地址，否则
 *			在调用connect、listen时，系统会自动随机分配一个端口。
 *
 *			③ 系统刚创建socket的时候，内部大多数的字段是没有填写的，后续在使用这个socket
 *			之前，再调用其他的过程来填充这些字段。
 *
 *		2，bind
 *			int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
 *			bind()函数把一个地址族中的特定地址赋给socket。
 *			例如对应AF_INET、AF_INET6就是把一个ipv4或ipv6地址和端口号组合赋给socket。
 *			参数
 *			① socket描述字，它是通过socket函数创建，唯一表示一个socket。
 *
 *			② addr，const struct sockaddr *指针，指向要绑定给sockfd的协议族地址。
 *			这个地址结构根据地址创建socket时的地址协议族的不同而不同，如ipv4对应的是：
 *			struct sockaddr_in {
				sa_family_t    sin_family;
				in_port_t      sin_port;
				struct in_addr sin_addr;
 			};


			struct in_addr {
				uint32_t       s_addr;
			};
			
			ipv6对应的是：
			struct sockaddr_in6 { 
			    sa_family_t     sin6_family;    
			    in_port_t       sin6_port;      
			    uint32_t        sin6_flowinfo;  
			    struct in6_addr sin6_addr;      
			    uint32_t        sin6_scope_id;  
			};

			struct in6_addr { 
			    unsigned char   s6_addr[16];    
			};

			Unix域对应的是：
			#define UNIX_PATH_MAX    108

			struct sockaddr_un { 
			    sa_family_t sun_family;                
			    char        sun_path[UNIX_PATH_MAX];   
			};

			③ addrlen 对应的地址长度。
 *			
 *			注意事项：
 *			在建立通信的时候，前面我们知道，客户端需要发送协议类型+ip地址+端口号
 *			去访问服务器。客户端本身是不需要指定这三个条件的，因为在connect的时候，
 *			系统自动随机生成一个。
 *
 *		3，listen
 *			int listen(int sockfd, int backlog);
 *			listen函数的第一个参数即为要监听的socket描述字
 *			第二个参数为相应socket可以排队的最大连接个数
 *			socket()函数创建的socket默认是一个主动类型的，listen函数将socket
 *			变为被动类型的，等待客户的连接请求。
 *
 *		4，connect
 *			int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
 *			第一个参数即为客户端的socket描述字
 *			第二参数为服务器的socket地址
 *			第三个参数为socket地址的长度
 *			客户端通过调用connect函数来建立与TCP服务器的连接。
 *
 *		5，accept
 *			int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
 *			client发送connect请求，
 *			TCP服务器监听到这个请求之后，就会调用accept()函数取接收请求，这样连接就建立，
 *			之后就可以开始网络I/O操作了，即类同于普通文件的读写I/O操作。
 *			第一个参数为服务器的socket描述字
 *			第二个参数为指向struct sockaddr *的指针，用于返回客户端的协议地址
 *			第三个参数为协议地址的长度
 *			如果accpet成功，那么其返回值是由内核自动生成的一个全新的描述字，
 *			代表与返回客户的TCP连接。
 *
 *			注意事项：
 *			accept的第一个参数为服务器的socket描述字，是服务器开始调用socket()函数
 *			生成的，称为监听socket描述字；而accept函数返回的是已连接的socket描述字。
 *			一个服务器通常仅仅只创建一个监听socket描述字，它在该服务器的生命周期内
 *			一直存在。内核为每个由服务器进程接受的客户连接创建了一个已连接socket描
 *			述字，当服务器完成了对某个客户的服务，相应的已连接socket描述字就被关闭。
 *
 *		6，read()、write()等函数
 *			服务器与客户已经建立好连接了。
 *			可以调用网络I/O进行读写操作了，即实现了网咯中不同进程之间的通信！
 *			网络I/O操作有下面几组：
 *			read()/write()
 *			recv()/send()
 *			readv()/writev()
 *			recvmsg()/sendmsg() --->推荐使用
 *			recvfrom()/sendto()
 *
 *			推荐使用recvmsg()/sendmsg()函数，这两个函数是最通用的I/O函数，
 *			实际上可以把上面的其它函数都替换成这两个函数，可以使用man手册去查看。
 *
 *		7，close()函数
 *			int close(int fd);
 *
 *			注意事项：
 *			close操作只是使相应socket描述字的引用计数-1，只有当引用计数为0的时候，
 *			才会触发TCP客户端向服务器发送终止连接请求。
 *
 *
 *	【TCP三次握手，建立连接】
 *		
 *
 *				client                            server
 *                       socket   |                                  |socket->bind->listen	
 *		   connect(阻塞)  |--------------SYN=J-------------->|Accept(阻塞)
 *		主动打开，请求连接|                                  |被动打开，接受连接
 *				  |                                  ||
 *				  |                                  ||
 *				  |                                  ||
 *		      connect返回 |<----响应的SYN=K,确认的ACK=J+1----|<
 *				 ||                                  |
 *				 >|----------确认ACK=K+1------------>|Accept返回
 *				  |			             |
 *
 *
 *		文字描述三次握手的过程：
 *			位码即tcp标志位，有6种标示：
 *			① SYN(synchronous建立联机)
 *			② ACK(acknowledgement 确认) 
 *			③ PSH(push传送)
 *			④ FIN(finish结束)
 *			⑤ RST(reset重置)
 *			⑥ URG(urgent紧急)
 *			⑦ Sequence number(顺序号码)？
 *
 *
 *			从图中可以看出，当客户端调用connect时，触发了连接请求，向服务器发送了
 *			SYN J包，这时connect进入阻塞状态；
 *			服务器监听到连接请求，即收到SYN J包，调用accept函数接收请求向客户端
 *			发送SYN K ，ACK J+1，这时accept进入阻塞状态；
 *			客户端收到服务器的SYN K ，ACK J+1之后，这时connect返回，
 *			并对SYN K进行确认；
 *			服务器收到ACK K+1时，accept返回，
 *			至此三次握手完毕，连接建立。
 *
 *			回忆一下，我曾经看过一本有意思的书籍<码农翻身，用故事给技术加点料>，
 *			书籍的前面就使用了王国建立邮件寄送系统的故事。
 *			印象中大概是这样描述的：
 *			你想邮件一个包裹，在寄送之前，先建立通信，
 *			① 邮差先给目的地发一条信息，告诉目的地，想建立通信，此刻，邮差不知道
 *			虽然发了信息，但是他不知道他是否发成功，他也不知道自己能否有接收能力
 *			② 目的地接收到邮差的信息了，此刻，目的地知道了自己有收发能力，并且
 *			知道邮差的发信能力正常，可是不知道
 *			自己是否有发送能力，他就给邮差回了一个邮件，还问一下自己能否发送。
 *			③ 邮差收到了目的地的信息，知道自己发送、接收的能力都正常，
 *			也知道目的地的收发能力正常，但是还是要再回复了一条
 *			信息给目的地，告诉目的地，他的发信能力正常。
 *			④ 目的地再次收到信息，确认了自己的发信能力，这样双方就建立了通信。
 *
 *	
 *
 *	【TCP四次挥手释放连接模型】
 *		
 *		一、client主动关闭，server被动关闭
 *
 *				client			  server	
 *				  |			      |	
 *			   close()|---------FIN M------------>|被动关闭
 *			主动关闭  |			      |Read返回0
 *				  |<--------ACK M+1-----------|
 *				  |<--------FIN N-------------|close主动关闭
 *				  |			      |		  |
 *				  |			      |		  V
 *				  |---------ACK N+1---------->|		这里有个过程，接受到结束符的
 *				  |    			      |		应用程序调用close关闭。
 *				  |   			      |
 *
 *			文字描述：
 *			① 某个应用程序首先调用close主动关闭连接，TCP发送一个FIN=M的包
 *				告诉server,client端没有数据发送了，server你要有心理准备，也不用急
 *				关闭，server是可以继续发送数据。
 *				client进入FIN_WAIT_1状态
 *			② 服务器收到FIN的结束码之后，先回复client，告诉client，你的请求我收到了，
 *				好的，我需要准备一下，你继续等我信息。
 *				client进入FIN_WAIT_2状态
 *			③ 一段时间后，服务器的数据发送完成，就向client端发送一个FIN=N的报文，告诉
 *				client端，好了，我的数据发完了，准备好关闭链接。
 *				server进入LAST_ACK状态
 *			④ client收到服务器的关闭报文，就知道可以关闭了，再回复server，开始关闭吧。
 *				client端进入TIME_WAIT状态，
 *				这里有个细节，server端如果没有收到ACK则会继续重传。
 *				server收到ACK后，确定断开连接，执行关闭。
 *				client在TIME_WAIT状态，等2MSL后后再也没有收到server的信息了，
 *				知道server已经关闭，那好，client也执行关闭。
 *				最终，完成四次挥手。
 *
 *		二、双方主动关闭
 *				
 *				client					server
 *				  |					    |
 *			FIN_WAIT_1|---------------FIN=M-------------------->|FIN_WAIT_1
 *			CLOSE_WAIT|<---------------FIN=N--------------------|CLOSE_WAIT
 *				  |					    | 
 *				  |<----------ACK=1,ack=M+1-----------------|
 *			 TIME_WAIT|-----------ACK=1,ack=N+1-----------------|TIME_WAIT
 *
 *
 *			注意事项：
 *			前面我因为效率的问题，只在最后双方关闭的描述中加入了client端和server端的
 *			状态码，实际上，在整个通信过程，双方都处于状态码的描述下，标志双方处于
 *			通信的哪个阶段。
 *
 *		
 *	
 *	【抓包】
 *		工具tcpdump  (windows下图形抓包工具Wireshark)
 *		使用	sudo tcpdump -iany tcp port *** //dump 任意网口与tcp的***port口通信
 *			sudo tcpdump -i eth0 udp port *** //dump eth0与udp的***port口通信
 *
 *		log分析:
 *
 *		第一次握手
 *		09:52:59.466061 IP localhost.41618 > localhost.36666: Flags [S], seq 1907759069, 
 *		win 65495, options [mss 65495,sackOK,TS val 12462626 ecr 0,nop,wscale 7], length 0
 *
 *		① 时间精准到微秒
 *		② IP localhost.41618 > localhost.36666，通信流方向
 *		③ Flags [S]
 *			[S],SYN请求
 *			[S.],SYN请求+ACK确认包
 *			[.],ACK确认包
 *			[P],数据推送
 *			[F],FIN包，关闭连接
 *			[R],RST包，与F包作用相同，差异是R包更加猛烈，理解为强制切断，不等任何数据处理
 *			win 65495,滑动窗口大小
 *			length 0,数据包的大小，这里为0
 *
 *
 * */						
		
