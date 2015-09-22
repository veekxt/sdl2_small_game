/****
Xie Tao,
2015-06-04
****/
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "SDL2/SDL_ttf.h"
#include "easy_sdl.h"
#define TIME_delay 500
Uint32 TIME_DELAY=TIME_delay;//延时时间，控制自动下落速度（ms）
//保存整个面板状态的数组,用1标记块已被占用,
//使用24*12包围22*10的面板,二维数组的最外围标记1,为了统一判断不要超出面板范围,尽管前两行不显示
int zt[24][12];
//标记每种形状与第一块的相对位置和旋转中心与第一块的关系(每一行最后两个数据),

int block_type[7][5][2] =
{
    0, 0, -1, 0,  1, 0, 2, 0, 1, 1, //I
    0, 0, -1, -1, -1, 0,  1, 0, 2, 0, //L
    0, 0, -1, 0,  1, 0, 1, -1, 2, 0, //J
    0, 0, 1, 0, 0, 1, 1, 1, 1, 1, //O
    0, 0, -1, 0,  0, -1, 1, -1, 2, 0, //S
    0, 0, -1, 0,  0, -1, 1, 0, 2, 0, //T
    0, 0, -1, -1, 0, -1,  1, 0, 2, 0, //Z
};
//每种形状的颜色
Uint32 blockcolor[7] = {0x00ffff, 0xffa500, 0x0000ff, 0xffff00, 0x008000, 0x800080, 0xff0000};

typedef struct ablock_s //形状的结构体
{
    int xy[5][2];       //每一块的坐标和旋转中心(xy[4][x | y])
    Uint32 color;       //颜色
} Tblock;

//实际坐标到虚坐标转换,虚坐标:22*10的方格
#define xu_x(n) (((n)-zuo_x-line_w)/(line_w+rec_w))
#define xu_y(n) (((n)-zuo_y-line_w)/(line_w+rec_w))

//全局变量,设置:网格起始坐标x,y;网格线宽;方格宽;方格个数(x坐标,y坐标);网格颜色;
//因为旋转函数，line_w 和rec_w都应该是奇数，待修复2015-06-06
const int zuo_x = 30, zuo_y = 30, line_w = 1, rec_w = 27, widths = 10 , heights = 20, wg_color = 0x888888;
//枚举:方向
enum direct {up, down, left, right, here};

//当前形状和下个形状以及类型
Tblock * ablock, *next_ablock;
int type_block_now;
int type_block_next;

//分数和消行数量
Uint32 score=0,rows_kill=0;
//显示分数和消行数量
void printinfo(void)
{
    char score_tmp[50];
    char rows_tmp[50];
    sprintf(score_tmp,"Score: %u",score);
    sprintf(rows_tmp,"kRows: %u",rows_kill);
    drawtext(zuo_x + (widths + 2) * (line_w + rec_w),zuo_y + (line_w + 5 * rec_w),score_tmp,16,0xffffff,0);
    drawtext(zuo_x + (widths + 2) * (line_w + rec_w),zuo_y + (line_w + 5 * rec_w)+30,rows_tmp,16,0xffffff,0);
}

