// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#ifndef GAME_H
#define GAME_H

#include "irrlicht_manager.h"
#include "game_enums.h"
#include <vector>
#include <string>

class Physics;
struct PhysicsObject;
class Timer;
class TiXmlElement;
class Player;
class Camera;
class Recorder;
class Record;
class MemArchive;
class Highscores;
class GuiHud;
struct LevelSettings;

struct SteeringSettings
{
    SteeringSettings();
    void ReadFromXml(const TiXmlElement * settings_);
    void WriteToXml(TiXmlElement * settings_) const;

	int   mSteeringStyle;	// 0 = classic (up to h-craft 1.2). 1 = modern
    float mRotationSpeed;
    float mScaleMinSpeed;
    float mScaleRotationBySpeed;
    float mForceFactor;	// force forward
    float mBrakeFactor;
    float mForceSideFactor;
    float mScaleRolling;      // how fast does hover roll from controller
    float mScaleRollingBack;
    float mMaxRolling;        // don't roll more than that
    float mScalePitchFlying;  // how fast is pitch changed when in air
    float mScaleAlignment;    // how fast is hover aligned to floor
    int   mRelocateNoColl;    // ms to disable collisions after hover got teleported back to track
    int   mRelocateFreeze;    // ms to avoid moving after hover got teleported back to track
    int   mResumeFreeze;
};

// runtime settings for the game (options set each time before a new level is played)
struct GameSettings
{
    GameSettings();

    GAME_TYPE       mGameType;
    std::wstring    mLevelName;
    int             mNrOfBots;
    int             mNrOfLaps;  // 0 for infinite laps
    irr::u32        mTargetTime;
    BOT_DIFFICULTY  mBotDifficulty;
    GHOST_TYPE      mGhostType; // which ghost should be played
    std::string     mExportGhostRecord; // name without path
};

struct GameResult
{
    bool mGameWasFinished;
    int mLocalPlayerPlacing;
    int mLocalPlayerTrackTimeMs;
    int mLocalPlayerLastLapTimeMs;
    int mLocalPlayerBestLapTimeMs;  // only for last race
    int mLocalPlayerBestSessionLapTimeMs;
    int mLocalPlayerNumLaps;

    int mChampionshipScore;
};

struct GhostRecordSettings
{
    GhostRecordSettings();
    void Write(MemArchive * serializeArchive_) const;
    void Read( MemArchive * serializeArchive_);
    int CalcChecksum();

    int mVersion;
    int mChecksum;

    std::wstring    mProfileName;
    std::wstring    mLevelName;
    int             mGhostType;
    unsigned int    mTime;

private:
    static int VERSION;
};

class Game
{
public:
    Game();
    ~Game();

    bool Init();
    void StartSession();
    void PrepareStart();
    void Start();
    void Update();
    void Pause();
    void Resume();
    void Finish(bool finishByPlaying_);

    bool OnEvent(const irr::SEvent &event);

    GAME_MODES GetMode() const              { return mMode; }

    const Timer* GetGameTimer() const       { return mGameTimer; }
    irr::u32 GetRaceStartTime() const       { return mRaceStartTime; }  // time since end of countdown

    const Player* GetPlayer(size_t index_) const { return mPlayers[index_]; }
    const Player* GetLocalPlayer() const         { return mPlayers[mLocalPlayerIndex]; }

    void SetNextCameraActive();
    void SetActiveCameraIndex(size_t index_);
    size_t GetActiveCameraIndex() const     { return mActiveCameraIndex; }

    size_t GetNumCameras() const            { return mCameras.size(); }
    Camera* GetCamera(size_t index_)        { return mCameras[index_]; }

    void ReloadSettingsCamera();
    void ReloadSettingsSteering();
    void ReloadSettingsPhysics();
    void ReloadSettingsSound();

    const SteeringSettings& GetSteeringSettings() const  { return mSteeringSettings; }

