#ifndef GUARD_PORT_ZOOM_H
#define GUARD_PORT_ZOOM_H

/* INI zoom: 0 = classic v1.4 preset; otherwise twice the pixel scale.
 * Even IDs are whole pixel scales, odd IDs are halfway between them. */
#define PORT_ZOOM_LEVELS 128
#define PORT_ZOOM_CLASSIC 0
#define PORT_ZOOM_CONFIG_VERSION 9
static inline int PortZoomMin(int width, int height) {
    int x = (width + 2559) / 2560, y = (height + 1439) / 1440;
    return x > y ? x : y > 0 ? y : 1;
}
static inline int PortZoomClassicScale(int width, int height) {
    /* Exact v1.4 fourth preset, including its distinct-step rules. */
    int s = height / 180, n = height / 216, least = PortZoomMin(width, height);
    if (s < 1) s = 1;
    if (n >= s) n = s - 1;
    if (n < 1) n = 1;
    s = height / 240;
    if (s >= n) s = n - 1;
    if (s < least) s = least;
    return s;
}
static inline int PortZoomMax(int width, int height) {
    int n = height / 160, least = PortZoomMin(width, height);
    if (n < least) n = least;
    if (n > PORT_ZOOM_LEVELS / 2) n = PORT_ZOOM_LEVELS / 2;
    return n;
}
static inline int PortZoomNormalize(int id, int only, int width, int height) {
    int low = PortZoomMin(width, height) * 2, high = PortZoomMax(width, height) * 2;
    if (!id) id = PortZoomClassicScale(width, height) * 2;
    if (only && (id & 1)) id++; /* toward a cheaper, whole pixel scale */
    if (id < low) id = low;
    if (id > high) id = high;
    return id;
}
static inline int PortZoomCount(int only, int width, int height) {
    int span = PortZoomMax(width, height) - PortZoomMin(width, height);
    return span * (only ? 1 : 2) + 1;
}
static inline int PortZoomAt(int index, int only, int width, int height) {
    int count = PortZoomCount(only, width, height);
    if (index < 0) index = 0;
    if (index >= count) index = count - 1;
    return PortZoomMax(width, height) * 2 - index * (only ? 2 : 1);
}
static inline int PortZoomIndex(int id, int only, int width, int height) {
    id = PortZoomNormalize(id, only, width, height);
    return (PortZoomMax(width, height) * 2 - id) / (only ? 2 : 1);
}
static inline int PortZoomStep(int id, int dir, int only, int width, int height) {
    return PortZoomAt(PortZoomIndex(id, only, width, height) + dir, only, width, height);
}
#endif
