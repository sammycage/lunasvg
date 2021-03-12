#include <X11/Xlib.h>
#include<Imlib2.h>


#include "../include/svgdocument.h"
using namespace lunasvg;

int
main (int argc, char ** argv)
{
  Display *display;
  Window window;
  XEvent event;
  int screen;

  int quit = 0;

  int width;
  int height;


  if (argc != 2)
    {
      printf ("use: %s file.svg\n", argv[0]);
      return 1;
    }


  display = XOpenDisplay (NULL);
  if (display == NULL)
    {
      fprintf (stderr, "Cannot open display\n");
      exit (1);
    }

  SVGDocument document;
  if (document.loadFromFile (argv[1]))
    {
      width = document.documentWidth (96.0);
      height = document.documentHeight (96.0);

      Bitmap bitmap = document.renderToBitmap (width, height, 96.0, 0);

      screen = DefaultScreen (display);
      window = XCreateSimpleWindow (display, RootWindow (display, screen), 10, 10, width, height, 1,
                                    BlackPixel (display, screen), WhitePixel (display, screen));

      XStoreName(display, window, "lunaSVG X11");
              
      imlib_set_cache_size (2048 * 1024);
      imlib_context_set_display (display);
      imlib_context_set_visual (DefaultVisual (display, screen));
      imlib_context_set_colormap (DefaultColormap (display, screen));

      XSelectInput (display, window, ExposureMask | ButtonPressMask);
      XMapWindow (display, window);


      const unsigned char * bmp = bitmap.data ();
      int size = bitmap.width () * bitmap.height ();
      unsigned char * data = (unsigned char *) malloc (size * 4);

      //RGBA to  ARGB 
      for (int i = 0; i < size; i++)
        {
          data[(4 * i) + 0] = bmp[(4 * i) + 2];
          data[(4 * i) + 1] = bmp[(4 * i) + 1];
          data[(4 * i) + 2] = bmp[(4 * i) + 0];
          data[(4 * i) + 3] = bmp[(4 * i) + 3];
        }

      Imlib_Image Image = imlib_create_image_using_copied_data (bitmap.width (), bitmap.height (), (unsigned int*) data);

      free (data);

      imlib_context_set_image (Image);

      Pixmap XPixmap = XCreatePixmap (display, RootWindow (display, screen), width, height, XDefaultDepthOfScreen (XDefaultScreenOfDisplay (display)));

      imlib_context_set_drawable (XPixmap);

      imlib_render_image_on_drawable (0, 0);

      imlib_free_image ();


      while (!quit)
        {
          XNextEvent (display, &event);
          if (event.type == Expose)
            {
              XCopyArea (display, XPixmap, window, DefaultGC (display, screen), 0, 0, width, height, 0, 0);
            }
          if (event.type == ButtonPress)
            {
              quit = 1;
            }
        }

      XCloseDisplay (display);
      return 0;
    }
  else
    {
      printf ("Error open file: %s\n", argv[1]);
    }
  return 1;
}
