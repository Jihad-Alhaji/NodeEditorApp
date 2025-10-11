#include "NGEditor.h"
#include"GraphView.h"
#include"UI_HorizontalBox.h"
#include"UI_VerticalBox.h"

namespace NodeEditor
{
	NGEditor::NGEditor(std::string GraphName):Widget(GraphName)
	{
		HB_Main = std::make_shared<UI_HorizontalBox>();
		
		Graph = std::make_shared<GraphView>();
		HB_Main->AddChild(Graph);

		VB_Details = std::make_shared<UI_VerticalBox>();
		HB_Main->AddChild(VB_Details);

		AddChild(HB_Main);
	}
	void NGEditor::Draw()
	{
		bool bOpen = bVisible;

		ImGui::SetNextWindowSize(Size, ImGuiCond_FirstUseEver);
		if (!ImGui::Begin(Name.c_str(), &bOpen))
		{
			SetVisible(false);
			return;
		}
		else if (!bOpen) // close was pressed
		{
			SetVisible(false);
			ImGui::End();
			return;
		}

		//this so we update children based on our own rect , ignoting parent restrictiosn
		Size =  ImGui::GetContentRegionAvail();
		AbsoluteRect.Min = ImGui::GetWindowContentRegionMin() + ImGui::GetWindowPos();
		AbsoluteRect.Max = ImGui::GetWindowContentRegionMax() + ImGui::GetWindowPos();

		Widget::Draw();
		ImGui::End();
	}

	void NGEditor::UpdateLayout(const Rect& ParentRect)
	{
		HB_Main->SetSize(Size);
		Graph->SetSize(Size * ImVec2{ UI_MainPanelSplit,1 });
		VB_Details->SetSize(Size - Graph->GetSize());

		for (auto& c : Children) {
			c->UpdateLayout(AbsoluteRect);
		}
	}
}

