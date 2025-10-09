#pragma once
#include<stdint.h>
#include<imgui/imgui.h>
#include<vector>
#include<string>
#include<memory>
#include<functional>

enum class EAlign
{
    Left,
    Center,
    Right,
    Top,
    Bottom
};

using Vec2 = ImVec2;

struct Rect
{
    Vec2 Min;
    Vec2 Max;

    inline bool Contains(const Vec2& P) const
    {
        return P.x >= Min.x && P.x <= Max.x &&
            P.y >= Min.y && P.y <= Max.y;
    }

    inline Vec2 Size() const { return Max - Min; }
};

enum class EEventType
{
    None,
    MouseMove,
    MouseClick,
    MouseRelease,
    MouseWheel,
    MouseEnter,
    MouseLeave,
    FocusGained,
    FocusLost,
    KeyDown,
    KeyUp
};

struct WidgetEvent
{
    EEventType Type = EEventType::None;
    Vec2 MousePos = {};
    Vec2 Value = {};
    int Key = 0;
   
    WidgetEvent(EEventType InType = EEventType::None, Vec2 mousePos = {}, int key = 0, Vec2 value = {})
        : Type(InType), MousePos(mousePos), Key{ key }, Value{value} {
    }
};

class Widget
{
protected:
    std::string Name;
    Widget* Parent = nullptr;
    std::vector<std::shared_ptr<Widget>> Children;

    // Tick & visibility
    uint32_t bTickAllowed : 1 = false;
    uint32_t bTickEnabled : 1 = true;
    uint32_t bVisible : 1 = true;
    uint32_t bHovered : 1 = false;
    uint32_t bFocused : 1 = false;

    // Layout
    Vec2 Position = { 0, 0 };
    Vec2 Size = { 100, 30 };
    Rect AbsoluteRect = {};
    bool bAutoSize = false;

    EAlign HorizontalAlign = EAlign::Left;
    EAlign VerticalAlign = EAlign::Top;

public:
    Widget(std::string InName = "Widget") : Name(std::move(InName)) {}
    virtual ~Widget() = default;

    // --- Lifecycle ---
    virtual void Construct() {}
    virtual void Destruct() {}

    virtual void Tick(double DeltaTime) {}
    virtual void Draw() {}

    // --- Layout ---
    virtual void UpdateLayout(const Rect& ParentRect);

    // Returns the topmost widget under a given screen position
    virtual Widget* FindWidgetAt(const Vec2& P);

    // --- Event Routing ---
    virtual bool HandleEvent(WidgetEvent& Event);

    virtual bool OnEvent(WidgetEvent& Event);

    //basic events
    virtual bool OnMouseEnter(WidgetEvent& Even) { return false; }
    virtual bool OnMouseLeave(WidgetEvent& Even) { return false; }
    virtual bool OnMouseClick(WidgetEvent& Even) { return false; }
    virtual bool OnMouseRelease(WidgetEvent& Even) { return false; }
    virtual bool OnMouseMove(WidgetEvent& Even) { return false; }
    virtual bool OnMouseWheel(WidgetEvent& Even) { return false; }

    virtual bool OnFocusGained(WidgetEvent& Even) { return false; }
    virtual bool OnFocusLost(WidgetEvent& Even)  { return false; }

    virtual bool OnKeyDown(WidgetEvent& Even) { return false; }
    virtual bool OnKeyUp(WidgetEvent& Even) { return false; }

    //hover management
    inline bool IsHovered() const { return bHovered; }

    // Focus management
    inline bool IsFocused() const { return bFocused; }

    // --- Hierarchy ---
    void AddChild(std::shared_ptr<Widget> Child);

    const std::vector<std::shared_ptr<Widget>>& GetChildren() const { return Children; }
    Widget* GetParent() const { return Parent; }

    // --- Position & Size ---
    inline void SetPosition(Vec2 InPos) { Position = InPos; }
    inline void SetSize(Vec2 InSize) { Size = InSize; }
    inline void SetAutoSize(bool b) { bAutoSize = b; }

    inline Vec2 GetPosition() const { return Position; }
    inline Vec2 GetSize() const { return Size; }
    inline const Rect& GetAbsoluteRect() const { return AbsoluteRect; }

    // --- Visibility & Tick ---
    inline bool IsVisible() const { return bVisible; }
    inline void SetVisible(bool v) { bVisible = v; }

    inline bool CanEverTick() const { return bTickAllowed; }
    inline bool IsTickEnabled() const { return bTickEnabled; }
    inline bool CanTick() const { return bTickAllowed && bTickEnabled && bVisible; }

    // --- Alignments ---
    void SetAlignment(EAlign HAlign, EAlign VAlign)
    {
        HorizontalAlign = HAlign;
        VerticalAlign = VAlign;
    }

    const std::string& GetName() const { return Name; }
};


