// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#include "level.h"
#include "level_manager.h"
#include "irrlicht_manager.h"
#include "main.h"
#include "config.h"
#include "physics.h"
#include "editor.h"
#include "mesh_texture_loader.h"
#include "node_manager.h"
#include "game.h"
#include "logging.h"
#include "helper_str.h"
#include "globals.h"
#include "tinyxml/tinyxml.h"
#include <math.h>
#include <float.h>

using namespace irr;

// --------------------------------------------------------
LevelModelSettings::LevelModelSettings()
{
    mScale.set(1.f, 1.f, 1.f);
    mAmbientRed = 255;
    mAmbientGreen = 255;
    mAmbientBlue = 255;
    mHasCollisions = false;
}

void LevelModelSettings::ReadFromXml(const TiXmlElement * settings_)
{
    assert(settings_);

    const char * val = settings_->Attribute("name");
    if ( val )
        mName = val;

    settings_->QueryFloatAttribute("c_x", &mCenter.X);
    settings_->QueryFloatAttribute("c_y", &mCenter.Y);
    settings_->QueryFloatAttribute("c_z", &mCenter.Z);

    settings_->QueryFloatAttribute("r_x", &mRotation.X);
    settings_->QueryFloatAttribute("r_y", &mRotation.Y);
    settings_->QueryFloatAttribute("r_z", &mRotation.Z);

    settings_->QueryFloatAttribute("s_x", &mScale.X);
    settings_->QueryFloatAttribute("s_y", &mScale.Y);
    settings_->QueryFloatAttribute("s_z", &mScale.Z);

    settings_->QueryIntAttribute("ar", &mAmbientRed);
    settings_->QueryIntAttribute("ag", &mAmbientGreen);
    settings_->QueryIntAttribute("ab", &mAmbientBlue);

    int iVal=0;
    settings_->QueryIntAttribute("coll", &iVal);
    mHasCollisions = iVal ? true : false;
}

void LevelModelSettings::WriteToXml(TiXmlElement * settings_) const
{
    assert(settings_);

    settings_->SetAttribute("name", mName.c_str() );

    settings_->SetDoubleAttribute("c_x", mCenter.X);
    settings_->SetDoubleAttribute("c_y", mCenter.Y);
    settings_->SetDoubleAttribute("c_z", mCenter.Z);

    settings_->SetDoubleAttribute("r_x", mRotation.X);
    settings_->SetDoubleAttribute("r_y", mRotation.Y);
    settings_->SetDoubleAttribute("r_z", mRotation.Z);

    settings_->SetDoubleAttribute("s_x", mScale.X);
    settings_->SetDoubleAttribute("s_y", mScale.Y);
    settings_->SetDoubleAttribute("s_z", mScale.Z);

    settings_->SetAttribute("ar", mAmbientRed);
    settings_->SetAttribute("ag", mAmbientGreen);
    settings_->SetAttribute("ab", mAmbientBlue);

    settings_->SetAttribute("coll", mHasCollisions ? 1 : 0 );
}

// --------------------------------------------------------
LevelModel::LevelModel()
: mNodeModel(0)
{
}

bool LevelModel::GetCenter( core::vector3df &pos_)
{
    pos_ = mSettings.mCenter;
    return true;
}


// --------------------------------------------------------
Level::Level()
{
    mLevelNode = NULL;
    mSelector = NULL;
    mEditDataNode = NULL;
    mTrackDataNode = NULL;
    mNodeCamera = NULL;
}

Level::~Level()
{
    if ( APP.GetIrrlichtManager()->GetIrrlichtDevice() )
    {
        if ( mSelector )
        {
            mSelector->drop();
        }
    }
}

