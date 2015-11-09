// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "level_manager.h"
#include "config.h"
#include "helper_str.h"
#include "irrlicht_manager.h"
#include "level.h"
#include "logging.h"
#include "main.h"
#include "tinyxml/tinyxml.h"

using namespace irr;

// --------------------------------------------------------
AiBotSettings::AiBotSettings()
{
    mName = "unknown bot";
    mModel = "hover_player";
    mLowPower = 1.f;
    mUppPower = 1.f;
    mSlowBorder = 0.9f;
    mLowSlow = 1.f;
    mUppSlow = 1.f;
    mLowEnergy = 1.f;
    mUppEnergy = 1.f;
    mLowIdeal = 1.f;
    mUppIdeal = 1.f;
    mCollTime = 1000;
    mCollTimeReturn = 2000;
    mStartTime = 1000;
    mLowPowerStart = 1.f;
    mUppPowerStart = 1.f;
    mPreviewDist1 = 150.f;
    mPreviewDist2 = 800.f;
}

void AiBotSettings::ReadFromXml(const TiXmlElement * settings_)
{
    if ( !settings_ )
        return;

    const char * val = settings_->Attribute("name");
    if ( val )
        mName = val;
    val = settings_->Attribute("model");
    if ( val )
        mModel = val;

    settings_->QueryFloatAttribute("pow_min", &mLowPower);
    settings_->QueryFloatAttribute("pow_max", &mUppPower);
    settings_->QueryFloatAttribute("slow_val", &mSlowBorder);
    settings_->QueryFloatAttribute("slow_min", &mLowSlow);
    settings_->QueryFloatAttribute("slow_max", &mUppSlow);
    settings_->QueryFloatAttribute("change_min", &mLowEnergy);
    settings_->QueryFloatAttribute("change_max", &mUppEnergy);
    settings_->QueryFloatAttribute("ideal_min", &mLowIdeal);
    settings_->QueryFloatAttribute("ideal_max", &mUppIdeal);
    settings_->QueryIntAttribute("coll_time", &mCollTime);
    settings_->QueryIntAttribute("coll_time_return", &mCollTimeReturn);
    settings_->QueryIntAttribute("start_time", &mStartTime);
    settings_->QueryFloatAttribute("start_min", &mLowPowerStart);
    settings_->QueryFloatAttribute("start_max", &mUppPowerStart);
    settings_->QueryFloatAttribute("preview_1", &mPreviewDist1);
    settings_->QueryFloatAttribute("preview_2", &mPreviewDist2);
}

// --------------------------------------------------------
void LevelCameraSettings::ReadFromXml(const TiXmlElement * settings_)
{
    if ( !settings_ )
        return;

    const char * val = settings_->Attribute("model");
    if ( val )
        mModel = val;
}

// --------------------------------------------------------
LevelSettings::LevelSettings()
{
    mId = 0;

    mLaps = 3;
    mTargetTime = 600.f;
    mDifficulty = LD_EASY;

    mAmbientAlpha = 150;
    mAmbientRed = 150;
    mAmbientGreen = 150;
    mAmbientBlue = 150;

    mAmbHoverRed = 255;
    mAmbHoverGreen = 255;
    mAmbHoverBlue = 255;

    mDefaultMarkerLeft = -500.f;
    mDefaultMarkerRight = 500.f;
    mDefaultMarkerTop = 300.f;
    mDefaultMarkerBottom = -300.f;

    mHudPosScaling = 1.f;
}

void LevelSettings::ReadFromXml(const TiXmlElement * settings_)
{
    if ( !settings_ )
        return;

    settings_->Attribute("id", &mId);
    const char * val = settings_->Attribute("name");
    if ( val )
    {
        std::string str(val);
        mName = ExtStr::FromUtf8(str);
    }
    val = settings_->Attribute("obj_file");
    if ( val )
        mMeshFile = val;
    val = settings_->Attribute("dat_file");
    if ( val )
        mDataFile = val;
    val = settings_->Attribute("lm_file");
    if ( val )
        mLightmapFile = val;
    val = settings_->Attribute("amb_alpha");
    if ( val )
        mAmbientAlpha = atoi(val);
    val = settings_->Attribute("amb_red");
    if ( val )
        mAmbientRed = atoi(val);
    val = settings_->Attribute("amb_green");
    if ( val )
        mAmbientGreen = atoi(val);
    val = settings_->Attribute("amb_blue");
    if ( val )
        mAmbientBlue = atoi(val);

    val = settings_->Attribute("hover_red");
    if ( val )
        mAmbHoverRed = atoi(val);
    val = settings_->Attribute("hover_green");
    if ( val )
        mAmbHoverGreen = atoi(val);
    val = settings_->Attribute("hover_blue");
    if ( val )
        mAmbHoverBlue = atoi(val);

    val = settings_->Attribute("laps");
    if ( val )
        mLaps = atoi(val);
    settings_->QueryFloatAttribute("targettime", &mTargetTime);

    val = settings_->Attribute("difficulty");
    if ( val )
    {
        int d = atoi(val);
        if (d >= LD_EASY && d <= LD_HARD )
            mDifficulty  = (LevelDifficulty)d;
        else
            mDifficulty = LD_EASY;
    }

    settings_->QueryFloatAttribute("hud_pos_scaling", &mHudPosScaling);
    settings_->QueryFloatAttribute("marker_left", &mDefaultMarkerLeft);
    settings_->QueryFloatAttribute("marker_right", &mDefaultMarkerRight);
    settings_->QueryFloatAttribute("marker_top", &mDefaultMarkerTop);
    settings_->QueryFloatAttribute("marker_bottom", &mDefaultMarkerBottom);

    int botId = 0;
    const TiXmlNode * nodeAiBot = settings_->IterateChildren("ai_bot", NULL);
    while ( nodeAiBot && botId < MAX_BOT_SETTINGS)
    {
        mBotSettings[botId].ReadFromXml(nodeAiBot->ToElement());
        ++botId;
        nodeAiBot = settings_->IterateChildren("ai_bot", nodeAiBot);
    }

    const TiXmlNode * nodeCam = settings_->IterateChildren("cam_node", NULL);
    if ( nodeCam )
    {
        mCameraSettings.ReadFromXml( nodeCam->ToElement() );
    }
}

