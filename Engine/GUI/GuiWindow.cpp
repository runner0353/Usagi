/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/ThirdParty/imgui/imgui.h"
#include "GuiItems.h"
#include "GuiWindow.h"

namespace usg
{
	GUIWindow::GUIWindow() 
		: m_menuBar(false)
		, m_items(20)
	{
		m_fScale = 1.0f;
		m_bShowBorders = false;
		m_bDefaultCollapsed = false;
		m_bCollapsed = false;
	}

	GUIWindow::~GUIWindow()
	{

	}

	void GUIWindow::Init(const char* szName, const Vector2f& vPos, const Vector2f& vSize, WindowType eType)
	{
		m_vPosition = vPos;
		m_vSize = vSize;

		str::Copy(m_szName, szName, USG_IDENTIFIER_LEN);
		m_eWindowType = eType;
	
	}
	
	void GUIWindow::AddItem(GUIItem* pItem)
	{
		m_items.AddToEnd(pItem);
	}

	bool GUIWindow::UpdateAndAddToDrawList()
	{
		Vector2f vPos = m_vPosition * m_fScale;
		Vector2f vScale = m_vSize * m_fScale;
		bool bChanged = false;
		real rTime = (real)ImGui::GetTime();

		switch(m_eWindowType)
		{
			case WINDOW_TYPE_CHILD:
			{
				ImGui::BeginChild(m_szName, ImVec2(vScale.x, vScale.y), true, 0);// m_bShowBorders ? ImGuiWindowFlags_ShowBorders : 0);
			}
			break;
			case WINDOW_TYPE_PARENT:
			{
				ImGui::SetNextWindowPos(ImVec2(vPos.x, vPos.y), ImGuiCond_Once);	// Don't allow our menus to be moved (for now)
				ImGui::SetNextWindowSize(ImVec2(vScale.x, vScale.y), ImGuiCond_Once);
				bool bReturn;
				ImGui::Begin(m_szName, &bReturn, m_menuBar.IsVisible() ? ImGuiWindowFlags_MenuBar : 0);
			}
			break;
			case WINDOW_TYPE_COLLAPSABLE:
			{
				m_bCollapsed = !ImGui::CollapsingHeader(m_szName, m_bDefaultCollapsed ? 0 : ImGuiTreeNodeFlags_DefaultOpen);// NULL, true, !m_bDefaultCollapsed);
				if( m_bCollapsed )
					return false;
			}
			break;
			case WINDOW_TYPE_DUMMY:
				break;
			default:
				ASSERT(false);
		}	
		
		m_menuBar.UpdateAndAddToDrawList();

		ImGui::PushItemWidth(180.f);
		ImGui::SetWindowFontScale(m_fScale);
		if(m_bVisible)
		{
			for(List<GUIItem>::Iterator it = m_items.Begin(); !it.IsEnd(); ++it)
			{
				GUIItem* pItem = (*it);
				if(pItem->IsVisible())
				{
					bChanged = pItem->UpdateAndAddToDrawList() || bChanged;
					pItem->SetHovered(ImGui::IsItemHovered(), rTime);
				}
				else
				{
					pItem->SetHovered(false, rTime);
				}
				pItem->CommonDraw();
			}
		}
		ImGui::PopItemWidth();

		switch(m_eWindowType)
		{
			case WINDOW_TYPE_CHILD:
				ImGui::EndChild();
			break;
			case WINDOW_TYPE_PARENT:
				ImGui::End();
			break;
			case WINDOW_TYPE_DUMMY:
				break;
			default:
				break;
		}

		return bChanged;
	}
}