bool Level::Load(const LevelSettings * settings_)
{
    LOG.Debug("Level::Load\n");

    if ( !settings_ )
    {
        LOG.Warn("Level::Load - settings_ is NULL\n");
        return false;
    }

	MeshTextureLoader* staticMeshTextureLoader = APP.GetIrrlichtManager()->GetStaticMeshTextureLoader();

    ClearTrackData();

    APP.GetPhysics()->ClearCollisionMeshes();

	if (mLevelNode)
	{
	    std::string newLevelName = APP.GetConfig()->MakeFilenameLevel(settings_->mMeshFile);
	    if( newLevelName != mLevelFileName )
	    {
	        scene::IAnimatedMesh * mesh = APP.GetIrrlichtManager()->GetMeshInCache( mLevelFileName.c_str() );
	        if ( mesh )
	        {
                APP.GetIrrlichtManager()->GetSceneManager()->getMeshCache()->removeMesh(mesh);
	        }

			if ( staticMeshTextureLoader )
			{
				staticMeshTextureLoader->clearTextureCache();
				staticMeshTextureLoader->setTextureCaching(MeshTextureLoader::ADD_TEXTURES);
			}
	    }

		mLevelNode->remove();
        mLevelNode = 0;
	}

    if ( mSelector)
    {
        mSelector->drop();
        mSelector = NULL;
    }

    if ( !mEditDataNode )
    {
        mEditDataNode = APP.GetIrrlichtManager()->GetSceneManager()->addEmptySceneNode();
        mEditDataNode->setVisible(false);
    }
    if ( !mTrackDataNode )
    {
        mTrackDataNode = APP.GetIrrlichtManager()->GetSceneManager()->addEmptySceneNode();
        mTrackDataNode->setVisible(false);
    }

    LOG.Debug("old level removed\n");

    mLevelFileName = APP.GetConfig()->MakeFilenameLevel(settings_->mMeshFile);
    LOG.LogLn( LP_DEBUG, "Load level ", mLevelFileName.c_str() );

    mLevelNode = APP.GetIrrlichtManager()->LoadModel(*APP.GetConfig(), mLevelFileName.c_str(), &mSelector);

    mLevelNode->setPosition(core::vector3df(0,0,0));
    mLevelNode->setRotation(core::vector3df(0,0,0));

	if ( staticMeshTextureLoader )
	{
		staticMeshTextureLoader->setTextureCaching(MeshTextureLoader::REMOVE_TEXTURES);
	}

    APP.GetIrrlichtManager()->GetSceneManager()->setAmbientLight(video::SColor(settings_->mAmbientAlpha, settings_->mAmbientRed, settings_->mAmbientGreen, settings_->mAmbientBlue));

    if ( mNodeCamera )
    {
        mNodeCamera->remove();
        mNodeCamera = NULL;
    }
    if ( APP.GetConfig()->GetParticleMode() == EPM_ON)
    {
        mNodeCamera = APP.GetNodeManager()->LoadNode( settings_->mCameraSettings.mModel );
    }
    APP.GetIrrlichtManager()->SetShadersUseDynamicLight(APP.GetConfig()->GetLightMode() == ELM_DYNAMIC);

    LOG.Debug("add level physics\n");
    APP.GetPhysics()->AddCollisionMesh(mSelector);

    LOG.Debug("load track data\n");
    LoadTrackData(settings_->mDataFile);

    LOG.Debug("build track data\n");
    BuildTrackData();

    APP.GetGame()->UpdateHoverAmbience(*settings_);

    LOG.Debug("level loaded\n");

    return true;
}

void Level::BuildTrackData()
{
    for ( int i=0; i < MAX_SPAWNS; ++i )
    {
        SetTrackStart(GetTrackStart(i).mSettings, i);
    }

    for ( int i=0; i < (int)mTrackMarkers.size(); ++i )
    {
        SetTrackMarker(mTrackMarkers[i].mSettings, i);
    }

    SetFinishLine(mFinishLine.mSettings);
    SetTeleportSource(mTpSource.mSettings);
    SetTeleportTarget(mTpTarget.mSettings);

    for ( int i=0; i <(int)mModels.size(); ++i )
    {
        SetModel(mModels[i].mSettings, i);
    }

    mAiTrack.CalculateCushions();
}

void Level::ClearTrackData()
{
    for ( int i=0; i < MAX_SPAWNS; ++i )
    {
        if ( GetTrackStart(i).mEditNode )
        {
            GetTrackStart(i).mEditNode->remove();
            GetTrackStart(i).mEditNode = NULL;
            GetTrackStart(i).mSettings.Reset();
        }
    }
    for ( size_t i=0; i < mTrackMarkers.size(); ++i )
    {
        mTrackMarkers[i].Clear();
    }
    mTrackMarkers.clear();
    mFinishLine.Clear();
    mTpSource.Clear();
    mTpTarget.Clear();
    for ( size_t i=0; i < mModels.size(); ++i )
    {
        ClearModelData(mModels[i]);
    }
    mModels.clear();
}

