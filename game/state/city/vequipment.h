#pragma once
#include "game/state/equipment/equipment.h"
#include "game/state/rules/vequipment_type.h"
#include "library/sp.h"
#include "library/vec.h"

namespace OpenApoc
{

class VEquipmentType;
class VGeneralEquipmentType;
class VWeaponType;
class VEngineType;
class Vehicle;
class Projectile;

class VEquipment : public Equipment
{
  public:
	VEquipment();
	~VEquipment() = default;

	StateRef<VEquipmentType> type;

	// All equipment state
	Vec2<int> equippedPosition;
	// General equipment state
	// Engine equipemnt state
	// Weapon equipment state
	enum class WeaponState
	{
		Ready,
		Disabled,
		Reloading,
		OutOfAmmo,
	};
	static const std::map<WeaponState, UString> WeaponStateMap;
	WeaponState weaponState;
	StateRef<Vehicle> owner;
	int ammo;
	int reloadTime;

	// All equipment methods
	// General equipment methods
	// Engine equipment methods
	// Weapon equipment methods
	float getRange() const;
	bool canFire() const;
	void update(int ticks);
	void setReloadTime(int ticks);
	// Reload uses up to 'ammoAvailable' to reload the weapon. It returns the amount
	// actually used.
	int reload(int ammoAvailable);
	sp<Projectile> fire(Vec3<float> target);

	void setInventoryPosition(Vec2<int> position) override;
	Vec2<int> getInventoryPosition() const override;
	sp<EquipmentType> getType() const override;
	EquipmentClass getClass() const override;
};
} // namespace OpenApoc
