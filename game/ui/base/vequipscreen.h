#pragma once

#include "framework/stage.h"
#include "game/state/rules/vequipment_type.h"
#include "library/colour.h"
#include "library/rect.h"
#include "library/sp.h"
#include <map>

namespace OpenApoc
{

class Vehicle;
class Form;
class Palette;
class Equipment;
class BitmapFont;
class GameState;
class Control;
class EquipmentOwner;

class VEquipScreen : public Stage
{
  private:
	StageCmd stageCmd;
	sp<Form> form;
	sp<EquipmentOwner> selected;
	EquipmentClass selectionType;
	sp<Palette> pal;
	sp<BitmapFont> labelFont;

	sp<EquipmentOwner> highlightedVehicle;
	sp<Equipment> highlightedEquipment;

	bool drawHighlightBox;
	Colour highlightBoxColour;
	Rect<int> highlightBox;

	Vec2<int> draggedEquipmentOffset;
	sp<Equipment> draggedEquipment;

	static const Vec2<int> EQUIP_GRID_SLOT_SIZE;
	static const Vec2<int> EQUIP_GRID_SLOTS;

	// List of screen-space rects for all equipped items
	std::list<std::pair<Rect<int>, sp<Equipment>>> equippedItems;
	// List of screen-space rects for all inventory items
	std::list<std::pair<Rect<int>, sp<Equipment>>> inventoryItems;

	std::map<sp<Control>, sp<Vehicle>> vehicleSelectionControls;

	sp<GameState> state;

	float glowCounter;

  public:
	VEquipScreen(sp<GameState> state);
	~VEquipScreen() override;

	void begin() override;
	void pause() override;
	void resume() override;
	void finish() override;
	void eventOccurred(Event *e) override;
	void update(StageCmd *const cmd) override;
	void render() override;
	bool isTransition() override;

	void setSelectedVehicle(sp<EquipmentOwner> vehicle);
};

} // namespace OpenApoc
