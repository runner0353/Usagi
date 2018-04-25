/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Input_ps.h"
#include "Mouse_ps.h"

namespace usg
{
	void Mouse_ps::Update()
	{
		for(uint32 i=0; i<_MouseButton_count; i++)
		{
			m_prevButtons[i] = m_buttons[i];
			m_buttons[i] = m_pOwner->GetMouseButtonDown(i);
		}


		RECT screen;
		POINT mousePos;
		GetCursorPos(&mousePos);
		GetWindowRect(m_hwnd, &screen);
		// Per arnauds request release the buttons outside of the window
		if(mousePos.x > screen.right || mousePos.x < screen.left
			|| mousePos.y < screen.top || mousePos.y > screen.bottom)
		{
			m_buttons[MOUSE_BUTTON_LEFT] = false;
			m_buttons[MOUSE_BUTTON_RIGHT] = false;
			m_buttons[MOUSE_BUTTON_MIDDLE] = false;
		}
		ScreenToClient(m_hwnd, &mousePos);
		m_fAxis[MOUSE_DELTA_X] = m_fAxis[MOUSE_POS_X] - (float)mousePos.x;
		m_fAxis[MOUSE_DELTA_Y] = m_fAxis[MOUSE_POS_Y] - (float)mousePos.y;
		m_fAxis[MOUSE_POS_X] = (float)mousePos.x;
		m_fAxis[MOUSE_POS_Y] = (float)mousePos.y;
		long mouseWheel = m_pOwner->GetMouseWheel();
		if(abs(mouseWheel) > 0)
			m_fAxis[MOUSE_DELTA_WHEEL] = Math::Sign((float)m_pOwner->GetMouseWheel());
		else
			m_fAxis[MOUSE_DELTA_WHEEL] = 0.0f;
	}

}