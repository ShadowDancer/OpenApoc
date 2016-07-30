#pragma once
#include <library/sp.h>
#include <library/vec.h>
#include <map>
#include <set>

namespace OpenApoc
{
class UString;
class Image;
enum class EquipmentUserType;
enum class EquipmentClass;

// general description of what slots entity type have
class EquipmentType
{
  public:
	virtual ~EquipmentType() {}

	virtual EquipmentClass getClass() const = 0;
	virtual UString getName() const = 0;
	virtual Vec2<int> getInventorySize() const = 0;
	virtual sp<Image> getImage() const = 0;
	virtual EquipmentUserType getUserType() const = 0;
	virtual std::map<UString, UString> getStats() const = 0;

	virtual std::set<EquipmentUserType> getUsers() const = 0;

	static const std::map<EquipmentClass, UString> TypeMap;

	static const std::map<EquipmentUserType, UString> UserTypeMap;
};

} // namespace OpenApoc