#pragma once
#include "game/state/equipment/equipmentstore.h"
#include <game/state/base/base.h>
#include <library/sp.h>

namespace OpenApoc
{
class Equipment;

class BaseEquipmentStore : public EquipmentStore
{
  private:
	sp<Base> base;
	sp<GameState> state;

  public:
	~BaseEquipmentStore() override;
	bool addEqipment(sp<Equipment> item) override;

	//	base->inventory[pair.second->type.id]++;

	void removeEqiupment(sp<Equipment> item) override;
	//	base->inventory[pair.second->type.id];

	BaseEquipmentStore(sp<Base> base, sp<GameState> state) : base(base), state(state) {}

	bool canManipulateEquipment() override;
	int getEquipmentCount() override;
	std::pair<sp<Equipment>, unsigned> getEquipmentAt(int index) override;
	// onAddEquipment checks if you have space in stores, if so you may deequip item
};

} // namespace OpenApoc
