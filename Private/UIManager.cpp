#include "UIManager.h"
#include"imgui/imgui.h"

UIManager::UIManager(Vec2 Size)
{
    ViewRect.Min = { 0, 0 };
    ViewRect.Max = std::move(Size);
}

// Add root widget
void UIManager::AddRootWidget(std::shared_ptr<Widget> Widget)
{
    RootWidgets.push_back(std::move(Widget));
}

// Update layout for all root widgets
void UIManager::UpdateLayout()
{
    for (auto& Root : RootWidgets)
    {
        if (Root->IsVisible())
            Root->UpdateLayout(ViewRect);
    }
}

void UIManager::UpdateHover(const Vec2& MousePos)
{
    Widget* NewHover = nullptr;

    // Find topmost widget under mouse
    for (auto it = RootWidgets.rbegin(); it != RootWidgets.rend(); ++it)
    {
        if (!(*it)->IsVisible()) continue;
        NewHover = (*it)->FindWidgetAt(MousePos);
        if (NewHover)
            break;
    }

    if (NewHover != HoveredWidget)
    {
        // Send MouseLeave to old hover
        if (HoveredWidget)
        {
            WidgetEvent LeaveEvent(EEventType::MouseLeave, MousePos);
            HoveredWidget->OnEvent(LeaveEvent);  
        }

        HoveredWidget = NewHover;

        // Send MouseEnter to new hover
        if (HoveredWidget)
        {
            WidgetEvent EnterEvent(EEventType::MouseEnter, MousePos);
            HoveredWidget->OnEvent(EnterEvent);
        }
    }
}

void UIManager::FocusHovered()
{
    // Set focus
    if (HoveredWidget && HoveredWidget != FocusedWidget)
    {
        if (FocusedWidget)
        {
            WidgetEvent e(EEventType::FocusLost);
            FocusedWidget->OnEvent(e);
        }
        FocusedWidget = HoveredWidget;
        WidgetEvent e(EEventType::FocusGained);
        FocusedWidget->OnEvent(e);
    }
    else if (FocusedWidget && !HoveredWidget)//clear focus when clicking on empty spaces
    {
        WidgetEvent e(EEventType::FocusLost);
        FocusedWidget->OnEvent(e);

        FocusedWidget = nullptr;
    }
}

// Tick all
void UIManager::Tick(double DeltaTime)
{
    for (auto& Root : RootWidgets)
    {
        if (Root->CanTick())
            Root->Tick(DeltaTime);
    }
}

// Draw all
void UIManager::Draw()
{
    for (auto& Root : RootWidgets)
    {
        if (Root->IsVisible())
            Root->Draw();
    }
}

// Handle ImGui events
void UIManager::HandleEvents()
{
    ImGuiIO& io = ImGui::GetIO();

    // Always update hover
    UpdateHover({ io.MousePos.x, io.MousePos.y });

    // Left Mouse click
    if (io.MouseClicked[ImGuiMouseButton_Left])
    {
        FocusHovered();

        WidgetEvent Click(EEventType::MouseClick, io.MousePos, ImGuiMouseButton_Left);
        DispatchEvent(Click);
    }

    // left Mouse release
    if (io.MouseReleased[ImGuiMouseButton_Left])
    {
        WidgetEvent Release(EEventType::MouseRelease, io.MousePos, ImGuiMouseButton_Left);
        DispatchEvent(Release);
    }

    // right Mouse click
    if (io.MouseClicked[ImGuiMouseButton_Right])
    {
        FocusHovered();

        WidgetEvent Click(EEventType::MouseClick, io.MousePos, ImGuiMouseButton_Right);
        DispatchEvent(Click);
    }

    // right Mouse release
    if (io.MouseReleased[ImGuiMouseButton_Right])
    {
        WidgetEvent Release(EEventType::MouseRelease, io.MousePos, ImGuiMouseButton_Right);
        DispatchEvent(Release);
    }

    //mouse wheel
    if (io.MouseWheel != 0.f)
    {
        WidgetEvent e(EEventType::MouseWheel, io.MousePos, 0,ImVec2{ io.MouseWheel,io.MouseWheelH });
        DispatchEvent(e);
    }

    if (io.MouseDelta != ImVec2{0.f,0.f})
    {
        WidgetEvent e(EEventType::MouseMove, io.MousePos, 0, io.MouseDelta);
        DispatchEvent(e);
    }
    
    // Keyboard events (only to focused widget)
    if (FocusedWidget)
    {
        
        for (int key = 0; key < IM_ARRAYSIZE(io.KeysDown); ++key)
        {
            auto iter = mDownKeys.find(key);
            bool PrevDown = iter != mDownKeys.end();

            if (io.KeysDown[key] != PrevDown)
            {
                if (PrevDown)
                {
                    mDownKeys.erase(iter);
                    WidgetEvent KeyEvt(EEventType::KeyUp, {}, key);
                    FocusedWidget->OnEvent(KeyEvt);
                }
                else
                {
                    mDownKeys.insert(key);
                    WidgetEvent KeyEvt(EEventType::KeyDown, {}, key);
                    FocusedWidget->OnEvent(KeyEvt);
                }
            }
        }
    }
}

// Main dispatch point
void UIManager::DispatchEvent(WidgetEvent& Event, bool OnlyOnHovered)
{
    Widget* Hit = HoveredWidget;

    if (!Hit && OnlyOnHovered) return;

    if(!Hit)
    // Find topmost hit widget among all roots
    for (auto it = RootWidgets.rbegin(); it != RootWidgets.rend(); ++it)
    {
        if (!(*it)->IsVisible()) continue;
        Hit = (*it)->FindWidgetAt(Event.MousePos);
        if (Hit)
            break;
    }

    if (Hit)
    {
        // Send event to that widget, allow bubbling up
        Widget* Current = Hit;
        while (Current)
        {
            bool Handled = Current->OnEvent(Event);
            if (Handled)
                break;
            Current = Current->GetParent();
        }
    }
}

void UIManager::SetViewRect(Rect newRect)
{
    ViewRect = std::move(newRect);
}
