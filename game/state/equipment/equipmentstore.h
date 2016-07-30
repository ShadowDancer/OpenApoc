#include <library/sp.h>

#include <game/state/equipment/equipment.h>
#include <vector>

namespace OpenApoc
{
	class Equipment;

	// abstracts all places where player can manage equipment
	// for instance base, cityscape or battescape
	class EquipmentStore
	{
	private:

		// list of available equipment (i.e. on the ground or in store)
		std::vector<sp<Equipment>> equipment;
		
	public:
		// adds equipment, returns false if action cannot be completed
		// for instance no space in store, or cannot drop equipment from vehicile
		bool virtual addEqipment(sp<Equipment> item) = 0;

		void virtual removeEqiupment(sp<Equipment> item) = 0;
	};

	class GroundEquipmentStore
	{
		// add/remove equipment from given tile or nearby tiles
		// list of equipment is constructed on click
		// addEqipment always returns true (you can drop any item)
	};

	class CityEquipmentStore
	{
		// addEqipment always returns false (cannot deequip vehicile out of base)
	};

	class BaseEquipmentStore
	{
		// addEquipment checks if you have space in stores, if so you may deequip item
	};

}// namespace OpenApoc
