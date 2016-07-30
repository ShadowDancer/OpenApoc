#include "game/state/equipment/store/lockedequipmentstore.h"

namespace OpenApoc
{
LockedEquipmentStore::~LockedEquipmentStore() {}

bool LockedEquipmentStore::addEqipment(sp<Equipment> item) { return false; }

void LockedEquipmentStore::removeEqiupment(sp<Equipment> item) { return; }

bool LockedEquipmentStore::canManipulateEquipment() { return false; }

int LockedEquipmentStore::getEquipmentCount() { return 0; }

std::pair<sp<Equipment>, unsigned> LockedEquipmentStore::getEquipmentAt(int index)
{
	return {nullptr, 0};
}
}
