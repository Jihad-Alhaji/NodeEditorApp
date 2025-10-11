#pragma once
#include"Widget.h"

class UI_HorizontalBox : public Widget
{
public:
	UI_HorizontalBox();
	inline size_t GetNumChildren()const { return Children.size(); };
	std::shared_ptr<Widget> GetChildAt(size_t index);
	void RemoveChildAt(size_t index);
	void UpdateLayout(const Rect& ParentRect)override;
};

