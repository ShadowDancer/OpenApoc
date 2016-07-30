#pragma once
#include "game/state/equipment/equipmentstore.h"
#include <library/sp.h>

namespace OpenApoc
{
class Equipment;

// store for items dropped by agents on mission
class GroundEquipmentStore : public EquipmentStore
{
  public:
	// add/remove equipment from given tile or nearby tiles
	// list of equipment is constructed on click
	// addEqipment always returns true (you can drop any item)
	~GroundEquipmentStore() override;
	bool addEqipment(sp<Equipment> item) override;
	void removeEqiupment(sp<Equipment> item) override;
};

} // namespace OpenApoc
