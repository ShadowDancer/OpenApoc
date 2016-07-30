#include "game/state/equipment/vehicle/vinventory.h"
#include "game/state/city/vehicle.h"
#include "game/state/equipment/filteriterator.h"

namespace OpenApoc
{
static bool weaponSelector(const sp<Equipment> &input)
{
	return input->getClass() == EquipmentClass::VehicleWeapon;
}

static bool engineSelector(const sp<Equipment> &input)
{
	return input->getClass() == EquipmentClass::VehicleEngine;
}

static bool generalSelector(const sp<Equipment> &input)
{
	return input->getClass() == EquipmentClass::VehicleGeneral;
}

VehicleEquipment::VehicleEquipment(Vehicle *owner) { this->owner = owner; }

EquipmentIteratorFactory<EquipmentList, VEquipment> VehicleEquipment::weapons() const
{
	return EquipmentIteratorFactory<EquipmentList, VEquipment>(
	    const_cast<EquipmentList &>(weaponsList), weaponSelector);
}

EquipmentIteratorFactory<EquipmentList, VEquipment> VehicleEquipment::engines() const
{
	return EquipmentIteratorFactory<EquipmentList, VEquipment>(
	    const_cast<EquipmentList &>(equipment), engineSelector);
}

EquipmentIteratorFactory<EquipmentList, VEquipment> VehicleEquipment::general() const
{
	return EquipmentIteratorFactory<EquipmentList, VEquipment>(
	    const_cast<EquipmentList &>(equipment), generalSelector);
}

VEquipmentListIterator VehicleEquipment::begin()
{
	return VEquipmentListIterator(equipment.begin());
}

VEquipmentListIterator VehicleEquipment::end() { return VEquipmentListIterator(equipment.end()); }

VEquipmentListConstIterator VehicleEquipment::cbegin() const
{
	return VEquipmentListConstIterator(equipment.cbegin());
}

VEquipmentListConstIterator VehicleEquipment::cend() const
{
	return VEquipmentListConstIterator(equipment.cend());
}

std::list<sp<Equipment>>::iterator VehicleEquipment::inventory_begin() { return equipment.begin(); }

std::list<sp<Equipment>>::iterator VehicleEquipment::inventory_end() { return equipment.end(); }

std::list<sp<Equipment>>::const_iterator VehicleEquipment::inventory_cbegin() const
{
	return equipment.cbegin();
}

std::list<sp<Equipment>>::const_iterator VehicleEquipment::inventory_cend() const
{
	return equipment.cend();
}

EquipmentOwner *VehicleEquipment::getOwner() const { return owner; }

void VehicleEquipment::clearEquipment()
{
	this->equipment.clear();
	this->weaponsList.clear();
}

void VehicleEquipment::onAddEquipment(const sp<Equipment> equipmentToAdd)
{
	sp<VEquipment> vehicleEquipment = std::dynamic_pointer_cast<VEquipment>(equipmentToAdd);
	if (vehicleEquipment == nullptr)
	{
		if (equipmentToAdd == nullptr)
		{
			LogError("Attempt to add nullptr to inventory");
		}
		else
		{
			LogError("Attempt to add non vehicle equipment to vehicle inventory");
		}
		return;
	}
	addEquipmentInternal(vehicleEquipment);
}

void VehicleEquipment::removeEquipment(sp<Equipment> equipmentToDelete)
{
	sp<VEquipment> vehicleEquipment = std::dynamic_pointer_cast<VEquipment>(equipmentToDelete);
	if (vehicleEquipment == nullptr)
	{
		if (equipmentToDelete == nullptr)
		{
			LogError("Attempt to remove nullptr to inventory");
		}
		else
		{
			LogError("Attempt to remove non vehicle equipment from vehicle inventory");
		}
		return;
	}
	removeEquipmentInternal(vehicleEquipment);
}

void VehicleEquipment::addEquipment(sp<VEquipment> equipmentToAdd)
{
	if (equipmentToAdd == nullptr)
	{
		LogError("Attempt to add nullptr to inventory");
		return;
	}
	addEquipmentInternal(equipmentToAdd);
}

void VehicleEquipment::removeEquipment(sp<VEquipment> equipmentToDelete)
{
	if (equipmentToDelete == nullptr)
	{
		LogError("Attempt to remove nullptr from inventory");
		return;
	}
	removeEquipmentInternal(equipmentToDelete);
}

void VehicleEquipment::addEquipmentInternal(sp<VEquipment> equipmentToAdd)
{
	equipment.push_back(equipmentToAdd);
	if (equipmentToAdd->type->type == VEquipmentType::VEquipmentClass::Weapon)
	{
		weaponsList.push_back(equipmentToAdd);
	}
}

void VehicleEquipment::removeEquipmentInternal(sp<VEquipment> equipmentToDelete)
{
	equipment.remove(equipmentToDelete);
	if (equipmentToDelete->type->type == VEquipmentType::VEquipmentClass::Weapon)
	{
		weaponsList.remove(equipmentToDelete);
	}
}
} // namespace OpenApoc
