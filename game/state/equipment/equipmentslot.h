#pragma once
#include "game/state/equipment/equipmentclass.h"
#include <library/rect.h>
#include <library/strings.h>
#include <map>

namespace OpenApoc
{

class EquipmentSlot
{
  public:
	enum class AlignmentX
	{
		Left,
		Right,
		Centre,
	};
	static const std::map<AlignmentX, UString> AlignmentXMap;
	enum class AlignmentY
	{
		Top,
		Bottom,
		Centre,
	};
	static const std::map<AlignmentY, UString> AlignmentYMap;
	EquipmentClass type;
	AlignmentX align_x;
	AlignmentY align_y;
	Rect<int> bounds;
	EquipmentSlot() = default;
	EquipmentSlot(EquipmentClass type, AlignmentX align_x, AlignmentY align_y, Rect<int> bounds)
	    : type(type), align_x(align_x), align_y(align_y), bounds(bounds)
	{
	}
};
}