void Level::ClearModelData(LevelModel & model_)
{
    if ( model_.mNodeModel )
    {
        if ( model_.mSettings.mHasCollisions )
        {
            APP.GetPhysics()->RemoveCollisionMesh(model_.mNodeModel->getTriangleSelector());
        }

        model_.mNodeModel->remove();
        model_.mNodeModel = NULL;

        APP.GetNodeManager()->UnloadNode(model_.mSettings.mName);
    }
}

void Level::ShowEditData(bool enable_)
{
    if ( enable_)
    {
        if ( mEditDataNode )
            mEditDataNode->setVisible(true);
        if ( mTrackDataNode )
            mTrackDataNode->setVisible(true);
    }
    else
    {
        if ( mEditDataNode )
            mEditDataNode->setVisible(false);
        if ( mTrackDataNode )
            mTrackDataNode->setVisible(false);
    }
}

void Level::SetTrackStart(const TrackStartSettings& ts_, int player_)
{
    if ( ts_.mIsValid )
    {
#ifdef HC1_ENABLE_EDITOR
        scene::ISceneNode * node = GetTrackStart(player_).mEditNode;
        if ( !node )
        {
            scene::SMesh * mesh = APP.GetEditor()->GetDefaultArrowMesh();
            node = APP.GetIrrlichtManager()->GetSceneManager()->addMeshSceneNode(mesh, mEditDataNode, /*id*/1, ts_.mCenter);
            assert(node);

            scene::ITriangleSelector* selector = APP.GetIrrlichtManager()->GetSceneManager()->createTriangleSelector(mesh, node);
            node->setTriangleSelector(selector);
            selector->drop();

            GetTrackStart(player_).mEditNode = node;
        }
        node->setPosition(ts_.mCenter);
        node->setRotation(ts_.mRotation);
        node->updateAbsolutePosition();
#endif
        GetTrackStart(player_).mSettings = ts_;
    }
    else    // !ts_.mIsValid
    {
        GetTrackStart(player_).mSettings.Reset();
    }
}

TrackStart& Level::GetTrackStart(unsigned int index_)
{
    assert( index_ < MAX_SPAWNS );
    return mTrackStarts[index_];
}

unsigned int Level::GetNrOfTrackMarkers() const
{
    return mTrackMarkers.size();
}

void Level::SetTrackMarker(const TrackMarkerSettings &markerSettings_, int index_)
{
    mTrackMarkers[index_].SetTrackMarkerSettings(markerSettings_, mEditDataNode, mTrackDataNode);
    if ( index_ > 0 )
    {
        mTrackMarkers[index_-1].SetCollisionWalls(mTrackMarkers[index_], mTrackDataNode);
    }
    if ( index_+1 < (int)mTrackMarkers.size() )
    {
        mTrackMarkers[index_].SetCollisionWalls(mTrackMarkers[index_+1], mTrackDataNode);
    }
    if( mTrackMarkers.size() > 1 && (index_ == 0 || (index_+1 == (int)mTrackMarkers.size())) )
    {
        mTrackMarkers[mTrackMarkers.size()-1].SetCollisionWalls(mTrackMarkers[0], mTrackDataNode);
    }
}

void Level::InsertTrackMarker(const TrackMarkerSettings &markerSettings_, int index_)
{
    TrackMarker marker;
    marker.SetTrackMarkerSettings(markerSettings_, mEditDataNode, mTrackDataNode);
    mTrackMarkers.insert( mTrackMarkers.begin() + index_, marker);
    if ( index_ > 0 )
    {
        mTrackMarkers[index_-1].SetCollisionWalls(mTrackMarkers[index_], mTrackDataNode);
    }
    if ( index_+1 < (int)mTrackMarkers.size() )
    {
        mTrackMarkers[index_].SetCollisionWalls(mTrackMarkers[index_+1], mTrackDataNode);
    }
    if( mTrackMarkers.size() > 1 && index_ == 0 )
    {
        mTrackMarkers[mTrackMarkers.size()-1].SetCollisionWalls(mTrackMarkers[0], mTrackDataNode);
    }
}

void Level::AppendTrackMarker(const TrackMarkerSettings &markerSettings_, int index_)
{
    TrackMarker marker;
    marker.SetTrackMarkerSettings(markerSettings_, mEditDataNode, mTrackDataNode);
    mTrackMarkers.push_back(marker);
    if ( mTrackMarkers.size() > 1 )
    {
        mTrackMarkers[mTrackMarkers.size()-2].SetCollisionWalls(mTrackMarkers[mTrackMarkers.size()-1], mTrackDataNode);
        mTrackMarkers[mTrackMarkers.size()-1].SetCollisionWalls(mTrackMarkers[0], mTrackDataNode);
    }
}

