#pragma once
#include"Widget.h"

namespace NodeEditor
{
    class GraphView;
	class NGEditor : public Widget
	{
    public:
        NGEditor(std::string GraphName);
        //virtual void Construct();
        //virtual void Destruct();

        //virtual void Tick(double DeltaTime);
        virtual void Draw()override;

        // --- Layout ---
        virtual void UpdateLayout(const Rect& ParentRect)override;

    protected:
        
        std::shared_ptr<Widget> HB_Main;
        std::shared_ptr<GraphView> Graph;
        std::shared_ptr<Widget> VB_Details;

        float UI_MainPanelSplit = 0.7;
	};
}