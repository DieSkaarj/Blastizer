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

	if( Fl::event_button()==FL_LEFT_MOUSE ){
		
		const int pIdx{ BE_ColorPicker::popUp( Fl::event_x_root(),Fl::event_y_root(),&mapPalette,&color ) };

		BE_Color &c{ mapPalette.at( pIdx ) };

		TWidget->color( fl_rgb_color( c.red,c.green,c.blue ) );

		const int palBtnIdx{ static_cast< Fl_Group* >( TWidget->parent() )->find( TWidget ) };

		importer->swapColor( idx,c );
	}
	else
	if( Fl::event_button()==FL_RIGHT_MOUSE ){
			importer->setTransparencyIndex( idx );
	}

	TWidget->parent()->redraw();
}

void BE_Import::exportCB( Fl_Widget *TWidget,void *TVPtr ){
	BE_Import* self{ static_cast< BE_Import *>( TVPtr ) };
	self->exportImage();
}

void BE_Import::updatePreviewCB( Fl_Widget *TWidget,void *TVPtr ){
	BE_Import* self{ static_cast< BE_Import *>( TVPtr ) };

	if( !self->m_magickImage.rows() ) return;

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

		if( ++ib >=len ){ ib=0; ++ig; }
		if( ig>=len ){ ig=0; ++ir; }
		if( ir>=len ){ ir=0; }

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

Fl_RGB_Image* BE_Import::indexToFl( std::vector< short > &ImgData,int W,int H ){
	BE_Color	*data{ new BE_Color[ ImgData.size() ] },
				*iter{ data };
	size_t count{ 0 };

	while( count < ImgData.size() ){
		*( iter++ )=m_palette_ptr->at( ImgData.at( count++ ) );
	}
	
	return new Fl_RGB_Image( reinterpret_cast< const uint8_t * >( data ),W,H,4 );
}

Magick::Image BE_Import::generateImage( const Magick::Image Image, const quantize_t QuantizeInfo ){
	Magick::Image newImage( Image );

	if( QuantizeInfo.isEqual ) newImage.equalize();
	if( QuantizeInfo.isNormal ) newImage.normalize();

	newImage.map( m_colorMap );

	newImage.quantizeColors( QuantizeInfo.paletteSize );

	switch( QuantizeInfo.isDithered ){
		case 0: newImage.quantizeDither( false ); break;
		case 1: newImage.quantizeDither( true ); break;
	}
	
	newImage.quantize();

	m_palette_ptr->clear();
	return newImage;
}

std::vector< short > BE_Import::indexImage( const Fl_Image* FlImage){

	int len{ FlImage->w()*FlImage->h() },count{ 0 };
	const BE_Color *iteration{ reinterpret_cast< const BE_Color* >( p_flImage->data()[0] ) };
	std::vector< short > image_data;

	while( count < len ){
		const BE_Color &c{ *( iteration++ ) };
		image_data.push_back( m_palette_ptr->index( c )-1 );
		++count;
	}

	return image_data;
}

void BE_Import::drawPaletteFromFl( const Fl_Image* FlImage ){
	if( nullptr==FlImage ) return;

	size_t	i{ static_cast< size_t >( 0 ) },
				j{ static_cast< size_t >( FlImage->w()*FlImage->h() ) };
	const BE_Color* data{ reinterpret_cast< const BE_Color* >( FlImage->data()[0] ) };

	if( nullptr==data ) return;
	
	m_palette_ptr->clear();
	setTransparencyIndex( 0 );

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
/*
void BE_Import::swapColor( BE_Color In,BE_Color Out ){
	int w{ p_flImage->w() },h{ p_flImage->h() },merge{ 0 },idx{ 0 };

	// Find out if the color chosen by the user is already in the current palette
	// and save the value
	if( ( idx=m_palette_ptr->index( Out ) ) ){
		merge=fl_choice( "The new color is already indexed inside the current palette\nDo you want to merge the indices?","No","Yes",0 );
	}

	int change{ m_palette_ptr->index( In )-1 };

	// If user wants to merge the color we need to find all instances of the current
	// color and replace them with the index of the already indexed color
	if( merge ){
		for( size_t i{ 0 }; i<m_palette_idx.size(); ++i )
			if( m_palette_idx.at( i )==change ){
				m_palette_idx.at( i )=--idx;
			}
	}
	else{
		m_palette_ptr->at( change )=Out;
	}

	p_flImage=indexToFl( m_palette_idx,w,h );
	Fl_Image *copy=fitToAspect( p_flImage,m_preview_ptr,10 );

	if( m_preview_ptr->image() ) m_preview_ptr->image()->release();
	m_preview_ptr->image( copy );

	m_preview_ptr->redraw();

	updatePaletteGrp();
}
*/
void BE_Import::swapColor( int Swatch,BE_Color Out ){

	int w{ p_flImage->w() },h{ p_flImage->h() },merge{ 0 },match{ m_palette_ptr->index( Out ) };

	// Find out if the color chosen by the user is already in the current palette
	// and save the value

	if( ( match ) && ( --match ) != Swatch ){
		merge=fl_choice( "The new color is already indexed inside the current palette\nDo you want to merge the indices?","No","Yes",0 );
	}

//	freopen("log.txt","w",stdout);

	// If user wants to merge the color we need to find all instances of the current
	// color and replace them with the index of the already indexed color
	if( merge ){

		m_palette_ptr->at( Swatch )=m_palette_ptr->at( match );

		if( match>Swatch ){
			std::swap( match,Swatch );
		}

		for( auto &index:m_palette_idx ){

			if( index==Swatch ){
				index=match;
			}
			if( index>Swatch ){
				--index;
			}
		}
		
//		m_palette_ptr->erase( m_palette_ptr->begin()+Swatch );
	
		for( int i{ Swatch }; i<m_palette_ptr->size()-1; ++i ){

			m_palette_ptr->at( i )=m_palette_ptr->at( i+1 );
		}
		
		m_palette_ptr->pop_back();
/*
		std::shared_ptr< BE_Palette > new_pal{ BE_Palette::create( palette_t( m_palette_ptr->size()-1 ) ) };
		
		for( int i{ 0 }; i<new_pal->size(); ++i ){
		
				new_pal->at( i )=m_palette_ptr->at( i );
		}

		m_palette_ptr=new_pal;
*/
	}
	else{
		m_palette_ptr->at( Swatch )=Out;
	}

	p_flImage=indexToFl( m_palette_idx,w,h );
	
	Fl_Image *copy=fitToAspect( p_flImage,m_preview_ptr,10 );

	if( m_preview_ptr->image() ) m_preview_ptr->image()->release();
	m_preview_ptr->image( copy );
	m_preview_ptr->redraw();

	updatePaletteGrp();
}


void BE_Import::updatePaletteGrp( ){
	size_t colorsSize{ static_cast< size_t >( m_colors_ptr->children() ) };

	for( size_t i{ 0 };i<colorsSize;++i ){
		if( i>static_cast< size_t >( m_palette_ptr->size()-1 ) ){
			m_colors_ptr->child( i )->color( FL_BLACK );
			m_colors_ptr->child( i )->box( FL_DIAMOND_UP_BOX );
		}else{
			BE_Color &color{ m_palette_ptr->at( i ) };
			m_colors_ptr->child( i )->color( fl_rgb_color( color.red,color.green,color.blue ) );
			m_colors_ptr->child( i )->box( FL_OVAL_BOX );
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
		static_cast< uint16_t >( m_dither_choice_ptr->value() ),
		16
	};

	Magick::Image tmpMagick( generateImage( Image,qInf ) );

	if( p_flImage ) p_flImage->release();

	p_flImage=magickToFl( tmpMagick );
	
	drawPaletteFromFl( p_flImage );
	m_palette_idx=indexImage( p_flImage );

	drawPaletteFromFl( p_flImage );
	updatePaletteGrp();

	p_flImage=indexToFl( m_palette_idx,tmpMagick.columns(),tmpMagick.rows() );

	Fl_Image *copy=fitToAspect( p_flImage,m_preview_ptr,10 );

	if( m_preview_ptr->image() ) m_preview_ptr->image()->release();
	m_preview_ptr->image( copy );

	m_preview_ptr->redraw();
}

void BE_Import::setTransparencyIndex( int IDx ){

		if( IDx > ( m_palette_ptr->size()-1 ) ) IDx=m_palette_ptr->size()-1;
		if( IDx < 0 ) IDx=0;
		m_colors_ptr->child( m_transparency_color )->label( "" );
		m_transparency_color=IDx;
		m_colors_ptr->child( m_transparency_color )->label( "@-92UpArrow" );
}
