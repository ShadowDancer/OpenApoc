#include "game/ui/base/basescreen.h"
#include "forms/ui.h"
#include "framework/event.h"
#include "framework/framework.h"
#include "framework/image.h"
#include "game/state/base/base.h"
#include "game/state/base/facility.h"
#include "game/ui/base/basegraphics.h"
#include "game/ui/base/researchscreen.h"
#include "game/ui/base/vequipscreen.h"
#include "game/ui/general/messagebox.h"
#include "game/ui/ufopaedia/ufopaediacategoryview.h"

namespace OpenApoc
{

const Vec2<int> BaseScreen::NO_SELECTION = {-1, -1};

BaseScreen::BaseScreen(sp<GameState> state) : BaseStage(state), selection(NO_SELECTION), drag(false)
{
	form = ui().getForm("FORM_BASESCREEN");
	viewHighlight = BaseGraphics::FacilityHighlight::Construction;
}

BaseScreen::~BaseScreen() = default;

void BaseScreen::changeBase(sp<Base> newBase)
{
	BaseStage::changeBase(newBase);
	form->findControlTyped<TextEdit>("TEXT_BASE_NAME")->setText(state->current_base->name);
	form->findControlTyped<Graphic>("GRAPHIC_MINIMAP")
	    ->setImage(BaseGraphics::drawMinimap(state, state->current_base->building));
}

void BaseScreen::begin()
{
	BaseStage::begin();

	baseView = form->findControlTyped<Graphic>("GRAPHIC_BASE_VIEW");
	selText = form->findControlTyped<Label>("TEXT_SELECTED_FACILITY");
	selGraphic = form->findControlTyped<Graphic>("GRAPHIC_SELECTED_FACILITY");
	for (int i = 0; i < 3; i++)
	{
		auto labelName = UString::format("LABEL_%d", i + 1);
		auto label = form->findControlTyped<Label>(labelName);
		if (!label)
		{
			LogError("Failed to find UI control matching \"%s\"", labelName.cStr());
		}
		statsLabels.push_back(label);

		auto valueName = UString::format("VALUE_%d", i + 1);
		auto value = form->findControlTyped<Label>(valueName);
		if (!value)
		{
			LogError("Failed to find UI control matching \"%s\"", valueName.cStr());
		}
		statsValues.push_back(value);
	}

	auto facilities = form->findControlTyped<ListBox>("LISTBOX_FACILITIES");
	for (auto &i : state->facility_types)
	{
		auto &facility = i.second;
		if (!facility->isVisible())
			continue;

		auto graphic = mksp<Graphic>(facility->sprite);
		graphic->AutoSize = true;
		graphic->setData(mksp<UString>(i.first));
		graphic->Name = "FACILITY_BUILD_TILE";
		facilities->addItem(graphic);
	}

	form->findControlTyped<GraphicButton>("BUTTON_OK")
	    ->addCallback(FormEventType::ButtonClick,
	                  [this](Event *) { this->stageCmd.cmd = StageCmd::Command::POP; });
	form->findControlTyped<GraphicButton>("BUTTON_BASE_EQUIPVEHICLE")
	    ->addCallback(FormEventType::ButtonClick, [this](Event *) {
		    // FIXME: If you don't have any vehicles this button should do nothing
		    this->stageCmd.cmd = StageCmd::Command::PUSH;
		    this->stageCmd.nextStage = mksp<VEquipScreen>(state);
		});
	form->findControlTyped<GraphicButton>("BUTTON_BASE_RES_AND_MANUF")
	    ->addCallback(FormEventType::ButtonClick, [this](Event *) {
		    // FIXME: If you don't have any facilities this button should do nothing
		    this->stageCmd.cmd = StageCmd::Command::PUSH;
		    this->stageCmd.nextStage = mksp<ResearchScreen>(state);
		});
	form->findControlTyped<TextEdit>("TEXT_BASE_NAME")
	    ->addCallback(FormEventType::TextEditFinish, [this](Event *e) {
		    this->state->current_base->name =
		        std::dynamic_pointer_cast<TextEdit>(e->forms().RaisedBy)->getText();
		});
}

void BaseScreen::pause() {}

void BaseScreen::resume() { textFunds->setText(state->getPlayerBalance()); }

void BaseScreen::finish() {}

void BaseScreen::eventOccurred(Event *e)
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

	if (e->type() == EVENT_MOUSE_MOVE)
	{
		mousePos = {e->mouse().X, e->mouse().Y};
	}

