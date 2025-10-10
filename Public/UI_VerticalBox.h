#pragma once
#include"Widget.h"
class UI_VerticalBox :public Widget
{
public:
	UI_VerticalBox();

	inline size_t GetNumChildren()const { return Children.size(); };
	std::shared_ptr<Widget> GetChildAt(size_t index);
	void RemoveChildAt(size_t index);
	void UpdateLayout(const Rect& ParentRect)override;
};

