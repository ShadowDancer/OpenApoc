#include "game/ui/ufopaedia/ufopaediaview.h"
#include "forms/ui.h"
#include "framework/event.h"
#include "framework/framework.h"
#include "game/state/gamestate.h"
#include "game/ui/ufopaedia/ufopaediacategoryview.h"
#include "library/sp.h"

namespace OpenApoc
{

UfopaediaView::UfopaediaView(sp<GameState> state)
    : Stage(), menuform(ui().getForm("FORM_UFOPAEDIA_TITLE")), state(state)
{
}

UfopaediaView::~UfopaediaView() = default;

void UfopaediaView::begin() {}

void UfopaediaView::pause() {}

void UfopaediaView::resume() {}

void UfopaediaView::finish() {}

void UfopaediaView::eventOccurred(Event *e)
{
	menuform->eventOccured(e);

	if (e->type() == EVENT_KEY_DOWN)
	{
		if (e->keyboard().KeyCode == SDLK_ESCAPE)
		{
			stageCmd.cmd = StageCmd::Command::POP;
			return;
		}
	}

	if (e->type() == EVENT_FORM_INTERACTION && e->forms().EventFlag == FormEventType::ButtonClick)
	{
		if (e->forms().RaisedBy->Name == "BUTTON_QUIT")
		{
			stageCmd.cmd = StageCmd::Command::POP;
			return;
		}

		if (e->forms().RaisedBy->Name.substr(0, 7) == "BUTTON_")
		{
			for (auto &cat : state->ufopaedia)
			{
				auto catName = cat.first;
				UString butName = "BUTTON_" + catName;
				if (butName == e->forms().RaisedBy->Name)
				{
					stageCmd.cmd = StageCmd::Command::PUSH;
					stageCmd.nextStage = mksp<UfopaediaCategoryView>(state, cat.second);
					LogInfo("Clicked category \"%s\"", catName.cStr());
					return;
				}
			}
		}
	}
}

void UfopaediaView::update(StageCmd *const cmd)
{
	menuform->update();
	*cmd = this->stageCmd;
	// Reset the command to default
	this->stageCmd = StageCmd();
}

void UfopaediaView::render()
{
	fw().stageGetPrevious(this->shared_from_this())->render();
	fw().renderer->drawFilledRect({0, 0}, fw().displayGetSize(), Colour{0, 0, 0, 128});
	menuform->render();
}

bool UfopaediaView::isTransition() { return false; }

}; // namespace OpenApoc
