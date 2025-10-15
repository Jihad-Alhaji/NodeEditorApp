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
	for (size_t i = 1; i < Children.size(); i++)
	{
		auto ThisChild = Children[i];
		auto PrevChil = Children[i - 1];
		
		ThisChild->SetPosition(ImVec2{ ThisChild->GetPosition().x,PrevChil->GetPosition().y + PrevChil->GetSize().y});
	}
	if (bAutoSize)
	{
		if (!Children.empty())
		{
			auto min = Children[0]->GetAbsoluteRect().Min;
			Size = Children.back()->GetAbsoluteRect().Max - min;
		}
	}
	Widget::UpdateLayout(ParentRect);
}
