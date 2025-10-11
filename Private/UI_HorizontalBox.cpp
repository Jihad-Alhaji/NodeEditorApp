#include "UI_HorizontalBox.h"

UI_HorizontalBox::UI_HorizontalBox() :Widget("HorizontalBox")
{
	bTickAllowed = true;
	bAutoSize = false;
}

std::shared_ptr<Widget> UI_HorizontalBox::GetChildAt(size_t index)
{
	if (index < Children.size())
	{
		return Children.at(index);
	}
	return nullptr;
}

void UI_HorizontalBox::RemoveChildAt(size_t index)
{
	Children.erase(Children.begin() + index);
}

void UI_HorizontalBox::UpdateLayout(const Rect& ParentRect)
{
	//if auto size, then we resize to content accomulated size
	if (bAutoSize)
	{
		Size = {};
		for (auto& c : Children) {
			ImVec2 cSize = c->GetSize();
			Size.y = std::max(Size.y, cSize.y);
			Size.x += cSize.x;
		}
	}

	Widget::UpdateLayout(ParentRect);
}