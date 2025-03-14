#pragma once
#include "Activity.h"
#include "common/String.h"
#include "gui/interface/Point.h"
#include "gui/interface/Fade.h"
#include <vector>

class Tool;
class ToolButton;
class GameController;

namespace ui
{
	class ScrollPanel;
	class Textbox;
}

class ElementSearchActivity: public WindowActivity
{
	Tool * firstResult;
	GameController * gameController;
	std::vector<Tool*> tools;
	ui::Textbox * searchField;
	std::vector<ToolButton*> toolButtons;
	ui::ScrollPanel *scrollPanel = nullptr;
	String toolTip;
	ui::Fade toolTipPresence{ ui::Fade::LinearProfile{ 120.f, 60.f }, 0, 0 };
	bool shiftPressed;
	bool ctrlPressed;
	bool altPressed;
	bool isToolTipFadingIn;
	void searchTools(String query);

public:
	bool exit;
	Tool * GetFirstResult() { return firstResult; }
	ElementSearchActivity(GameController * gameController, std::vector<Tool*> tools);
	void SetActiveTool(int selectionState, Tool * tool);
	virtual ~ElementSearchActivity();
	void OnTick() override;
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnDraw() override;
	void ToolTip(ui::Point senderPosition, String ToolTip) override;
};
