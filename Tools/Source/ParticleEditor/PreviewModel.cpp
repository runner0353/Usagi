#include "Engine/Common/Common.h"
#include "Engine/HID/Gamepad.h"
#include "Engine/HID/Input.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Scene/Model/Bone.h"
#include "PreviewModel.h"

const float g_fTrailSpeed = 20.f;


PreviewModel::~PreviewModel()
{
	if (m_pModel)
	{
		vdelete m_pModel;
	}
}

void PreviewModel::Init(usg::GFXDevice* pDevice, usg::Scene* pScene, usg::IMGuiRenderer* pRenderer)
{
	usg::Vector2f vPos(0.0f, 130.0f);
	usg::Vector2f vScale(340.f, 100.f);
	m_window.Init("Preview Model", vPos, vScale, 20);
	m_visible.Init("Show", true);
	m_visible.SetSameLine(false);

	pRenderer->AddWindow(&m_window);

	m_modelFileList.Init("Models/particle_editor/", ".vmdc");
	m_loadFilePaths.Init("Load Dir", m_modelFileList.GetFileNamesRaw(), 0);

	m_loadButton.Init("Load");
	m_loadButton.SetSameLine(true);
	float fDefault[] = { 0.0f, 0.0f, 0.0f };
	m_position.Init("Position", -30.f, 30.f, fDefault, 3);

	m_window.AddItem(&m_loadFilePaths);
	m_window.AddItem(&m_loadButton);
	m_window.AddItem(&m_position);
	m_window.AddItem(&m_visible);

	m_pScene = pScene;

	//m_pModel = vnew(usg::ALLOC_OBJECT) usg::Model;
	//m_pModel->Load(pDevice, m_pScene, "particle_editor/Grandmaster.vmdc", false, usg::RenderNode::RENDER_MASK_ALL, true, true, NULL, NULL, false);
}


void PreviewModel::Update(usg::GFXDevice* pDevice, float fElapsed)
{
	if(m_loadButton.GetValue())
	{
		usg::U8String modelName = "particle_editor/";
		modelName += m_loadFilePaths.GetSelectedName();
	
		if (m_pModel)
		{
			vdelete m_pModel;
		}

		m_pModel = vnew(usg::ALLOC_OBJECT) usg::Model;
		m_pModel->Load(pDevice, m_pScene, modelName.CStr(), false, usg::RenderNode::RENDER_MASK_ALL, true, false);
	}
	if (m_pModel)
	{
		m_pModel->AddToScene(m_visible.GetValue());
		if (m_visible.GetValue())
		{
			usg::Matrix4x4 mat = usg::Matrix4x4::Identity();
			mat.SetPos(m_position.GetValueV3());
			m_pModel->GetSkeleton().GetBone(0)->SetTransform(mat);
			m_pModel->GetSkeleton().GetBone(0)->UpdateConstants(pDevice);
		}
	}
}
