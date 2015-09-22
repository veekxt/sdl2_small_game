/****
Xie Tao,
One-Point,
2015-06-06
****/
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "SDL2/SDL_ttf.h"
#include "easy_sdl.h"
#define TIME_DELAY 250
#define TRUEZBX(n) (zuo_x+((n)+1)*(line_w)+(n)*(rec_w))
#define TRUEZBY(n) (zuo_y+((n)+1)*(line_w)+(n)*(rec_w))

typedef enum {up, down, left, right, here} direct;
int move(direct where);
Uint32 test(Uint32 interval, void * p);

direct zt[2] = {here,here};
char STATE[50];
typedef struct snake_node
{
    int x;
    int y;
    struct snake_node * next;
} * snake;

snake aSnake, aFood;
Uint32 chang = 1;
int gv=0;
//设置:网格起始坐标x,y;网格线宽;方格宽;方格个数(x坐标,y坐标);网格颜色;
const int zuo_x = 30, zuo_y = 30, line_w = 1, rec_w = 23, widths = 20 , heights = 17, wg_color = 0x888888;

//显示长度和其他信息
void printinfo(void)
{
    char score_tmp[50];
    sprintf(score_tmp, "Size: %u", chang);
    drawtext(zuo_x + (widths + 2) * (line_w + rec_w), zuo_y + (line_w +  rec_w), score_tmp, 16, 0xffffff, 0);
    drawtext(zuo_x + (widths + 2) * (line_w + rec_w), (bl.h+3)+zuo_y + (line_w +  rec_w), "---Key---", 16, 0xffffff, 0);
    drawtext(zuo_x + (widths + 2) * (line_w + rec_w), 2*(bl.h+3)+zuo_y + (line_w +  rec_w), "space:pause,continue", 16, 0xffffff, 0);
    drawtext(zuo_x + (widths + 2) * (line_w + rec_w), 3*(bl.h+3)+zuo_y + (line_w +  rec_w), "s : restart", 16, 0xffffff, 0);
    drawtext(zuo_x + (widths + 2) * (line_w + rec_w), 5*(bl.h+3)+zuo_y + (line_w +  rec_w), "---State---", 16, 0xffffff, 0);
    drawtext(zuo_x + (widths + 2) * (line_w + rec_w), 6*(bl.h+3)+zuo_y + (line_w +  rec_w), STATE, 16, 0xffffff, 0);
}

//生成一个food。注意不能生成在蛇身上，所以用参数S排除S
//也可以生成Snake，参数二标记是生成food还是snake
snake init_food(snake S, int isS)
{
    snake F = (snake)malloc(sizeof(struct snake_node));
    int flag;

    for(;;)
    {
        flag = 1;
        F->x = rand() % widths;
        F->y = rand() % heights;

        if(isS)
        {
            goto ends;
        }

        while(S != NULL)
        {
            if(S->x == F->x && S->y == F->y)
            {
                flag = 0;
                break;
            }

            S = S->next;
        }

        if(flag == 0)
        {
            continue;
        }
        else
        {
            break;
        }
    }

ends:
    F->next = NULL;
    return F;
}

void draw_snake(void)
{
    snake S = aSnake;
    fillRect(TRUEZBX(S->x), TRUEZBY(S->y), rec_w, rec_w, 0xff00ff);
    S = S->next;

    while(S != NULL)
    {
        fillRect(TRUEZBX(S->x), TRUEZBY(S->y), rec_w, rec_w, 0x008800);
        S = S->next;
    }

    fillRect(TRUEZBX(aFood->x), TRUEZBY(aFood->y), rec_w, rec_w, 0xff0000);
    ref();
}
void initgame(void)
{
    gv=0;
    zt[0]=zt[1] = here;
    sprintf(STATE,"Press a direction key");
    fillRect(0, 0,300 + zuo_x + line_w * (widths + 1) + rec_w * widths, 50 + zuo_y + line_w * (heights + 1) + rec_w * heights, 0x000000);
    for(int n = 0; n <= widths; n++)
    {
        fillRect(zuo_x + n * (line_w + rec_w), zuo_y + 0, line_w, line_w * (heights + 1) + rec_w * heights, 0x777777);
    }

    for(int n = 0; n <= heights; n++)
    {
        fillRect(zuo_x + 0, zuo_y + n * (line_w + rec_w), line_w * (widths + 1) + rec_w * widths, line_w, 0x777777);
    }
    free(aSnake);
    free(aFood);
    aSnake = init_food(aSnake, 1);
    aFood = init_food(aSnake, 0);
    draw_snake();
    printinfo();
    ref();
    myTime = SDL_AddTimer(TIME_DELAY, test, NULL);
}
//判断一个方格的位置是否合法，就是不能在蛇身（aSnake）内部或面板区域之外
int isRight(snake S)
{
    if(S->x > widths - 1 || S->x < 0 || S->y < 0 || S->y > heights - 1)
    {
        return 0;
    }

    snake tmp = aSnake;

    while(tmp != NULL)
    {
        if(tmp->x == S->x && tmp->y == S->y)
        {
            return 0;
        }

        tmp = tmp->next;
    }

    return 1;
}

