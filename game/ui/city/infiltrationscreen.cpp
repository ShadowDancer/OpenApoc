#include "game/ui/city/infiltrationscreen.h"
#include "forms/ui.h"
#include "framework/event.h"
#include "framework/framework.h"
#include "game/state/gamestate.h"

namespace OpenApoc
{

InfiltrationScreen::InfiltrationScreen(sp<GameState> state)
    : Stage(), menuform(ui().getForm("FORM_INFILTRATION_SCREEN")), state(state)
{
}

InfiltrationScreen::~InfiltrationScreen() = default;

void InfiltrationScreen::begin()
{
	menuform->findControlTyped<Label>("TEXT_FUNDS")->setText(state->getPlayerBalance());
}

void InfiltrationScreen::pause() {}

void InfiltrationScreen::resume() {}

void InfiltrationScreen::finish() {}

void InfiltrationScreen::eventOccurred(Event *e)
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
	}
}

void InfiltrationScreen::update(StageCmd *const cmd)
{
	menuform->update();
	*cmd = this->stageCmd;
	// Reset the command to default
	this->stageCmd = StageCmd();
}

void InfiltrationScreen::render()
{
	fw().stageGetPrevious(this->shared_from_this())->render();
	fw().renderer->drawFilledRect({0, 0}, fw().displayGetSize(), Colour{0, 0, 0, 128});
	menuform->render();
}

bool InfiltrationScreen::isTransition() { return false; }

}; // namespace OpenApoc
