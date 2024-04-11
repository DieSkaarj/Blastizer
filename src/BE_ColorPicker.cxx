#include <FL/fl_draw.H>
#include <FL/Fl_Window.H>

#include "BE_ColorPicker.hxx"

BE_ColorPicker::BE_ColorPicker():
Fl_Window( 0,0 ),
selected( 0 ),previous( selected ),
isSelected( false ),
palette( nullptr ),
m_hexInput( 0,0,85,20 ){
	const int	sizeX{ ( columns*swatchSize )+( border<<2 ) },
					sizeY{ ( rows*swatchSize )+( ( border<<2 )+( header-border ) ) };

	size( sizeX,sizeY );
	color( FL_DARK3 );
	m_hexInput.Fl_Widget::position( w()-90,y()+5 );

	end();
	clear_border();
	set_menu_window();
	set_modal();		
}

void BE_ColorPicker::drawSwatch( int ColorIdx ){
	const BE_Color &Color{ palette->at( ColorIdx ) };

	const int	X{ palX+( ColorIdx%columns )*swatchSize },
					Y{ palY+( ColorIdx/columns )*swatchSize };

	if( selected==ColorIdx ){
		const int size=swatchSize+5;

		fl_draw_box(
			FL_BORDER_BOX,
			X-2,Y-2,
			size,size,
			fl_rgb_color( Color.red,Color.green,Color.blue )
		);
	}else{
		const int size=swatchSize;

		fl_draw_box(
			FL_THIN_UP_BOX,
			X,Y,
			size,size,
			fl_rgb_color( Color.red,Color.green,Color.blue )
		);	
	}
}

void BE_ColorPicker::draw(){
	Fl_Window::draw();

	fl_draw_box( FL_THIN_DOWN_BOX,border,header,w()-(border<<1),h()-(border+header),FL_DARK1 );

	for( size_t i{ 0 };i<palette->size();++i ){
		drawSwatch( i );
	}

	drawSwatch( selected );

	previous=selected;
}

int BE_ColorPicker::handle( int Ev ){
	int	selection{ selected },
			retv{ Fl_Window::handle( Ev ) };

	switch( Ev ){
		case FL_ENTER: [[ fallthrough ]];
		case FL_MOVE: [[ fallthrough ]];
		case FL_PUSH: [[ fallthrough ]];
		case FL_DRAG:{
			int	X{ Fl::event_x_root()-palX-x() },
					Y{ Fl::event_y_root()-palY-y() };	

			if( X>=0 ) X=X/swatchSize;
			if( Y>=0 ) Y=Y/swatchSize;

			if( X>=0 && X<columns \
			&&  Y>=0 && Y<rows ){
				selection=Y*columns+X<static_cast< int >( palette->size() )-1?Y*columns+X:palette->size()-1;
			}else{
				selection=previous;
			}
		}
		break;
		case FL_RELEASE: isSelected=true; break;
	}

	if( selection!=selected ){
		selected=selection;
		redraw();
		
		return 1;
	}

	return retv;
}

int BE_ColorPicker::popUp( int X,int Y,const palette_t *Pal,const BE_Color *Color ){

	BE_ColorPicker colorPicker;

	auto search{ std::find( Pal->begin(),Pal->end(),*Color ) };

	if( search!=Pal->end() ) colorPicker.previous=colorPicker.selected=( std::distance( Pal->begin(),search ) );
	else std::cerr << *Color << ": not found in palette.\n";

	char* hexv{ new char[12] };

	sprintf( hexv," 0x%06X",static_cast< uint32_t >( *Color>>8 ) );

	colorPicker.m_hexInput.value( hexv );

	colorPicker.palette=Pal;
	colorPicker.position( X,Y );

	colorPicker.set_modal();
	colorPicker.show();
	Fl::grab( colorPicker );

	while( !colorPicker.isSelected ) Fl::wait();

	colorPicker.isSelected=false;
	colorPicker.hide();
	Fl::grab( 0 );

	return colorPicker.selected;
}
