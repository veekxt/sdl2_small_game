#include "easy_sdl.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include "SDL2/SDL_ttf.h"
#define ref() SDL_UpdateWindowSurface(window)

void fillRect(int x,int y,int w,int h,Uint32 color)
{
    bl.h=h;
    bl.w=w;
    bl.x=x;
    bl.y=y;
    SDL_FillRect(s, &bl,color);
}

void initwin(int w,int h,char *title,Uint32 flag)
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, w, h, flag);
    s = SDL_GetWindowSurface(window);
}

void draw_wangge(int x,int y,int line_w,int rec_w,int width ,int height,Uint32 color)
{
    for(int n=0;n<=width;n++)
    {
        fillRect(x+n*(line_w+rec_w),0+y,line_w, line_w*(height+1)+rec_w*height,color);
    }
    for(int n=0;n<=height;n++)
    {
        fillRect(0+x,n*(line_w+rec_w)+y,line_w*(width+1)+rec_w*width,line_w,color);
    }
}

void drawtext(int x,int y,const char *S,int fontsize,Uint32 color,Uint32 bkcolor)
{
    TTF_Init();
    TTF_Font *font= TTF_OpenFont("cour.ttf",fontsize);
    int w,h;
    TTF_SizeText(font,S,&w,&h);
    if(font == NULL)
    {
       fprintf(stderr,"font open failure %s\n",SDL_GetError());
    }
    SDL_Color textColor = { (unsigned char)(color>>16), (unsigned char)(color>>8), (unsigned char)(color) };
    SDL_Surface * text = TTF_RenderText_Solid( font, S, textColor );
    bl.h=h;
    bl.w=w;
    bl.x=x;
    bl.y=y;
    SDL_FillRect(s, &bl,bkcolor);
    SDL_UpperBlit(text,NULL,s,&bl);
    TTF_CloseFont(font);
    TTF_Quit();
}
Uint32 getpixel(int x, int y)
{
    int bpp = s->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)s->pixels + y * s->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}