void Level::RotateTrackMarker(float angle_, int index_)
{
    if ( index_ < 0 || index_ >= (int)mTrackMarkers.size() )
        return;

    mTrackMarkers[index_].mSettings.mRotation.Y += 180.f;
    SetTrackMarker(mTrackMarkers[index_].mSettings, index_);
}

void Level::AutoAlignTrackMarker(int index_)
{
#ifdef HC1_ENABLE_EDITOR
    if ( index_ < 0 || index_ >= (int)mTrackMarkers.size() )
        return;

    core::vector3df center = mTrackMarkers[index_].mSettings.mCenter;
    float angle_step_size = 18;
    float radius = 400.f;
    core::vector3df rotBase(radius, 0, 0);
    float angle=0.f;
    int switchCount = 0;
    bool firstCheckNoCollision = false;
    float switchAngles[2];
    while ( angle < 360.f )
    {
        core::line3d<f32> line;
        core::vector3df rotVec(rotBase);
        core::matrix4 rotMat;
        rotMat.setRotationDegrees( core::vector3df(0, angle, 0) );
        rotMat.rotateVect(rotVec);

        line.start = center+rotVec;
        line.end = line.start;
        line.start.Y += 150;
        line.end.Y -= 300;

        core::vector3df intersection;
        if ( APP.GetEditor()->CheckTrackCollision(line, intersection) )
        {
            if ( switchCount == 1 )
                angle = 360.f;
        }
        else
        {
            if ( angle == 0 )
                firstCheckNoCollision = true;
            switchAngles[switchCount] = angle;
            if ( switchCount == 0 )
            {
                ++switchCount;
                switchAngles[switchCount] = angle;
            }
        }

        angle += angle_step_size;
    }
    if ( firstCheckNoCollision )
    {
        angle = 360.f;

        while ( angle > 0.f )
        {
            core::line3d<f32> line;
            core::vector3df rotVec(rotBase);
            core::matrix4 rotMat;
            rotMat.setRotationDegrees( core::vector3df(0, angle, 0) );
            rotMat.rotateVect(rotVec);

            line.start = center+rotVec;
            line.end = line.start;
            line.start.Y += 150;
            line.end.Y -= 300;

            core::vector3df intersection;
            if ( APP.GetEditor()->CheckTrackCollision(line, intersection) )
                angle = 0.f;
            else
                switchAngles[0] = angle;

            angle -= angle_step_size;
        }
    }
    if ( switchCount > 0 )
    {
        if ( switchAngles[0] > switchAngles[1] + 180.f )
            switchAngles[1] += 360.f;
        else if ( switchAngles[1] > switchAngles[0] + 180.f )
            switchAngles[0] += 360.f;
        mTrackMarkers[index_].mSettings.mRotation.Y = (int)((switchAngles[0] + switchAngles[1]) * 0.5f) % 360;
        SetTrackMarker(mTrackMarkers[index_].mSettings, index_);
    }
#endif // HC1_ENABLE_EDITOR
}

void Level::ChangeTrackMarkerSizes(bool useDefault_, float changeLeft_, float changeRight_, float changeTop_, float changeBottom_, int index_)
{
    assert ( index_ >= 0 && index_ < (int)mTrackMarkers.size() );
    TrackMarkerSettings & settings = mTrackMarkers[index_].mSettings;
    if ( useDefault_ )
    {
        settings.mUseDefaultSizes = true;
    }
    else
    {
        if ( settings.mUseDefaultSizes )
        {
			const LevelSettings& defaultSettings = APP.GetLevelManager()->GetCurrentLevelSettings();
            settings.mLeft = defaultSettings.mDefaultMarkerLeft;
            settings.mRight = defaultSettings.mDefaultMarkerRight;
            settings.mTop = defaultSettings.mDefaultMarkerTop;
            settings.mBottom = defaultSettings.mDefaultMarkerBottom;
        }
        settings.mUseDefaultSizes = false;
        settings.mLeft += changeLeft_;
        settings.mRight += changeRight_;
        settings.mTop += changeTop_;
        settings.mBottom += changeBottom_;
    }
    SetTrackMarker(settings, index_);
}

