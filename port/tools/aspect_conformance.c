#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "port/port_aspect.h"
int main(void) {
 int sizes[][2]={{1920,1080},{5120,1440},{1280,800},{800,1280},{640,480},{240,160},{320,240}};
 int i,m;
 for(i=0;i<7;i++)for(m=0;m<PORT_ASPECT_COUNT;m++){
  int w=sizes[i][0],h=sizes[i][1];PortAspectBounds a=PortAspectFit(w,h,m);
  assert(a.w>0&&a.h>0&&a.x>=0&&a.y>=0&&a.x+a.w<=w&&a.y+a.h<=h);
  assert(abs(w-a.w-2*a.x)<=1&&abs(h-a.h-2*a.y)<=1);
  if(m==PORT_ASPECT_4_3)assert(abs(3*a.w-4*a.h)<=2);
  if(m==PORT_ASPECT_GBA)assert(abs(2*a.w-3*a.h)<=2);
  if(m==PORT_ASPECT_FILL)assert(a.w==w&&a.h==h);
 }
 puts("Aspect bounds: 49 combinations PASS");return 0;
}