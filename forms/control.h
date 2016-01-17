
#pragma once
#include "library/sp.h"

#include "framework/includes.h"
#include "library/colour.h"
#include "framework/font.h"
#include "framework/logger.h"

namespace OpenApoc
{

class Form;
class Event;
class Surface;
class Palette;
class RadioButton;

class Control
{
  private:
	sp<Surface> controlArea;
	void *data;
	std::map<UString, RadioButton**> radiogroups;

  protected:
	sp<Palette> palette;
	Control *owningControl;
	bool mouseInside;
	bool mouseDepressed;
	Vec2<int> resolvedLocation;

	virtual void PreRender();
	virtual void PostRender();
	virtual void OnRender();

	bool IsFocused() const;

	void ResolveLocation();
	virtual void ConfigureFromXML(tinyxml2::XMLElement *Element);

	Control *GetRootControl();

	std::list<UString> WordWrapText(sp<OpenApoc::BitmapFont> UseFont, UString WrapText) const;

	void CopyControlData(Control *CopyOf);

  public:
	UString Name;
	Vec2<int> Location;
	Vec2<int> Size;
	Colour BackgroundColour;
	bool takesFocus;
	bool showBounds;
	bool Visible;

	bool canCopy;
	Control *lastCopiedTo;
	std::vector<Control *> Controls;

	Control(Control *Owner, bool takesFocus = true);
	virtual ~Control();

	Control *GetActiveControl() const;

	virtual void EventOccured(Event *e);
	void Render();
	virtual void Update();
	virtual void UnloadResources();

	Control *operator[](int Index) const;
	Control *FindControl(UString ID) const;

	template <typename T> T *FindControlTyped(const UString &name) const
	{
		Control *c = this->FindControl(name);
		if (!c)
		{
			LogError("Failed to find control \"%s\" within form \"%s\"", name.c_str(),
			         this->Name.c_str());
			return nullptr;
		}
		T *typedControl = dynamic_cast<T *>(c);
		if (!c)
		{
			LogError("Failed to cast control \"%s\" within form \"%s\" to type \"%s\"",
			         name.c_str(), this->Name.c_str(), typeid(T).name());
			return nullptr;
		}
		return typedControl;
	}

	Control *GetParent() const;
	Form *GetForm();
	void SetParent(Control *Parent);

	Vec2<int> GetLocationOnScreen() const;

	virtual Control *CopyTo(Control *CopyParent);

	template <typename T> T *GetData() const { return static_cast<T *>(data); }
	void SetData(void *Data) { data = Data; }

	bool eventIsWithin(const Event *e) const;
};

}; // namespace OpenApoc