void Level::RemoveTrackMarker(int index_)
{
    assert ( index_ >= 0 && index_ < (int)mTrackMarkers.size() );
    mTrackMarkers[index_].Clear();
    mTrackMarkers.erase( mTrackMarkers.begin() + index_);
    if ( index_ > 0 )
    {
        if ( index_ < (int)mTrackMarkers.size() )
        {
            mTrackMarkers[index_-1].SetCollisionWalls(mTrackMarkers[index_], mTrackDataNode);
        }
        else
        {
            mTrackMarkers[index_-1].RemoveCollisionWalls();
        }
    }
    if( mTrackMarkers.size() > 1 && (index_ == 0 || index_ >= (int)mTrackMarkers.size()) )
    {
        mTrackMarkers[mTrackMarkers.size()-1].SetCollisionWalls(mTrackMarkers[0], mTrackDataNode);
    }
    else if ( mTrackMarkers.size() == 1 )
    {
        mTrackMarkers[0].RemoveCollisionWalls();
    }
}

void Level::SetTeleportSource(const TrackMarkerSettings &markerSettings_)
{
    mTpSource.SetTrackMarkerSettings(markerSettings_, mEditDataNode, mTrackDataNode);
}

void Level::SetTeleportTarget(const TrackMarkerSettings &markerSettings_)
{
    mTpTarget.SetTrackMarkerSettings(markerSettings_, mEditDataNode, mTrackDataNode);
}

void Level::SetFinishLine(const TrackMarkerSettings &markerSettings_)
{
    mFinishLine.SetTrackMarkerSettings(markerSettings_, mEditDataNode, mTrackDataNode);
}

void Level::AddModel(const std::string &name_)
{
    LevelModel model;
    model.mSettings.mName = name_;

    model.mNodeModel = APP.GetNodeManager()->LoadNode(name_);

    mModels.push_back(model);
}

void Level::SetModel(const LevelModelSettings &settings_, int index_)
{
    LevelModel & model = mModels[index_];
    model.mSettings = settings_;
    if ( model.mNodeModel )
    {
        model.mNodeModel->setPosition(model.mSettings.mCenter);
        model.mNodeModel->setRotation(model.mSettings.mRotation);
        model.mNodeModel->updateAbsolutePosition();

        if ( model.mSettings.mHasCollisions )
        {
            APP.GetPhysics()->RemoveCollisionMesh(model.mNodeModel->getTriangleSelector());
            APP.GetPhysics()->AddCollisionMesh(model.mNodeModel->getTriangleSelector());
        }

        APP.GetIrrlichtManager()->SetNodeAndChildsAmbientMaterial(model.mNodeModel, model.mSettings.mAmbientRed, model.mSettings.mAmbientGreen, model.mSettings.mAmbientBlue, 0);
    }
}

void Level::ChangeModelRotation(float rotX_, float rotY_, float rotZ_, int index_)
{
    LevelModel & model = mModels[index_];
    model.mSettings.mRotation.X += rotX_;
    model.mSettings.mRotation.Y += rotY_;
    model.mSettings.mRotation.Z += rotZ_;
    SetModel(model.mSettings, index_);
}

void Level::ChangeModelPosition(float posX_, float posY_, float posZ_, int index_)
{
    LevelModel & model = mModels[index_];
    model.mSettings.mCenter.X += posX_;
    model.mSettings.mCenter.Y += posY_;
    model.mSettings.mCenter.Z += posZ_;
    SetModel(model.mSettings, index_);
}

void Level::SetModelAmbient(int ambR_, int ambG_, int ambB_, int index_)
{
    LevelModel & model = mModels[index_];
    model.mSettings.mAmbientRed = ambR_;
    model.mSettings.mAmbientGreen = ambG_;
    model.mSettings.mAmbientBlue = ambB_;
    SetModel(model.mSettings, index_);
}

void Level::RemoveModel(int index_)
{
    if ( index_ < 0 || index_ >= (int)mModels.size() )
        return;
    ClearModelData(mModels[index_]);
    mModels.erase( mModels.begin() + index_);
}

bool Level::CheckLineNodeCollision2T(const core::line3d<f32> &line_, scene::ISceneNode* node_, core::vector3df &outIntersection_) const
{
    if ( !node_ )
        return false;

    core::aabbox3d<f32> box(line_.start, line_.end);
    box.repair();
    core::aabbox3d<f32> box2(node_->getTransformedBoundingBox());

    if ( box.intersectsWithBox(box2) )
    {
        const s32 MAX_TRIANGLES = 2;
        s32 trianglesReceived = 0;
        core::triangle3df triangles[MAX_TRIANGLES];

        scene::ITriangleSelector* selector = node_->getTriangleSelector();
        selector->getTriangles( triangles, MAX_TRIANGLES, trianglesReceived, box, /*transform*/ 0 );

        for ( int i=0; i < trianglesReceived; ++i )
        {
            if ( triangles[i].getIntersectionWithLimitedLine(line_, outIntersection_) )
            {
                return true;
            }
        }
    }

    return false;
}

