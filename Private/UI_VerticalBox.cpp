#include "UI_VerticalBox.h"

UI_VerticalBox::UI_VerticalBox():Widget("VerticalBox")
{
	bTickAllowed = true;
	bAutoSize = false;
}

std::shared_ptr<Widget> UI_VerticalBox::GetChildAt(size_t index)
{
	if (index < Children.size())
	{
		return Children.at(index);
	}
	return nullptr;
}

void UI_VerticalBox::RemoveChildAt(size_t index)
{
	Children.erase(Children.begin() + index);
}

void UI_VerticalBox::UpdateLayout(const Rect& ParentRect)
{
	//if auto size, then we resize to content accomulated size
	if (bAutoSize)
	{
		Size = {};
		for (auto& c : Children) {
			ImVec2 cSize = c->GetSize();
			Size.x = std::max(Size.x, cSize.x);
			Size.y += cSize.y;
		}
	}
	
	Widget::UpdateLayout(ParentRect);
}
