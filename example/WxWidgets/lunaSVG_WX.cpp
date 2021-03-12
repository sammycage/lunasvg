#include "wx/wx.h"


#include "../include/svgdocument.h"
using namespace lunasvg;


//Main window class

class MyFrame : public wxFrame
{
public:
  wxBitmap * wbitmap;
  MyFrame ();
  void OnPaint (wxPaintEvent& event);
      void OnLeftDown(wxMouseEvent& event);
private:
  //Declaration event table
  DECLARE_EVENT_TABLE ()
};

//Define the event table

BEGIN_EVENT_TABLE (MyFrame, wxFrame)
EVT_PAINT (MyFrame::OnPaint)
EVT_LEFT_DOWN(MyFrame::OnLeftDown)
END_EVENT_TABLE ()


//Main window class constructor

MyFrame::MyFrame ()
: wxFrame (NULL, wxID_ANY, wxT ("lunaSVG WxWidgets"), wxDefaultPosition, wxSize (460, 300))
{
  //Center window
  CentreOnScreen ();

  //Set the background color
  SetBackgroundColour (wxColour (0xF4F4F4));
}

void
MyFrame::OnPaint (wxPaintEvent& event)
{
  wxPaintDC dc (this);
  dc.DrawBitmap (*wbitmap, 0, 0);
}

//The left mouse button press event processing
void MyFrame::OnLeftDown(wxMouseEvent& event)
{
  Destroy ();
}


//Application class

class MyApp : public wxApp
{
public:
  int width;
  int height;
  virtual bool OnInit ();
};

//Automatically instantiate the MyApp object and assign it to the global variable wxTheApp

DECLARE_APP (MyApp)
//Declare that you can use wxGetApp() to get the instance object
IMPLEMENT_APP (MyApp)



//Application entrance

bool
MyApp::OnInit ()
{
  //Instantiate a main window
  MyFrame *frame = new MyFrame ();

  frame->SetWindowStyle (frame->GetWindowStyle () | wxFULL_REPAINT_ON_RESIZE);

  //Display the main window
  frame->Show (true);


  if (argc != 2)
    {
      printf ("use: %s file.svg\n", (const char *) argv[0].c_str ());
      return false;
    }

  SVGDocument document;
  if (document.loadFromFile ((const char *) argv[1].c_str ()))
    {
      width = document.documentWidth (96.0);
      height = document.documentHeight (96.0);

      frame->SetSize (width + 10, height + 40);

      Bitmap bitmap = document.renderToBitmap (width, height, 96.0, 0);

      const unsigned char * bmp = bitmap.data ();
      int size = bitmap.width () * bitmap.height ();
      unsigned char * data = (unsigned char *) malloc (size * 3);
      unsigned char * alpha = (unsigned char *) malloc (size);

      for (int i = 0; i < size; i++)
        {
          data[(3 * i)] = bmp[(4 * i)];
          data[(3 * i) + 1] = bmp[(4 * i) + 1];
          data[(3 * i) + 2] = bmp[(4 * i) + 2];
          alpha[i] = bmp[(4 * i) + 3];
        }

      wxImage image (bitmap.width (), bitmap.height (), data, alpha);

      frame->wbitmap = new wxBitmap (image);

      image.Destroy ();

      //Start the event loop
      return true;
    }
  else
    {
      printf ("Error open file: %s\n", (const char *) argv[1].c_str ());
    }
  return false;
}


