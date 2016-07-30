#include "game/ui/base/vequipscreen.h"
#include "forms/forms.h"
#include "forms/ui.h"
#include "framework/event.h"
#include "framework/framework.h"
#include "game/state/city/vehicle.h"
#include "game/state/city/vequipment.h"
#include "game/state/equipment/equipmentstore.h"
#include "game/state/gamestate.h"
#include <cmath>

namespace OpenApoc
{
const Vec2<int> VEquipScreen::EQUIP_GRID_SLOT_SIZE{16, 16};
const Vec2<int> VEquipScreen::EQUIP_GRID_SLOTS{16, 16};

static const Colour EQUIP_GRID_COLOUR{40, 40, 40, 255};
static const Colour EQUIP_GRID_COLOUR_ENGINE{255, 255, 40, 255};
static const Colour EQUIP_GRID_COLOUR_WEAPON{255, 40, 40, 255};
static const Colour EQUIP_GRID_COLOUR_GENERAL{255, 40, 255, 255};
static const float GLOW_COUNTER_INCREMENT = static_cast<float>(M_PI) / 15.0f;

VEquipScreen::VEquipScreen(sp<GameState> state)
    : Stage(), form(ui().getForm("FORM_VEQUIPSCREEN")),
      selectionType(EquipmentClass::VehicleWeapon),
      pal(fw().data->loadPalette("xcom3/UFODATA/VROADWAR.PCX")),
      labelFont(ui().getFont("SMALFONT")), drawHighlightBox(false), state(state), glowCounter(0)

{
	form->findControlTyped<RadioButton>("BUTTON_SHOW_WEAPONS")->setChecked(true);

	for (auto &v : state->vehicles)
	{
		auto vehicle = v.second;
		if (vehicle->owner != state->getPlayer())
			continue;
		this->setSelectedVehicle(std::static_pointer_cast<EquipmentOwner>(vehicle));
		break;
	}
	if (!this->selected)
	{
		LogError("No vehicles found - the original apoc didn't open the equip screen in this case");
	}
}

VEquipScreen::~VEquipScreen() = default;

void VEquipScreen::begin()
{
	form->findControlTyped<Label>("TEXT_FUNDS")->setText(state->getPlayerBalance());

	auto list = form->findControlTyped<ListBox>("VEHICLE_SELECT_BOX");
	for (auto &v : state->vehicles)
	{
		auto vehicle = v.second;
		if (vehicle->owner != state->getPlayer())
			continue;
		auto graphic = mksp<Graphic>(vehicle->type->equip_icon_big);
		graphic->AutoSize = true;
		list->addItem(graphic);
		this->vehicleSelectionControls[graphic] = vehicle;

		if (vehicle == this->selected)
		{
			list->setSelected(graphic);
		}
	}
}

void VEquipScreen::pause() {}

void VEquipScreen::resume() {}

void VEquipScreen::finish() {}

void VEquipScreen::eventOccurred(Event *e)
{
	form->eventOccured(e);

	if (e->type() == EVENT_KEY_DOWN)
	{
		if (e->keyboard().KeyCode == SDLK_ESCAPE)
		{
			stageCmd.cmd = StageCmd::Command::POP;
			return;
		}
	}
	if (e->type() == EVENT_FORM_INTERACTION && e->forms().EventFlag == FormEventType::MouseDown)
	{
		auto it = this->vehicleSelectionControls.find(e->forms().RaisedBy);
		if (it != this->vehicleSelectionControls.end())
		{
			this->setSelectedVehicle(std::static_pointer_cast<EquipmentOwner>(it->second));
			return;
		}
	}
	else if (e->type() == EVENT_FORM_INTERACTION &&
	         e->forms().EventFlag == FormEventType::ButtonClick)
	{

		if (e->forms().RaisedBy->Name == "BUTTON_OK")
		{
			stageCmd.cmd = StageCmd::Command::POP;
			return;
		}
	}
	else if (e->type() == EVENT_FORM_INTERACTION &&
	         e->forms().EventFlag == FormEventType::CheckBoxChange)
	{
		if (form->findControlTyped<RadioButton>("BUTTON_SHOW_WEAPONS")->isChecked())
		{
			this->selectionType = EquipmentClass::VehicleWeapon;
			return;
		}
		else if (form->findControlTyped<RadioButton>("BUTTON_SHOW_ENGINES")->isChecked())
		{
			this->selectionType = EquipmentClass::VehicleEngine;
			return;
		}
		else if (form->findControlTyped<RadioButton>("BUTTON_SHOW_GENERAL")->isChecked())
		{
			this->selectionType = EquipmentClass::VehicleGeneral;
			return;
		}
	}

	// Reset the highlight box even if we're dragging
	if (e->type() == EVENT_MOUSE_MOVE)
	{
		this->drawHighlightBox = false;
	}

	// Check if we've moused over equipment/vehicle so we can show the stats.
	if (e->type() == EVENT_MOUSE_MOVE && !this->draggedEquipment)
	{
		// Wipe any previously-highlighted stuff
		this->highlightedVehicle = nullptr;
		this->highlightedEquipment = nullptr;

		Vec2<int> mousePos{e->mouse().X, e->mouse().Y};

		// Check if we're over any equipment in the paper doll
		for (auto &pair : this->equippedItems)
		{
			if (pair.first.within(mousePos))
			{
				this->highlightedEquipment = pair.second;
				return;
			}
		}

		// Check if we're over any equipment in the list at the bottom
		for (auto &pair : this->inventoryItems)
		{
			if (pair.first.within(mousePos))
			{
				this->highlightedEquipment = pair.second;
				this->drawHighlightBox = true;
				this->highlightBoxColour = {255, 255, 255, 255};
				this->highlightBox = pair.first;
				return;
			}
		}

		// Check if we're over any vehicles in the side bar
	}

	sp<EquipmentStore> store = this->selected->getStore(state);
	if (e->type() == EVENT_MOUSE_DOWN && store->canManipulateEquipment())
	{
		Vec2<int> mousePos{e->mouse().X, e->mouse().Y};

		// Check if we're over any equipment in the paper doll
		for (auto &pair : this->equippedItems)
		{
			if (pair.first.within(mousePos))
			{
				//// FIXME: base->addBackToInventory(item); vehicle->unequip(item);
				this->draggedEquipment = pair.second;
				this->draggedEquipmentOffset = pair.first.p0 - mousePos;

				//// Return the equipment to the inventory
				this->selected->getInventory().removeEquipment(pair.second);
				store->addEqipment(pair.second);
				//// FIXME: Return ammo to inventory
				//// FIXME: what happens if we don't have the stores to return?
				return;
			}
		}

		// Check if we're over any equipment in the list at the bottom
		for (auto &pair : this->inventoryItems)
		{
			if (pair.first.within(mousePos))
			{
				// Dragging an object doesn't (Yet) remove it from the inventory
				this->draggedEquipment = pair.second;
				this->draggedEquipmentOffset = pair.first.p0 - mousePos;
				return;
			}
		}
	}
	if (e->type() == EVENT_MOUSE_UP)
	{
		if (this->draggedEquipment)
		{
			// Are we over the grid? If so try to place it on the vehicle.
			auto paperDollControl = form->findControlTyped<Graphic>("PAPER_DOLL");
			Vec2<int> equipOffset = paperDollControl->Location + form->Location;

			Vec2<int> equipmentPos = fw().getCursorPosition() + this->draggedEquipmentOffset;
			// If this is within the grid try to snap it
			Vec2<int> equipmentGridPos = equipmentPos - equipOffset;
			equipmentGridPos /= EQUIP_GRID_SLOT_SIZE;
			if (this->selected->getInventory().canAddEquipment(this->draggedEquipment,
			                                                   equipmentGridPos))
			{
				store->removeEqiupment(draggedEquipment);

				this->draggedEquipment->setInventoryPosition(equipmentGridPos);
				this->selected->getInventory().addEquipment(this->draggedEquipment);
				// FIXME: Add ammo to equipment
			}
			this->draggedEquipment = nullptr;
		}
	}
}

void VEquipScreen::update(StageCmd *const cmd)
{
	this->glowCounter += GLOW_COUNTER_INCREMENT;
	// Loop the increment over the period, otherwise we could start getting lower precision etc. if
	// people leave the screen going for a few centuries
	while (this->glowCounter > 2.0f * static_cast<float>(M_PI))
	{
		this->glowCounter -= 2.0f * static_cast<float>(M_PI);
	}
	form->update();
	*cmd = stageCmd;
	stageCmd = StageCmd();
}

void VEquipScreen::render()
{
	this->equippedItems.clear();
	this->inventoryItems.clear();

	fw().stageGetPrevious(this->shared_from_this())->render();

	fw().renderer->setPalette(this->pal);

	fw().renderer->drawFilledRect({0, 0}, fw().displayGetSize(), Colour{0, 0, 0, 128});

	// The labels/values in the stats column are used for lots of different things, so keep them
	// around clear them and keep them around in a vector so we don't have 5 copies of the same
	// "reset unused entries" code around
	std::vector<sp<Label>> statsLabels;
	std::vector<sp<Label>> statsValues;
	for (int i = 0; i < 9; i++)
	{
		auto labelName = UString::format("LABEL_%d", i + 1);
		auto label = form->findControlTyped<Label>(labelName);
		if (!label)
		{
			LogError("Failed to find UI control matching \"%s\"", labelName.cStr());
		}
		label->setText("");
		statsLabels.push_back(label);

		auto valueName = UString::format("VALUE_%d", i + 1);
		auto value = form->findControlTyped<Label>(valueName);
		if (!value)
		{
			LogError("Failed to find UI control matching \"%s\"", valueName.cStr());
		}
		value->setText("");
		statsValues.push_back(value);
	}
	auto nameLabel = form->findControlTyped<Label>("NAME");
	auto iconGraphic = form->findControlTyped<Graphic>("SELECTED_ICON");
	// If no vehicle/equipment is highlighted (mouse-over), or if we're dragging equipment around
	// show the currently selected vehicle stats.
	//
	// Otherwise we show the stats of the vehicle/equipment highlighted.

	if (highlightedEquipment)
	{
		iconGraphic->setImage(highlightedEquipment->getType()->getImage());
		nameLabel->setText(tr(highlightedEquipment->getType()->getName()));

		int statCounter = 0;
		for (auto &stat : highlightedEquipment->getType()->getStats())
		{
			statsLabels[statCounter]->setText(stat.first);
			statsValues[statCounter]->setText(stat.second);
			statCounter++;
		}
	}
	else
	{
		auto vehicle = this->highlightedVehicle;
		if (!vehicle)
			vehicle = this->selected;

		iconGraphic->setImage(vehicle->getImageSmall());
		nameLabel->setText(vehicle->getName());

		int statCounter = 0;
		for (auto &item : vehicle->getStats())
		{
			statsLabels[statCounter]->setText(item.first);
			statsValues[statCounter]->setText(item.second);
			statCounter++;
		}
		iconGraphic->setImage(vehicle->getImageSmall());
	}

	// Now draw the form, the actual equipment is then drawn on top
	form->render();

	auto paperDollControl = form->findControlTyped<Graphic>("PAPER_DOLL");
	Vec2<int> equipOffset = paperDollControl->Location + form->Location;
	// Draw the equipment grid
	{
		for (auto &slot : selected->getEquipmentSlots())
		{
			Vec2<int> p00 = (slot.bounds.p0 * EQUIP_GRID_SLOT_SIZE) + equipOffset;
			Vec2<int> p11 = (slot.bounds.p1 * EQUIP_GRID_SLOT_SIZE) + equipOffset;
			Vec2<int> p01 = {p00.x, p11.y};
			Vec2<int> p10 = {p11.x, p00.y};
			if (slot.type == selectionType)
			{
				// Scale the sin curve from (-1, 1) to (0, 1)
				float glowFactor = (sin(this->glowCounter) + 1.0f) / 2.0f;
				Colour equipColour;
				switch (selectionType)
				{
					case EquipmentClass::VehicleEngine:
						equipColour = EQUIP_GRID_COLOUR_ENGINE;
						break;
					case EquipmentClass::VehicleWeapon:
						equipColour = EQUIP_GRID_COLOUR_WEAPON;
						break;
					case EquipmentClass::VehicleGeneral:
						equipColour = EQUIP_GRID_COLOUR_GENERAL;
						break;
				}
				Colour selectedColour;
				selectedColour.r =
				    static_cast<uint8_t>(mix(equipColour.r, EQUIP_GRID_COLOUR.r, glowFactor));
				selectedColour.g =
				    static_cast<uint8_t>(mix(equipColour.g, EQUIP_GRID_COLOUR.g, glowFactor));
				selectedColour.b =
				    static_cast<uint8_t>(mix(equipColour.b, EQUIP_GRID_COLOUR.b, glowFactor));
				selectedColour.a = 255;
				fw().renderer->drawLine(p00, p01, selectedColour, 2);
				fw().renderer->drawLine(p01, p11, selectedColour, 2);
				fw().renderer->drawLine(p11, p10, selectedColour, 2);
				fw().renderer->drawLine(p10, p00, selectedColour, 2);
			}
			else
			{
				fw().renderer->drawLine(p00, p01, EQUIP_GRID_COLOUR, 2);
				fw().renderer->drawLine(p01, p11, EQUIP_GRID_COLOUR, 2);
				fw().renderer->drawLine(p11, p10, EQUIP_GRID_COLOUR, 2);
				fw().renderer->drawLine(p10, p00, EQUIP_GRID_COLOUR, 2);
			}
		}
	}
	// Draw the equipped stuff
	for (auto &e : selected->getInventory())
	{
		auto pos = e->getInventoryPosition();

		EquipmentSlot::AlignmentX alignX = EquipmentSlot::AlignmentX::Left;
		EquipmentSlot::AlignmentY alignY = EquipmentSlot::AlignmentY::Top;
		Rect<int> slotBounds;
		bool slotFound = false;

		for (auto &slot : this->selected->getEquipmentSlots())
		{
			if (slot.bounds.p0 == pos)
			{
				alignX = slot.align_x;
				alignY = slot.align_y;
				slotBounds = slot.bounds;
				slotFound = true;
				break;
			}
		}

		if (!slotFound)
		{
			LogError("No matching slot for equipment at {%d,%d}", pos.x, pos.y);
		}

		if (pos.x >= EQUIP_GRID_SLOTS.x || pos.y >= EQUIP_GRID_SLOTS.y)
		{
			LogError("Equipment at {%d,%d} outside grid", pos.x, pos.y);
		}
		pos *= EQUIP_GRID_SLOT_SIZE;
		pos += equipOffset;

		int diffX = slotBounds.getWidth() - e->getType()->getInventorySize().x;
		int diffY = slotBounds.getHeight() - e->getType()->getInventorySize().y;

		switch (alignX)
		{
			case EquipmentSlot::AlignmentX::Left:
				pos.x += 0;
				break;
			case EquipmentSlot::AlignmentX::Right:
				pos.x += diffX * EQUIP_GRID_SLOT_SIZE.x;
				break;
			case EquipmentSlot::AlignmentX::Centre:
				pos.x += (diffX * EQUIP_GRID_SLOT_SIZE.x) / 2;
				break;
		}

		switch (alignY)
		{
			case EquipmentSlot::AlignmentY::Top:
				pos.y += 0;
				break;
			case EquipmentSlot::AlignmentY::Bottom:
				pos.y += diffY * EQUIP_GRID_SLOT_SIZE.y;

				break;
			case EquipmentSlot::AlignmentY::Centre:
				pos.y += (diffY * EQUIP_GRID_SLOT_SIZE.y) / 2;
				break;
		}

		fw().renderer->draw(e->getType()->getImage(), pos);
		Vec2<int> endPos = pos;
		endPos.x += e->getType()->getImage()->size.x;
		endPos.y += e->getType()->getImage()->size.y;
		this->equippedItems.emplace_back(std::make_pair(Rect<int>{pos, endPos}, e));
	}

	// Only draw inventory that can be used by this type of craft
	EquipmentUserType allowedEquipmentUser = this->selected->getUserType();

	sp<EquipmentStore> store = this->selected->getStore(state);
	if (store->canManipulateEquipment())
	{
		auto inventoryControl = form->findControlTyped<Graphic>("INVENTORY");
		Vec2<int> inventoryPosition = inventoryControl->Location + form->Location;
		int equipmentCount = store->getEquipmentCount();
		for (int equipmentIndex = 0; equipmentIndex < equipmentCount; equipmentIndex++)
		{
			auto invPair = store->getEquipmentAt(equipmentIndex);
			// The gap between the bottom of the inventory image and the count label
			static const int INVENTORY_COUNT_Y_GAP = 4;
			// The gap between the end of one inventory image and the start of the next
			static const int INVENTORY_IMAGE_X_GAP = 4;
			auto equipmentType = invPair.first->getType();
			if (equipmentType->getClass() != this->selectionType)
			{
				// Skip equipment of different types
				continue;
			}
			if (!equipmentType->getUsers().count(allowedEquipmentUser))
			{
				// The selected vehicle is not a valid user of the equipment, don't draw
				continue;
			}
			int count = invPair.second;
			if (count == 0)
			{
				// Not in stock
				continue;
			}
			auto countImage = labelFont->getString(UString::format("%d", count));
			auto equipmentImage = equipmentType->getImage();

			fw().renderer->draw(equipmentImage, inventoryPosition);

			Vec2<int> countLabelPosition = inventoryPosition;
			countLabelPosition.y += INVENTORY_COUNT_Y_GAP + equipmentImage->size.y;
			// FIXME: Center in X?
			fw().renderer->draw(countImage, countLabelPosition);

			Vec2<int> inventoryEndPosition = inventoryPosition;
			inventoryEndPosition.x += equipmentImage->size.x;
			inventoryEndPosition.y += equipmentImage->size.y;

			this->inventoryItems.emplace_back(Rect<int>{inventoryPosition, inventoryEndPosition},
			                                  invPair.first);

			// Progress inventory offset by width of image + gap
			inventoryPosition.x += INVENTORY_IMAGE_X_GAP + equipmentImage->size.x;
		}
	}
	if (this->drawHighlightBox)
	{
		Vec2<int> p00 = highlightBox.p0;
		Vec2<int> p11 = highlightBox.p1;
		Vec2<int> p01 = {p00.x, p11.y};
		Vec2<int> p10 = {p11.x, p00.y};
		fw().renderer->drawLine(p00, p01, highlightBoxColour, 1);
		fw().renderer->drawLine(p01, p11, highlightBoxColour, 1);
		fw().renderer->drawLine(p11, p10, highlightBoxColour, 1);
		fw().renderer->drawLine(p10, p00, highlightBoxColour, 1);
	}
	if (this->draggedEquipment)
	{
		// Draw equipment we're currently dragging (snapping to the grid if possible)
		Vec2<int> equipmentPos = fw().getCursorPosition() + this->draggedEquipmentOffset;
		// If this is within the grid try to snap it
		Vec2<int> equipmentGridPos = equipmentPos - equipOffset;
		equipmentGridPos /= EQUIP_GRID_SLOT_SIZE;
		if (equipmentGridPos.x < 0 || equipmentGridPos.x >= EQUIP_GRID_SLOTS.x ||
		    equipmentGridPos.y < 0 || equipmentGridPos.y >= EQUIP_GRID_SLOTS.y)
		{
			// This is outside thge grid
		}
		else
		{
			// Inside the grid, snap
			equipmentPos = equipmentGridPos * EQUIP_GRID_SLOT_SIZE;
			equipmentPos += equipOffset;
		}
		fw().renderer->draw(this->draggedEquipment->getType()->getImage(), equipmentPos);
	}
}

bool VEquipScreen::isTransition() { return false; }

void VEquipScreen::setSelectedVehicle(sp<EquipmentOwner> vehicle)
{
	if (!vehicle)
	{
		LogError("Trying to set invalid selected vehicle");
		return;
	}
	LogInfo("Selecting vehicle \"%s\"", vehicle->getName().cStr());
	this->selected = vehicle;
	auto backgroundImage = vehicle->getEqScreenBackgound();
	if (!backgroundImage)
	{
		LogError("Trying to view equipment screen of vehicle \"%s\" which has no equipment screen "
		         "background",
		         vehicle->getTName().cStr());
	}

	auto backgroundControl = form->findControlTyped<Graphic>("BACKGROUND");
	backgroundControl->setImage(backgroundImage);
}

} // namespace OpenApoc
