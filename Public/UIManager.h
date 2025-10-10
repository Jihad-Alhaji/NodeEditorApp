#pragma once
#include "Widget.h"
#include <memory>
#include <vector>
#include<set>

class UIManager
{
private:
    std::vector<std::shared_ptr<Widget>> RootWidgets;
    std::set<int> mDownKeys;
    // Screen or viewport dimensions
    Rect ViewRect;

    // Internal states
    Widget* HoveredWidget = nullptr;
    Widget* FocusedWidget = nullptr;

    Widget* DraggedWidget = nullptr;

public:
    UIManager(Vec2 Size);

    // Add root widget
    void AddRootWidget(std::shared_ptr<Widget> Widget);

    // Update layout for all root widgets
    void UpdateLayout();

    void UpdateHover(const Vec2& MousePos);
    void FocusHovered();

    // Tick all
    void Tick(double DeltaTime);

    // Draw all
    void Draw();

    // Handle ImGui events
    void HandleEvents();

    // Main dispatch point
    void DispatchEvent(WidgetEvent& Event , bool OnlyOnHovered = true);

    void SetViewRect(Rect newRect);

    const Rect& GetViewRect() const { return ViewRect; }

    //drag drop features
    
    bool BeginDrag(Widget* wid);
    void UpdateDrag();

    //if drop = true, then will fire a drop event with the Widget* as payload
    void EndDrag(bool drop = true);

    bool IsDragging()const { return DraggedWidget != nullptr; };

protected:
    bool ShouldStartDrag()const;
};
