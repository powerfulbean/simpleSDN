a) Reused Code:
For stage one, I didn't use code from anywhere. 
It is a basic socket and fork program, also including file reading and writing.

b) Complete:
Yes, I complete this stage.

c) Portable:
I think my code may not work on a big endian machine such as IBM powerPC. 
It is because that my machine is a little endian one, and the data on the network is coded based on big endian system.
Thus, my codes invoke funtions to convert big endian bytes into little endian one, 
which I think will not lead to correct result in a big endian machine.