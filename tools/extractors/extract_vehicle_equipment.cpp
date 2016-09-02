#include "framework/framework.h"
#include "game/state/equipment/equipmentusertype.h"
#include "game/state/rules/vequipment_type.h"
#include "tools/extractors/common/ufo2p.h"
#include "tools/extractors/extractors.h"

namespace OpenApoc
{

void InitialGameStateExtractor::extractVehicleEquipment(GameState &state, Difficulty)
{
	auto &data = this->ufo2p;

	// FIXME: Track these as some things (the weapon icon?) seem to be ordered by when they're
	// defined
	int weapon_count = 0;
	int engine_count = 0;
	int general_count = 0;

	for (unsigned i = 0; i < data.vehicle_equipment->count(); i++)
	{
		auto e = mksp<VEquipmentType>();
		auto edata = data.vehicle_equipment->get(i);

		e->name = data.vehicle_equipment_names->get(i);
		UString id = UString::format("%s%s", VEquipmentType::getPrefix(), canon_string(e->name));

		e->id = id;

		// Some data fields are common for all equipment types
		switch (edata.usable_by)
		{
			case VEHICLE_EQUIPMENT_USABLE_GROUND:
				e->users.insert(EquipmentUserType::Ground);
				break;
			case VEHICLE_EQUIPMENT_USABLE_AIR:
				e->users.insert(EquipmentUserType::Air);
				break;
			case VEHICLE_EQUIPMENT_USABLE_GROUND_AIR:
				e->users.insert(EquipmentUserType::Ground);
				e->users.insert(EquipmentUserType::Air);
				break;
			case VEHICLE_EQUIPMENT_USABLE_AMMO:
				// FIXME: Not sure what 'AMMO' usable is used for?
				e->users.insert(EquipmentUserType::Ammo);
				break;
			default:
				LogWarning("Unexpected 'usable_by' %d for ID %s", static_cast<int>(edata.usable_by),
				           id.cStr());
				continue;
		}
		e->weight = edata.weight;
		// FIXME: max_ammo 0xffff is used for 'no ammo' (IE automatically-recharging stuff)

		e->max_ammo = edata.max_ammo;
		e->ammo_type = UString::format("%d", (int)edata.ammo_type);
		// Force all sprites into the correct palette by using A_RANDOM_VEHICLES_BACKGROUND pcx
		//(I assume the parts of the palette used for this are the same on all?)
		e->equipscreen_sprite = fw().data->loadImage(UString::format(
		    "PCK:xcom3/ufodata/vehequip.pck:xcom3/ufodata/vehequip.tab:%d:xcom3/ufodata/vhawk.pcx",
		    (int)edata.sprite_idx));
		e->equipscreen_size = {edata.size_x, edata.size_y};
		e->manufacturer = {&state, data.getOrgId(edata.manufacturer)};
		e->store_space = edata.store_space;

		switch (edata.type)
		{
			case VEHICLE_EQUIPMENT_TYPE_ENGINE:
			{
				auto engData = data.vehicle_engines->get(edata.data_idx);
				e->type = VEquipmentType::VEquipmentClass::Engine;
				e->power = engData.power;
				e->top_speed = engData.top_speed;
				engine_count++;
				break;
			}
			case VEHICLE_EQUIPMENT_TYPE_WEAPON:
			{
				auto wData = data.vehicle_weapons->get(edata.data_idx);
				e->type = VEquipmentType::VEquipmentClass::Weapon;
				e->speed = wData.speed;
				e->damage = wData.damage;
				e->accuracy = wData.accuracy;
				e->fire_delay = wData.fire_delay;
				e->tail_size = wData.tail_size;
				e->guided = wData.guided != 0 ? true : false;
				e->turn_rate = wData.turn_rate;
				e->range = wData.range;
				e->firing_arc_1 = wData.firing_arc_1;
				e->firing_arc_2 = wData.firing_arc_2;
				e->point_defence = wData.point_defence != 0 ? true : false;
				e->fire_sfx =
				    fw().data->loadSample(UString::format("RAWSOUND:%d", (int)wData.fire_sfx));
				e->explosion_graphic = wData.explosion_graphic;
				e->icon = fw().data->loadImage(UString::format(
				    "PCK:xcom3/ufodata/vs_obs.pck:xcom3/ufodata/vs_obs.tab:%d", weapon_count));

				auto projectile_sprites = data.projectile_sprites->get(wData.projectile_image);
				for (int i = 0; i < e->tail_size; i++)
				{
					UString sprite_path = "";
					if (projectile_sprites.sprites[i] != 255)
					{
						sprite_path = UString::format("bulletsprites/city/%02u.png",
						                              (unsigned)projectile_sprites.sprites[i]);
					}
					else
					{
						sprite_path = ""; // a 'gap' in the projectile trail
					}
					e->projectile_sprites.push_back(fw().data->loadImage(sprite_path));
				}
				weapon_count++;
				break;
			}
			case VEHICLE_EQUIPMENT_TYPE_GENERAL:
			{
				auto gData = data.vehicle_general_equipment->get(edata.data_idx);
				e->type = VEquipmentType::VEquipmentClass::General;
				e->accuracy_modifier = gData.accuracy_modifier;
				e->cargo_space = gData.cargo_space;
				e->passengers = gData.passengers;
				e->alien_space = gData.alien_space;
				e->missile_jamming = gData.missile_jamming;
				e->shielding = gData.shielding;
				e->cloaking = gData.cloaking != 0 ? true : false;
				e->teleporting = gData.teleporting != 0 ? true : false;
				general_count++;
				break;
			}
			default:
				LogError("Unexpected vequipment type %d for ID %s", (int)e->type, id.cStr());
		}

		state.vehicle_equipment[id] = e;
	}
}

} // namespace OpenApoc
