#ifndef DRAW_RECT_H
#define DRAW_RECT_H

#include "amcomdef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __tag_yuv_color{
    int y;
    int u;
    int v;
} YUV_COLOR, *LPYUV_COLOR;

//struct YUV_COLOR{
//    int y;
//    int u;
//    int v;
//};

// red
//    int y = 76;
//    int u = 85;
//    int v = 255;
// white
//    int y = 252;
//    int u = 128;
//    int v = 128;
// cyan
//    int y = 179;
//    int u = 171;
//    int v = 1;

void drawRect(char* map, int dst_w, int dst_h, YUV_COLOR color, MRECT rect);

#ifdef __cplusplus
}
#endif

#endif // DRAW_RECT_H
