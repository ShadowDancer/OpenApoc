#pragma once
#include "game/state/equipment/equipmentclass.h"
#include "game/state/equipment/equipmenttype.h"
#include "game/state/organisation.h"
#include "game/state/stateobject.h"
#include "library/strings.h"
#include "library/vec.h"
#include <map>
#include <set>

namespace OpenApoc
{

class Rules;
class Image;
class Sample;
class VEquipmentType : public StateObject<VEquipmentType>, public EquipmentType
{
  public:
	// implementation of EquipmentType
	UString getName() const override;
	Vec2<int> getInventorySize() const override;
	sp<Image> getImage() const override;
	EquipmentClass getClass() const override;
	EquipmentUserType getUserType() const override;

	VEquipmentType();
	enum class VEquipmentClass : int
	{
		Engine = static_cast<int>(EquipmentClass::VehicleEngine),
		Weapon = static_cast<int>(EquipmentClass::VehicleWeapon),
		General = static_cast<int>(EquipmentClass::VehicleGeneral),
	};

	~VEquipmentType() override = default;

	// Shared stuff
	VEquipmentClass type;
	UString id;
	UString name;
	int weight;
	int max_ammo;
	UString ammo_type;
	sp<Image> equipscreen_sprite;
	Vec2<int> equipscreen_size;
	StateRef<Organisation> manufacturer;
	int store_space;
	std::set<EquipmentUserType> users;

	// Weapons
	int speed;
	std::list<sp<Image>> projectile_sprites; // A list of sprites forming the projectile
	                                         // 'bullet'/'beam' - 'nullptr' gaps are expected
	int damage;
	int accuracy;
	int fire_delay;
	int tail_size;
	bool guided;
	int turn_rate;
	int range;
	int firing_arc_1;
	int firing_arc_2;
	bool point_defence;
	sp<Sample> fire_sfx;
	int explosion_graphic;
	sp<Image> icon;

	// Engine stuff
	int power;
	int top_speed;

	// Other ('general') equipment stuff
	int accuracy_modifier;
	int cargo_space;
	int passengers;
	int alien_space;
	int missile_jamming;
	int shielding;
	bool cloaking;
	bool teleporting;

	std::map<UString, UString> getStats() const override;
	std::set<EquipmentUserType> getUsers() const override;
};

bool operator!=(const EquipmentClass &lhs, const VEquipmentType::VEquipmentClass &rhs);
bool operator==(const EquipmentClass &lhs, const VEquipmentType::VEquipmentClass &rhs);
} // namespace OpenApoc