bool Level::CheckFinishLineCollision(const core::line3d<f32> &moveLine_) const
{
    if ( !mFinishLine.mNodeCollision )
        return false;

    core::vector3df outIntersection;
    return CheckLineNodeCollision2T(moveLine_, mFinishLine.mNodeCollision, outIntersection);
}

bool Level::CheckWallCollision(const core::line3d<f32> &moveLine_, int &index_) const
{
    for ( int i=0; i < (int)mTrackMarkers.size(); ++i)
    {
        core::vector3df outIntersection;
        const TrackMarker & marker = mTrackMarkers[i];

        if ( marker.mSettings.mHasLeftWall && marker.mNodeWallLeft)
        {
            if ( CheckLineNodeCollision2T(moveLine_, marker.mNodeWallLeft, outIntersection) )
            {
                index_ = i;
                return true;
            }
        }
        if ( marker.mSettings.mHasRightWall && marker.mNodeWallRight)
        {
            if ( CheckLineNodeCollision2T(moveLine_, marker.mNodeWallRight, outIntersection) )
            {
                index_ = i;
                return true;
            }
        }
        if ( marker.mSettings.mHasBottomWall && marker.mNodeWallBottom)
        {
            if ( CheckLineNodeCollision2T(moveLine_, marker.mNodeWallBottom, outIntersection) )
            {
                index_ = i;
                return true;
            }
        }
    }

    return false;
}

bool Level::CheckMarkerCollision(const core::line3d<f32> &moveLine_, int &index_) const
{
    for ( int i=0; i < (int)mTrackMarkers.size(); ++i)
    {
        core::vector3df outIntersection;

        const TrackMarker & marker = mTrackMarkers[i];
        if (    marker.mSettings.mRelocate
            &&  marker.mNodeCollision
            &&  CheckLineNodeCollision2T(moveLine_, marker.mNodeCollision, outIntersection)
            )
        {
            index_ = i;
            return true;
        }
    }

    return false;
}

bool Level::CheckTeleportLineCollision(const core::line3d<f32> &moveLine_, core::vector3df &targetPos_, core::vector3df &rotation_, core::vector3df &velocity_) const
{
    if ( !mTpSource.mNodeCollision || !mTpTarget.mNodeCollision )
        return false;

    core::vector3df outIntersection;
    if ( CheckLineNodeCollision2T(moveLine_, mTpSource.mNodeCollision, outIntersection) )
    {
        targetPos_ = outIntersection - mTpSource.mNodeCollision->getAbsolutePosition();
        core::matrix4 mat;
        mat.setRotationDegrees( mTpSource.mNodeCollision->getRotation() );
        mat.inverseRotateVect( targetPos_ );
        mat.setRotationDegrees( mTpTarget.mNodeCollision->getRotation() );
        mat.rotateVect( targetPos_ );

        targetPos_ += mTpTarget.mNodeCollision->getAbsolutePosition();
        rotation_ = mTpTarget.mSettings.mRotation;

        float oldVel = velocity_.getLength();
        velocity_.set(0,0,-1);
        core::matrix4 rotVel;
        rotVel.setRotationDegrees (mTpTarget.mNodeCollision->getRotation());
        rotVel.transformVect(velocity_);
        velocity_ *= oldVel;

        return true;
    }

    return false;
}

bool Level::CheckEditCollision(scene::ISceneNode* node_, const core::line3d<f32> line_, float &dist_) const
{
    scene::ISceneCollisionManager * collMan = APP.GetIrrlichtManager()->GetSceneManager()->getSceneCollisionManager();

    if ( node_ )
    {
        core::vector3df intersection;
        core::triangle3df tri;
        scene::ITriangleSelector* selector = node_->getTriangleSelector();
		irr::scene::ISceneNode* resultNode = 0;

        if (collMan->getCollisionPoint(line_, selector, intersection, tri, resultNode))
        {
            float len = (intersection - line_.start).getLength();
            if ( len < dist_ )
            {
                dist_ = len;
                return true;
            }
        }
    }
    return false;
}

