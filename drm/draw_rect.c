#include "draw_rect.h"

int validPos(int pos, int min, int max) {
    return pos >= min && pos < max;
}

int validRect(MRECT rc) {
    return rc.right - rc.left > 0 && rc.bottom - rc.top > 0;
}

void drawRect(char* map, int dst_w, int dst_h, YUV_COLOR color, MRECT rect) {
    if (!validRect(rect))
        return;
    int i,j;
    int left = rect.left,top = rect.top,right = rect.right,bottom = rect.bottom;
    int offset = dst_w * dst_h;
    //red
    int y = color.y;
    int u = color.u;
    int v = color.v;

    int offsetY,offsetX;
    for (i = 0; i < (right-left+1)/2; i++) {
        for (j = 0; j < 2; j++) {
            offsetY = j*2+top;
            offsetX = left+2*i;
            if (validPos(offsetX,0,dst_w)) {
                if (validPos(offsetY,0,dst_h)){
                    map[offsetX + offsetY*dst_w] = y;
                }
                if (validPos(offsetY+1,0,dst_h)) {
                    map[offsetX + (offsetY+1)*dst_w] = y;
                }
            }
            if (validPos(offsetX+1,0,dst_w)) {
                if (validPos(offsetY,0,dst_h)){
                    map[offsetX + 1 + offsetY*dst_w] = y;
                }
                if (validPos(offsetY+1,0,dst_h)) {
                    map[offsetX + 1 + (offsetY+1)*dst_w] = y;
                }
            }
            offsetY = j + top/2;
            offsetX = left/2*2 + 2*i;
            if (validPos(offsetX,0,dst_w-1)&&validPos(offsetY,0,dst_h/2)){
                map[offsetX + offsetY*dst_w + offset] = u;
                map[offsetX + 1 + offsetY*dst_w + offset] = v;
            }
        }

        for (j = 0; j < 2; j++) {
            offsetY = bottom - j*2 - 1;
            offsetX = left + 2*i;
            if (validPos(offsetX,0,dst_w)) {
                if (validPos(offsetY,0,dst_h)){
                    map[offsetY*dst_w + offsetX] = y;
                }
                if (validPos(offsetY+1,0,dst_h)) {
                    map[(offsetY+1)*dst_w + offsetX] = y;
                }
            }
            if (validPos(offsetX+1,0,dst_w)) {
                if (validPos(offsetY,0,dst_h)){
                    map[offsetY*dst_w + offsetX +1] = y;
                }
                if (validPos(offsetY+1,0,dst_h)) {
                    map[(offsetY+1)*dst_w + offsetX + 1] = y;
                }
            }

            offsetY = bottom/2 - j;
            offsetX = left/2*2 + 2*i;
            if (validPos(offsetX,0,dst_w-1)&&validPos(offsetY,0,dst_h/2)){
                map[offsetY*dst_w + offsetX + offset] = u;
                map[offsetY*dst_w + offsetX + 1 + offset] = v;
            }
        }
    }

    for (i = 0; i < (bottom-top+1)/2; i++) {
        for (j = 0; j < 2; j++) {
            offsetY = 2*i+top;
            offsetX = left+2*j;
            if (validPos(offsetX,0,dst_w)) {
                if (validPos(offsetY,0,dst_h)){
                    map[offsetX + offsetY*dst_w] = y;
                }
                if (validPos(offsetY+1,0,dst_h)) {
                    map[offsetX + (offsetY+1)*dst_w] = y;
                }
            }
            if (validPos(offsetX+1,0,dst_w)) {
                if (validPos(offsetY,0,dst_h)){
                    map[offsetX + 1 + offsetY*dst_w] = y;
                }
                if (validPos(offsetY+1,0,dst_h)) {
                    map[offsetX + 1 +(offsetY+1)*dst_w ] = y;
                }
            }

            offsetY = i+top/2;
            offsetX = left/2*2 + 2*j;
            if (validPos(offsetX,0,dst_w-1)&&validPos(offsetY,0,dst_h/2)){
                map[offsetX + offsetY*dst_w + offset] = u;
                map[offsetX + 1 + offsetY*dst_w + offset] = v;
            }
        }

        for (j = 0; j < 2; j++) {
            offsetY = 2*i+top;
            offsetX = right - 2*j - 1;
            if (validPos(offsetX,0,dst_w)) {
                if (validPos(offsetY,0,dst_h)){
                    map[offsetY*dst_w + offsetX] = y;
                }
                if (validPos(offsetY+1,0,dst_h)) {
                    map[(offsetY+1)*dst_w + offsetX] = y;
                }
            }
            if (validPos(offsetX+1,0,dst_w)) {
                if (validPos(offsetY,0,dst_h)){
                    map[offsetY*dst_w + offsetX + 1] = y;
                }
                if (validPos(offsetY+1,0,dst_h)) {
                    map[(offsetY+1)*dst_w + offsetX + 1] = y;
                }
            }

            offsetY = i+top/2;
            offsetX = right/2*2 - 2*j;
            if (validPos(offsetX,0,dst_w-1)&&validPos(offsetY,0,dst_h/2)){
                map[offsetY*dst_w + offsetX + offset] = u;
                map[offsetY*dst_w + offsetX + 1 + offset] = v;
            }
        }
    }
}

