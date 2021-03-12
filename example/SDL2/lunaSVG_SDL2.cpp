
#include "../include/svgdocument.h"
using namespace lunasvg;

#include<SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

int quit = 0;
SDL_Event event;
SDL_Renderer * Renderer = NULL;

void loop (void);

int
main (int argc, char ** argv)
{
  int width;
  int height;


  if (argc != 2)
    {
      printf ("use: %s file.svg\n", argv[0]);
      return 1;
    }


  if (SDL_Init (SDL_INIT_VIDEO) < 0)
    {
      printf ("SDL could not initialize! SDL_Error: %s\n", SDL_GetError ());
      return 1;
    }

  SVGDocument document;
  if (document.loadFromFile (argv[1]))
    {
      width = document.documentWidth (96.0);
      height = document.documentHeight (96.0);

      Bitmap bitmap = document.renderToBitmap (width, height, 96.0, 0);

      SDL_Window * WWindow = SDL_CreateWindow ("lunaSVG SDL2", 50, 50, width, height, 0);

      Renderer = SDL_CreateRenderer (WWindow, -1, SDL_RENDERER_ACCELERATED);
      if (Renderer == NULL)
        {
          printf ("Switching to software renderer...\n");
          Renderer = SDL_CreateRenderer (WWindow, -1, SDL_RENDERER_SOFTWARE);
        }
      if (Renderer == NULL)
        {
          printf ("Renderer could not be created! SDL Error: %s\n", SDL_GetError ());
          return 1;
        }

      SDL_Surface * Surface = SDL_CreateRGBSurfaceFrom ((void *) bitmap.data (),
                                                        bitmap.width (),
                                                        bitmap.height (),
                                                        32,
                                                        4 * bitmap.width (),
                                                        0x000000FF,
                                                        0x0000FF00,
                                                        0x00FF0000,
                                                        0xFF000000);

      SDL_Texture * mTexture = SDL_CreateTextureFromSurface (Renderer, Surface);
      SDL_SetRenderTarget (Renderer, NULL);
      SDL_SetRenderDrawColor (Renderer, 0xFF, 0xFF, 0xFF, 0xFF);
      SDL_RenderClear (Renderer);
      SDL_RenderCopy (Renderer, mTexture, NULL, NULL);
      SDL_DestroyTexture (mTexture);
      SDL_RenderPresent (Renderer);
      SDL_FreeSurface (Surface);

#ifdef __EMSCRIPTEN__
      emscripten_set_main_loop (loop, 0, 1);
#else 
      while (!quit)
        {
          loop ();
        }
#endif
      SDL_DestroyRenderer (Renderer);
      SDL_DestroyWindow (WWindow);
      SDL_Quit ();

      return 0;
    }
  else
    {
      printf ("Error open file: %s\n", argv[1]);
    }
  return 1;
}

void
loop (void)
{

  if (SDL_WaitEvent (&event))
    {
      //User requests quit 
      if ((event.type == SDL_QUIT) || (event.type == SDL_MOUSEBUTTONDOWN))
        {
          quit = true;
#ifdef __EMSCRIPTEN__
          emscripten_cancel_main_loop ();
#endif
        }
    }
}
