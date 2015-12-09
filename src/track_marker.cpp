// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.
#include "track_marker.h"
#include "editor.h"
#include "irrlicht_manager.h"
#include "globals.h"
#include "level_manager.h"
#include "main.h"
#include "tinyxml/tinyxml.h"

using namespace irr;

// --------------------------------------------------------
void TrackStartSettings::ReadFromXml(const TiXmlElement * settings_)
{
    assert(settings_);

    int valid=0;
    settings_->QueryIntAttribute("is_valid", &valid);
    mIsValid = valid ? true : false;

    settings_->QueryFloatAttribute("c_x", &mCenter.X);
    settings_->QueryFloatAttribute("c_y", &mCenter.Y);
    settings_->QueryFloatAttribute("c_z", &mCenter.Z);

    settings_->QueryFloatAttribute("r_x", &mRotation.X);
    settings_->QueryFloatAttribute("r_y", &mRotation.Y);
    settings_->QueryFloatAttribute("r_z", &mRotation.Z);
}

void TrackStartSettings::WriteToXml(TiXmlElement * settings_) const
{
    assert(settings_);

    settings_->SetAttribute("is_valid", mIsValid ? 1 : 0 );

    settings_->SetDoubleAttribute("c_x", mCenter.X);
    settings_->SetDoubleAttribute("c_y", mCenter.Y);
    settings_->SetDoubleAttribute("c_z", mCenter.Z);

    settings_->SetDoubleAttribute("r_x", mRotation.X);
    settings_->SetDoubleAttribute("r_y", mRotation.Y);
    settings_->SetDoubleAttribute("r_z", mRotation.Z);
}

// --------------------------------------------------------
TrackMarkerSettings::TrackMarkerSettings()
{
    Reset();
}

void TrackMarkerSettings::Reset()
{
    mIsValid = false;
    mUseDefaultSizes = true;
    mHasLeftWall = false;
    mHasRightWall = false;
//    mHasTopWall = false;
    mHasBottomWall = false;
    mRelocate = true;
}

void TrackMarkerSettings::EnableWalls()
{
    mHasLeftWall = true;
    mHasRightWall = true;
//    mHasTopWall = true;
    mHasBottomWall = true;
}

void TrackMarkerSettings::ReadFromXml(const TiXmlElement * settings_)
{
    assert(settings_);

    int val=0;
    settings_->QueryIntAttribute("is_valid", &val);
    mIsValid = val ? true : false;

    const char * valChar = settings_->Attribute("relocate");
    if ( valChar )
        mRelocate = atoi(valChar) ? true : false;

    settings_->QueryFloatAttribute("c_x", &mCenter.X);
    settings_->QueryFloatAttribute("c_y", &mCenter.Y);
    settings_->QueryFloatAttribute("c_z", &mCenter.Z);

    settings_->QueryFloatAttribute("u_x", &mUpVector.X);
    settings_->QueryFloatAttribute("u_y", &mUpVector.Y);
    settings_->QueryFloatAttribute("u_z", &mUpVector.Z);

    settings_->QueryFloatAttribute("r_x", &mRotation.X);
    settings_->QueryFloatAttribute("r_y", &mRotation.Y);
    settings_->QueryFloatAttribute("r_z", &mRotation.Z);

    valChar = settings_->Attribute("def_sizes");
    if ( valChar )
        mUseDefaultSizes = atoi(valChar) ? true : false;

    settings_->QueryFloatAttribute("left", &mLeft);
    settings_->QueryFloatAttribute("right", &mRight);
    settings_->QueryFloatAttribute("top", &mTop);
    settings_->QueryFloatAttribute("bottom", &mBottom);

    settings_->QueryIntAttribute("wall_left", &val);
    mHasLeftWall = val ? true : false;
    settings_->QueryIntAttribute("wall_right", &val);
    mHasRightWall = val ? true : false;
//    settings_->QueryIntAttribute("wall_top", &val);
//    mHasTopWall = val ? true : false;
    settings_->QueryIntAttribute("wall_bottom", &val);
    mHasBottomWall = val ? true : false;
}

void TrackMarkerSettings::WriteToXml(TiXmlElement * settings_) const
{
    assert(settings_);

    settings_->SetAttribute("is_valid", mIsValid ? 1 : 0 );

    settings_->SetAttribute("relocate", mRelocate ? 1 : 0 );

    settings_->SetDoubleAttribute("c_x", mCenter.X);
    settings_->SetDoubleAttribute("c_y", mCenter.Y);
    settings_->SetDoubleAttribute("c_z", mCenter.Z);

    settings_->SetDoubleAttribute("u_x", mUpVector.X);
    settings_->SetDoubleAttribute("u_y", mUpVector.Y);
    settings_->SetDoubleAttribute("u_z", mUpVector.Z);

    settings_->SetDoubleAttribute("r_x", mRotation.X);
    settings_->SetDoubleAttribute("r_y", mRotation.Y);
    settings_->SetDoubleAttribute("r_z", mRotation.Z);

    settings_->SetAttribute("def_sizes", mUseDefaultSizes ? 1 : 0 );
    settings_->SetDoubleAttribute("left", mLeft);
    settings_->SetDoubleAttribute("right", mRight);
    settings_->SetDoubleAttribute("top", mTop);
    settings_->SetDoubleAttribute("bottom", mBottom);

    settings_->SetAttribute("wall_left", mHasLeftWall ? 1 : 0 );
    settings_->SetAttribute("wall_right", mHasRightWall? 1 : 0 );
//    settings_->SetAttribute("wall_top", mHasTopWall ? 1 : 0 );
    settings_->SetAttribute("wall_bottom", mHasBottomWall ? 1 : 0 );
}