//绘制一个形状
void draw_block(Tblock * myblock, Uint32 color)
{
    //遍历4个坐标输出
    for(int i = 0; i < 4; i++)
    {
        if(myblock->xy[i][1] > zuo_y)
        {
            fillRect(myblock->xy[i][0], myblock->xy[i][1], rec_w, rec_w, color);
        }
    }
}
//初始化形状,参数:类型(0-6);是当前块还是下一个块
int init_block(Tblock * block, int type_rand_block, int isNext)
{
    //下一个块显示在右边
    block->xy[0][0] = isNext ? zuo_x + (widths + 2) * (line_w + rec_w) : zuo_x + (widths / 2 - 1) * (line_w + rec_w) + line_w;
    block->xy[0][1] = isNext ? zuo_y + (line_w + 2 * rec_w) : zuo_y - (rec_w);
    //计算旋转中心放在xy[4][x | y]
    block->xy[4][0] = block->xy[0][0] + (rec_w - 1) / block_type[type_rand_block][4][0] + (line_w + 1) / 2 * block_type[type_rand_block][4][1];
    block->xy[4][1] = block->xy[0][1] + block->xy[4][0] - block->xy[0][0];

    block->color = blockcolor[type_rand_block];

    //根据block_type的信息,由第一个坐标产生其余坐标
    for(int i = 1; i < 4; i++)
    {
        block->xy[i][0] = block->xy[0][0] + (line_w + rec_w) * block_type[type_rand_block][i][0];
        block->xy[i][1] = block->xy[0][1] + (line_w + rec_w) * block_type[type_rand_block][i][1];
    }

    return type_rand_block;
}
//预测经过某个方向移动之后的结果
int forecast(enum direct iwhere)
{
    int dx = 0, dy = 0;

    switch(iwhere)
    {
    case down:
        dy = 1;
        break;

    case up:
        dy = -1;
        break;

    case left:
        dx = -1;
        break;

    case right:
        dx = 1;
        break;

    case here:

        break;
    }

    for(int i = 0; i < 4; i++)
    {
        int x = xu_x(ablock->xy[i][0] + dx * (line_w + rec_w));
        int y = xu_y(ablock->xy[i][1] + dy * (line_w + rec_w));
        //之后的坐标如果被占用
        if(zt[y + 3][x + 1] == 1)
        {
            if(iwhere == down)
            {
                return 1;//如果是下降后到达的结果,返回1,表示遇到阻碍,需要即将固定这个形状
            }
            else
            {
                return -1;//如果不是说明到达边缘
            }
        }
    }
    return 0;//正常移动
}

//判断是否消行,是则处理
void update_rows(void)
{
    int flag_isok;
    int rows_kill_tmp=0;
    for(int row = 3; row <= 22; row++)
    {
        flag_isok = 1;
        //遍历zt数组的一行,所有数字为1说明要消除此行
        for(int i = 1; i <= 10; ++i)
        {
            if(zt[row][i] == 0)
            {
                flag_isok = 0;
                break;
            }
        }
        //如果所有数字为1
        if(flag_isok == 1)
        {
            rows_kill_tmp++;
            rows_kill++;

            //从要消除的行开始,逐渐下降
            for(int row_top = row; row_top >= 3; row_top--)
                for(int i = 1; i <= 10; ++i)
                {
                    zt[row_top][i] = zt[row_top - 1][i];
                    fillRect(zuo_x + line_w * i + rec_w * (i - 1), zuo_y + line_w * (row_top - 2) + rec_w * (row_top - 3), rec_w, rec_w, getpixel(zuo_x + line_w * i + rec_w * (i - 1),
                             zuo_y + line_w * (row_top - 2) + rec_w * (row_top - 3) - rec_w - line_w));
                }
        }
    }
    //一次消多行分数更多
    if(rows_kill_tmp==1){score+=1;}
    if(rows_kill_tmp==2){score+=3;}
    if(rows_kill_tmp==3){score+=5;}
    if(rows_kill_tmp==4){score+=8;}

    printinfo();
}
//把当前形状占用的块写到zt数组中,当前形状被阻塞的时候调用
int updatezt(void)
{
    for(int i = 0 ; i < 4; i++)
    {
        zt[xu_y(ablock->xy[i][1]) + 3][xu_x(ablock->xy[i][0]) + 1] = 1;
    }
    //判断是否game over,也就是隐藏的两行是否被占用
    for(int i = 1 ; i < 3; i++)
    {
        for(int j=1;j<11;++j)
        if(zt[i][j]==1)return 0;
    }
    //判断消行
    update_rows();
    //当前形状固定,产生新的形状
    type_block_now = type_block_next;
    type_block_now = init_block(ablock, type_block_next, 0);
    draw_block(next_ablock, 0);
    type_block_next = init_block(next_ablock, rand() % 7, 1);
    draw_block(ablock, ablock->color);
    draw_block(next_ablock, next_ablock->color);
    ref();
    return 1;
}
//根据方向移动
void move_block(enum direct iwhere)
{
    //移动之前用forecast判断移动之后是什么状态,再决定是否移动
    if(forecast(iwhere) == -1)
    {
        return;
    }

    if(forecast(iwhere) == 1)
    {
        if(0==updatezt())
		{
			//free(ablock);free(next_ablock);
			//initgame();//game over ！不注释则自动下一局
		}
        return;
    }

    draw_block(ablock, 0);
    int dx = 0, dy = 0;
//坐标平移
    switch(iwhere)
    {
    case down:
        dy = 1;
        break;

    case up:
        dy = -1;
        break;

    case left:
        dx = -1;
        break;

    case right:
        dx = 1;
        break;

    case here:
        break;
    }

    for(int i = 0; i < 5; i++)
    {
        ablock->xy[i][0] += dx * (line_w + rec_w);
        ablock->xy[i][1] += dy * (line_w + rec_w);
    }

    draw_block(ablock, ablock->color);
    ref();
}

