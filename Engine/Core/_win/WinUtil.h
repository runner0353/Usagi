/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Author: Alex Barnfield
//	Description: Windows specific hardware level functions
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/Display.h"

namespace WINUTIL
{
	bool Init(HINSTANCE hInst);

	HINSTANCE	GetInstanceHndl();
	// FIXME: Not ideal
	WindHndl		GetWindow();

	// Call the following on the returned handle when
	// Input::GetPlatform().RegisterHwnd(0, hndl);
	WindHndl	CreateDisplayWindow(const char* szName, const usg::DisplaySettings* pDisplaySettings, bool bHidden);

};