bool Level::CheckTrackStartEditCollision(const core::line3d<f32> line_, int &index_, float &dist_)
{
    bool hasColl = false;
    for ( int i=0; i < MAX_SPAWNS; ++i )
    {
        if ( CheckEditCollision(GetTrackStart(i).mEditNode, line_, dist_) )
        {
            index_ = i;
            hasColl = true;
        }
    }

    return hasColl;
}

bool Level::CheckTrackMarkerEditCollision(const core::line3d<f32> line_, int &index_, float &dist_)
{
    bool hasColl = false;

    for ( int i=0; i < (int)mTrackMarkers.size(); ++i )
    {
        if ( CheckEditCollision(mTrackMarkers[i].mEditNodeCenter, line_, dist_) )
        {
            index_ = i;
            hasColl = true;
        }
    }

    return hasColl;
}

bool Level::CheckTpSourceEditCollision(const core::line3d<f32> line_, float &dist_)
{
    return CheckEditCollision(mTpSource.mEditNodeCenter, line_, dist_);
}

bool Level::CheckTpTargetEditCollision(const core::line3d<f32> line_, float &dist_)
{
    return CheckEditCollision(mTpTarget.mEditNodeCenter, line_, dist_);
}

bool Level::CheckFinishLineEditCollision(const core::line3d<f32> line_, int &index_, float &dist_)
{
    if ( CheckEditCollision(mFinishLine.mEditNodeCenter, line_, dist_) )
    {
        index_ = 0;
        return true;
    }

    return false;
}

bool Level::CheckModelEditCollision(const core::line3d<f32> line_, int &index_, float &dist_)
{
    bool hasColl = false;

    for ( int i=0; i < (int)mModels.size(); ++i )
    {
        if ( CheckEditCollision(mModels[i].mNodeModel, line_, dist_) )
        {
            index_ = i;
            hasColl = true;
        }
    }

    return hasColl;
}


bool Level::LoadTrackData(const std::string &fileName_)
{
	mTrackDataFilename = fileName_;
    std::string dataFileName = APP.GetConfig()->MakeFilenameLevel(fileName_);
    TiXmlDocument doc(dataFileName.c_str());
    doc.SetIrrFs(APP.GetIrrlichtManager()->GetIrrlichtDevice()->getFileSystem(), TiXmlDocument::E_ON_READ_ANDROID);
    if ( !doc.LoadFile())
    {
		LOG.DebugLn("TiXmlDocument::ErrorDesc: ", doc.ErrorDesc());
        return false;
	}

    TiXmlNode * nodeData = doc.FirstChild("data");
	if (!nodeData)
        return false;

    bool result = true;
    const TiXmlNode * node = nodeData->IterateChildren("trackstart", NULL);
    for ( int i=0; i<MAX_SPAWNS; ++i )
    {
        if ( node )
        {
            GetTrackStart(i).mSettings.ReadFromXml(node->ToElement());
            node = nodeData->IterateChildren("trackstart", node);
        }
    }

    node = nodeData->IterateChildren("finishline", NULL);
    if ( node )
        mFinishLine.mSettings.ReadFromXml(node->ToElement());

    node = nodeData->IterateChildren("tp_source", NULL);
    if ( node )
        mTpSource.mSettings.ReadFromXml(node->ToElement());

    node = nodeData->IterateChildren("tp_target", NULL);
    if ( node )
        mTpTarget.mSettings.ReadFromXml(node->ToElement());

    mTrackMarkers.clear();
    node = nodeData->IterateChildren("track_markers", NULL);
    if ( node )
    {
        const TiXmlNode * nodeMarker = node->IterateChildren("marker", NULL);
        while ( nodeMarker )
        {
            TrackMarker marker;
            marker.mSettings.ReadFromXml(nodeMarker->ToElement());
            mTrackMarkers.push_back(marker);
            nodeMarker = node->IterateChildren("marker", nodeMarker);
        }
    }

    mModels.clear();
    node = nodeData->IterateChildren("models", NULL);
    if ( node )
    {
        const TiXmlNode * nodeModel = node->IterateChildren("model", NULL);
        while ( nodeModel )
        {
            LevelModelSettings settings;
            settings.ReadFromXml(nodeModel->ToElement());

            AddModel(settings.mName);
            SetModel(settings, mModels.size()-1);

            nodeModel = node->IterateChildren("model", nodeModel);
        }
    }

    node = nodeData->IterateChildren("ai_track", NULL);
    if ( node )
    {
        result &= mAiTrack.ReadFromXml(node);
    }

    return result;
}

