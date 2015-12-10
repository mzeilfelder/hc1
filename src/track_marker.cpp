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
    mHasBottomWall = false;
    mRelocate = true;
}

void TrackMarkerSettings::EnableWalls()
{
    mHasLeftWall = true;
    mHasRightWall = true;
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
{
	for ( int i=0; i < WALL_COUNT; ++i )
		mWallNode[i] = NULL;
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

irr::core::vector3df TrackMarker::GetCenter() const
{
	return mSettings.mCenter;
}

bool TrackMarker::CheckLineWallCollision(const irr::core::line3d<irr::f32> &line_, EWall wall, irr::core::vector3df &outIntersection_) const
{
	if ( wall == WALL_LEFT && !mSettings.mHasLeftWall )
		return false;
	else if ( wall == WALL_RIGHT && !mSettings.mHasRightWall )
		return false;
	else if ( wall == WALL_BOTTOM && !mSettings.mHasBottomWall )
		return false;

    core::aabbox3d<f32> box(line_.start, line_.end);
    box.repair();
    const core::aabbox3d<f32>& box2 = mWallBoxes[wall];

    if ( box.intersectsWithBox(box2) )
    {
        for ( int i=0; i < 2; ++i )
        {
        	const irr::core::triangle3df& triangle = mWallTriangles[wall][i];
			if ( 	!triangle.isTotalOutsideBox(box)
				&&	triangle.getIntersectionWithLimitedLine(line_, outIntersection_) )
            {
                return true;
            }
        }
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

	if ( mWallNode[WALL_CENTER] )
	{
		mWallNode[WALL_CENTER]->remove();
		mWallNode[WALL_CENTER] = NULL;
	}
    RemoveSideWalls();

    mSettings.Reset();
}

void TrackMarker::SetWall(EWall wall, irr::scene::ISceneNode* parent_, const irr::core::vector3df &leftTop_, const irr::core::vector3df &rightTop_, const irr::core::vector3df &leftBottom_, const irr::core::vector3df &rightBottom_)
{
	mWallNode[wall] = APP.GetIrrlichtManager()->AddQuadradicNode(parent_, leftTop_, rightTop_, leftBottom_, rightBottom_);
	if ( mWallNode[wall] )
	{
        mWallNode[wall]->setPosition(mSettings.mCenter);
        mWallNode[wall]->updateAbsolutePosition();

        mWallBoxes[wall] = mWallNode[wall]->getTransformedBoundingBox();

		scene::ITriangleSelector* selector = mWallNode[wall]->getTriangleSelector();
		if ( selector  )
		{
			s32 trianglesReceived = 0;
			selector->getTriangles(mWallTriangles[wall], 2, trianglesReceived);
		}
	}
}

void TrackMarker::SetSideWalls(const TrackMarker & nextMarker, irr::scene::ISceneNode* parentNode)
{
    RemoveSideWalls();

    core::vector3df leftTopVec1, rightBottomVec1, rightTopVec1, leftBottomVec1;
    CalcMarkerBorders(leftTopVec1, rightTopVec1, leftBottomVec1, rightBottomVec1);

	irr::core::vector3df toNext(nextMarker.mSettings.mCenter - mSettings.mCenter);
    core::vector3df leftTopVec2, rightBottomVec2, rightTopVec2, leftBottomVec2;
    nextMarker.CalcMarkerBorders(leftTopVec2, rightTopVec2, leftBottomVec2, rightBottomVec2);
    leftTopVec2 += toNext;
    rightTopVec2 += toNext;
    leftBottomVec2 += toNext;
    rightBottomVec2 += toNext;

    if ( mSettings.mHasLeftWall )
    {
    	SetWall(WALL_LEFT, parentNode, leftTopVec1, leftTopVec2, leftBottomVec1, leftBottomVec2);
    }
    if ( mSettings.mHasRightWall )
    {
        SetWall(WALL_RIGHT, parentNode, rightTopVec1, rightTopVec2, rightBottomVec1, rightBottomVec2 );
    }
    if ( mSettings.mHasBottomWall )
    {
        SetWall(WALL_BOTTOM, parentNode, leftBottomVec1, rightBottomVec1, leftBottomVec2, rightBottomVec2 );
    }
}

void TrackMarker::RemoveSideWalls()
{
	for ( int i = WALL_LEFT; i <= WALL_BOTTOM; ++i )
	{
		if ( mWallNode[i] )
		{
			mWallNode[i]->remove();
			mWallNode[i] = NULL;
		}
	}
}

void TrackMarker::CalcMarkerBorders(core::vector3df & leftTop_, core::vector3df & rightTop_, core::vector3df & leftBottom_, core::vector3df & rightBottom_) const
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
    leftTop_ = (leftVec * left + mSettings.mUpVector * top);
    rightBottom_ = (leftVec * right + mSettings.mUpVector * bottom);
    rightTop_ = (leftVec * right + mSettings.mUpVector * top);
    leftBottom_ = (leftVec * left + mSettings.mUpVector * bottom);
}

void TrackMarker::SetTrackMarkerSettings(const TrackMarkerSettings &settings_, irr::scene::ISceneNode* editNodeParent, irr::scene::ISceneNode* trackNodeParent)
{
    if ( settings_.mIsValid )
    {
        mSettings = settings_;

        core::vector3df leftTopVec, rightBottomVec, rightTopVec, leftBottomVec;
        CalcMarkerBorders(leftTopVec, rightTopVec, leftBottomVec, rightBottomVec );

#ifdef HC1_ENABLE_EDITOR
        if ( !mEditNodeCenter )
        {
            scene::SMesh * mesh = APP.GetEditor()->GetDefaultArrowMesh();
            mEditNodeCenter = APP.GetIrrlichtManager()->GetSceneManager()->addMeshSceneNode(mesh, editNodeParent);
            assert(mEditNodeCenter);

            scene::ITriangleSelector* selector = APP.GetIrrlichtManager()->GetSceneManager()->createTriangleSelector(mesh, mEditNodeCenter);
            mEditNodeCenter->setTriangleSelector(selector);
            selector->drop();
        }
        mEditNodeCenter->setPosition(settings_.mCenter);
        mEditNodeCenter->setRotation(settings_.mRotation);
        mEditNodeCenter->updateAbsolutePosition();

        if ( !mEditNodeLeftTop )
        {
            scene::SMesh * mesh = APP.GetEditor()->GetDefaultBoxMesh();
            mEditNodeLeftTop = APP.GetIrrlichtManager()->GetSceneManager()->addMeshSceneNode(mesh, editNodeParent);
            assert(mEditNodeLeftTop);

            scene::ITriangleSelector* selector = APP.GetIrrlichtManager()->GetSceneManager()->createTriangleSelector(mesh, mEditNodeLeftTop);
            mEditNodeLeftTop->setTriangleSelector(selector);
            selector->drop();
		}
        mEditNodeLeftTop->setPosition(settings_.mCenter + leftTopVec);
        mEditNodeLeftTop->updateAbsolutePosition();

        if ( !mEditNodeRightBottom )
        {
            scene::SMesh * mesh = APP.GetEditor()->GetDefaultBoxMesh();
            mEditNodeRightBottom = APP.GetIrrlichtManager()->GetSceneManager()->addMeshSceneNode(mesh, editNodeParent);
            assert(mEditNodeRightBottom);

            scene::ITriangleSelector* selector = APP.GetIrrlichtManager()->GetSceneManager()->createTriangleSelector(mesh, mEditNodeRightBottom);
            mEditNodeRightBottom->setTriangleSelector(selector);
            selector->drop();
        }
        mEditNodeRightBottom->setPosition(settings_.mCenter + rightBottomVec);
        mEditNodeRightBottom->updateAbsolutePosition();
#endif // HC1_ENABLE_EDITOR

		if ( mWallNode[WALL_CENTER] )
        {
            mWallNode[WALL_CENTER]->remove();
            mWallNode[WALL_CENTER] = NULL;
        }
        SetWall(WALL_CENTER, trackNodeParent, leftTopVec, rightTopVec, leftBottomVec, rightBottomVec);
    }
    else    // !mIsValid
    {
        mSettings.Reset();
    }
}
