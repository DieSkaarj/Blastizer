#ifndef _BE_PALETTE_H
#define _BE_PALETTE_H

#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>

#include "BE_Color.hxx"

typedef std::vector< BE_Color > palette_t;

class BE_Palette : public palette_t,public std::enable_shared_from_this< BE_Palette >{

	typedef std::vector< std::shared_ptr< BE_Palette > > cache_t;
	static cache_t m_cache;

	public:

	BE_Palette( cache_t &TCache ){
		// Empty
	};

	static std::shared_ptr< BE_Palette > create( palette_t TPal=palette_t() ){
		std::shared_ptr< BE_Palette > p{ std::make_shared< BE_Palette >( BE_Palette( m_cache ) ) };

		for( auto& color:TPal ) p->push_back( color );

		m_cache.push_back( p );
		
		return p;
	}

	void clear(){ palette_t::clear(); }

	std::shared_ptr< BE_Palette > get_ptr(){ return shared_from_this(); }
	
	int index( const BE_Color& Color ){
		auto search( std::find( begin(),end(),Color ) );
		
		if( search!=end() ) return 1+( std::distance( begin(),search ) );
		else return 0;
	}
};

inline BE_Palette::cache_t BE_Palette::m_cache;

#endif//_BE_PALETTE_H
