#include <library/vec.h>
#include <library/sp.h>
#include <framework/image.h>

namespace OpenApoc
{

// general description of what slots entity type have
class EquipmentDefinition
{
	UString name;
	Vec2<int> size;
	sp<Image> sprite;
};

// equipment on particular entity
class Equipment
{
private:
	sp<EquipmentDefinition> definition;
};

}// namespace OpenApoc