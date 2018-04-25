/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#ifndef USG_LIGHT_MGR_H
#define USG_LIGHT_MGR_H
#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Core/stl/Vector.h"
#include "Engine/Memory/FastPool.h"
#include "Engine/Graphics/Color.h"

namespace usg {

class Camera;
class Light;
class PointLight;
class DirLight;
class SpotLight;
class ProjectionLight;
class GFXDevice;
class Scene;
class ViewContext;


class LightMgr 
{
public:
	LightMgr(void);
	~LightMgr(void);

	// TODO: Add names to these lights?
	void			Init(GFXDevice* pDevice, Scene* pParent);
	void			CleanUp(GFXDevice* pDevice);
	void			Update(float fDelta, uint32 uFrame);
	void			GPUUpdate(GFXDevice* pDevice);
	void			GlobalShadowRender(GFXContext* pContext, Scene* pScene);
	void			ViewShadowRender(GFXContext* pContext, Scene* pScene, ViewContext* pView);

	DirLight*		AddDirectionalLight(GFXDevice* pDevice, bool bSupportsShadow, const char* szName = NULL);
	void			RemoveDirLight(DirLight* pLight);
	PointLight*		AddPointLight(GFXDevice* pDevice, bool bSupportsShadow, const char* szName = NULL);
	void			RemovePointLight(PointLight* pLight);
	SpotLight*		AddSpotLight(GFXDevice* pDevice, bool bSupportsShadow, const char* szName = NULL);
	void			RemoveSpotLight(SpotLight* pLight);
	ProjectionLight* AddProjectionLight(GFXDevice* pDevice, bool bSupportsShadow, const char* szName = NULL);
	void			RemoveProjectionLight(ProjectionLight* pLight);
	Light*			FindLight(const char* szName);

	void			RemoveLight(const Light* pLight);

	Light*			CreateLight(GFXDevice* pDevice, const struct _LightSpec &spec);
	
	void			GetActiveDirLights(List<DirLight>& lightsOut) const; 
	void			GetPointLightsInView(const Camera* pCamera, List<PointLight>& lightsOut) const;
	void			GetSpotLightsInView(const Camera* pCamera, List<SpotLight>& lightsOut) const;
	void			GetProjectionLightsInView(const Camera* pCamera, List<ProjectionLight>& lightsOut) const;
	const Color&	GetAmbientLight() { return m_ambient; }

	const Color& GetSkyColor() const { return m_skyColor; }
	const Color& GetGroundColor() const { return m_groundColor; }
	const Vector3f& GetHemisphereDir() const { return m_hemisphereDir; }
	float GetHemisphereLerp() const { return m_hemipshereLerp; }
	void SetSkyColor(const Color& color) { m_skyColor = color; }
	void SetGroundColor(const Color& color) { m_groundColor = color; }

	uint32 GetShadowedDirLightCount() const { return m_uShadowedDirLights; }
	uint32 GetShadowedDirLightIndex() const { return m_uShadowedDirLightIndex; }


private:	
	
	template <class LightType>
	class LightInstances
	{
	public:
		LightInstances() {}
		~LightInstances()
		{
			ASSERT(m_freeLights.empty());
		}

		void CleanUp(GFXDevice* pDevice, Scene* pScene)
		{
			ASSERT(m_allocatedLights.empty());
			for (auto&& it : m_freeLights)
			{
				it->CleanUp(pDevice, pScene);
				vdelete it;
				it = nullptr;
			}
			m_freeLights.clear();
			for (auto&& it : m_allocatedLights)
			{
				it->CleanUp(pDevice, pScene);
				vdelete it;
				it = nullptr;
			}
			m_allocatedLights.clear();
		}

		LightType* GetLight(GFXDevice* pDevice, Scene* pScene, bool bSupportsShadow)
		{
			for (usg::vector<LightType*>::reverse_iterator it = m_freeLights.rbegin(); it != m_freeLights.rend(); it++)
			{
				if ((*it)->SupportsShadow() == bSupportsShadow)
				{
					LightType* pLight = (*it);
					m_allocatedLights.push_back(pLight);
					RemoveItem(pLight, m_freeLights);
					return pLight;
				}
			}
			LightType* pReturn = vnew(ALLOC_OBJECT) LightType;
			pReturn->Init(pDevice, pScene, bSupportsShadow);
			m_allocatedLights.push_back(pReturn);
			return pReturn;
		}

		void Free(LightType* pLight)
		{
			RemoveItem(pLight, m_allocatedLights);
			m_freeLights.push_back(pLight);
		}

		void RemoveItem(LightType* pLight, usg::vector<LightType*>& vec)
		{
			for (auto it = vec.begin(); it != vec.end(); ++it)
			{
				if (*it == pLight)
				{
					vec.erase_unsorted(it);
					return;
				}
			}
			ASSERT(false);
		}

		const auto& GetActiveLights() const { return m_allocatedLights; }
	private:
		usg::vector<LightType*>		m_allocatedLights;
		usg::vector<LightType*>		m_freeLights;
	};
	LightInstances<DirLight>		m_dirLights;
	LightInstances<PointLight>		m_pointLights;
	LightInstances<SpotLight>		m_spotLights;
	LightInstances<ProjectionLight>	m_projLights;
	Color							m_ambient;

	Scene*					m_pParent;
	Color					m_skyColor;
	Color					m_groundColor;
	Vector3f				m_hemisphereDir;
	uint32					m_uShadowedDirLights;
	uint32					m_uShadowedDirLightIndex;
	uint32					m_uActiveFrame;
	float					m_hemipshereLerp;
};

} // namespace

#endif // USG_LIGHT_MGR_H