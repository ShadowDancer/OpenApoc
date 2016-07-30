#pragma once
#include <library/sp.h>
#include <library/vec.h>

namespace OpenApoc
{
enum class EquipmentClass;
class EquipmentType;

// instance of given equipment type on entity
class Equipment
{
  public:
	virtual ~Equipment() {}

	virtual Vec2<int> getInventoryPosition() const = 0;
	virtual void setInventoryPosition(Vec2<int> position) = 0;
	virtual sp<EquipmentType> getType() const = 0;
	virtual EquipmentClass getClass() const = 0;
};
} // namespace OpenApoc
