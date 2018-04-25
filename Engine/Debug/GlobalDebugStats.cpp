/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Stats which should always be available regardless of the mode
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Memory/MemHeap.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Core/Timer/ProfilingTimer.h"
#include "GlobalDebugStats.h"

namespace usg {

	GlobalDebugStats::GlobalDebugStats() :
		IDebugStatGroup(),
		m_pCpuTimer(NULL),
		m_uFrameCounter(0)
	{
		for (uint32 i = 0; i < WARNING_COUNT; i++)
		{
			m_warnings[i].bActive = false;
			m_warnings[i].fTimeSinceActive = 0.0f;
		}

		m_fTotalElapsed = 0.0f;
		m_fMaxElapsed = 0.0f;
		m_fGPUTime = 0.0f;

		m_uFrameRate = 60;
	}

	GlobalDebugStats::~GlobalDebugStats()
	{

	}

	void GlobalDebugStats::UpdateFPS(float fElapsed)
	{
		m_uFrameCounter++;
		m_fTotalElapsed += fElapsed;
		if (fElapsed > m_fMaxElapsed)
		{
			m_fMaxElapsed = fElapsed;
		}
		if (m_uFrameCounter >= 5)
		{
			float fAvgTime = m_fTotalElapsed / 5.0f;
			float fFPS = 1.f / fAvgTime;
			m_fTotalElapsed = 0.0f;
			m_fMaxElapsed = 0.0;
			m_uFrameCounter = 0;

			m_uFrameRate = (uint32)(fFPS + 0.5f);
		}
	}

	void GlobalDebugStats::Update(float fElapsed)
	{
		UpdateFPS(fElapsed);

		for (uint32 i = 0; i < WARNING_COUNT; i++)
		{
			if (m_warnings[i].bActive)
			{
				m_warnings[i].fTimeSinceActive = 0.0f;
			}
			else
			{
				m_warnings[i].fTimeSinceActive += fElapsed;
			}
		}
	}

	void GlobalDebugStats::Draw(DebugRender* pRender)
	{
		switch (m_uActivePage)
		{
		case PAGE_MAIN:
			DrawMainPage(pRender);
			break;
		case PAGE_MEMORY:
			DrawMemoryPage(pRender, mem::GetMainHeap(), "Main Mem");
			break;
		//case PAGE_THREADS:
//			break;
		default:
			ASSERT(false);
		}
	}

	void GlobalDebugStats::SetThreadActivity(int iIdx, const char* szActivitys)
	{
		//m_lastThread[iIdx] = m_currentThread[iIdx];
		//m_currentThread[iIdx] = szActivitys;
	}

	void GlobalDebugStats::PreDraw(GFXDevice* pDevice)
	{

	}

	void GlobalDebugStats::PostDraw(GFXDevice* pDevice)
	{
		if (m_pCpuTimer && m_pCpuTimer->GetTimerTotal() > 0)
		{
			m_fCPUTime = m_pCpuTimer->GetTotalMilliSeconds();
		}
		else
		{
			m_fCPUTime = 0.0f;
		}

		m_warnings[WARNING_CPU_HEAVY].bActive = m_fCPUTime > 16.0f;
		m_fGPUTime = pDevice->GetGPUTime();
		m_warnings[WARNING_GPU_HEAVY].bActive = m_fGPUTime > 15.5f;

		m_warnings[WARNING_MEMORY].bActive = ((float)mem::GetMainHeap()->GetFreeSize() / (float)mem::GetMainHeap()->GetSize()) < 0.05f;
	}

	void GlobalDebugStats::AppendWarnings(U8String& string)
	{
		usg::U8String warningString;
		if (m_warnings[WARNING_CPU_HEAVY].fTimeSinceActive < 1.5f)
		{
			warningString.ParseString("CPU %f ms", m_fCPUTime);
			string += warningString;
		}
		if (m_warnings[WARNING_GPU_HEAVY].fTimeSinceActive < 1.5f)
		{
			warningString.ParseString("GPU %f ms", m_fGPUTime);
			string += warningString;
		}

		if (m_warnings[WARNING_MEMORY].fTimeSinceActive < 1.5f)
		{
			MemHeap* pHeap = mem::GetMainHeap();
			float fSize = static_cast<float>(pHeap->GetSize());
			float fUsed = static_cast<float>(pHeap->GetSize() - pHeap->GetFreeSize());
			U8String str;
			float fMemMB = ((float)fUsed) / (1024.f * 1024.f);
			float fSizeMB = ((float)fSize) / (1024.f * 1024.f);

			warningString.ParseString("Memory (%.1fMB / %.1fMB)", fMemMB, fSizeMB);
			string += warningString;
		}
	}


