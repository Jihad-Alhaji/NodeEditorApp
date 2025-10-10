#include "Widget.h"

// --- Layout ---
void Widget::UpdateLayout(const Rect& ParentRect)
{
    Vec2 ParentPos = ParentRect.Min;
    Vec2 ParentSize = ParentRect.Size();

    // horizontal alignment
    float X = ParentPos.x + Position.x;
    if (HorizontalAlign == EAlign::Center)
        X = ParentPos.x + (ParentSize.x - Size.x) / 2.0f + Position.x;
    else if (HorizontalAlign == EAlign::Right)
        X = ParentPos.x + ParentSize.x - Size.x - Position.x;

    // vertical alignment
    float Y = ParentPos.y + Position.y;
    if (VerticalAlign == EAlign::Center)
        Y = ParentPos.y + (ParentSize.y - Size.y) / 2.0f + Position.y;
    else if (VerticalAlign == EAlign::Bottom)
        Y = ParentPos.y + ParentSize.y - Size.y - Position.y;

    AbsoluteRect.Min = { X, Y };
    AbsoluteRect.Max = AbsoluteRect.Min + Size;

    // Propagate to children
    for (auto& Child : Children)
    {
        if (Child->IsVisible())
            Child->UpdateLayout(AbsoluteRect);
    }
}

// Returns the topmost widget under a given screen position
Widget* Widget::FindWidgetAt(const Vec2& P)
{
    if (!bVisible)
        return nullptr;

    // Check children first (reverse order for proper stacking)
    for (auto it = Children.rbegin(); it != Children.rend(); ++it)
    {
        Widget* ChildHit = (*it)->FindWidgetAt(P);
        if (ChildHit)
            return ChildHit;
    }

    // Then check self
    if (AbsoluteRect.Contains(P))
        return this;

    return nullptr;
}


// --- Event Routing ---
bool Widget::HandleEvent(WidgetEvent& Event)
{
    if (!bVisible)
        return false;

    // Let children try first
    for (auto it = Children.rbegin(); it != Children.rend(); ++it)
    {
        if ((*it)->HandleEvent(Event))
            return true;
    }

    // Then try self
    if (AbsoluteRect.Contains(Event.MousePos))
    {
        return OnEvent(Event);
    }

    return false;
}

// --- Hierarchy ---
void Widget::AddChild(std::shared_ptr<Widget> Child)
{
    Child->Parent = this;
    Children.push_back(std::move(Child));
}

void Widget::OnDragStarted()
{
    bDragged = true;
}

void Widget::OnDragEnded()
{
    bDragged = false;
}

bool Widget::OnEvent(WidgetEvent& Event)
{
    switch (Event.Type)
    {
    case EEventType::MouseMove:
        return OnMouseMove(Event);
    case EEventType::MouseClick:
        return OnMouseClick(Event);
    case EEventType::MouseRelease:
        return OnMouseRelease(Event);
    case EEventType::MouseWheel:
        return OnMouseWheel(Event);
    case EEventType::MouseEnter:
        bHovered = true;
        return OnMouseEnter(Event);
    case EEventType::MouseLeave:
        bHovered = false;
        return OnMouseLeave(Event);
    case EEventType::FocusGained:
        bFocused = true;
        return OnFocusGained(Event);
    case EEventType::FocusLost:
        bFocused = false;
        return OnFocusLost(Event);
    case EEventType::KeyDown:
        return OnKeyDown(Event);
    case EEventType::KeyUp:
        return OnKeyUp(Event);
    case EEventType::Drop:
        return OnRecieveDrop(Event);
    default:
        break;
    }
    
    return false;
}

void Widget::Tick(double dt)
{
    for (auto& c : Children)
    {
        if (c->CanTick())
           c->Tick(dt);
    }
}

void Widget::Draw()
{
    for (auto& c : Children)
    {
        if (c->IsVisible())
            c->Draw();
    }
}

void Widget::Construct()
{
    for (auto& c : Children)
    {
        c->Construct();
    }
}
void Widget::Destruct()
{
    for (auto& c : Children)
    {
        c->Destruct();
    }
}