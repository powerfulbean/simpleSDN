a) Reused Code:
For stage ten, I didn't use code from anywhere. 

b) Complete:
Yes, I complete this stage.

c) Load balancing by packet or by ﬂow: 
Packets have the same size limit, so their size do not vary a lot.
The good thing is that it overcomes the problem of possible big bandwidth difference between many flows, and thus increase the performance of load balancing.

d) Load balancing by packet or by ﬂow, part II: 
The TCP connection requires the src and dst ip addresses remain the same.
But the export ip addresses of the secondary routers are different, so we need rebuild the program and allow the secondary routers to change their ip addresses to send different packet.
  
 
