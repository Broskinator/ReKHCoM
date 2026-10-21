#ifndef PORT_MENU_LAYOUT_H
#define PORT_MENU_LAYOUT_H
/* Presentation coordinates only: never rewrite the game's GBA positions. */
typedef struct PortMenuLayout {
    int width, height;
    float scale;
    int split;
} PortMenuLayout;
static inline PortMenuLayout PortMenuLayoutMake(int width,int height,float scale,int split) {
    PortMenuLayout v;
    float fitX=width/240.0f,fitY=height/160.0f;
    if(scale>fitX)scale=fitX;
    if(scale>fitY)scale=fitY;
    if(scale<=0)scale=1;
    v.width=width;v.height=height;v.scale=scale;v.split=split;return v;
}
/* An axis is one of -1 (leading edge), 0 (centre), +1 (trailing edge). */
static inline float PortMenuLayoutX(const PortMenuLayout* v,int ax) {
    return ax<0?0:ax>0?v->width-240*v->scale:(int)((v->width-240*v->scale)*.5f);
}
static inline float PortMenuLayoutY(const PortMenuLayout* v,int ay) {
    return ay<0?0:ay>0?v->height-160*v->scale:(int)((v->height-160*v->scale)*.5f);
}
static inline int PortMenuGroup(int ax,int ay) {return (ay+1)*3+ax+1;}
static inline int PortMenuGroupX(int group) {return group%3-1;}
static inline int PortMenuGroupY(int group) {return group/3-1;}
#endif
