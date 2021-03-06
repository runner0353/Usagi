/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include API_HEADER(Engine/Oculus, OculusHMD_ps.h)
#include "Engine/Graphics/Device/Display.h"
#include "Extras/OVR_Math.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Memory/Mem.h"
#include "OculusHMD.h"
#include <SetupAPI.h>
#include "Engine/Core/stl/string.h"

#pragma comment(lib, "SetupAPI.lib")

#if defined(_WIN32)
#include <dxgi.h> // for GetDefaultAdapterLuid
#pragma comment(lib, "dxgi.lib")
#endif

namespace usg
{
	static bool g_sbInitialised = false;

	static int Compare(const ovrGraphicsLuid& lhs, const ovrGraphicsLuid& rhs)
	{
		return memcmp(&lhs, &rhs, sizeof(ovrGraphicsLuid));
	}


	static ovrGraphicsLuid GetDefaultAdapterLuid()
	{
		ovrGraphicsLuid luid = ovrGraphicsLuid();

#if defined(_WIN32)
		IDXGIFactory* factory = nullptr;

		if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory))))
		{
			IDXGIAdapter* adapter = nullptr;

			if (SUCCEEDED(factory->EnumAdapters(0, &adapter)))
			{
				DXGI_ADAPTER_DESC desc;

				adapter->GetDesc(&desc);
				memcpy(&luid, &desc.AdapterLuid, sizeof(luid));
				adapter->Release();
			}

			factory->Release();
		}
#endif

		return luid;
	}

	Matrix4x4 OculusHMD::Convert(const OVR::Matrix4f &in) const
	{
		Matrix4x4 out;

		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				out.M[j][i] = in.M[i][j];

		return out;
	}

	Quaternionf OculusHMD::Convert(const ovrQuatf &in) const
	{
		return Quaternionf(in.x, in.y, in.z, in.w);
	}

	Vector3f OculusHMD::Convert(const ovrVector3f& in) const
	{
		return Vector3f(in.x, in.y, in.z);
	}


	Matrix4x4 OculusHMD::Convert(const ovrPosef& in) const
	{
		Quaternionf qRot = Convert(in.Orientation);
		Vector3f vPos = Convert(in.Position);

		Matrix4x4 mOut = qRot;
		mOut.Translate(vPos.x, vPos.y, vPos.z);

		return mOut;
	}

	static void FlipHandedness(const ovrPosef* inPose, ovrPosef* outPose) {
		outPose->Orientation.x = inPose->Orientation.x;
		outPose->Orientation.y = inPose->Orientation.y;
		outPose->Orientation.z = -inPose->Orientation.z;
		outPose->Orientation.w = -inPose->Orientation.w;

		outPose->Position.x = inPose->Position.x;
		outPose->Position.y = inPose->Position.y;
		outPose->Position.z = -inPose->Position.z;
	}

	OculusHMD* OculusHMD::TryCreate()
	{
		if (!g_sbInitialised)
		{
			ovrInitParams initParams = { ovrInit_RequestVersion, OVR_MINOR_VERSION, NULL, 0, 0 };
			ovrResult result = ovr_Initialize(&initParams);
			if (OVR_SUCCESS(result))
			{
				g_sbInitialised = true;
			}
			else
			{
				return nullptr;
			}
		}
		ovrSession session;
		ovrGraphicsLuid luid;
		ovrResult result = ovr_Create(&session, &luid);
		if (!OVR_SUCCESS(result))
			return nullptr;


		if (Compare(luid, GetDefaultAdapterLuid())) // If luid that the Rift is on is not the default adapter LUID...
		{
			ASSERT_MSG(false, "OpenGL supports only the default graphics adapter.");
			return nullptr;
		}

		// We have an oculus available, so return one!
		return vnew(ALLOC_OBJECT) OculusHMD_ps(session, luid);
	}

