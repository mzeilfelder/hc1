// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#ifndef LEVEL_MANAGER_H
#define LEVEL_MANAGER_H

#include "globals.h"
#include "game_enums.h"
#include <string>
#include <vector>

class TiXmlElement;
class Level;

enum LevelDifficulty
{
    LD_EASY = 0,
    LD_MEDIUM,
    LD_HARD,
    LD_ALL,
};

struct AiBotSettings
{
    AiBotSettings();
    void ReadFromXml(const TiXmlElement * settings_);

    std::string mName;
    std::string mModel;
    float mLowPower;  // lower border for range of random power modifier
    float mUppPower;  // upper border for range of random power modifier
    float mSlowBorder; // if the speed is below mSlowBorder*maxspeed the slow modifiers are used
    float mLowSlow;   // lower border for range of random power modifier for slow speeds
    float mUppSlow;   // upper border for range of random power modifier for slow speeds
    float mLowEnergy;   // lower border for range of energy modifier (energy pushing object along a path)
    float mUppEnergy;   // upper border for range of energy modifier (energy pushing object along a path)
    float mLowIdeal;    // lower border for range or random modifier towards ideal line (instead of line throught track center)
    float mUppIdeal;    // upper border for range or random modifier towards ideal line (instead of line throught track center)
    int mCollTime;    // milliseconds after collision where bot's don't cheat
    int mCollTimeReturn;      // milliseconds it takes to get the bot back on track
    int mStartTime;   // first milliseconds in which startsettings are used
    float mLowPowerStart;  // lower border for random power first seconds
    float mUppPowerStart;  // upper border for random power first seconds
    float mPreviewDist1;
    float mPreviewDist2;
};

struct LevelCameraSettings
{
    void ReadFromXml(const TiXmlElement * settings_);

    std::string     mModel;
};

// settings used in levels.xml
struct LevelSettings
{
    LevelSettings();
    void ReadFromXml(const TiXmlElement * settings_);

    int             mId;
    std::wstring    mName;
    AiBotSettings   mBotSettings[MAX_BOT_SETTINGS];
    LevelCameraSettings mCameraSettings;
    std::string     mMeshFile;
    std::string     mDataFile;
    std::string     mLightmapFile;

    LevelDifficulty mDifficulty;
    int             mLaps;
    float           mTargetTime;

    int             mAmbientAlpha;
    int             mAmbientRed;
    int             mAmbientGreen;
    int             mAmbientBlue;

    int             mAmbHoverRed;
    int             mAmbHoverGreen;
    int             mAmbHoverBlue;

    float           mDefaultMarkerLeft;
    float           mDefaultMarkerRight;
    float           mDefaultMarkerTop;
    float           mDefaultMarkerBottom;

    float           mHudPosScaling;
};

class LevelManager
{
public:
    LevelManager();
    ~LevelManager();

    void LoadSettings();    // metadata for all levels
    bool LoadLevel(const std::wstring &levelName_, Level& level);

    int GetNumLevels(LevelDifficulty difficulty_ = LD_ALL) const;
    int GetCurrentLevelIndex()  const { return mCurrentLevel; }
    unsigned int GetRealIndex( unsigned int index_, LevelDifficulty difficulty_ = LD_ALL) const;
    int GetIndexForId(int id_) const;
    int GetIndexForName( const std::wstring &levelName_ ) const;

    const LevelSettings& GetLevelSettings(unsigned int index_, LevelDifficulty difficulty_ = LD_ALL) const;
    const LevelSettings& GetCurrentLevelSettings() const { return GetLevelSettings(mCurrentLevel); }

    // Bot settings for current level settings. index can just be increased for each bot - it wraps around when more bots are requested than we have settings.
    const AiBotSettings& GetBotSettings(BOT_DIFFICULTY botLevel, int index) const;

private:
    LevelSettings mDefaultLevelSettings;
    std::vector<LevelSettings> mLevelSettings;
    int mCurrentLevel;
};

#endif // LEVEL_MANAGER_H
