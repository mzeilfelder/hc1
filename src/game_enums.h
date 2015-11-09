// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#ifndef GAME_ENUMS_H
#define GAME_ENUMS_H

enum GAME_MODES
{
    GM_START,
    GM_RUNNING,
    GM_PAUSED,
    GM_RESUME_PAUSE,
    GM_FINISHED,
    GM_REPLAY,
};

enum GAME_TYPE
{
    GT_UNKOWN,
    GT_CHAMPIONSHIP,
    GT_ARCADE,
    GT_HOTLAP,
    GT_TIMERACE,
    GT_RIVALS,
};

enum GHOST_TYPE
{
    GHT_NONE,
    GHT_LAP,
    GHT_TRACK,
    GHT_EXPORTED_LAP,
    GHT_EPORTED_TRACK,
};

enum BOT_DIFFICULTY
{
    BD_CHAMPIONSHIP = 0,
    BD_EASY,
    BD_MEDIUM,
    BD_HARD,
};

#endif // GAME_ENUMS_H
