#include "game/state/tileview/tileobject_shadow.h"
#include "framework/renderer.h"
#include "game/state/city/vehicle.h"
#include "game/state/tileview/voxel.h"

namespace OpenApoc
{

void TileObjectShadow::draw(Renderer &r, TileTransform &transform, Vec2<float> screenPosition,
                            TileViewMode mode)
{
	std::ignore = transform;
	auto vehicle = this->owner.lock();
	if (!vehicle)
	{
		LogError("Called with no owning vehicle object");
		return;
	}
	if (this->fellOffTheBottomOfTheMap)
	{
		return;
	}
	switch (mode)
	{
		case TileViewMode::Isometric:
		{
			float closestAngle = FLT_MAX;
			sp<Image> closestImage;
			for (auto &p : vehicle->type->directional_shadow_sprites)
			{
				float angle =
				    glm::angle(glm::normalize(p.first), glm::normalize(vehicle->getDirection()));
				if (angle < closestAngle)
				{
					closestAngle = angle;
					closestImage = p.second;
				}
			}
			if (!closestImage)
			{
				LogError("No image found for vehicle");
				return;
			}
			r.draw(closestImage, screenPosition - vehicle->type->shadow_offset);
			break;
		}
		case TileViewMode::Strategy:
		{
			// No shadows in strategy view
			break;
		}
		default:
			LogError("Unsupported view mode");
	}
}

void TileObjectShadow::setPosition(Vec3<float> newPosition)
{
	// This projects a line downwards and draws places the shadow at the z of the first thing hit

	auto shadowPosition = newPosition;
	auto c = map.findCollision(newPosition, Vec3<float>{newPosition.x, newPosition.y, -1});
	if (c)
	{
		shadowPosition.z = c.position.z;
		this->fellOffTheBottomOfTheMap = false;
	}
	else
	{
		// May be a normal occurance (e.g. landing pads have a 'hole'
		LogInfo("Nothing beneath {%f,%f,%f} to receive shadow", newPosition.x, newPosition.y,
		        newPosition.z);
		shadowPosition.z = 0;
		// Mark it as not to be drawn
		this->fellOffTheBottomOfTheMap = true;
	}

	this->shadowPosition = shadowPosition;

	TileObject::setPosition(shadowPosition);
}

TileObjectShadow::~TileObjectShadow() = default;

TileObjectShadow::TileObjectShadow(TileMap &map, sp<Vehicle> vehicle)
    : TileObject(map, Type::Vehicle, Vec3<float>{0, 0, 0}), owner(vehicle),
      fellOffTheBottomOfTheMap(false)
{
}

Vec3<float> TileObjectShadow::getPosition() const { return this->shadowPosition; }

} // namespace OpenApoc
