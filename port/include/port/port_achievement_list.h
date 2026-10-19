#ifndef PORT_ACHIEVEMENT_LIST_H
#define PORT_ACHIEVEMENT_LIST_H
/* Read-only menu view. Official status is the launcher's last account query;
 * no credentials, networking or award submission belongs to this interface. */
typedef struct PortAchievementInfo {
    char title[192], description[768];
    char badge[8]; /* validated public cache basename; empty for local */
    int state; /* -1 account not queried, 0 locked, 1 unlocked */
} PortAchievementInfo;
void PortAchievementsListRefresh(void);
int PortAchievementsListCount(int official);
int PortAchievementsListGet(int official, int index, int spanish, PortAchievementInfo* out);
#endif
