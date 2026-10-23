#include <assert.h>
#include "port/port_scale2x.h"
int main(void) {
    uint32_t out[36], one=0x12345678, solid[9], edge[9]={0,1,0,1,2,0,0,0,0};
    int i;
    PortScale2x(&one,1,1,out);
    for(i=0;i<4;i++) assert(out[i]==one);
    for(i=0;i<9;i++) solid[i]=0x7f123456;
    PortScale2x(solid,3,3,out);
    for(i=0;i<36;i++) assert(out[i]==0x7f123456);
    PortScale2x(edge,3,3,out);
    assert(out[14]==1); /* reconstruct upper-left corner at centre */
    assert(out[15]==2 && out[20]==2 && out[21]==0);
    return 0;
}
