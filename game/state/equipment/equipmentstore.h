#pragma once
#include <library/sp.h>
#include <vector>

namespace OpenApoc
{
class Equipment;
class EquipmentType;
// abstracts all places where player can manage equipment
// for instance base, cityscape or battescape
class EquipmentStore
{
  private:
	// list of available equipment (i.e. on the ground or in store)
	std::vector<sp<Equipment>> equipment;

  public:
	virtual ~EquipmentStore() {}

	// vehicles outside of base cannot be rearmed
	bool virtual canManipulateEquipment() = 0;

	int virtual getEquipmentCount() = 0;

	std::pair<sp<Equipment>, unsigned int> virtual getEquipmentAt(int index) = 0;

	// adds equipment, returns false if action cannot be completed
	// due to lack of space, being outside of base etc
	bool virtual addEqipment(sp<Equipment> item) = 0;

	void virtual removeEqiupment(sp<Equipment> item) = 0;
};
} // namespace OpenApoc
