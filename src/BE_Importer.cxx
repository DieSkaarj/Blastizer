#include <cstdlib>

#include "BE_Importer.hxx"
#include "BE_Color.hxx"
#include "BE_ColorPicker.hxx"

static palette_t mapPalette;

/*
 * Widget Callbacks
 */

void BE_Import::browseCB( Fl_Widget *TWidget,void *TVPtr ){
	BE_Import &importWgt{ *static_cast< BE_Import * >( TVPtr ) };
	Magick::Image tmpImg;
	Fl_Native_File_Chooser chooser{ Fl_Native_File_Chooser::BROWSE_FILE };
	chooser.show();

	if( chooser.filename()!=0 ){
		try{
			tmpImg.ping( chooser.filename() );
		}catch( Magick::Exception &Error ){
			std::cerr << "Caught exception: " << Error.what() << '\n';
			return;
		}
	}

	importWgt.m_magickImage.read( chooser.filename() );	
	importWgt.m_name_ptr->value( chooser.filename() );
	importWgt.updatePreview( importWgt.m_magickImage );
}

void BE_Import::colorCB( Fl_Widget *TWidget,void *TVPtr ){

	const Fl_Group* grp{ static_cast< Fl_Group* >( TWidget->parent() ) };
	BE_Import* importer{ static_cast< BE_Import* >( TVPtr ) };

	BE_Color color( 0 );
	int idx{ grp->find( TWidget ) };

	if( static_cast< size_t >( idx ) < importer->m_palette_ptr->size() )
		color=importer->m_palette_ptr->at( idx );

	const int pIdx{ BE_ColorPicker::popUp( Fl::event_x_root(),Fl::event_y_root(),&mapPalette,&color ) };

	BE_Color &c{ mapPalette.at( pIdx ) };

	TWidget->color( fl_rgb_color( c.red,c.green,c.blue ) );

	const int palBtnIdx{ static_cast< Fl_Group* >( TWidget->parent() )->find( TWidget ) };

	importer->swapColor( color,c );
	importer->m_palette_ptr->at( palBtnIdx )=c;

	TWidget->parent()->redraw();
}

void BE_Import::exportCB( Fl_Widget *TWidget,void *TVPtr ){
	BE_Import* self{ static_cast< BE_Import *>( TVPtr ) };
	self->exportImage();
}

void BE_Import::updatePreviewCB( Fl_Widget *TWidget,void *TVPtr ){
	BE_Import* self{ static_cast< BE_Import *>( TVPtr ) };

	self->updatePreview( self->m_magickImage );
}

/*
 *	Class Functions
 */

Fl_RGB_Image* BE_Import::fitToAspect( const Fl_Image* Image,const Fl_Box *Box,const int Border ){

	size_t	frmW{ static_cast< size_t >( Box->w()-( Border<<1 ) ) },
				frmH{ static_cast< size_t >( Box->h()-( Border<<1 ) ) },
				imgW{ static_cast< size_t >( Image->w() ) },
				imgH{ static_cast< size_t >( Image->h() ) },
				w{ imgW },h{ imgH };

	float		frm_aspect{ static_cast< float >( frmW )/static_cast< float >( frmH ) },
				img_aspect{ static_cast< float >( imgW )/static_cast< float >( imgH ) };

	if( img_aspect>frm_aspect ){
		w=frmW; h=imgH*static_cast< float >( frmW )/static_cast< float >( imgW ) ;
	}else if( img_aspect<=frm_aspect ){
		w=imgW*static_cast< float >( frmH )/static_cast< float >( imgH ); h=frmH;
	}

	return reinterpret_cast< Fl_RGB_Image* >( Image->copy( w,h ) );
}

Fl_RGB_Image* BE_Import::generateMap( const size_t W,const size_t H,const std::vector< int > Value ){
	size_t	rows{ H },columns{ W },
				size{ rows*columns },
				len{ Value.size() };
	uint8_t depth{ 4 };

	BE_Color *newData{ new BE_Color[ size ] },*newDataIter{ newData };

	size_t ir{ 0 },ig{ 0 },ib{ 0 },counter{ 0 };

	mapPalette.clear();

	while( counter < size ){
		BE_Color &blastedColor{ *( newDataIter++ ) };
		
		blastedColor.red=SYSTEM::MD::VALUE.at( ir );
		blastedColor.green=SYSTEM::MD::VALUE.at( ig );
		blastedColor.blue=SYSTEM::MD::VALUE.at( ib );
		blastedColor.alpha=255;

		mapPalette.push_back( blastedColor );

		if( ++ir >=len ){ ir=0; ++ig; }
		if( ig>=len ){ ig=0; ++ib; }
		if( ib>=len ){ ib=0; }

		++counter;
	}

	return new Fl_RGB_Image( reinterpret_cast< const uint8_t * >( newData ),columns,rows,depth );
}

