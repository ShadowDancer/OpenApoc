#include "game/state/equipment/store/baseequipmentstore.h"
#include "game/state/city/vequipment.h"

namespace OpenApoc
{
BaseEquipmentStore::~BaseEquipmentStore() {}

bool BaseEquipmentStore::addEqipment(sp<Equipment> item)
{
	auto id = std::dynamic_pointer_cast<VEquipmentType>(item->getType())->id;
	base->inventoryVehicleEquipment[id]++;
	return true;
}

void BaseEquipmentStore::removeEqiupment(sp<Equipment> item)
{

	auto id = std::dynamic_pointer_cast<VEquipmentType>(item->getType())->id;
	base->inventoryVehicleEquipment[id]--;
}

bool BaseEquipmentStore::canManipulateEquipment() { return true; }

int BaseEquipmentStore::getEquipmentCount() { return base->inventoryVehicleEquipment.size(); }

std::pair<sp<Equipment>, unsigned> BaseEquipmentStore::getEquipmentAt(int index)
{
	auto iterator = base->inventoryVehicleEquipment.begin();
	std::advance(iterator, index);

	auto equipIt = state->vehicle_equipment.find(iterator->first);
	if (equipIt == state->vehicle_equipment.end())
	{
		// It's not vehicle equipment, skip
		return {nullptr, 0};
	}

	auto equipmentType = StateRef<VEquipmentType>{state.get(), equipIt->first};

	auto newEquipment = mksp<VEquipment>();
	newEquipment->type = equipmentType;

	return {newEquipment, iterator->second};
}
}
