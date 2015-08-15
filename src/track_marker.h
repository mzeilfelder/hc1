// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

#ifndef TRACK_MARKER_H
#define TRACK_MARKER_H

#include "irrlicht.h"

class TiXmlElement;

struct TrackStartSettings
{
    TrackStartSettings() : mIsValid(false) {}
    void ReadFromXml(const TiXmlElement * settings_);
    void WriteToXml(TiXmlElement * settings_) const;
    void Reset()    { mIsValid = false; }

    bool            mIsValid;
    irr::core::vector3df mCenter;
    irr::core::vector3df mRotation;
};

struct TrackMarkerSettings
{
    TrackMarkerSettings();
    void ReadFromXml(const TiXmlElement * settings_);
    void WriteToXml(TiXmlElement * settings_) const;
    void Reset();
    void EnableWalls();

    bool            mIsValid;
    irr::core::vector3df mCenter;
    irr::core::vector3df mUpVector;
    irr::core::vector3df mRotation;
    bool            mRelocate;  // hover can teleport to this track when he drops off
    bool            mUseDefaultSizes;
    float           mLeft;
    float           mRight;
    float           mTop;
    float           mBottom;

    bool            mHasLeftWall;
    bool            mHasRightWall;
    bool            mHasBottomWall;
//    bool            mHasTopWall;
};

struct TrackStart
{
    TrackStart() : mEditNode(0) {}

    bool GetCenter( irr::core::vector3df &pos_);

    irr::scene::ISceneNode*  mEditNode;
    TrackStartSettings  mSettings;
};

struct TrackMarker
{
	friend class Level;

    TrackMarker();

    bool GetCenter( irr::core::vector3df &pos_);

    TrackMarkerSettings mSettings;
    irr::scene::ISceneNode*  mEditNodeCenter;

protected:
	void Clear();
    void SetCollisionWalls(const TrackMarker & nextMarker, irr::scene::ISceneNode* parentNode);
    void RemoveCollisionWalls();
    void CalcMarkerBorders(irr::core::vector3df & leftTop_, irr::core::vector3df & rightTop_, irr::core::vector3df & leftBottom_, irr::core::vector3df & rightBottom_, bool relative_=true) const;
    void SetTrackMarkerSettings(const TrackMarkerSettings &settings_, irr::scene::ISceneNode* editNodeParent, irr::scene::ISceneNode* trackNodeParent);

private:

    irr::scene::ISceneNode*  mEditNodeLeftTop;
    irr::scene::ISceneNode*  mEditNodeRightBottom;
    irr::scene::ISceneNode*  mNodeCollision;
    irr::scene::ISceneNode*  mNodeWallLeft;
    irr::scene::ISceneNode*  mNodeWallRight;
    irr::scene::ISceneNode*  mNodeWallTop;
    irr::scene::ISceneNode*  mNodeWallBottom;
};

#endif // TRACK_MARKER_H