// --------------------------------------------------------
LevelManager::LevelManager()
: mCurrentLevel(-1)
{
}

LevelManager::~LevelManager()
{
}

void LevelManager::LoadSettings()
{
	HC1_PROFILE(CProfileScope ps1(L"LvlMan:LoadSet", L"STARTUP");)

    std::string xmlName ( APP.GetConfig()->GetPathLevels() + "levels.xml" );
    TiXmlDocument xmlDoc(xmlName.c_str());
    xmlDoc.SetIrrFs(APP.GetIrrlichtManager()->GetIrrlichtDevice()->getFileSystem(), TiXmlDocument::E_ON_READ_ANDROID);

    if ( !xmlDoc.LoadFile() )
    {
		LOG.DebugLn("TiXmlDocument::ErrorDesc: ", xmlDoc.ErrorDesc());
		LOG.WarnLn("LevelManager::LoadSettings: could not load xml ", xmlName.c_str());
        return;
    }

    TiXmlNode * levelsNode = xmlDoc.FirstChild("levellist");
	if (!levelsNode)
		return;

    TiXmlNode * defaultNode = levelsNode->FirstChild("default");
    if ( defaultNode )
    {
        mDefaultLevelSettings.ReadFromXml(defaultNode->ToElement());
    }

    mLevelSettings.clear();
    TiXmlNode* nodeLevel = levelsNode->IterateChildren("level", NULL);
    while ( nodeLevel )
    {
        LevelSettings settings;
        settings = mDefaultLevelSettings;
        settings.ReadFromXml(nodeLevel->ToElement());
        mLevelSettings.push_back(settings);

        nodeLevel = levelsNode->IterateChildren("level", nodeLevel);
    }
}

bool LevelManager::LoadLevel(const std::wstring &levelName_, Level& level)
{
    int index = -1;
    for ( unsigned int i=0; i<mLevelSettings.size(); ++i)
    {
        if ( mLevelSettings[i].mName == levelName_ )
        {
            index = (int)i;
            break;
        }
    }

    if ( index < 0 )
        return false;

    mCurrentLevel = index;

    return level.Load(&(mLevelSettings[index]));
}

int LevelManager::GetNumLevels(LevelDifficulty difficulty_) const
{
    if ( LD_ALL == difficulty_ )
        return mLevelSettings.size();

    int count = 0;
    for ( unsigned int i=0; i < mLevelSettings.size(); ++i )
    {
        if ( mLevelSettings[i].mDifficulty == difficulty_ )
        {
            ++count;
        }
    }
    return count;
}

unsigned int LevelManager::GetRealIndex( unsigned int index_, LevelDifficulty difficulty_) const
{
    if ( index_ >= mLevelSettings.size() || difficulty_ == LD_ALL )
        return index_;

    unsigned int count = 0;
    for ( unsigned int i=0; i < mLevelSettings.size(); ++i )
    {
        if ( mLevelSettings[i].mDifficulty == difficulty_ )
        {
            if ( count == index_ )
            {
                return i;
            }
            ++count;
        }
    }
    return index_;
}

const LevelSettings& LevelManager::GetLevelSettings(unsigned int index_, LevelDifficulty difficulty_) const
{
    index_ = GetRealIndex( index_, difficulty_ );
    if ( index_ >= mLevelSettings.size() )
    {
        return mDefaultLevelSettings;
    }

    return mLevelSettings[index_];
}

const AiBotSettings& LevelManager::GetBotSettings(BOT_DIFFICULTY botLevel, int index) const
{
	const LevelSettings& levelSettings = GetCurrentLevelSettings();
	return levelSettings.mBotSettings[(int)botLevel*3 + index%3];
}

int LevelManager::GetIndexForId(int id_) const
{
    for ( unsigned int i=0; i < mLevelSettings.size(); ++i )
    {
        if ( mLevelSettings[i].mId == id_ )
        {
            return i;
        }
    }

    return -1;
}

int LevelManager::GetIndexForName( const std::wstring &levelName_ ) const
{
    LOG.Log(LP_DEBUG, "GetIndexForName - levelName: ");
    LOG.Log(LP_DEBUG, levelName_);
    LOG.Log(LP_DEBUG, "\n");
    for ( unsigned int i=0; i < mLevelSettings.size(); ++i )
    {
        if ( mLevelSettings[i].mName == levelName_ )
        {
            return i;
        }
    }

    return -1;
}
