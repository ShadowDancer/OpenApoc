#pragma once
#include "game/state/equipment/equipmentslot.h"
#include <library/sp.h>
#include <map>

namespace OpenApoc
{
class UString;
class Inventory;
class Image;
class EquipmentStore;
class GameState;
enum class EquipmentUserType;
// instance of given equipment type on entity
class EquipmentOwner
{
  public:
	virtual ~EquipmentOwner() {}

	virtual sp<EquipmentStore> getStore(sp<GameState> state) = 0;

	// get list of all stats name: value etc.
	virtual std::map<UString, UString> getStats() = 0;
	virtual std::list<EquipmentSlot> getEquipmentSlots() = 0;
	virtual sp<Image> getEqScreenBackgound() const = 0;
	virtual sp<Image> getImage() const = 0;
	virtual sp<Image> getImageSmall() const = 0;
	virtual const UString &getName() const = 0;
	virtual const UString &getTName() const = 0;
	virtual const EquipmentUserType getUserType() const = 0;
	virtual Inventory &getInventory() = 0;
};
} // namespace OpenApoc
