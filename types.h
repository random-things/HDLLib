/*
 * $Id: types.h,v 1.4 2005/06/20 12:14:05 cipher Exp $
 */
#ifndef _TYPES_H_
#define _TYPES_H_

#ifndef u8
	#define u8 unsigned char
#endif

#ifndef u16
	#define u16 unsigned short
#endif

#ifndef u32
	#define u32 unsigned int
#endif

#ifndef s32
	#define s32 signed int
#endif

enum packet_ids { REGISTER = 0x00, GETFILE, GETCRC, GETARGS };

#endif //end of _TYPES_H_
