#include <library/vec.h>
#include <library/sp.h>

#include <vector>

namespace OpenApoc
{
	class Equipment;

	enum class SlotType
	{
		VehicleWeapon,
		VehicleEngine,
		VehicleGeneral,
		AgentArmor,
		AgentGeneral
	};


	class EquipmentSlotDefinition
	{
		// position on equipment screen
		Vec2<int> position;
		// limits what kind of equipment may be loaded
		Vec2<int> size;

		// exclusive slots may hold only one item
		bool IsExclusive;

		// restricts kind of equipment that can be placed in this slot
		SlotType SlotType;
	};

	// O
	class EquipmentSlot
	{
	private:
		std::vector<sp<Equipment>> equipment;
		sp<EquipmentSlotDefinition> definition;
	public:

		const Vec2<int> getPosition();

		bool isExclusive();

		bool equipItem(sp<Equipment> item);
		bool unequipItem(sp<Equipment> item);
	};

}// namespace OpenApoc