	if (e->type() == EVENT_FORM_INTERACTION)
	{
		if (e->forms().RaisedBy == baseView)
		{
			if (e->forms().EventFlag == FormEventType::MouseMove)
			{
				selection = {e->forms().MouseInfo.X, e->forms().MouseInfo.Y};
				selection /= BaseGraphics::TILE_SIZE;
				if (!drag)
				{
					selFacility = state->current_base->getFacility(selection);
				}
				return;
			}
			else if (e->forms().EventFlag == FormEventType::MouseLeave)
			{
				selection = NO_SELECTION;
				selFacility = nullptr;
				return;
			}
		}
		if (e->forms().RaisedBy->Name == "LISTBOX_FACILITIES")
		{
			if (!drag && e->forms().EventFlag == FormEventType::ListBoxChangeHover)
			{
				auto list = form->findControlTyped<ListBox>("LISTBOX_FACILITIES");
				auto dragFacilityName = list->getHoveredData<UString>();
				if (dragFacilityName)
				{
					dragFacility = StateRef<FacilityType>{state.get(), *dragFacilityName};
					return;
				}
			}
		}
		if (e->forms().RaisedBy->Name == "FACILITY_BUILD_TILE")
		{
			if (!drag && e->forms().EventFlag == FormEventType::MouseLeave)
			{
				selection = NO_SELECTION;
				selFacility = nullptr;
				dragFacility = "";
			}
		}

		if (e->forms().EventFlag == FormEventType::MouseDown)
		{
			if (e->forms().MouseInfo.Button == 1)
			{
				if (!drag && dragFacility)
				{
					if (e->forms().RaisedBy->Name == "LISTBOX_FACILITIES")
					{
						drag = true;
					}
				}
			}
			if (e->forms().MouseInfo.Button == 4)
			{
				auto list = form->findControlTyped<ListBox>("LISTBOX_FACILITIES");
				auto clickedFacilityName = list->getHoveredData<UString>();
				StateRef<FacilityType> clickedFacility;
				if (clickedFacilityName)
					clickedFacility = StateRef<FacilityType>{state.get(), *clickedFacilityName};
				if (!clickedFacility)
					return;

				auto ufopaedia_entry = clickedFacility->ufopaedia_entry;
				sp<UfopaediaCategory> ufopaedia_category;
				if (ufopaedia_entry)
				{
					for (auto &cat : this->state->ufopaedia)
					{
						for (auto &entry : cat.second->entries)
						{
							if (ufopaedia_entry == entry.second)
							{
								ufopaedia_category = cat.second;
								break;
							}
						}
						if (ufopaedia_category)
							break;
					}
					if (!ufopaedia_category)
					{
						LogError("No UFOPaedia category found for entry %s",
						         ufopaedia_entry->title.cStr());
					}
					fw().stagePush(
					    mksp<UfopaediaCategoryView>(state, ufopaedia_category, ufopaedia_entry));
				}
			}
		}

		if (e->forms().EventFlag == FormEventType::MouseUp)
		{
			// Facility construction
			if (drag && dragFacility)
			{
				if (selection != NO_SELECTION)
				{
					Base::BuildError error =
					    state->current_base->canBuildFacility(dragFacility, selection);
					switch (error)
					{
						case Base::BuildError::NoError:
							state->current_base->buildFacility(*state, dragFacility, selection);
							textFunds->setText(state->getPlayerBalance());
							refreshView();
							break;
						case Base::BuildError::Occupied:
							stageCmd.cmd = StageCmd::Command::PUSH;
							stageCmd.nextStage = mksp<MessageBox>(
							    tr("Area Occupied By Existing Facility"),
							    tr("Existing facilities in this area of the base must be destroyed "
							       "before construction work can begin."),
							    MessageBox::ButtonOptions::Ok);
							break;
						case Base::BuildError::OutOfBounds:
							stageCmd.cmd = StageCmd::Command::PUSH;
							stageCmd.nextStage = mksp<MessageBox>(
							    tr("Planning Permission Denied"),
							    tr("Planning permission is denied for this proposed extension to "
							       "the base, on the grounds that the additional excavations "
							       "required would seriously weaken the foundations of the "
							       "building."),
							    MessageBox::ButtonOptions::Ok);
							break;
						case Base::BuildError::NoMoney:
							stageCmd.cmd = StageCmd::Command::PUSH;
							stageCmd.nextStage = mksp<MessageBox>(
							    tr("Funds exceeded"), tr("The proposed construction work is not "
							                             "possible with your available funds."),
							    MessageBox::ButtonOptions::Ok);
							break;
						case Base::BuildError::Indestructible:
							// Indestrictible facilities (IE the access lift) are just silently
							// ignored
							break;
					}
				}
				drag = false;
				dragFacility = "";
			}
			// Facility removal
			else if (selFacility)
			{
				if (selection != NO_SELECTION)
				{
					Base::BuildError error = state->current_base->canDestroyFacility(selection);
					switch (error)
					{
						case Base::BuildError::NoError:
							stageCmd.cmd = StageCmd::Command::PUSH;
							stageCmd.nextStage =
							    mksp<MessageBox>(tr("Destroy facility"), tr("Are you sure?"),
							                     MessageBox::ButtonOptions::YesNo, [this] {
								                     this->state->current_base->destroyFacility(
								                         *this->state, this->selection);
								                     this->selFacility = nullptr;
								                     this->refreshView();
								                 });
							break;
						case Base::BuildError::Occupied:
							stageCmd.cmd = StageCmd::Command::PUSH;
							stageCmd.nextStage = mksp<MessageBox>(tr("Facility in use"), tr(""),
							                                      MessageBox::ButtonOptions::Ok);
						default:
							break;
					}
				}
			}
		}
	}

