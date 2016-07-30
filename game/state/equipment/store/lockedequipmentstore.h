#pragma once
#include "game/state/equipment/equipmentstore.h"
#include <game/state/base/base.h>
#include <library/sp.h>

namespace OpenApoc
{
class Equipment;

// Equpment store used when entity cannot manipulate its inventory
// ie. vehicle outside of base
class LockedEquipmentStore : public EquipmentStore
{
  public:
	~LockedEquipmentStore() override;
	bool addEqipment(sp<Equipment> item) override;
	void removeEqiupment(sp<Equipment> item) override;
	bool canManipulateEquipment() override;
	int getEquipmentCount() override;
	std::pair<sp<Equipment>, unsigned> getEquipmentAt(int index) override;
};
} // namespace OpenApoc
