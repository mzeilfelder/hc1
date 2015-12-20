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

    bool mIsValid;
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

    bool  mIsValid;
    irr::core::vector3df mCenter;
    irr::core::vector3df mUpVector;	// Bad name, it's usually the normal to the track (nearest point on collision triangle to sphere-center used for collision)
    irr::core::vector3df mRotation;
    bool  mRelocate;  // hover can teleport to this track when he drops off
    bool  mUseDefaultSizes;
    float mLeft;
    float mRight;
    float mTop;
    float mBottom;

    bool mHasLeftWall;
    bool mHasRightWall;
    bool mHasBottomWall;
};

struct TrackStart
{
    TrackStart() : mEditNode(0) {}

    bool GetCenter( irr::core::vector3df &pos_) const;

    irr::scene::ISceneNode*  mEditNode;
    TrackStartSettings  mSettings;
};

struct TrackMarker
{
	enum EWall
	{
		WALL_CENTER,	// like start-lines and teleports. Perpendicular to the center node.
		WALL_LEFT,
		WALL_RIGHT,
		WALL_BOTTOM,

		WALL_COUNT
	};

    TrackMarker();

    bool GetCenter( irr::core::vector3df &pos_) const;
    irr::core::vector3df GetCenter() const;
	bool CheckLineWallCollision(const irr::core::line3d<irr::f32> &line_, EWall wall, irr::core::vector3df &outIntersection_) const;

    void SetTrackMarkerSettings(const TrackMarkerSettings &settings_, irr::scene::ISceneNode* editNodeParent, irr::scene::ISceneNode* trackNodeParent);
    void SetSideWalls(const TrackMarker & nextMarker, irr::scene::ISceneNode* parentNode);
    void RemoveSideWalls();
	void Clear();

    TrackMarkerSettings mSettings;
    irr::scene::ISceneNode*  mEditNodeCenter;

protected:
    void SetWall(EWall wall, irr::scene::ISceneNode* parent_, const irr::core::vector3df &leftTop_, const irr::core::vector3df &rightTop_, const irr::core::vector3df &leftBottom_, const irr::core::vector3df &rightBottom_);
    void CalcMarkerBorders(irr::core::vector3df & leftTop_, irr::core::vector3df & rightTop_, irr::core::vector3df & leftBottom_, irr::core::vector3df & rightBottom_) const;

private:

    irr::scene::ISceneNode* mEditNodeLeftTop;
    irr::scene::ISceneNode* mEditNodeRightBottom;
    irr::scene::ISceneNode* mWallNode[WALL_COUNT];
	irr::core::triangle3df mWallTriangles[WALL_COUNT][2];
	irr::core::aabbox3df mWallBoxes[WALL_COUNT];
};

#endif // TRACK_MARKER_H