	selText->setText("");
	selGraphic->setImage(nullptr);
	for (auto label : statsLabels)
	{
		label->setText("");
	}
	for (auto value : statsValues)
	{
		value->setText("");
	}
	if (dragFacility)
	{
		selText->setText(tr(dragFacility->name));
		selGraphic->setImage(dragFacility->sprite);
		statsLabels[0]->setText(tr("Cost to build"));
		statsValues[0]->setText(UString::format("$%d", dragFacility->buildCost));
		statsLabels[1]->setText(tr("Days to build"));
		statsValues[1]->setText(UString::format("%d", dragFacility->buildTime));
		statsLabels[2]->setText(tr("Maintenance cost"));
		statsValues[2]->setText(UString::format("$%d", dragFacility->weeklyCost));
	}
	else if (selFacility != nullptr)
	{
		selText->setText(tr(selFacility->type->name));
		selGraphic->setImage(selFacility->type->sprite);
		if (selFacility->type->capacityAmount > 0)
		{
			statsLabels[0]->setText(tr("Capacity"));
			statsValues[0]->setText(UString::format("%d", selFacility->type->capacityAmount));
			statsLabels[1]->setText(tr("Usage"));
			statsValues[1]->setText(
			    UString::format("%d%%", state->current_base->getUsage(selFacility)));
		}
	}
	else if (selection != NO_SELECTION)
	{
		int sprite = BaseGraphics::getCorridorSprite(state->current_base, selection);
		auto image = UString::format(
		    "PCK:xcom3/UFODATA/BASE.PCK:xcom3/UFODATA/BASE.TAB:%d:xcom3/UFODATA/BASE.PCX", sprite);
		if (sprite != 0)
		{
			selText->setText(tr("Corridor"));
		}
		else
		{
			selText->setText(tr("Earth"));
		}
		selGraphic->setImage(fw().data->loadImage(image));
	}
}

void BaseScreen::update(StageCmd *const cmd)
{
	form->update();
	*cmd = stageCmd;
	stageCmd = StageCmd();
}

void BaseScreen::render()
{
	fw().stageGetPrevious(this->shared_from_this())->render();
	fw().renderer->drawFilledRect({0, 0}, fw().displayGetSize(), Colour{0, 0, 0, 128});
	form->render();
	renderBase();
	BaseStage::render();
}

bool BaseScreen::isTransition() { return false; }

void BaseScreen::renderBase()
{
	const Vec2<int> BASE_POS = form->Location + baseView->Location;

	BaseGraphics::renderBase(BASE_POS, state->current_base);

	// Draw selection
	if (selection != NO_SELECTION)
	{
		Vec2<int> pos = selection;
		Vec2<int> size = {BaseGraphics::TILE_SIZE, BaseGraphics::TILE_SIZE};
		if (drag && dragFacility)
		{
			size *= dragFacility->size;
		}
		else if (selFacility != nullptr)
		{
			pos = selFacility->pos;
			size *= selFacility->type->size;
		}
		pos = BASE_POS + pos * BaseGraphics::TILE_SIZE;
		fw().renderer->drawRect(pos, size, Colour{255, 255, 255});
	}

	// Draw dragged facility
	if (drag && dragFacility)
	{
		sp<Image> facility = dragFacility->sprite;
		Vec2<int> pos;
		if (selection == NO_SELECTION)
		{
			pos = mousePos -
			      Vec2<int>{BaseGraphics::TILE_SIZE, BaseGraphics::TILE_SIZE} / 2 *
			          dragFacility->size;
		}
		else
		{
			pos = BASE_POS + selection * BaseGraphics::TILE_SIZE;
		}
		fw().renderer->draw(facility, pos);
	}
}

}; // namespace OpenApoc