	void GlobalDebugStats::DrawMainPage(DebugRender* pRender)
	{
		usg::Color cCol;
		if (m_uFrameRate >= 59)
		{
			cCol.Assign(0.0f, 1.0f, 0.0f, 1.0f);
		}
		else if (m_uFrameRate >= 29)
		{
			cCol.Assign(1.0f, 1.0f, 0.0f, 1.0f);
		}
		else
		{
			cCol.Assign(1.0f, 0.0f, 0.0f, 1.0f);
		}

		usg::U8String fpsString;
		usg::U8String cpuString;
		usg::U8String gpuString;
		if (m_pCpuTimer)
		{
			cpuString.ParseString(" (CPU %2.0fms)", m_fCPUTime);
		}
		gpuString.ParseString(" (GPU %2.0fms)", m_fGPUTime);

		fpsString.ParseString("FPS: %d", m_uFrameRate);
		fpsString += cpuString;
		fpsString += gpuString;
		pRender->AddString(fpsString.CStr(), 0.25f, 1.0f, cCol);
	}


	void GlobalDebugStats::DrawThreadsPage(DebugRender* pRender)
	{
		Color cTextCol(0.0f, 1.0f, 0.0f, 1.0f);
		U8String tmpString;
		pRender->AddString("Disabled", 0.0f, 0.0f, cTextCol);
		pRender->AddString("Job Manager Activities:", 0.0f, 0.0f, cTextCol);
		pRender->AddString(tmpString.CStr(), 0.0f, 1.0f, cTextCol);

		/*for(uint32 i = 0; i < uCpuCount; i++)
		{
			tmpString.ParseString("Worker Thread #%d: Current: %s (Last: %s)", i + 1, m_currentThread[i].CStr(), m_lastThread[i].CStr());
			pRender->AddString(tmpString.CStr(), 0.0f, 2.0f + (1.0f * (float)i), cTextCol);
		}*/
	}

	void GlobalDebugStats::DrawMemoryPage(DebugRender* pRender, const MemHeap* pHeap, const char* szName)
	{
		Color cTitle(0.0f, 1.0f, 1.0f, 1.0f);
		Color cUsage(1.0f, 0.0f, 0.0f, 1.0f);

		float fSize = static_cast<float>(pHeap->GetSize());
		float fUsed = static_cast<float>(pHeap->GetSize() - pHeap->GetFreeSize());

		U8String str;
		float fMemMB = ((float)fUsed) / (1024.f * 1024.f);
		float fSizeMB = ((float)fSize) / (1024.f * 1024.f);
		str.ParseString("%s (%.1fMB / %.1fMB)", szName, fMemMB, fSizeMB);

		pRender->AddString(str.CStr(), 0.0f, 1.0f, cTitle);
		pRender->AddBar(2.0f, 0.0f, 1.0f * (fUsed / fSize), cTitle);

#ifdef DEBUG_MEMORY
		float fPos = 4.0f;
		float fMaxSize = 0.0f;

		for (int i = 0; i < ALLOC_TYPE_COUNT; i++)
		{
			fMaxSize = Math::Max(fMaxSize, (float)pHeap->GetAllocated((MemAllocType)i));
		}

		for (int i = 0; i < ALLOC_TYPE_COUNT; i++)
		{
			uint32 uIndex = 0;
			for (int j = 0; j < ALLOC_TYPE_COUNT; j++)
			{
				if (pHeap->GetAllocated((MemAllocType)j) > pHeap->GetAllocated((MemAllocType)i))
				{
					uIndex++;
				}
				else if (pHeap->GetAllocated((MemAllocType)j) == pHeap->GetAllocated((MemAllocType)i) && i > j)
				{
					uIndex++;
				}
			}
			DrawMemoryStat(pRender, pHeap, (MemAllocType)i, fPos + uIndex, fMaxSize);
		}
#endif
	}

	float GlobalDebugStats::DrawMemoryStat(DebugRender* pRenderer, const MemHeap* pHeap, MemAllocType eType, float fPos, float fMaxSize)
	{
#ifdef DEBUG_MEMORY
		Color cBreakDown(0.0f, 1.0f, 0.0f, 1.0f);
		memsize uAllocated = pHeap->GetAllocated(eType);

		if (uAllocated > 10 * 1024)
		{
			U8String str;
			float fMemMB = ((float)uAllocated) / (1024.f * 1024.f);
			str.ParseString("%.2fMB", fMemMB);
			pRenderer->AddString(str.CStr(), 0.0f, fPos, cBreakDown);
			pRenderer->AddString(mem::GetAllocString(eType), 0.125f, fPos, cBreakDown);
			pRenderer->AddBar(fPos, 0.4f, (((float)uAllocated) / fMaxSize) * 0.5f, cBreakDown);
			fPos += 1.0f;
		}
#endif

		return fPos;
	}


}
