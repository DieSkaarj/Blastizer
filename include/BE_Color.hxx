#ifndef _BE_COLOR_H
#define _BE_COLOR_H

struct BE_Color{

	operator uint32_t() const{
		return (red<<24)|(green<<16)|(blue<<8)|alpha;
	}

	uint32_t red:8,green:8,blue:8,alpha:8;

	BE_Color( uint32_t RGBA=255 );
	BE_Color( uint8_t R,uint8_t G,uint8_t B,uint8_t A );
};

inline BE_Color::BE_Color( uint32_t RGBA ):
red( ( RGBA>>24 )&0xff ),
green( ( RGBA>>16 )&0xff ),
blue( ( RGBA>>8 )&0xff ),
alpha( ( RGBA )&0xff ){
	// Empty
}

inline BE_Color::BE_Color( uint8_t R,uint8_t G,uint8_t B,uint8_t A ):
red( R ),
green( G ),
blue( B ),
alpha( A ){
	// Empty
}

#endif//_BE_COLOR_H
