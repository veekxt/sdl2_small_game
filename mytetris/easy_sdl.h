#ifndef	_PPM_H
#define	_PPM_H
#include<SDL2/SDL.h>
#include<SDL2/SDL_ttf.h>
#define ref() SDL_UpdateWindowSurface(window)

SDL_Rect bl;
SDL_Window * window;
SDL_TimerID myTime;
SDL_Surface * s;

void fillRect(int x,int y,int w,int h,Uint32 color);
void initwin(int w,int h,Uint32 flag);
void draw_wangge(int x,int y,int line_w,int rec_w,int width ,int height,Uint32 color);
void drawtext(int x,int y,const char *S,int fontsize,Uint32 color,Uint32 bkcolor);
Uint32 getpixel(int x, int y);
#endif
