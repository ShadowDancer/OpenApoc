#include "game/ui/city/basebuyscreen.h"
#include "forms/ui.h"
#include "framework/event.h"
#include "framework/framework.h"
#include "game/state/city/building.h"
#include "game/state/gamestate.h"
#include "game/ui/base/basegraphics.h"
#include "game/ui/city/cityview.h"
#include "game/ui/general/messagebox.h"

namespace OpenApoc
{

BaseBuyScreen::BaseBuyScreen(sp<GameState> state, sp<Building> building)
    : Stage(), form(ui().getForm("FORM_BUY_BASE_SCREEN")), state(state)
{
	Vec2<int> size = building->bounds.size();
	price = std::min(size.x, 8) * std::min(size.y, 8) * COST_PER_TILE;
	base = mksp<Base>(*state, StateRef<Building>{state.get(), building});
}

BaseBuyScreen::~BaseBuyScreen() = default;

void BaseBuyScreen::begin()
{
	baseView = form->findControlTyped<Graphic>("GRAPHIC_BASE_VIEW");

	form->findControlTyped<Label>("TEXT_FUNDS")->setText(state->getPlayerBalance());

	auto text = form->findControlTyped<Label>("TEXT_PRICE");
	text->setText(UString::format(tr("This Building will cost $%d"), price));

	form->findControlTyped<Graphic>("GRAPHIC_MINIMAP")
	    ->setImage(BaseGraphics::drawMinimap(state, base->building));
}

void BaseBuyScreen::pause() {}

void BaseBuyScreen::resume() {}

void BaseBuyScreen::finish() {}

void BaseBuyScreen::eventOccurred(Event *e)
{
	form->eventOccured(e);

	if (e->type() == EVENT_KEY_DOWN)
	{
		if (e->keyboard().KeyCode == SDLK_ESCAPE)
		{
			stageCmd.cmd = StageCmd::Command::POP;
		}
	}

	else if (e->type() == EVENT_FORM_INTERACTION &&
	         e->forms().EventFlag == FormEventType::ButtonClick)
	{
		if (e->forms().RaisedBy->Name == "BUTTON_OK")
		{
			stageCmd.cmd = StageCmd::Command::POP;
		}
		else if (e->forms().RaisedBy->Name == "BUTTON_BUY_BASE")
		{
			if (state->getPlayer()->balance >= price)
			{
				state->getPlayer()->balance -= price;
				base->building->owner = state->getPlayer();
				base->name = "Base " + Strings::fromInteger(state->player_bases.size() + 1);
				state->player_bases[Base::getPrefix() +
				                    Strings::fromInteger(state->player_bases.size() + 1)] = base;

				stageCmd.cmd = StageCmd::Command::REPLACE;
				stageCmd.nextStage = mksp<CityView>(state);
			}
			else
			{
				stageCmd.cmd = StageCmd::Command::PUSH;
				stageCmd.nextStage =
				    mksp<MessageBox>(tr("No Sale"), tr("Not enough money to buy this building."),
				                     MessageBox::ButtonOptions::Ok);
			}
		}
	}
}

void BaseBuyScreen::update(StageCmd *const cmd)
{
	form->update();
	*cmd = this->stageCmd;
	// Reset the command to default
	this->stageCmd = StageCmd();
}

void BaseBuyScreen::render()
{
	fw().stageGetPrevious(this->shared_from_this())->render();
	fw().renderer->drawFilledRect({0, 0}, fw().displayGetSize(), Colour{0, 0, 0, 128});
	form->render();
	renderBase();
}

bool BaseBuyScreen::isTransition() { return false; }

void BaseBuyScreen::renderBase()
{
	const Vec2<int> BASE_POS = form->Location + baseView->Location;

	BaseGraphics::renderBase(BASE_POS, base);
}

}; // namespace OpenApoc
