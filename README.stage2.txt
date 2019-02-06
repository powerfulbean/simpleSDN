a) Reused Code:
For stage two, I used checksum code (the file is icmp_checksum.c) provided by Guillermo Baltra. 
I built a header file (icmp_checksum.h) for it, so I can isolate this code from my own code.

I also used the code about tunnel ( sample_tunnel.c ) provided by Zi Hu.
I simply seperate the tunnel_reader() function into two functions in this file( set_tunnel_reader and read_tunnel ).
I used a function (cwrite) about write to tunnel provided by Davide Brini and add it into the sample_tunnel.c file.
The related blog about cwrite is https://backreference.org/2010/03/26/tuntap-interface-tutorial/.
I built a header file (sample_tunnel.h) for sample_tunnel.c, so I can again isolate this code from my own code.

In addition, (actually I did not directly use their code) 
I learnt about how to read IP header and caculate IP header length based on codes provided by
"https://blog.csdn.net/u014634338/article/details/48951345"
"https://www.ibm.com/developerworks/cn/linux/network/ping/index.html"
I built two function based on these two blogs (icmpReply_Edit, and icmpUnpack).
I want to thank them for sharing their experience with ip packet analysis.


b) Complete:
Yes, I complete this stage.

c) Portable:
Although the IPheader struct defined in <netinet/ip.h> consider the different conditions of Big Endian and Short Endian.
I think my own code may not work on a big endian machine such as IBM powerPC. 
Just the same reason in stage one,  my code calls some funtions to convert big endian bytes into little endian one, 
which I think will not lead to correct results in a big endian machine.
