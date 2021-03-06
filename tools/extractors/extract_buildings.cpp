#include "framework/data.h"
#include "framework/framework.h"
#include "game/state/city/building.h"
#include "tools/extractors/common/ufo2p.h"
#include "tools/extractors/extractors.h"

namespace OpenApoc
{

void InitialGameStateExtractor::extractBuildings(GameState &state, UString bldFileName,
                                                 sp<City> city, bool alienBuilding)
{
	auto &data = this->ufo2p;

	auto fileName = "xcom3/ufodata/" + bldFileName + ".bld";

	auto inFile = fw().data->fs.open(fileName);
	if (!inFile)
	{
		LogError("Failed to open \"%s\"", fileName.cStr());
	}
	auto fileSize = inFile.size();
	auto bldCount = fileSize / sizeof(struct BldFileEntry);

	LogInfo("Loading %lu buildings from %s", (unsigned long)bldCount, fileName.cStr());

	for (unsigned i = 0; i < bldCount; i++)
	{
		struct BldFileEntry entry;
		inFile.read((char *)&entry, sizeof(entry));

		auto b = mksp<Building>();
		if (alienBuilding)
		{
			LogInfo("Alien bld %d func %d", entry.name_idx, entry.function_idx);
			// FIXME: albld.bld seems to have unexpected name_idx and function_idx?
			b->name = data.alien_building_names->get(i);
			b->function = b->name;
		}
		else
		{
			b->name = data.building_names->get(entry.name_idx);
			b->function = data.building_functions->get(entry.function_idx);
		}
		b->owner = {&state, data.getOrgId(entry.owner_idx)};
		// Our rects are exclusive of p2
		// Shift position by 20 tiles
		b->bounds = {entry.x0 + 20, entry.y0 + 20, entry.x1 + 21, entry.y1 + 21};
		auto id = UString::format("%s%s", Building::getPrefix(), canon_string(b->name));

		city->buildings[id] = b;
	}
}

} // namespace OpenApoc
