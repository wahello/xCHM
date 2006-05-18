/*

  Copyright (C) 2003  Razvan Cojocaru <razvanco@gmx.net>
  Most of the code in this file is a modified version of code from
  Pabs' GPL chmdeco project, credits and thanks go to him.
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/


#include <wx/string.h>


#define FIXENDIAN16(x) (x = wxUINT16_SWAP_ON_BE(x))
#define FIXENDIAN32(x) (x = wxUINT32_SWAP_ON_BE(x))
#define UINT16ARRAY(x) ((unsigned char)(x)[0] | ((u_int16_t)(x)[1] << 8))
#define UINT32ARRAY(x) (UINT16ARRAY(x) | ((u_int32_t)(x)[2] << 16) \
		| ((u_int32_t)(x)[3] << 24))


#if wxUSE_UNICODE

#	define CURRENT_CHAR_STRING(x) \
	wxString(reinterpret_cast<const char *>(x), wxConvISO8859_1)

#	define CURRENT_CHAR_STRING_CV(x, cv) \
	wxString(reinterpret_cast<const char *>(x), cv)

#else

#	define CURRENT_CHAR_STRING(x) \
	wxString(reinterpret_cast<const char *>(x))

#	define CURRENT_CHAR_STRING_CV(x, cv) \
	wxString(reinterpret_cast<const char *>(x))

#endif


inline u_int64_t be_encint(unsigned char* buffer, size_t& length)
{
	u_int64_t result = 0;
	int shift=0;
	length = 0;

	do {
		result |= ((*buffer) & 0x7f) << shift;
		shift += 7;
		++length;

	} while (*(buffer++) & 0x80);
	
	return result;
}


/* 
   Finds the first unset bit in memory. Returns the number of set bits found.
   Returns -1 if the buffer runs out before we find an unset bit.
*/
inline int ffus(unsigned char* byte, int* bit, size_t& length)
{
	int bits = 0;
	length = 0;

	while(*byte & (1 << *bit)){
		if(*bit) 
			--(*bit);
		else { 
			++byte;
			++length;
			*bit = 7; 
		}
		++bits;
	}

	if(*bit) 
		--(*bit);
	else { 
		++length;
		*bit = 7; 
	}
	
	return bits;
}


inline u_int64_t sr_int(unsigned char* byte, int* bit,
			unsigned char s, unsigned char r, size_t& length)
{
	u_int64_t ret; 
	unsigned char mask;
	int n, n_bits, num_bits, base, count;
	length = 0;
	size_t fflen;

	if(!bit || *bit > 7 || s != 2)
		return ~(u_int64_t)0;
	ret = 0;

	count = ffus(byte, bit, fflen);
	length += fflen;
	byte += length;

	n_bits = n = r + (count ? count-1 : 0) ;

	while(n > 0) {
		num_bits = n > *bit ? *bit : n-1;
		base = n > *bit ? 0 : *bit - (n-1);
		
		switch(num_bits){
			case 0: 
				mask = 1; 
				break;
			case 1: 
				mask = 3; 
				break;
			case 2: 
				mask = 7; 
				break;
			case 3: 
				mask = 0xf; 
				break;
			case 4: 
				mask = 0x1f; 
				break;
			case 5: 
				mask = 0x3f; 
				break;
			case 6: 
				mask = 0x7f; 
				break;
			case 7: 
				mask = 0xff; 
				break;
                        default:
                                mask = 0xff;
				break;
		}

		mask <<= base;
		ret = (ret << (num_bits+1)) | 
			(u_int64_t)((*byte & mask) >> base);
		
		if( n > *bit ){
			++byte;
			++length;
			n -= *bit+1;
			*bit = 7;
		} else {
			*bit -= n;
			n = 0;
		}
	}

	if(count) 
		ret |= (u_int64_t)1 << n_bits;

	return ret;
}



/*
  Local Variables:
  mode: c++
  c-basic-offset: 8
  tab-width: 8
  c-indent-comments-syntactically-p: t
  c-tab-always-indent: t
  indent-tabs-mode: t
  End:
*/

// vim:shiftwidth=8:autoindent:tabstop=8:noexpandtab:softtabstop=8

