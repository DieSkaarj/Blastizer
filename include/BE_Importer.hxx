#ifndef _BE_IMPORTER_H
#define _BE_IMPORTER_H

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>

#include <Magick++.h>

#include "BE_Palette.hxx"
#include "BE_ColorPicker.hxx"

typedef std::vector< int > color_value_t;

namespace SYSTEM{
	namespace MD{
		inline color_value_t VALUE{ 0,52,87,116,144,172,206,255 };
	};
};

class BE_Import{
	typedef struct{
		uint16_t	isEqual:1,
					isNormal:1,
					isDithered:2,
					paletteSize:12;
	} quantize_t;

	static void	browseCB( Fl_Widget *TWidget,void *TVPtr );
	static void	colorCB( Fl_Widget *TWidget,void *TVPtr );
	static void	exportCB( Fl_Widget *TWidget,void *TVPtr );
	static void	updatePreviewCB( Fl_Widget *TWidget,void *TVPtr );

	Fl_Double_Window				*m_window_ptr;
	Fl_Input						*m_name_ptr,*m_source_ptr;
	Fl_Button						*m_browse_ptr,*m_transparency_ptr,
									*m_export_ptr;
	Fl_Check_Button					*m_dither_ptr,*m_normal_ptr,
									*m_equal_ptr;
	Fl_Choice						*m_dither_choice_ptr;
	Fl_Box							*m_preview_ptr;
	Fl_Group						*m_colors_ptr;

	std::shared_ptr< BE_Palette >	m_palette_ptr;
	std::vector< short >			m_palette_idx;
	palette_t						colorMap;
	BE_Color						m_color;	
	int								m_transparency_color;

	Magick::Image					m_magickImage,m_colorMap;
	Fl_Image						*p_flImage;

	Fl_RGB_Image*					fitToAspect( const Fl_Image*,const Fl_Box*,const int border );
	Fl_RGB_Image*					generateMap( size_t W,size_t H,const color_value_t );
	Fl_RGB_Image*					magickToFl( Magick::Image &Image );
	Fl_RGB_Image*					indexToFl( std::vector< short > &,int W,int H );
	Magick::Image					generateImage( const Magick::Image,const quantize_t );
	std::vector< short >			indexImage( const Fl_Image* );
	
	void							drawPaletteFromFl( const Fl_Image* );
	void							exportImage();
//	void							swapColor( BE_Color In,BE_Color Out );
	void							swapColor( int Swatch,BE_Color Out );
	void							updatePaletteGrp(  );
	void							updatePreview( const Magick::Image& Image );
	void							setTransparencyIndex( int IDx );
	
	public:

	BE_Import():
	m_transparency_color( 0 ){
		int		winWd{ 600 },winHt{ 360 },
				x{ 125 },y{ 45 },w{ 150 },h{ 20 },
				lblX{ 25 },lblW{ 100 },
				border{ 10 },borderOffset{ border<<1 },
				borderW{ ( winWd/2 )-borderOffset };

		auto label=[]( int X,int Y,int W,int H,const char* Title ){
			Fl_Box* label=new Fl_Box( X,Y,W,H,Title );
			label->align( FL_ALIGN_LEFT|FL_ALIGN_INSIDE );
 			
			return label;
		};

		auto create_border=[]( int X,int Y,int W,int H,const char* Title=0 ){
			Fl_Box* border=new Fl_Box( X,Y,W,H,Title );
//			border->align( FL_ALIGN_LEFT|FL_ALIGN_INSIDE );
			border->box( FL_ENGRAVED_FRAME );
 			
			return border;
		};

		int browseOffX{ x+w-30 },browseOffW{ 30 };

		m_window_ptr=new Fl_Double_Window( winWd,winHt,"Mega Drive/Genesis Image Converter" );
		create_border( border,25,borderW,( h<<2 )-25 );
		label( lblX,y,lblW,h,"File:" );
		m_name_ptr=new Fl_Input( x,y,w-browseOffW,h );
		m_browse_ptr=new Fl_Button( browseOffX,y,browseOffW,h,"..." );
		m_browse_ptr->callback( browseCB,this );		

//		y+=h; 
//		label( lblX,y,lblW,h,"Group name:" );
//		m_source_ptr=new Fl_Input( x,y,w,h );

		y+=( h<<1 )+( borderOffset );

		Fl_Widget* borderWgt;

		borderWgt=create_border( border,y-20,borderW,h+( h<<2 ) );
		label( lblX,y,lblW,h,"Dithering:" );
		m_dither_choice_ptr=new Fl_Choice( x,y,w,h );
		
		m_dither_choice_ptr->add( "None" );
		m_dither_choice_ptr->add( "Floyd\\/Steinberg" );
		m_dither_choice_ptr->value( 0 );
		m_dither_choice_ptr->callback( updatePreviewCB,this );
/*
		m_dither_ptr=new Fl_Check_Button( x,y,w,h );
		m_dither_ptr->callback( updatePreviewCB,this );
*/
		x+=( w-h ); w=h;

		y+=h;
		label( lblX,y,lblW,h,"Normalize:" );
		m_normal_ptr=new Fl_Check_Button( x,y,w,h );
		m_normal_ptr->callback( updatePreviewCB,this );

		y+=h;
		label( lblX,y,lblW,h,"Equalize:" );
		m_equal_ptr=new Fl_Check_Button( x,y,w,h );
		m_equal_ptr->callback( updatePreviewCB,this );

		borderWgt=create_border( border,borderWgt->y()+borderWgt->h()+5,borderW,( h<<2 )-border );
		m_colors_ptr=new Fl_Group( borderWgt->x()+10,borderWgt->y()+10,borderWgt->w()-20,borderWgt->h()-20 );
		m_colors_ptr->box( FL_EMBOSSED_BOX );
		m_colors_ptr->color( FL_LIGHT1 );

		y=m_colors_ptr->y()+18; x=m_colors_ptr->x()+12;

		int counter{ 0 };

		while( counter<16 ){
			Fl_Button *colorBtn{ new Fl_Button( x+( ( counter++ )*15 ),y,12,12 ) };
			colorBtn->callback( colorCB,this );
			colorBtn->box( FL_OVAL_BOX );
			colorBtn->clear_visible_focus();
			colorBtn->align( FL_ALIGN_TOP );
		}

		m_colors_ptr->end();
		
//		setTransparencyIndex( 1 );

		w=100; x=110; h=50; y=280;
		m_export_ptr=new Fl_Button( x,y,w,h,"Export" );
		m_export_ptr->callback( exportCB,this );

		// Move to the other half of the window
		x=winWd*0.5f; y=20; w=x-border; h=winHt-( borderOffset<<1 );
		m_preview_ptr=new Fl_Box( x,y,w,h );
		m_preview_ptr->box( FL_EMBOSSED_BOX );
		m_preview_ptr->color( FL_DARK1 );

		m_window_ptr->end();
//		m_window_ptr->set_modal();

		p_flImage=generateMap( 16,32,SYSTEM::MD::VALUE );
		m_colorMap=Magick::Image( 16,32,"RGBA",Magick::CharPixel,*&p_flImage->data()[0] );

		m_palette_ptr=BE_Palette::create( palette_t(0) );
		// Test
		
		m_window_ptr->show();

		while( m_window_ptr->shown() ) Fl::wait();
	}

};

#endif//_BE_IMPORTER_H