#include <initguid.h>
#include <mmdeviceapi.h>
	OculusHMD::OculusHMD(ovrSession session, ovrGraphicsLuid luid)
	{
		m_session = session;
		m_luid = luid;
		m_mirrorTexture = nullptr;
		m_uFrameIndex = 0;
		m_bVisible = true;

		m_hmdTransformMatrix = Matrix4x4::Identity();
		m_eyeTransformMatrix[(uint32)Eye::Left] = Matrix4x4::Identity();
		m_eyeTransformMatrix[(uint32)Eye::Right] = Matrix4x4::Identity();

		for (uint32 i = 0; i < 2; i++)
		{
			m_targets[i].swapChain = nullptr;
		}

		m_hmdDesc = ovr_GetHmdDesc(m_session);

		GUID guid;
		ovrResult result = ovr_GetAudioDeviceOutGuidStr(m_deviceOutStrBuffer);
		result = ovr_GetAudioDeviceOutGuid(&guid);

		usg::wstring deviceIdStr;
		deviceIdStr.reserve(112);
		deviceIdStr.append(L"\\\\?\\SWD#MMDEVAPI#");
		deviceIdStr.append(m_deviceOutStrBuffer);
		deviceIdStr.push_back(L'#');
		size_t offset = deviceIdStr.size();
		deviceIdStr.resize(deviceIdStr.capacity());


		StringFromGUID2(DEVINTERFACE_AUDIO_RENDER, &deviceIdStr[offset], OVR_AUDIO_MAX_DEVICE_STR_SIZE);
		wcscpy_s(m_deviceOutStrBuffer, OVR_AUDIO_MAX_DEVICE_STR_SIZE, deviceIdStr.c_str());


		if (!OVR_SUCCESS(result))
		{
			ovrErrorInfo info;
			ovr_GetLastErrorInfo(&info);
			DEBUG_PRINT("%s", info.ErrorString);
		}

	}

	OculusHMD::~OculusHMD()
	{

	}

	Matrix4x4 OculusHMD::ConvertPose(const ovrPosef &pose) const
	{
		ovrPosef poseTmp;
		FlipHandedness(&pose, &poseTmp);
		return Convert(pose);
	}


	bool OculusHMD::Init(GFXDevice* pDevice)
	{
		// Call an update frame
		m_hmdDesc = ovr_GetHmdDesc(m_session);
		Update();


		return true;
	}

	void OculusHMD::GetRenderTargetDim(Eye eye, float pixelDensity, uint32 &uWidthOut, uint32 &uHeightOut) const
	{
		ovrEyeType eyeType = eye == Eye::Left ? ovrEye_Left : ovrEye_Right;
		ovrSizei idealTextureSize = ovr_GetFovTextureSize(m_session, ovrEyeType(eye), m_hmdDesc.DefaultEyeFov[eyeType], pixelDensity);

		uWidthOut = (uint32)idealTextureSize.w;
		uHeightOut = (uint32)idealTextureSize.h;
	}

	void OculusHMD::Cleanup(GFXDevice* pDevice)
	{
		for (uint32 i = 0; i < 2; i++)
		{
			if (m_targets[i].swapChain)
			{
				ovr_DestroyTextureSwapChain(m_session, m_targets[i].swapChain);
			}
		}

		if (m_mirrorTexture)
		{
			ovr_DestroyMirrorTexture(m_session, m_mirrorTexture);
		}

		ovr_Destroy(m_session);
	}



	void OculusHMD::Update()
	{
		ovr_GetSessionStatus(m_session, &m_sessionStatus);

		if (!m_sessionStatus.HasInputFocus)
		{
			// TODO: Send a message to pause the application
		}

		ovrEyeRenderDesc eyeRenderDesc[2];
		eyeRenderDesc[0] = ovr_GetRenderDesc(m_session, ovrEye_Left, m_hmdDesc.DefaultEyeFov[0]);
		eyeRenderDesc[1] = ovr_GetRenderDesc(m_session, ovrEye_Right, m_hmdDesc.DefaultEyeFov[1]);

		// Get eye poses, feeding in correct IPD offset
		ovrPosef EyeRenderPose[2];
		ovrPosef HeadPose;
		ovrPosef HmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose,
			eyeRenderDesc[1].HmdToEyePose };

		ovr_GetEyePoses(m_session, m_uFrameIndex, ovrTrue, HmdToEyePose, m_eyeRenderPoses, &m_sensorSampleTime);
		//EyeRenderPose[0] = m_eyeRenderPoses[0];
		//EyeRenderPose[1] = m_eyeRenderPoses[1];
		FlipHandedness(&m_eyeRenderPoses[0], &EyeRenderPose[0]);
		FlipHandedness(&m_eyeRenderPoses[1], &EyeRenderPose[1]);

		double frameTime = ovr_GetPredictedDisplayTime(m_session, m_uFrameIndex);
		m_trackingState = ovr_GetTrackingState(m_session, frameTime, ovrTrue);
		FlipHandedness(&m_trackingState.HeadPose.ThePose, &HeadPose);


		// Head
		m_qRotation = Convert(HeadPose.Orientation);

		m_hmdTransformMatrix = Convert(HeadPose);
		m_eyeTransformMatrix[(uint32)Eye::Left] = Convert(EyeRenderPose[0]);
		m_eyeTransformMatrix[(uint32)Eye::Right] = Convert(EyeRenderPose[1]);

	}
	
	void OculusHMD::ResetTracking(bool bPos, bool bOri)
	{
		ovr_RecenterTrackingOrigin(m_session);
	}



	void OculusHMD::SubmitFrame()
	{
		// Do distortion rendering, Present and flush/sync

		// Now commit the swap chain for this eye
		for (int eye = 0; eye < 2; ++eye)
		{
			ovr_CommitTextureSwapChain(m_session, m_targets[(uint32)eye].swapChain);
		}

		ovrLayerEyeFov ld;
		ld.Header = m_layerHeader;

		for (int eye = 0; eye < 2; ++eye)
		{
			ld.ColorTexture[eye] = m_targets[eye].swapChain;
			ld.Viewport[eye].Pos.x = 0; ld.Viewport[eye].Pos.y = 0;
			ld.Viewport[eye].Size.w = m_targets[eye].uWidth;
			ld.Viewport[eye].Size.h = m_targets[eye].uHeight;
			ld.Fov[eye] = m_hmdDesc.DefaultEyeFov[eye];
			ld.RenderPose[eye] = m_eyeRenderPoses[eye];
			ld.SensorSampleTime = m_sensorSampleTime;
		}

		ovrLayerHeader* layers = &ld.Header;
		ovrResult result = ovr_SubmitFrame(m_session, m_uFrameIndex, nullptr, &layers, 1);

		if (!OVR_SUCCESS(result))
		{
			// TODO: We can't render to the headset, we should shut down the application or unload the HMD dll
		}
		
		m_bVisible = result == ovrSuccess;

		if (m_sessionStatus.ShouldQuit)
		{
			// TODO: Message to shut down the application
		}
		if (m_sessionStatus.ShouldRecenter)
		{
			ovr_RecenterTrackingOrigin(m_session);
		}

		m_uFrameIndex++;
	}

	void OculusHMD::GetHMDTransform(usg::Matrix4x4& matOut) const
	{
		matOut = m_hmdTransformMatrix;
	}

	void OculusHMD::GetEyeTransform(Eye eye, usg::Matrix4x4& mMatOut) const
	{
		mMatOut = m_eyeTransformMatrix[(uint32)eye];
	}
}