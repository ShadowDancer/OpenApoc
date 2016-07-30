#pragma once

namespace OpenApoc
{
// holds all types of equipment, so equip screen
// may filter items based on category
enum class EquipmentClass : int
{
	VehicleWeapon = 100,
	VehicleEngine = 200,
	VehicleGeneral = 300,
	AgentArmor = 1100,
	AgentGeneral = 1200
};
} // namespace OpenApoc
