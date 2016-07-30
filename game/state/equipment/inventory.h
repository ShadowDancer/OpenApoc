#pragma once
#include "equipmentowner.h"
#include "game/state/equipment/equipment.h"
#include <library/sp.h>

namespace OpenApoc
{
// equipment on particular entity
class Inventory
{

  protected:
	// defined that way so derived classes may expose strongly typed iterators
	virtual std::list<sp<Equipment>>::iterator inventory_begin() = 0;
	virtual std::list<sp<Equipment>>::iterator inventory_end() = 0;
	virtual std::list<sp<Equipment>>::const_iterator inventory_cbegin() const = 0;
	virtual std::list<sp<Equipment>>::const_iterator inventory_cend() const = 0;

	// function executed when item is added to entity
	virtual void onAddEquipment(sp<Equipment> equipmentToAdd) = 0;

  public:
	std::list<sp<Equipment>>::iterator begin() { return inventory_begin(); }
	std::list<sp<Equipment>>::iterator end() { return inventory_end(); }
	std::list<sp<Equipment>>::const_iterator cbegin() const { return inventory_cbegin(); }
	std::list<sp<Equipment>>::const_iterator cend() const { return inventory_cend(); }

	bool canAddEquipment(sp<Equipment> equipment, Vec2<int> position) const;

	virtual EquipmentOwner *getOwner() const = 0;

	void addEquipment(sp<Equipment> equipmentToAdd);

	// function executed when item is removed
	virtual void removeEquipment(sp<Equipment> equipmentToDelete) = 0;

	// remove all equipment
	virtual void clearEquipment() = 0;

	virtual ~Inventory() {}
};

} // namespace OpenApoc
