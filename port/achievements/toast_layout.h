#ifndef KH_ACHIEVEMENT_TOAST_LAYOUT_H
#define KH_ACHIEVEMENT_TOAST_LAYOUT_H
typedef struct KhToastLayout { float x, y, width, height, alpha; int scale; } KhToastLayout;
/* Output pixels, independent of game zoom. 300ms slide/fade, 3.9s hold, 300ms out. */
static KhToastLayout KhAchievementToastLayout(unsigned elapsed_ms, int width, int height, int logical_width) {
    KhToastLayout box;
    float visible = 1.0f;
    box.scale = height / 360;
    if (box.scale < 1) box.scale = 1;
    while (box.scale > 1 && logical_width * box.scale > width - 16) --box.scale;
    box.width = (float)(logical_width * box.scale);
    if (box.width > width - 8) box.width = (float)(width - 8);
    box.height = (float)(48 * box.scale);
    if (elapsed_ms < 300) { float t = 1.0f - elapsed_ms / 300.0f; visible = 1.0f - t*t*t; }
    else if (elapsed_ms > 4200) { float t = (elapsed_ms - 4200) / 300.0f; visible = 1.0f - t*t*t; }
    if (visible < 0.0f) visible = 0.0f;
    box.alpha = visible;
    box.x = (width - box.width) * 0.5f;
    box.y = -box.height + (box.height + 8 * box.scale) * visible;
    return box;
}
#endif
