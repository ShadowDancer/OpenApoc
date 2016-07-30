#include "game/state/equipment/equipmentusertype.h"
#include "game/state/gamestate.h"
#include "game/state/rules/vequipment_type.h"

namespace OpenApoc
{
UString VEquipmentType::getName() const { return name; }

Vec2<int> VEquipmentType::getInventorySize() const { return equipscreen_size; }

sp<Image> VEquipmentType::getImage() const { return this->equipscreen_sprite; }

EquipmentClass VEquipmentType::getClass() const { return static_cast<EquipmentClass>(this->type); }

EquipmentUserType VEquipmentType::getUserType() const
{
	// TODO: return correct type
	return EquipmentUserType::Air;
}

// A bit painful but as not everything is expected to be set we have to zero all the non-constructed
// types
VEquipmentType::VEquipmentType()
    : type(VEquipmentClass::General), weight(0), max_ammo(0), store_space(0), speed(0), damage(0),
      accuracy(0), fire_delay(0), tail_size(0), guided(false), turn_rate(0), range(0),
      firing_arc_1(0), firing_arc_2(0), point_defence(false), explosion_graphic(0), power(0),
      top_speed(0), accuracy_modifier(0), cargo_space(0), passengers(0), alien_space(0),
      missile_jamming(0), shielding(0), cloaking(false), teleporting(false)
{
}

template <> const UString &StateObject<VEquipmentType>::getPrefix()
{
	static UString prefix = "VEQUIPMENTTYPE_";
	return prefix;
}

template <> const UString &StateObject<VEquipmentType>::getTypeName()
{
	static UString name = "VEquipmentType";
	return name;
}

template <>
sp<VEquipmentType> StateObject<VEquipmentType>::get(const GameState &state, const UString &id)
{
	auto it = state.vehicle_equipment.find(id);
	if (it == state.vehicle_equipment.end())
	{
		LogError("No vequipement type matching ID \"%s\"", id.cStr());
		return nullptr;
	}
	return it->second;
}

bool operator!=(const EquipmentClass &lhs, const VEquipmentType::VEquipmentClass &rhs)
{
	return !(lhs == rhs);
}

bool operator==(const EquipmentClass &lhs, const VEquipmentType::VEquipmentClass &rhs)
{
	return lhs == static_cast<EquipmentClass>(rhs);
}

std::map<UString, UString> VEquipmentType::getStats() const
{
	std::map<UString, UString> result = {{"Weight", UString::format("%d", this->weight)}};
	switch (this->getClass())
	{
		case EquipmentClass::VehicleEngine:
		{
			result["Top Speed"] = UString::format("%d", this->top_speed);

			result["Power"] = UString::format("%d", this->power);
			break;
		}
		case EquipmentClass::VehicleWeapon:
		{
			result["Damage"] = UString::format("%d", this->damage);
			result["Range"] = UString::format("%dm", this->range / 2);
			result["Accuracy"] = UString::format("%d%%", 100 - this->accuracy);

			// Only show rounds if non-zero (IE not infinite ammo)
			if (this->max_ammo != 0)
			{
				result["Rounds"] = UString::format("%d", this->max_ammo);
			}
			break;
		}
		case EquipmentClass::VehicleGeneral:
		{
			if (this->accuracy_modifier)
			{
				result["Accuracy"] = UString::format("%d%%", 100 - this->accuracy_modifier);
			}
			if (this->cargo_space)
			{
				result["Cargo"] = UString::format("%d", this->cargo_space);
			}
			if (this->passengers)
			{
				result["Passengers"] = UString::format("%d", this->passengers);
			}
			if (this->alien_space)
			{
				result["Aliens Held"] = UString::format("%d", this->alien_space);
			}
			if (this->missile_jamming)
			{
				result["Jamming"] = UString::format("%d", this->missile_jamming);
			}
			if (this->shielding)
			{
				result["Shielding"] = UString::format("%d", this->shielding);
			}
			if (this->cloaking)
			{
				result["Cloaks Craft"] = "";
			}
			if (this->teleporting)
			{
				result["Teleports"] = "";
			}

			break;
		}
		default:
			break;
	}

	return result;
}

std::set<EquipmentUserType> VEquipmentType::getUsers() const { return users; }
} // namespace OpenApoc