bool Level::SaveTrackData()
{
    std::string dataFileName = APP.GetConfig()->MakeFilenameLevel(mTrackDataFilename);
    TiXmlDocument doc(dataFileName.c_str());
    doc.SetIrrFs(APP.GetIrrlichtManager()->GetIrrlichtDevice()->getFileSystem(), TiXmlDocument::E_ON_READ_FAIL_ANDROID);

    if ( !doc.LoadFile() )
        return false;

	TiXmlNode * nodeData = doc.FirstChild("data");
	if (!nodeData)
        return false;

    bool result = true;

    TiXmlNode* node = nodeData->IterateChildren("trackstart", NULL);
    for ( int i=0; i<MAX_SPAWNS; ++i )
    {
        if ( !node )
        {
            TiXmlElement ele("trackstart");
            node = nodeData->InsertEndChild( ele );
        }
        assert(node);

        GetTrackStart(i).mSettings.WriteToXml(node->ToElement());
        node = nodeData->IterateChildren("trackstart", node);
    }

    node = nodeData->IterateChildren("finishline", NULL);
    if ( !node )
    {
        TiXmlElement ele("finishline");
        node = nodeData->InsertEndChild( ele );
    }
    assert(node);
    mFinishLine.mSettings.WriteToXml(node->ToElement());

    node = nodeData->IterateChildren("tp_source", NULL);
    if ( !node )
    {
        TiXmlElement ele("tp_source");
        node = nodeData->InsertEndChild( ele );
    }
    assert(node);
    mTpSource.mSettings.WriteToXml(node->ToElement());

    node = nodeData->IterateChildren("tp_target", NULL);
    if ( !node )
    {
        TiXmlElement ele("tp_target");
        node = nodeData->InsertEndChild( ele );
    }
    assert(node);
    mTpTarget.mSettings.WriteToXml(node->ToElement());

    // track markers
    node = nodeData->IterateChildren("track_markers", NULL);
    if ( !node )
    {
        TiXmlElement ele("track_markers");
        node = nodeData->InsertEndChild( ele );
    }
    assert(node);
    TiXmlElement  * eleTrackMarkers = node->ToElement();
    eleTrackMarkers->Clear();
    for ( unsigned int i=0; i < mTrackMarkers.size(); ++i )
    {
        TiXmlElement ele("marker");
        mTrackMarkers[i].mSettings.WriteToXml(&ele);
        eleTrackMarkers->InsertEndChild(ele);
    }

    // models
    node = nodeData->IterateChildren("models", NULL);
    if ( !node )
    {
        TiXmlElement ele("models");
        node = nodeData->InsertEndChild( ele );
    }
    assert(node);
    TiXmlElement  * eleModels = node->ToElement();
    eleModels->Clear();
    for ( unsigned int i=0; i < mModels.size(); ++i )
    {
        TiXmlElement ele("model");
        mModels[i].mSettings.WriteToXml(&ele);
        eleModels->InsertEndChild(ele);
    }

    // mAiTrackInfo
    node = nodeData->IterateChildren("ai_track", NULL);
    if ( !node )
    {
        TiXmlElement ele("ai_track");
        node = nodeData->InsertEndChild( ele );
    }
    assert(node);
    result &= mAiTrack.WriteToXml(node);
    result &= doc.SaveFile();

    return result;
}

int Level::GetTrackRelocatesBetween(int indexFront_, int indexBack_) const
{
    if ( !mTrackMarkers.size() )
        return -1;
    if ( indexFront_ < 0 || indexBack_ < 0 || indexFront_ >= (int)mTrackMarkers.size() || indexBack_ >= (int)mTrackMarkers.size() )
    {
        return -1;
    }

    int realFront_ = indexFront_;
    int problem = indexFront_;
    while ( !mTrackMarkers[realFront_].mSettings.mRelocate )
    {
        if ( --realFront_ < 0 )
        {
            realFront_ = mTrackMarkers.size()-1;
        }
        if ( realFront_ == problem )
            return -1;
    }

    int result = 0;
    while ( realFront_ != indexBack_ )
    {
        if ( --realFront_ < 0 )
        {
            realFront_ = mTrackMarkers.size()-1;
        }
        if ( mTrackMarkers[realFront_].mSettings.mRelocate )
            ++result;
    }
    return result;
}
