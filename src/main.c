#include "Application.h"

#ifdef __ANDROID__
/* On Android SDL's Java side calls SDL_main() from libmain.so;
   SDL.h renames main() to SDL_main() for us. */
#include <SDL.h>
#endif

int main(int argc, char ** args) { 
    return ApplicationZuma_Start();
}