//顺时针旋转当前形状,line_w和rec_必须为奇数！待添加为偶数时的旋转2015-06-06
void rotation(void)
{
    int center_x = ablock->xy[4][0];
    int center_y = ablock->xy[4][1];
    int dx, dy;
    int to_x, to_y;

    for(int i = 0; i < 4; ++i)
    {
        dx = ablock->xy[i][0] - center_x;
        dy = ablock->xy[i][1] - center_y;
        to_x = center_x - dy + 1 - rec_w;
        to_y = center_y + dx;

        if(1 == zt[xu_y(to_y) + 3][xu_x(to_x) + 1])
        {
            return;
        }
    }

    draw_block(ablock, 0);

    for(int i = 0; i < 4; ++i)
    {
        dx = ablock->xy[i][0] - center_x;
        dy = ablock->xy[i][1] - center_y;
        ablock->xy[i][0] = center_x - dy + 1 - rec_w;
        ablock->xy[i][1] = center_y + dx;
    }

    draw_block(ablock, ablock->color);
    ref();
}
//定时调用的这个函数，自动下落
Uint32 test(Uint32 interval, void * p)
{
    move_block(down);
    return TIME_DELAY;
}
//初始化游戏
void initgame(void)
{
    for(int i=0;i<24;i++){zt[i][11]=1;zt[i][0]=1;}
    for(int i=0;i<12;i++){zt[23][i]=1;zt[0][i]=1;}
    for(int i=1;i<11;i++)
        for(int j=1;j<23;j++)
            zt[j][i]=0;
    fillRect(0, 0, 300 + line_w * (widths + 1) + rec_w * widths, 60 + line_w * (heights + 1) + rec_w * heights, 0);
    draw_wangge(zuo_x, zuo_y, line_w, rec_w, widths , heights, wg_color);
    score=0,rows_kill=0;
    ablock = (Tblock *)malloc(sizeof(struct ablock_s));
    next_ablock = (Tblock *)malloc(sizeof(struct ablock_s));
    type_block_now = init_block(ablock, rand() % 7, 0);
    type_block_next = init_block(next_ablock, rand() % 7, 1);
    draw_block(ablock, ablock->color);
    draw_block(next_ablock, next_ablock->color);
    printinfo();
    ref();
}

//按键处理
void e_press_key(SDL_Event event)
{
    switch(event.key.keysym.sym)
    {
    case SDLK_LEFT:
        move_block(left);
        break;

    case SDLK_RIGHT:
        move_block(right);
        break;

    case SDLK_UP:
        rotation();
        break;

    case SDLK_DOWN:
        move_block(down);
        break;
    case SDLK_SPACE:
        initgame();
        break;
    }

}

int main(int argc, char ** argv)
{
    srand((unsigned)time(NULL));
    initwin(300 + line_w * (widths + 1) + rec_w * widths, 60 + line_w * (heights + 1) + rec_w * heights, 0);
    _Bool quit = 0;
    SDL_Event event;
    initgame();
    //定时器
    myTime = SDL_AddTimer(TIME_DELAY, test, NULL);

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
        }
    }

    return 0;
}

