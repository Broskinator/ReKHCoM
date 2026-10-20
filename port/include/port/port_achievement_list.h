#ifndef PORT_ACHIEVEMENT_LIST_H
#define PORT_ACHIEVEMENT_LIST_H
/* Read-only menu view of the official set: earned in this game (kept in the
 * data folder) or on the RetroAchievements account the launcher last read.
 * No credentials, networking or award submission belongs to this interface. */
typedef struct PortAchievementInfo {
    char title[192], description[768];
    char badge[8]; /* validated public cache basename */
    unsigned points;
    int state; /* 0 locked, 1 unlocked */
} PortAchievementInfo;
void PortAchievementsListRefresh(void);
int PortAchievementsListCount(void);
int PortAchievementsListGet(int index, PortAchievementInfo* out);
#endif
