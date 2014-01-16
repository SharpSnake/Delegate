#include <stdio.h>
#include <iostream>

#include "Delegate.h"


typedef delegate< void, int, int > MouseEventHandler;

// define a virtual button
class Button
{
public:
	Button()
	{}

	~Button()
	{
		ButtonClick.Release();
	}

public:
	// click event
	Delegate< void, MouseEventHandler > ButtonClick;

public:
	// simulate the click motion
	void DoClick( int x, int y )
	{
		if( ButtonClick.NotEmpty() )
			ButtonClick.Invoke( x, y );// fire event list
	}
};

// define a virtual dialog
class VDialog
{
public:
	VDialog( Button *button )
	{
		m_Button = button;

		// register 3 callbacks to button's click event
		m_Button->ButtonClick += MouseEventHandler( this, &VDialog::m_Button_Click_CallBack1 );
		m_Button->ButtonClick += MouseEventHandler( this, &VDialog::m_Button_Click_CallBack2 );
		m_Button->ButtonClick += MouseEventHandler( this, &VDialog::m_Button_Click_CallBack3 );

		// then remove the second callback
		m_Button->ButtonClick -= MouseEventHandler( this, &VDialog::m_Button_Click_CallBack2 );
	}

private:
	Button *m_Button;

private:
	void m_Button_Click_CallBack1( int x, int y )
	{
		cout<< "Dialog CallBack1 knows button was clicked, mouse position is \r\n( " << x << ", " << y << " )\r\n" << endl;
	}

	void m_Button_Click_CallBack2( int x, int y )
	{
		cout<< "Dialog CallBack2 knows button was clicked, mouse position is \r\n( " << x << ", " << y << " )" << endl;
	}

	void m_Button_Click_CallBack3( int x, int y )
	{
		cout<< "Dialog CallBack3 knows button was clicked, mouse position is \r\n( " << x << ", " << y << " )" << endl;
	}
};


int main(int argc, char* argv[])
{
	Button *button = new Button();
	VDialog *dialog = new VDialog( button );
	
	button->DoClick( 100, 300 );   // you pretend to click button with the mouse.
}