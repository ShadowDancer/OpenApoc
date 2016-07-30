#include "game/state/equipment/inventory.h"
#include "framework/logger.h"
#include "game/state/equipment/equipmentowner.h"
#include "game/state/equipment/equipmenttype.h"

namespace OpenApoc
{
bool Inventory::canAddEquipment(sp<Equipment> equipment, Vec2<int> position) const
{
	Vec2<int> slotOrigin;
	bool slotFound = false;

	auto slots = getOwner()->getEquipmentSlots();

	// Check the slot this occupies hasn't already got something there
	for (auto &slot : slots)
	{
		if (slot.bounds.within(position))
		{
			slotOrigin = slot.bounds.p0;
			slotFound = true;
			break;
		}
	}
	// If this was not within a slot fail
	if (!slotFound)
	{
		return false;
	}
	// Check that the equipment doesn't overlap with any other and doesn't
	// go outside a slot of the correct type

	auto type = equipment->getType();
	Rect<int> bounds{position, position + type->getInventorySize()};

	for (auto it = cbegin(); it != cend(); ++it)
	{
		auto &otherEquipment = *it;

		// Something is already in that slot, fail
		if (otherEquipment->getInventoryPosition() == slotOrigin)
		{
			return false;
		}
		Rect<int> otherBounds{otherEquipment->getInventoryPosition(),
		                      otherEquipment->getInventoryPosition() +
		                          otherEquipment->getType()->getInventorySize()};
		if (otherBounds.intersects(bounds))
		{
			LogInfo("Equipping \"%s\" on \"%s\" at {%d,%d} failed: Intersects with other equipment",
			        equipment->getType()->getName().cStr(), getOwner()->getName().cStr(),
			        position.x, position.y);
			return false;
		}
	}

	// Check that this doesn't go outside a slot of the correct type
	for (int y = 0; y < type->getInventorySize().y; y++)
	{
		for (int x = 0; x < type->getInventorySize().x; x++)
		{
			Vec2<int> slotPos = {x, y};
			slotPos += position;
			bool validSlot = false;
			for (auto &slot : slots)
			{
				if (slot.bounds.within(slotPos) && slot.type == type->getClass())
				{
					validSlot = true;
					break;
				}
			}
			if (!validSlot)
			{
				LogInfo("Equipping \"%s\" on \"%s\" at {%d,%d} failed: No valid slot",
				        type->getName().cStr(), getOwner()->getName().cStr(), position.x,
				        position.y);
				return false;
			}
		}
	}
	return true;
}

void Inventory::addEquipment(sp<Equipment> equipmentToAdd)
{
	bool slotFound = false;
	// Check the slot this occupies hasn't already got something there
	auto slots = this->getOwner()->getEquipmentSlots();
	for (auto &slot : slots)
	{
		if (slot.bounds.within(equipmentToAdd->getInventoryPosition()))
		{
			Vec2<int> slotOrigin = slot.bounds.p0;
			equipmentToAdd->setInventoryPosition(slotOrigin);
			slotFound = true;
			break;
		}
	}
	// If this was not within a slow fail
	if (!slotFound)
	{
		auto pos = equipmentToAdd->getInventoryPosition();
		LogError("Equipping \"%s\" on \"%s\" at {%d,%d} failed: No valid slot",
		         getOwner()->getTName().cStr(), getOwner()->getName().cStr(), pos.x, pos.y);
		return;
	}

	this->onAddEquipment(equipmentToAdd);
}
}
