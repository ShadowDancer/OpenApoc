#include "game/state/equipment/equipmenttype.h"
#include "game/state/equipment/equipmentclass.h"
#include "game/state/equipment/equipmentusertype.h"
#include "library/strings.h"
#include <map>

namespace OpenApoc
{
const std::map<EquipmentClass, UString> EquipmentType::TypeMap = {
    {EquipmentClass::VehicleEngine, "engine"},
    {EquipmentClass::VehicleWeapon, "weapon"},
    {EquipmentClass::VehicleGeneral, "general"},
    {EquipmentClass::AgentArmor, "agent_armor"},
    {EquipmentClass::AgentGeneral, "agent_general"}};

const std::map<EquipmentUserType, UString> EquipmentType::UserTypeMap = {
    {EquipmentUserType::Ground, "ground"},
    {EquipmentUserType::Air, "air"},
    {EquipmentUserType::Ammo, "ammo"}};
}
