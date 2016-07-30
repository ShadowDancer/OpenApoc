#pragma once
#include "../filteriterator.h"
#include "game/state/city/vequipment.h"
#include "game/state/equipment/inventory.h"
#include <library/sp.h>
#include <list>

namespace OpenApoc
{
typedef std::list<sp<Equipment>> EquipmentList;
typedef TransformIterator<std::list<sp<Equipment>>::iterator, VEquipment> VEquipmentListIterator;
typedef TransformIterator<std::list<sp<Equipment>>::const_iterator, VEquipment>
    VEquipmentListConstIterator;

class Equipment;
class InventoryType;
class EquipmentOwner;

// equipment on particular entity
class VehicleEquipment : public Inventory
{

	// aditional list to hold weapons
	std::list<sp<Equipment>> weaponsList;
	// all equipment including weapons
	std::list<sp<Equipment>> equipment;

	void addEquipmentInternal(sp<VEquipment> equipmentToAdd);
	void removeEquipmentInternal(sp<VEquipment> equipmentToDelete);

	Vehicle *owner;

  public:
	explicit VehicleEquipment(Vehicle *owner);

	EquipmentIteratorFactory<EquipmentList, VEquipment> weapons() const;
	EquipmentIteratorFactory<EquipmentList, VEquipment> engines() const;
	EquipmentIteratorFactory<EquipmentList, VEquipment> general() const;

	VEquipmentListIterator begin();
	VEquipmentListIterator end();
	VEquipmentListConstIterator cbegin() const;
	VEquipmentListConstIterator cend() const;

	std::list<sp<Equipment>>::iterator inventory_begin() override;
	std::list<sp<Equipment>>::iterator inventory_end() override;
	std::list<sp<Equipment>>::const_iterator inventory_cbegin() const override;
	std::list<sp<Equipment>>::const_iterator inventory_cend() const override;

	EquipmentOwner *getOwner() const override;
	void clearEquipment() override;
	void onAddEquipment(sp<Equipment> equipmentToAdd) override;
	void removeEquipment(sp<Equipment> equipmentToDelete) override;
	void addEquipment(sp<VEquipment> equipmentToAdd);
	void removeEquipment(sp<VEquipment> equipmentToDelete);
};
} // namespace OpenApoc