// --------------------------------------------------------
bool TrackStart::GetCenter( core::vector3df &pos_) const
{
    if ( mSettings.mIsValid )
	{
		pos_ = mSettings.mCenter;
		return true;
	}
	return false;
}

// --------------------------------------------------------
TrackMarker::TrackMarker()
: mEditNodeCenter(0)
, mEditNodeLeftTop(0)
, mEditNodeRightBottom(0)
, mNodeCollision(0)
, mNodeWallLeft(0)
, mNodeWallRight(0)
, mNodeWallTop(0)
, mNodeWallBottom(0)
{
}

bool TrackMarker::GetCenter( core::vector3df &pos_) const
{
    if ( mSettings.mIsValid )
	{
		pos_ = mSettings.mCenter;
		return true;
	}
	return false;
}

void TrackMarker::Clear()
{
    if ( mEditNodeCenter )
    {
        mEditNodeCenter->remove();
        mEditNodeCenter = NULL;
    }
    if ( mEditNodeLeftTop )
    {
        mEditNodeLeftTop->remove();
        mEditNodeLeftTop = NULL;
    }
    if ( mEditNodeRightBottom )
    {
        mEditNodeRightBottom->remove();
        mEditNodeRightBottom = NULL;
    }
    if ( mNodeCollision )
    {
        mNodeCollision->remove();
        mNodeCollision = NULL;
    }
    RemoveCollisionWalls();

    mSettings.Reset();
}

void TrackMarker::SetCollisionWalls(const TrackMarker & nextMarker, irr::scene::ISceneNode* parentNode)
{
    RemoveCollisionWalls();

    core::vector3df leftTopVec1, rightBottomVec1, rightTopVec1, leftBottomVec1;
    CalcMarkerBorders(leftTopVec1, rightTopVec1, leftBottomVec1, rightBottomVec1, false);
    core::vector3df leftTopVec2, rightBottomVec2, rightTopVec2, leftBottomVec2;
    nextMarker.CalcMarkerBorders(leftTopVec2, rightTopVec2, leftBottomVec2, rightBottomVec2, false);

    if ( mSettings.mHasLeftWall )
    {
        mNodeWallLeft = APP.GetIrrlichtManager()->AddQuadradicNode(parentNode, leftTopVec1, leftTopVec2, leftBottomVec1, leftBottomVec2 );
    }
    if ( mSettings.mHasRightWall )
    {
        mNodeWallRight = APP.GetIrrlichtManager()->AddQuadradicNode(parentNode, rightTopVec1, rightTopVec2, rightBottomVec1, rightBottomVec2 );
    }
    if ( mSettings.mHasBottomWall )
    {
        mNodeWallBottom = APP.GetIrrlichtManager()->AddQuadradicNode(parentNode, leftBottomVec1, rightBottomVec1, leftBottomVec2, rightBottomVec2 );
    }
}

void TrackMarker::RemoveCollisionWalls()
{
    if ( mNodeWallLeft )
    {
        mNodeWallLeft->remove();
        mNodeWallLeft = NULL;
    }
    if ( mNodeWallRight )
    {
        mNodeWallRight->remove();
        mNodeWallRight = NULL;
    }
    if ( mNodeWallTop )
    {
        mNodeWallTop->remove();
        mNodeWallTop = NULL;
    }
    if ( mNodeWallBottom )
    {
        mNodeWallBottom->remove();
        mNodeWallBottom = NULL;
    }
}

