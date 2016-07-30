#include <library/sp.h>
#include <vector>

namespace OpenApoc
{
	class EquipmentSlot;
	class EquipmentSlotDefinition;

	// general description of what slots entity type have
	class EntityEquipmentDefinition
	{
		std::vector<sp<EquipmentSlotDefinition>> slots;
	};

	// equipment on particular entity
	class EntityEquipment
	{
	private:
		std::vector<sp<EquipmentSlot>> slots;
		sp<EntityEquipmentDefinition> type;

	public:
	};
}// namespace OpenApoc
