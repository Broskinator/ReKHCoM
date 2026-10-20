/* The set's badges, shipped as one atlas (assets/achievements/badges.png,
   10 per row in set order, 64x64): colour and grey textures, made once, so
   locked art stays recognisable. No network; the copy built into the
   executable stands in when the file is missing. */
static const unsigned char sBuiltInBadges[] = {
#include "port/badges_png.inc"
};

static struct {
    GLuint colour = 0, grey = 0;
    int rows = 0;
    bool tried = false;
} sBadgeAtlas;

static GLuint OfficialAchievementBadge(int index, bool unlocked, ImVec2* uv0, ImVec2* uv1) {
    if (!sBadgeAtlas.tried) {
        sBadgeAtlas.tried = true;
        std::vector<unsigned char> encoded;
        int w = 0, h = 0, channels;
        ReadFile(sRoot / "assets" / "achievements" / "badges.png", encoded);
        if (encoded.empty()) encoded.assign(sBuiltInBadges, sBuiltInBadges + sizeof(sBuiltInBadges));
        unsigned char* rgba = encoded.empty() || encoded.size() > (4u << 20) ? nullptr :
            stbi_load_from_memory(encoded.data(), (int)encoded.size(), &w, &h, &channels, 4);
        if (rgba && w == 640 && h > 0 && h % 64 == 0) {
            std::vector<unsigned char> grey(rgba, rgba + w * h * 4);
            for (int i = 0; i < w * h; ++i) {
                unsigned v = (grey[i*4] * 54u + grey[i*4+1] * 183u + grey[i*4+2] * 19u + 128u) >> 8;
                grey[i*4] = grey[i*4+1] = grey[i*4+2] = (unsigned char)v;
            }
            for (int t = 0; t < 2; ++t) {
                GLuint* texture = t ? &sBadgeAtlas.grey : &sBadgeAtlas.colour;
                glGenTextures(1, texture); glBindTexture(GL_TEXTURE_2D, *texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t ? grey.data() : rgba);
            }
            sBadgeAtlas.rows = h / 64;
        }
        if (rgba) stbi_image_free(rgba);
    }
    if (!sBadgeAtlas.colour || index < 0 || index / 10 >= sBadgeAtlas.rows) return 0;
    float rows = (float)sBadgeAtlas.rows;
    *uv0 = ImVec2((index % 10) / 10.0f, (index / 10) / rows);
    *uv1 = ImVec2((index % 10 + 1) / 10.0f, (index / 10 + 1) / rows);
    return unlocked ? sBadgeAtlas.colour : sBadgeAtlas.grey;
}

static void AchievementBadgesDestroy() {
    if (sBadgeAtlas.colour) glDeleteTextures(1, &sBadgeAtlas.colour);
    if (sBadgeAtlas.grey) glDeleteTextures(1, &sBadgeAtlas.grey);
    sBadgeAtlas.colour = sBadgeAtlas.grey = 0;
    sBadgeAtlas.rows = 0;
    sBadgeAtlas.tried = false;
}