void TrackMarker::CalcMarkerBorders(core::vector3df & leftTop_, core::vector3df & rightTop_, core::vector3df & leftBottom_, core::vector3df & rightBottom_, bool relative_) const
{
    core::matrix4 rotMat;
    rotMat.setRotationDegrees(mSettings.mRotation);
    core::vector3df forward(0, 0, 1);
    rotMat.rotateVect(forward);
    float left, right, top, bottom;
    if ( mSettings.mUseDefaultSizes )
    {
		const LevelSettings& defaultSettings = APP.GetLevelManager()->GetCurrentLevelSettings();
        left = defaultSettings.mDefaultMarkerLeft;
        right = defaultSettings.mDefaultMarkerRight;
        top = defaultSettings.mDefaultMarkerTop;
        bottom = defaultSettings.mDefaultMarkerBottom;
    }
    else
    {
        left = mSettings.mLeft;
        right = mSettings.mRight;
        top = mSettings.mTop;
        bottom = mSettings.mBottom;
    }

    core::vector3df leftVec(mSettings.mUpVector.crossProduct(forward));
//    printf("up: %.3f %.3f %.3f\n", mSettings.mUpVector.X, mSettings.mUpVector.Y, mSettings.mUpVector.Z);
//    printf("forward: %.3f %.3f %.3f\n", forward.X, forward.Y, forward.Z );
//    printf("leftVec: %.3f %.3f %.3f\n", leftVec.X, leftVec.Y, leftVec.Z );

    leftTop_ = (leftVec * left + mSettings.mUpVector * top);
    rightBottom_ = (leftVec * right + mSettings.mUpVector * bottom);
    rightTop_ = (leftVec * right + mSettings.mUpVector * top);
    leftBottom_ = (leftVec * left + mSettings.mUpVector * bottom);

    if ( !relative_ )
    {
        leftTop_ += mSettings.mCenter;
        rightBottom_ += mSettings.mCenter;
        rightTop_ += mSettings.mCenter;
        leftBottom_ += mSettings.mCenter;
    }
}

void TrackMarker::SetTrackMarkerSettings(const TrackMarkerSettings &settings_, irr::scene::ISceneNode* editNodeParent, irr::scene::ISceneNode* trackNodeParent)
{
    if ( settings_.mIsValid )
    {
        mSettings = settings_;

        core::vector3df leftTopVec, rightBottomVec, rightTopVec, leftBottomVec;
        CalcMarkerBorders(leftTopVec, rightTopVec, leftBottomVec, rightBottomVec );

        scene::ISceneNode * nodeCollision = mNodeCollision;
#ifdef HC1_ENABLE_EDITOR
        scene::ISceneNode * nodeCenter = mEditNodeCenter;
        scene::ISceneNode * nodeLeftTop = mEditNodeLeftTop;
        scene::ISceneNode * nodeRightBottom = mEditNodeRightBottom;
        if ( !nodeCenter )
        {
            scene::SMesh * mesh = APP.GetEditor()->GetDefaultArrowMesh();
            nodeCenter = APP.GetIrrlichtManager()->GetSceneManager()->addMeshSceneNode(mesh, editNodeParent);
            assert(nodeCenter);

            scene::ITriangleSelector* selector = APP.GetIrrlichtManager()->GetSceneManager()->createTriangleSelector(mesh, nodeCenter);
            nodeCenter->setTriangleSelector(selector);
            selector->drop();

            mEditNodeCenter = nodeCenter;
        }
        if ( !nodeLeftTop )
        {
            scene::SMesh * mesh = APP.GetEditor()->GetDefaultBoxMesh();
            nodeLeftTop = APP.GetIrrlichtManager()->GetSceneManager()->addMeshSceneNode(mesh, editNodeParent);
            assert(nodeLeftTop);

            scene::ITriangleSelector* selector = APP.GetIrrlichtManager()->GetSceneManager()->createTriangleSelector(mesh, nodeLeftTop);
            nodeLeftTop->setTriangleSelector(selector);
            selector->drop();

            mEditNodeLeftTop = nodeLeftTop;
        }
        if ( !nodeRightBottom )
        {
            scene::SMesh * mesh = APP.GetEditor()->GetDefaultBoxMesh();
            nodeRightBottom = APP.GetIrrlichtManager()->GetSceneManager()->addMeshSceneNode(mesh, editNodeParent);
            assert(nodeRightBottom);

            scene::ITriangleSelector* selector = APP.GetIrrlichtManager()->GetSceneManager()->createTriangleSelector(mesh, nodeRightBottom);
            nodeRightBottom->setTriangleSelector(selector);
            selector->drop();

            mEditNodeRightBottom = nodeRightBottom;
        }

        nodeCenter->setPosition(settings_.mCenter);
        nodeCenter->setRotation(settings_.mRotation);
        nodeCenter->updateAbsolutePosition();

        nodeLeftTop->setPosition(settings_.mCenter + leftTopVec);
        nodeLeftTop->updateAbsolutePosition();
        nodeRightBottom->setPosition(settings_.mCenter + rightBottomVec);
        nodeRightBottom->updateAbsolutePosition();
#endif // HC1_ENABLE_EDITOR

        if ( nodeCollision )
        {
            nodeCollision->remove();
            nodeCollision = NULL;
        }
        nodeCollision = APP.GetIrrlichtManager()->AddQuadradicNode(trackNodeParent, leftTopVec,rightTopVec, leftBottomVec, rightBottomVec);
        nodeCollision->setPosition(settings_.mCenter);
        nodeCollision->updateAbsolutePosition();

        mNodeCollision = nodeCollision;
    }
    else    // !mIsValid
    {
        mSettings.Reset();
    }
}
