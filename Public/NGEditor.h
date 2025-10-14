#pragma once
#include"Widget.h"

namespace NodeEditor
{
    class GraphView;
	class NGEditor : public Widget
	{
    public:
        NGEditor(std::shared_ptr<GraphView> EditedGraph);
        //virtual void Construct();
        //virtual void Destruct();

        //virtual void Tick(double DeltaTime);
        virtual void Draw()override;

        // --- Layout ---
        virtual void UpdateLayout(const Rect& ParentRect)override;

    protected:
        
        std::shared_ptr<Widget> HB_Main;//main horizontal box
        std::shared_ptr<GraphView> Graph;//edited graph
        std::shared_ptr<Widget> VB_Details;//details vertical box

        float UI_MainPanelSplit = 0.7;//main hor box split ratio
	};
}