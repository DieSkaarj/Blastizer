#ifndef _BE_COLOR_PICKER_H
#define _BE_COLOR_PICKER_H

#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>

#include "BE_Palette.hxx"

class BE_ColorPicker : public Fl_Window{
	const int	swatchSize{ 12 },
					palX{ 10 },palY{ 35 },
					columns{ 32 },rows{ 16 },
					border{ 5 },header{ 30 };
	int	selected,previous;
	bool	isSelected;

	const palette_t *palette;
	Fl_Input	m_hexInput;

	void drawSwatch( int );

	BE_ColorPicker();

	protected:

	void draw() override;
	int handle( int ) override;

	public:

	static int popUp( int X,int Y,const palette_t *Pal,const BE_Color *Color );
};

#endif//_BE_COLOR_PICKER_H
