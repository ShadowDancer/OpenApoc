#pragma once
#include "game/state/equipment/equipmentstore.h"
#include <library/sp.h>

namespace OpenApoc
{
class Equipment;

// items stored in vehicle cargo module
class VehicleEquipmentStore : public EquipmentStore
{
  public:
	~VehicleEquipmentStore() override;

	// allows to drop agent equipment if cargo module present
	// deny changing vehicle equipment outside of base
	bool addEqipment(sp<Equipment> item) override;

	void removeEqiupment(sp<Equipment> item) override;
};
} // namespace OpenApoc
