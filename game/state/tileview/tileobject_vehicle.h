#pragma once

#include "game/state/city/vehicle.h"
#include "game/state/tileview/tileobject.h"

namespace OpenApoc
{

class TileObjectVehicle : public TileObject
{
  public:
	void draw(Renderer &r, TileTransform &transform, Vec2<float> screenPosition,
	          TileViewMode mode) override;
	~TileObjectVehicle() override;

	sp<Vehicle> getVehicle() const;
	const Vec3<float> &getDirection() { return this->getVehicle()->velocity; }
	void setDirection(const Vec3<float> &dir)
	{
		this->getVehicle()->facing = dir;
		this->getVehicle()->velocity = dir;
	}

	Vec3<float> getCentrePosition();
	sp<VoxelMap> getVoxelMap() override;
	Vec3<float> getPosition() const override;
	void nextFrame(int ticks);

  private:
	friend class TileMap;
	std::weak_ptr<Vehicle> vehicle;
	std::list<sp<Image>>::iterator animationFrame;
	int animationDelay;

	TileObjectVehicle(TileMap &map, sp<Vehicle> vehicle);
};

} // namespace OpenApoc