Fl_RGB_Image* BE_Import::magickToFl( Magick::Image &Image ){
	size_t	rows{ Image.rows() },columns{ Image.columns() },
				size{ rows*columns };
	uint8_t depth{ 4 };

	BE_Color *newData{ new BE_Color[ size ] },*newDataIter{ newData };

	for( size_t iy{ 0 };iy<rows;++iy ){
		for( size_t ix{ 0 };ix<columns;++ix ){
			Magick::Color magickColor{ Image.pixelColor( ix,iy ) };
			BE_Color &blastedColor{ *( newDataIter++ ) };

			blastedColor.red=magickColor.quantumRed()/256;
			blastedColor.green=magickColor.quantumGreen()/256;
			blastedColor.blue=magickColor.quantumBlue()/256;
			blastedColor.alpha=255;
		}		
	}

	return new Fl_RGB_Image( reinterpret_cast< const uint8_t * >( newData ),columns,rows,depth );
}

Magick::Image BE_Import::generateImage( const Magick::Image Image, const quantize_t QuantizeInfo ){
	Magick::Image newImage( Image );

	if( QuantizeInfo.isEqual ) newImage.equalize();
	if( QuantizeInfo.isNormal ) newImage.normalize();

	newImage.quantizeColors( QuantizeInfo.paletteSize );
	newImage.map( m_colorMap,QuantizeInfo.isDithered );

	newImage.quantize();

	m_palette_ptr->clear();
	return newImage;
}

void BE_Import::drawPaletteFromFl( const Fl_Image* FlImage ){
	if( nullptr==FlImage ) return;

	size_t	i{ static_cast< size_t >( 0 ) },
				j{ static_cast< size_t >( FlImage->w()*FlImage->h() ) };
	const BE_Color* data{ reinterpret_cast< const BE_Color* >( *FlImage->data() ) };

	if( nullptr==data ) return;
	
	m_palette_ptr->clear();

	while( i<j ){
		const BE_Color &color{ data[ i++ ] };

		if( false==m_palette_ptr->index( color ) )
			m_palette_ptr->push_back( color );
	}
}

void BE_Import::exportImage(){

	Fl_Native_File_Chooser chooser{ Fl_Native_File_Chooser::BROWSE_SAVE_FILE };
	chooser.show();

	if( m_magickImage.columns()>0 && m_magickImage.rows()>0 ){

		Magick::Image imgSave( p_flImage->w(),p_flImage->h(),"RGBA",Magick::CharPixel,*&p_flImage->data()[0] );
		imgSave.write( chooser.filename() );
	}
}

void BE_Import::swapColor( BE_Color In,BE_Color Out ){
	int w{ p_flImage->w() },h{ p_flImage->h() },size{ w*h };
	const BE_Color* imgData{ reinterpret_cast< const BE_Color* >( p_flImage->data()[0] ) };

	BE_Color *newData{ new BE_Color[ size ] },
				*newIter{ newData };

	for( int i{ 0 };i<size;++i ){
		const BE_Color color{ imgData[ i ] };

		if( color==In ){
			memcpy( &newIter[ i ],&Out,sizeof( BE_Color ) );
		}
		else
			memcpy( &newIter[ i ],&color,sizeof( BE_Color ) );
	}

//	p_flImage->release();
	p_flImage=new Fl_RGB_Image( reinterpret_cast< const uint8_t* >( newData ),w,h,4 );

	Fl_Image *copy=fitToAspect( p_flImage,m_preview_ptr,10 );

	if( m_preview_ptr->image() ) m_preview_ptr->image()->release();
	m_preview_ptr->image( copy );

	m_preview_ptr->redraw();

//	updatePaletteGrp();
}

void BE_Import::updatePaletteGrp( ){
	size_t colorsSize{ static_cast< size_t >( m_colors_ptr->children() ) };

	drawPaletteFromFl( p_flImage );

	for( size_t i{ 0 };i<colorsSize;++i ){
		if( i>=m_palette_ptr->size() ){
			m_colors_ptr->child( i )->color( FL_LIGHT1 );
		}else{
			BE_Color *color{ &m_palette_ptr->at( i ) };
			m_colors_ptr->child( i )->color( fl_rgb_color( color->red,color->green,color->blue ) );
		}
	}

	m_colors_ptr->damage( FL_DAMAGE_ALL );
}

void BE_Import::updatePreview( const Magick::Image& Image ){
	/* 
	 * Extract quantization instructions from UI widgets
	 * and package them in a struct.
	 */
	quantize_t qInf{
		static_cast< bool >( m_equal_ptr->value() ),
		static_cast< bool >( m_normal_ptr->value() ),
		static_cast< bool >( m_dither_ptr->value() ),
		16
	};

	Magick::Image tmpMagick( generateImage( Image,qInf ) );

	if( p_flImage ) p_flImage->release();

	p_flImage=magickToFl( tmpMagick );

	Fl_Image *copy=fitToAspect( p_flImage,m_preview_ptr,10 );

	if( m_preview_ptr->image() ) m_preview_ptr->image()->release();
	m_preview_ptr->image( copy );

	m_preview_ptr->redraw();

	updatePaletteGrp();
}

