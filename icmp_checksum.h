#pragma once

/* icmp_checksum.h */
/*
* sample code to calculate checksum of an ICMP packet.
*/

/*
* This code "USC CSci551 SP2019 Practical Project" is
* Copyright (C) 2019 by Guillermo Baltra.
* All rights reserved.
*
* This program is released ONLY for the purposes of Spring 2019 CSci551
* students who wish to use it as part of their project assignments.
* Use for another other purpose requires prior written approval by
* Guillermo Baltra.
*
* Use in CSci551 is permitted only provided that ALL copyright notices
* are maintained and that this code is distinguished from new
* (student-added) code as much as possible.  We new services to be
* placed in separate (new) files as much as possible.  If you add
* significant code to existing files, identify your new code with
* comments.
*
* As per class assignments, use of any code OTHER than this provided
* code requires explicit approval, ahead of time, by the professor.
*
*/

#ifdef __cplusplus
extern "C" {
#endif

unsigned short checksum(char *addr, short count);

#ifdef __cplusplus
};
#endif