    // save the last recording of the local player
    void SaveLastRecording();
    void SetAutosaveRecording(bool enable_);
    bool GetAutosaveRecording() const;
    void SetAutoloadRecording(bool enable_);
    bool GetAutoloadRecording() const;
    void SetLocalPlayerStartTrack(int trackId_)     { mLocalPlayerStartTrack = trackId_; }
    int GetLocalPlayerStartTrack() const            { return mLocalPlayerStartTrack; }

    // time which is used for "best times" when no time is given yet
    static irr::s32 TIME_UNUSED;

    void SetAiTraining(bool enable_);
    bool GetAiTraining() const                      { return mAiTraining; }

    // game settings
    void SetSettings(const GameSettings &settings_);
    const GameSettings& GetSettings() const         { return mSettings; }
    const GhostRecordSettings& GetExportGhostRecordSettings() const { return mExportGhostRecordSettings; }

    const GameResult& GetResult() const             { return mResult; }

    void UpdateHoverAmbience(const LevelSettings& settings_);

    const Highscores* GetCurrentTrackHighscores() const { return mTrackScores; }
    const Highscores* GetCurrentLapHighscores() const   { return mLapScores; }

    void ResetSessionRecord();
    irr::s32 GetSessionTrackRecord() const          { return mSessionTrackRecord; }

protected:
    void UpdateStart(GuiHud & guiHud);
    void UpdateResume();
    void UpdateFinished();
    void OnCrossedFinishLine(Player& player_, irr::u32 time_);
    void UpdateHud(const Player& player_, const PhysicsObject& physicsObj_, irr::u32 time_);
    void CheckCollisions(Player& player_, irr::u32 time_);

    void ResetResult();
    void CalcGameResult(bool finishByPlaying_);

    void OnPhysicsTick(unsigned int ticksSinceGameStart_);
    void PlayRecords();
    void CheckTrackCollisions();
    void GetRelocationPos(int wallIndex_, irr::core::vector3df &pos_, irr::core::vector3df &rotation_);
    void ResetRecord(Record* record_);
    void AddPlayer(const wchar_t * name_, const char * model_);

    void UpdatePlacings();
    float GetRelativeTrackDistanceToPlayer(size_t sourcePlayer, size_t targetPlayer, irr::s32 &timeDiff_);
    void ResetHud();
    bool HasSessionGhost() const;

private:

    struct SortHelper
    {
        size_t mPlayerId;
        float mRating;

        bool operator<(const SortHelper& other_)
        {
            return other_.mRating < mRating;    // we want higher scores to have lower pos
        }
    };

    bool                mIsStartPrepared;
    bool                mFinishedByPlaying;
    GAME_MODES          mMode;
    GAME_MODES          mResumeMode;    // mode which will be set when the game is resumed after pause
    Timer*              mGameTimer;
    Timer*              mGameFinishedTimer;
    irr::u32            mGameStartTime;
    irr::u32            mRaceStartTime;
    irr::u32            mResumeGameTime;
    Recorder*           mRecorder;
    size_t              mActiveCameraIndex;
    size_t              mLocalPlayerIndex;  // always 0
    int                 mLocalPlayerStartTrack;
    int                 mLocalPlayerPreviousStartTrack;
    bool                mAutosaveRecording;
    bool                mAutoloadRecording;
    bool                mHasGhost;

    bool                mAiTraining;
    irr::core::position2d<irr::s32> mOldCursorPos;  // only needed for cam in training mode

    bool                mHasRecordBestLap;
    Record              *mRecordBestLap;        // for playing the best available lap
    Record              *mRecordLocalPlayerLap; // does always record the last lap
    bool                mHasRecordBestTrack;
    Record              *mRecordLocalPlayer;
    Record              *mRecordBestTrack;
    Record              *mRecordExportGhost;    // for playing exported ghost
    GhostRecordSettings mExportGhostRecordSettings;
    irr::s32            mSessionTrackRecord;    //  TIME_UNUSED for invalid
    Record              *mRecordSession;

    SteeringSettings    mSteeringSettings;

    std::vector<Player*> mPlayers;
    std::vector<Camera*> mCameras;

    int mSoundRefCountdown;

    GameResult          mResult;
    GameSettings        mSettings;
    Highscores*         mTrackScores;
    Highscores*         mLapScores;
};

#endif // GAME_H