//定时调用产生事件，检测到事件后再调用移动函数
Uint32 test(Uint32 interval, void * p)
{
    SDL_Event event;
    SDL_UserEvent userevent;
    userevent.type=SDL_USEREVENT;
    userevent.code=0;
    userevent.data1=p;
    event.type=SDL_USEREVENT;
    event.user=userevent;
    SDL_PushEvent(&event);
    return interval;
}

int move(direct where)
{
    if(gv==1)return;
    int nx = 0, ny = 0;

    switch(where)
    {
    case up:
        ny = -1;
        break;

    case down:
        ny = 1;
        break;

    case left:
        nx = -1;
        break;

    case right:
        nx = 1;
        break;

    case here:
        return;
    }
    snake news = (snake)malloc(sizeof(struct snake_node));
    news->next = aSnake;
    news->x = (aSnake)->x + nx;
    news->y = (aSnake)->y + ny;

    if(isRight(news) == 0)
    {
        free(news);
        sprintf(STATE,"GAME OVER !!!                  ");
        printinfo();
        ref();
        gv=1;
        return -1;    //Ganme over!
    }
    SDL_RemoveTimer(myTime);
    myTime = SDL_AddTimer(TIME_DELAY, test, NULL);
    aSnake = news;

    if((aFood)->x != news->x || (aFood)->y != news->y)
    {
        while(news->next->next != NULL)
        {
            news = news->next;
        }

        fillRect(TRUEZBX(news->next->x), TRUEZBY(news->next->y), rec_w, rec_w, 0);
        free(news->next);
        news->next = NULL;
    }
    else
    {
        chang++;
        printinfo();
        free(aFood);
        aFood = init_food(aSnake, 0);
    }

    draw_snake();
   sprintf(STATE,"Gaming...                        ");
    printinfo();
    return 0;
}

void e_press_key(SDL_Event event)
{
    switch(event.key.keysym.sym)
    {
    case SDLK_LEFT:
        if(zt[0] != right)
        {
            move(zt[0] = left);zt[1]=zt[0];
        }

        break;

    case SDLK_RIGHT:
        if(zt[0] != left)
        {
            move(zt[0] = right);zt[1]=zt[0];

        }

        break;

    case SDLK_UP:
        if(zt[0] != down)
        {
            move(zt[0] = up);zt[1]=zt[0];
        }

        break;

    case SDLK_DOWN:
        if(zt[0] != up)
        {
            move(zt[0] = down);zt[1]=zt[0];
        }

        break;

    case SDLK_SPACE:
        zt[0]=(zt[0]==here)?zt[1]:here;
        break;
    case SDLK_s:
        initgame();
    break;
    }
}

int main(int argc, char ** argv)
{
    srand((unsigned)time(NULL));
    initwin(300 + zuo_x + line_w * (widths + 1) + rec_w * widths, 50 + zuo_y + line_w * (heights + 1) + rec_w * heights, "SDL2 Snake", 0);
    _Bool quit = 0;
    SDL_Event event;
    initgame();

    for(; quit == 0;)
    {
        SDL_WaitEvent(&event);

        switch(event.type)
        {
        case SDL_QUIT:
            quit = 1;
            break;
        case SDL_KEYDOWN:
            e_press_key(event);
            break;
        case SDL_USEREVENT:
    {
        move(zt[0]);
    }
        }
    }

    return 0;
}

