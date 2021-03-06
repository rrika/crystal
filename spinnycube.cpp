#include <cmath>
#include <cstdio>
#include <functional>
#include <iterator>
#include <memory>

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>

#include "config.h" // for ENABLE_IMGUI
#include "spinnycube.h"
#include "types.h"
#include "mainloop.h" // for buildUnitsUI
#include "drm/DRMIndex.h"
#include "drm/ResolveObject.h"
#include "drm/ResolveReceiver.h"
#include "drm/ResolveSection.h"
#include "filesystem/ArchiveFileSystem.h"
#include "filesystem/FileHelpers.h" // for archiveFileSystem_default
#include "filesystem/FileUserBufferReceiver.h"
#include "gameshell/win32/MainVM.h" // for yellowCursor
#include "input/PCMouseKeyboard.h"
#include "math/Math.h" // for float4x4
#include "object/Object.h"
#include "object/ObjectManager.h" // for buildObjectsUI
#include "rendering/buffers/PCDX11ConstantBufferPool.h"
#include "rendering/buffers/PCDX11IndexBuffer.h"
#include "rendering/buffers/PCDX11SimpleStaticIndexBuffer.h"
#include "rendering/buffers/PCDX11SimpleStaticVertexBuffer.h"
#include "rendering/buffers/PCDX11UberConstantBuffer.h"
#include "rendering/IPCDeviceManager.h"
#include "rendering/IRenderPassCallback.h"
#include "rendering/PCDX11DeviceManager.h"
#include "rendering/PCDX11MatrixState.h"
#include "rendering/PCDX11RenderContext.h"
#include "rendering/PCDX11RenderDevice.h"
#include "rendering/PCDX11RenderModel.h"
#include "rendering/PCDX11RenderModelInstance.h"
#include "rendering/PCDX11Scene.h"
#include "rendering/PCDX11StateManager.h"
#include "rendering/PCDX11StreamDecl.h"
#include "rendering/RenderModelInstance.h"
#include "rendering/RenderPasses.h"
#include "rendering/shaders/PCDX11PixelShader.h"
#include "rendering/shaders/PCDX11VertexShader.h"
#include "rendering/surfaces/PCDX11DefaultRenderTarget.h"
#include "rendering/surfaces/PCDX11DepthBuffer.h"
#include "rendering/surfaces/PCDX11Texture.h"
#include "rendering/VertexAttribute.h"

#if ENABLE_IMGUI
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include "rendering/Inspector.h"
#endif

float VertexData[] = // float4 position, float3 normal, float2 texcoord, float3 color
{
	-1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	-0.6f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f,  2.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	 0.6f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f,  8.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f, 10.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	-0.6f,  0.6f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f,  2.0f,  2.0f,  0.973f,  0.480f,  0.002f,
	 0.6f,  0.6f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f,  8.0f,  2.0f,  0.973f,  0.480f,  0.002f,
	-0.6f, -0.6f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f,  2.0f,  8.0f,  0.973f,  0.480f,  0.002f,
	 0.6f, -0.6f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f,  8.0f,  8.0f,  0.973f,  0.480f,  0.002f,
	-1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 10.0f,  0.973f,  0.480f,  0.002f,
	-0.6f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f,  2.0f, 10.0f,  0.973f,  0.480f,  0.002f,
	 0.6f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f,  8.0f, 10.0f,  0.973f,  0.480f,  0.002f,
	 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f, 10.0f, 10.0f,  0.973f,  0.480f,  0.002f,
	 1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 1.0f,  1.0f, -0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  2.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 1.0f,  1.0f,  0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  8.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 10.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 1.0f,  0.6f, -0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  2.0f,  2.0f,  0.897f,  0.163f,  0.011f,
	 1.0f,  0.6f,  0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  8.0f,  2.0f,  0.897f,  0.163f,  0.011f,
	 1.0f, -0.6f, -0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  2.0f,  8.0f,  0.897f,  0.163f,  0.011f,
	 1.0f, -0.6f,  0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  8.0f,  8.0f,  0.897f,  0.163f,  0.011f,
	 1.0f, -1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 10.0f,  0.897f,  0.163f,  0.011f,
	 1.0f, -1.0f, -0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  2.0f, 10.0f,  0.897f,  0.163f,  0.011f,
	 1.0f, -1.0f,  0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  8.0f, 10.0f,  0.897f,  0.163f,  0.011f,
	 1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 10.0f, 10.0f,  0.897f,  0.163f,  0.011f,
	 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	 0.6f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  2.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	-0.6f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  8.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	-1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 10.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	 0.6f,  0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  2.0f,  2.0f,  0.612f,  0.000f,  0.069f,
	-0.6f,  0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  8.0f,  2.0f,  0.612f,  0.000f,  0.069f,
	 0.6f, -0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  2.0f,  8.0f,  0.612f,  0.000f,  0.069f,
	-0.6f, -0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  8.0f,  8.0f,  0.612f,  0.000f,  0.069f,
	 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 10.0f,  0.612f,  0.000f,  0.069f,
	 0.6f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  2.0f, 10.0f,  0.612f,  0.000f,  0.069f,
	-0.6f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  8.0f, 10.0f,  0.612f,  0.000f,  0.069f,
	-1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 10.0f, 10.0f,  0.612f,  0.000f,  0.069f,
	-1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-1.0f,  1.0f,  0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  2.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-1.0f,  1.0f, -0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  8.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-1.0f,  1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 10.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-1.0f,  0.6f,  0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  2.0f,  2.0f,  0.127f,  0.116f,  0.408f,
	-1.0f,  0.6f, -0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  8.0f,  2.0f,  0.127f,  0.116f,  0.408f,
	-1.0f, -0.6f,  0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  2.0f,  8.0f,  0.127f,  0.116f,  0.408f,
	-1.0f, -0.6f, -0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  8.0f,  8.0f,  0.127f,  0.116f,  0.408f,
	-1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 10.0f,  0.127f,  0.116f,  0.408f,
	-1.0f, -1.0f,  0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  2.0f, 10.0f,  0.127f,  0.116f,  0.408f,
	-1.0f, -1.0f, -0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  8.0f, 10.0f,  0.127f,  0.116f,  0.408f,
	-1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 10.0f, 10.0f,  0.127f,  0.116f,  0.408f,
	-1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	-0.6f,  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  2.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	 0.6f,  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  8.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 10.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	-0.6f,  1.0f,  0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  2.0f,  2.0f,  0.000f,  0.254f,  0.637f,
	 0.6f,  1.0f,  0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  8.0f,  2.0f,  0.000f,  0.254f,  0.637f,
	-0.6f,  1.0f, -0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  2.0f,  8.0f,  0.000f,  0.254f,  0.637f,
	 0.6f,  1.0f, -0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  8.0f,  8.0f,  0.000f,  0.254f,  0.637f,
	-1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 10.0f,  0.000f,  0.254f,  0.637f,
	-0.6f,  1.0f, -1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  2.0f, 10.0f,  0.000f,  0.254f,  0.637f,
	 0.6f,  1.0f, -1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  8.0f, 10.0f,  0.000f,  0.254f,  0.637f,
	 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 10.0f, 10.0f,  0.000f,  0.254f,  0.637f,
	-1.0f, -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	-0.6f, -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  2.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	 0.6f, -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  8.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	 1.0f, -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 10.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	-0.6f, -1.0f, -0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  2.0f,  2.0f,  0.001f,  0.447f,  0.067f,
	 0.6f, -1.0f, -0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  8.0f,  2.0f,  0.001f,  0.447f,  0.067f,
	-0.6f, -1.0f,  0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  2.0f,  8.0f,  0.001f,  0.447f,  0.067f,
	 0.6f, -1.0f,  0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  8.0f,  8.0f,  0.001f,  0.447f,  0.067f,
	-1.0f, -1.0f,  1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 10.0f,  0.001f,  0.447f,  0.067f,
	-0.6f, -1.0f,  1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  2.0f, 10.0f,  0.001f,  0.447f,  0.067f,
	 0.6f, -1.0f,  1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  8.0f, 10.0f,  0.001f,  0.447f,  0.067f,
	 1.0f, -1.0f,  1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 10.0f, 10.0f,  0.001f,  0.447f,  0.067f,
	-0.6f,  0.6f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	-0.6f,  0.6f, -0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	-0.6f, -0.6f, -0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	-0.6f, -0.6f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	 0.6f,  0.6f, -0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	 0.6f,  0.6f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	 0.6f, -0.6f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	 0.6f, -0.6f, -0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	-0.6f, -0.6f, -1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	-0.6f, -0.6f, -0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	 0.6f, -0.6f, -0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	 0.6f, -0.6f, -1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	-0.6f,  0.6f, -0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	-0.6f,  0.6f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	 0.6f,  0.6f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	 0.6f,  0.6f, -0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.973f,  0.480f,  0.002f,
	 1.0f,  0.6f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 0.6f,  0.6f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 0.6f, -0.6f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 1.0f, -0.6f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 0.6f,  0.6f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 1.0f,  0.6f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 1.0f, -0.6f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 0.6f, -0.6f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 1.0f,  0.6f,  0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 0.6f,  0.6f,  0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 0.6f,  0.6f, -0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 1.0f,  0.6f, -0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 0.6f, -0.6f,  0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 1.0f, -0.6f,  0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 1.0f, -0.6f, -0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 0.6f, -0.6f, -0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.897f,  0.163f,  0.011f,
	 0.6f,  0.6f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	 0.6f,  0.6f,  0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	 0.6f, -0.6f,  0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	 0.6f, -0.6f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	-0.6f,  0.6f,  0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	-0.6f,  0.6f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	-0.6f, -0.6f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	-0.6f, -0.6f,  0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	 0.6f, -0.6f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	 0.6f, -0.6f,  0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	-0.6f, -0.6f,  0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	-0.6f, -0.6f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	 0.6f,  0.6f,  0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	 0.6f,  0.6f,  1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	-0.6f,  0.6f,  1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	-0.6f,  0.6f,  0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.612f,  0.000f,  0.069f,
	-1.0f,  0.6f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-0.6f,  0.6f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-0.6f, -0.6f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-1.0f, -0.6f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-0.6f,  0.6f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-1.0f,  0.6f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-1.0f, -0.6f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-0.6f, -0.6f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-1.0f, -0.6f,  0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-0.6f, -0.6f,  0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-0.6f, -0.6f, -0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-1.0f, -0.6f, -0.6f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-0.6f,  0.6f,  0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-1.0f,  0.6f,  0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-1.0f,  0.6f, -0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-0.6f,  0.6f, -0.6f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.127f,  0.116f,  0.408f,
	-0.6f,  1.0f,  0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	-0.6f,  0.6f,  0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	-0.6f,  0.6f, -0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	-0.6f,  1.0f, -0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	 0.6f,  0.6f,  0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	 0.6f,  1.0f,  0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	 0.6f,  1.0f, -0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	 0.6f,  0.6f, -0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	-0.6f,  1.0f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	-0.6f,  0.6f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	 0.6f,  0.6f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	 0.6f,  1.0f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	-0.6f,  0.6f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	-0.6f,  1.0f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	 0.6f,  1.0f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	 0.6f,  0.6f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.000f,  0.254f,  0.637f,
	-0.6f, -0.6f,  0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	-0.6f, -1.0f,  0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	-0.6f, -1.0f, -0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	-0.6f, -0.6f, -0.6f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	 0.6f, -1.0f,  0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	 0.6f, -0.6f,  0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	 0.6f, -0.6f, -0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	 0.6f, -1.0f, -0.6f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	-0.6f, -0.6f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	-0.6f, -1.0f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	 0.6f, -1.0f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	 0.6f, -0.6f, -0.6f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	-0.6f, -1.0f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	-0.6f, -0.6f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	 0.6f, -0.6f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
	 0.6f, -1.0f,  0.6f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.001f,  0.447f,  0.067f,
};

unsigned short IndexData[] =
{
	  0,   1,   9,   9,   8,   0,   1,   2,   5,   5,   4,   1,   6,   7,  10,  10,   9,   6,   2,   3,  11,  11,  10,   2,
	 12,  13,  21,  21,  20,  12,  13,  14,  17,  17,  16,  13,  18,  19,  22,  22,  21,  18,  14,  15,  23,  23,  22,  14,
	 24,  25,  33,  33,  32,  24,  25,  26,  29,  29,  28,  25,  30,  31,  34,  34,  33,  30,  26,  27,  35,  35,  34,  26,
	 36,  37,  45,  45,  44,  36,  37,  38,  41,  41,  40,  37,  42,  43,  46,  46,  45,  42,  38,  39,  47,  47,  46,  38,
	 48,  49,  57,  57,  56,  48,  49,  50,  53,  53,  52,  49,  54,  55,  58,  58,  57,  54,  50,  51,  59,  59,  58,  50,
	 60,  61,  69,  69,  68,  60,  61,  62,  65,  65,  64,  61,  66,  67,  70,  70,  69,  66,  62,  63,  71,  71,  70,  62,
	 72,  73,  74,  74,  75,  72,  76,  77,  78,  78,  79,  76,  80,  81,  82,  82,  83,  80,  84,  85,  86,  86,  87,  84,
	 88,  89,  90,  90,  91,  88,  92,  93,  94,  94,  95,  92,  96,  97,  98,  98,  99,  96, 100, 101, 102, 102, 103, 100,
	104, 105, 106, 106, 107, 104, 108, 109, 110, 110, 111, 108, 112, 113, 114, 114, 115, 112, 116, 117, 118, 118, 119, 116,
	120, 121, 122, 122, 123, 120, 124, 125, 126, 126, 127, 124, 128, 129, 130, 130, 131, 128, 132, 133, 134, 134, 135, 132,
	136, 137, 138, 138, 139, 136, 140, 141, 142, 142, 143, 140, 144, 145, 146, 146, 147, 144, 148, 149, 150, 150, 151, 148,
	152, 153, 154, 154, 155, 152, 156, 157, 158, 158, 159, 156, 160, 161, 162, 162, 163, 160, 164, 165, 166, 166, 167, 164,
};

const char shaders [] = (
	"cbuffer WorldBuffer : register(b0) { float4x4 WorldViewProject; float4x4 World; float4x4 ViewProject; }\n"
	"cbuffer SceneBuffer : register(b2) { float4x4 View; }\n"
	"struct vs_in { float4 position : POSITION; float3 normal : NORMAL; float2 texcoord : TEXCOORD1; float3 color : COLOR; };\n"
	"struct vs_out { float4 position : SV_POSITION; float4 color : COLOR; float2 texcoord : TEXCOORD; };\n"
	"Texture2D    mytexture : register(t0);\n"
	"SamplerState mysampler : register(s0);\n"
	"static const float3 lightvector = { 1.0f, -1.0f, 1.0f };\n"
	"vs_out vs_main(vs_in input) {\n"
	"    float light = clamp(dot(normalize(mul(World, float4(input.normal, 0.0f)).xyz), normalize(-lightvector)), 0.0f, 1.0f) * 0.8f + 0.2f;\n"
	"    vs_out output;\n"
	"    output.position = mul(WorldViewProject, input.position);\n"
	"    output.texcoord = input.texcoord;\n"
	"    output.color = float4(input.color * light, 1.0f);\n"
	"    return output;\n"
	"}\n"
	"float4 ps_main(vs_out input) : SV_TARGET { return mytexture.Sample(mysampler, input.texcoord) * input.color; }\n"
);

struct float3 { float x, y, z; };

class SpinnyCubeDrawable : public cdc::IRenderDrawable {
public:
	cdc::PCDX11RenderDevice *renderDevice;
	cdc::PCDX11StateManager *stateManager;
	// cdc::PCDX11ConstantBuffer *cdcConstantBuffer;
	cdc::PCDX11VertexShader *cdcVertexShader;
	cdc::PCDX11PixelShader *cdcPixelShader;
	cdc::PCDX11IndexBuffer *cdcIndexBuffer;
	cdc::PCDX11StreamDecl *streamDecl;
	cdc::PCDX11VertexBuffer *cdcVertexBuffer;
	cdc::PCDX11Texture *texture;
	void draw(uint32_t funcSetIndex, IRenderDrawable *other) override;
	uint32_t compare(uint32_t funcSetIndex, IRenderDrawable *other) override { /*TODO*/ return 0; };
};

class ImGuiDrawable : public cdc::IRenderDrawable {
public:
	std::function<void()> lastMinuteAdditions;

	void draw(uint32_t funcSetIndex, IRenderDrawable *other) override;
	uint32_t compare(uint32_t funcSetIndex, IRenderDrawable *other) override { /*TODO*/ return 0; };
};

class SpinnyCubePass : public cdc::IRenderPassCallback {
public:
	D3D11_VIEWPORT *viewport;
	ID3D11RasterizerState1* rasterizerState;
	ID3D11BlendState1 *keepAlphaBlend;

	bool pre(
		cdc::CommonRenderDevice *renderDevice,
		uint32_t passId,
		uint32_t drawableCount,
		uint32_t priorPassesBitfield) override;
	void post(
		cdc::CommonRenderDevice *renderDevice,
		uint32_t passId) override;
};

DRMIndex drmIndex;

ResolveObject *requestDRM(
	const char *path,
	void **rootPtr = nullptr,
	void (*callback)(void*, void*, void*, ResolveObject*) = nullptr,
	void *callbackArg1 = nullptr,
	void *callbackArg2 = nullptr
) {
	printf("loading %s\n", path);

	ResolveObject *ro = new ResolveObject(path);
	auto *rr = new ResolveReceiver(callback, callbackArg1, callbackArg2, rootPtr, nullptr, nullptr, ro, 0, &drmIndex);
	FileRequest *req = archiveFileSystem_default->createRequest(rr, path, 0);
	req->submit(3);
	req->decrRefCount();
	archiveFileSystem_default->processAll();
	// req is owned by fs which takes care of it in processAll()
	// rr self-deletes

	return ro;
}

int spinnyCube(HWND window,
	ID3D11Device *baseDevice,
	ID3D11DeviceContext *baseDeviceContext) {

	std::unique_ptr<cdc::PCMouseKeyboard> mouseKeyboard(cdc::PCMouseKeyboard::create(window));
	auto renderDevice = static_cast<cdc::PCDX11RenderDevice*>(cdc::g_renderDevice);

	///////////////////////////////////////////////////////////////////////////////////////////////

	ID3D11Device1* device;

	baseDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&device));

	ID3D11DeviceContext1* deviceContext;

	baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&deviceContext));

	///////////////////////////////////////////////////////////////////////////////////////////////

	cdc::PCDX11RenderContext *renderContext = renderDevice->getRenderContextAny();
	renderContext->internalCreate();

	///////////////////////////////////////////////////////////////////////////////////////////////

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	renderContext->frameBuffer->GetDesc(&depthBufferDesc); // base on framebuffer properties

	{
		cdc::PCDX11RenderTarget& cdcRenderTarget = *renderContext->renderTarget2C;
		cdc::PCDX11DepthBuffer& cdcDepthBuffer = *renderContext->depthBuffer;
		cdcDepthBuffer.renderTexture.sampleCount = depthBufferDesc.SampleDesc.Count;
		cdcDepthBuffer.renderTexture.sampleQuality = depthBufferDesc.SampleDesc.Quality;

		cdcRenderTarget.getRenderTexture11()->createRenderTargetView();
		cdcDepthBuffer.renderTexture.createDepthStencilView();

		// do not keep the cdcRenderTarget and cdcDepthBuffer references around
		// these objects get destroyed on window resize
	}

	///////////////////////////////////////////////////////////////////////////////////////////////

#if ENABLE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(baseDevice, baseDeviceContext);
#endif

	///////////////////////////////////////////////////////////////////////////////////////////////

	ID3DBlob* vsBlob;

	D3DCompile(shaders, sizeof(shaders), "shaders.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, nullptr);

	cdc::PCDX11VertexShader cdcOwnVertexShader(
		(char*)vsBlob->GetBufferPointer(),
		/*takeCopy=*/false,
		/*isWrapped=*/false);
	cdcOwnVertexShader.requestShader();
	cdcOwnVertexShader.awaitResource();

	cdc::PCDX11VertexShader& cdcScavengedVertexShader = *renderDevice->shtab_vs_wvp_1_0.vertexShaders[0];
	cdcScavengedVertexShader.requestShader();
	cdcScavengedVertexShader.awaitResource();

	cdc::PCDX11VertexShader& cdcVertexShader = false
		? cdcScavengedVertexShader
		: cdcOwnVertexShader;

	uint32_t numAttr = 4;
	auto *layout = (cdc::VertexAttributeLayoutA*)new char[16 + 8 * numAttr];
	layout->numAttr = numAttr;
	layout->attrib[0] = {0xD2F7D823,  0, 3}; // position,  offset  0, DXGI_FORMAT_R32G32B32A32_FLOAT
	layout->attrib[1] = {0x36F5E414, 16, 2}; // normal,    offset 16, DXGI_FORMAT_R32G32B32_FLOAT
	layout->attrib[2] = {0x8317902A, 28, 1}; // texcoord1, offset 28, DXGI_FORMAT_R32G32_FLOAT
	layout->attrib[3] = {0XFFFFFFFF, 36, 2}; // color,     offset 36, DXGI_FORMAT_R32G32B32_FLOAT

	auto *inputElementDesc = new D3D11_INPUT_ELEMENT_DESC[layout->numAttr];
	memset(inputElementDesc, 0, sizeof(D3D11_INPUT_ELEMENT_DESC[layout->numAttr]));
	decodeVertexAttribA(inputElementDesc, layout->attrib, layout->numAttr, cdcVertexShader.m_sub.wineWorkaround);
	delete[] (char*)layout;

	cdc::PCDX11StreamDecl streamDecl(
		renderDevice, inputElementDesc, numAttr, &cdcVertexShader.m_sub);
	streamDecl.internalCreate();

	///////////////////////////////////////////////////////////////////////////////////////////////

	ID3DBlob* psBlob;

	D3DCompile(shaders, sizeof(shaders), "shaders.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, nullptr);

	cdc::PCDX11PixelShader cdcPixelShader(
		(char*)psBlob->GetBufferPointer(),
		/*takeCopy=*/false,
		/*isWrapped=*/false);

	///////////////////////////////////////////////////////////////////////////////////////////////

	D3D11_RASTERIZER_DESC1 rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;

	ID3D11RasterizerState1* rasterizerState;

	device->CreateRasterizerState1(&rasterizerDesc, &rasterizerState);

	ID3D11BlendState1* keepAlphaBlend = NULL;

	D3D11_BLEND_DESC1 BlendState;
	ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC1));
	BlendState.AlphaToCoverageEnable = false;
	BlendState.IndependentBlendEnable = false;
	BlendState.RenderTarget[0].BlendEnable = true;
	BlendState.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	BlendState.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	BlendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO; // ignore shader
	BlendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE; // keep RT value
	BlendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState1(&BlendState, &keepAlphaBlend);

	///////////////////////////////////////////////////////////////////////////////////////////////

	UINT stride = 12 * 4; // vertex size (11 floats: float4 position, float3 normal, float2 texcoord, float3 color)
	cdc::PCDX11SimpleStaticVertexBuffer cdcVertexBuffer(stride, sizeof(VertexData) / stride, (void*)VertexData);

	///////////////////////////////////////////////////////////////////////////////////////////////

	cdc::PCDX11StateManager stateManager(deviceContext, device);
	stateManager.internalCreate();
	cdc::PCDX11SimpleStaticIndexBuffer cdcIndexBuffer(sizeof(IndexData)/2, IndexData);
	cdc::deviceManager->stateManager = &stateManager; // hack

	auto bottleIndex = objectIdByName("alc_beer_bottle_a");
	requestObject3(bottleIndex);

	auto obj3 = ResolveObject::create(
		"pc-w\\scenario_database.drm",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		0,
		3
	);

	archiveFileSystem_default->processAll();

	ResolveSection *objectSection = g_resolveSections[11];
	ObjectBlob *bottleObject = (ObjectBlob*)objectSection->getWrapped(objectSection->getDomainId(0x04a8));
	printf("have bottle object: %p\n", bottleObject);

	auto bottleTexture = (cdc::PCDX11Texture*)g_resolveSections[5]->getWrapped(0x0396);
	printf("have bottle cdc texture: %p\n", bottleTexture);
	bottleTexture->asyncCreate();
	printf("have bottle d3d texture: %p\n", bottleTexture->d3dTexture128);

	// create the other four textures
	((cdc::PCDX11Texture*)g_resolveSections[5]->getWrapped(0x0395))->asyncCreate();
	((cdc::PCDX11Texture*)g_resolveSections[5]->getWrapped(0x005b))->asyncCreate();
	((cdc::PCDX11Texture*)g_resolveSections[5]->getWrapped(0x0061))->asyncCreate();

	auto bottleRenderModel_direct = (cdc::PCDX11RenderModel*)g_resolveSections[12]->getWrapped(0xA301);
	auto bottleRenderModel = (cdc::PCDX11RenderModel*)bottleObject->models[0]->renderMesh;


	printf("have bottle cdc render model: %p (directly)\n", bottleRenderModel_direct);
	printf("have bottle cdc render model: %p (via object)\n", bottleRenderModel);
	printf("have bottle cdc mesh blob: %p\n", bottleRenderModel->getMesh());

	for (uint32_t i = 0; i < bottleRenderModel->count0; i++)
		printf("  bottle->tab0Ext128Byte[i].material = %p\n", bottleRenderModel->tab0Ext128Byte[i].material);

	cdc::RenderModelInstance *bottleRenderModelInstance =
		renderDevice->createRenderModelInstance(bottleRenderModel);

	///////////////////////////////////////////////////////////////////////////////////////////////

	D3D11_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(depthBufferDesc.Width), static_cast<float>(depthBufferDesc.Height), 0.0f, 1.0f };

	///////////////////////////////////////////////////////////////////////////////////////////////

	float w = viewport.Width / viewport.Height; // width (aspect ratio)
	float h = 1.0f;                             // height
	float n = 1.0f;                             // near
	float f = 9.0f;                             // far

	float scale = 0.05f;
	float3 modelRotation    = { 0.0f, 0.0f, 0.0f };
	float3 modelScale       = { scale, scale, scale };
	float3 modelTranslation = { 0.0f, 0.0f, 4.0f };

	///////////////////////////////////////////////////////////////////////////////////////////////

	cdc::RenderViewport renderViewport;
	renderViewport.mask = 0x3103; // pass 0, 12. 13, 1, and 8
	// pass 12 normals (function set 10, draw bottle normals)
	// pass 13 deferred shading (just contains a cleardrawable)
	// pass 1 composite (draw bottle textures)
	// pass 8 runs last and is where I put imgui since it messes with the render state

	SpinnyCubePass cubePass;
	cubePass.viewport = &viewport;
	cubePass.rasterizerState = rasterizerState;
	cubePass.keepAlphaBlend = keepAlphaBlend;
	renderDevice->setPassCallback(0, &cubePass);

	SpinnyCubeDrawable cubeDrawable;
	cubeDrawable.renderDevice = renderDevice;
	cubeDrawable.stateManager = &stateManager;
	// cubeDrawable.cdcConstantBuffer = &cdcConstantBuffer;
	cubeDrawable.cdcVertexShader = &cdcVertexShader;
	cubeDrawable.cdcPixelShader = &cdcPixelShader;
	cubeDrawable.cdcIndexBuffer = &cdcIndexBuffer;
	cubeDrawable.streamDecl = &streamDecl;
	cubeDrawable.cdcVertexBuffer = &cdcVertexBuffer;
	cubeDrawable.texture = bottleTexture;

	ImGuiDrawable imGuiDrawable;

	///////////////////////////////////////////////////////////////////////////////////////////////

#if ENABLE_IMGUI
	bool showDrawablesWindow = false;
	bool showFilesystemWindow = false;
	bool showObjectsWindow = false;
	bool showDRMWindow = false;
	bool showUnitsWindow = false;
	std::vector<std::pair<void*, CommonScene*>> captures { { nullptr, nullptr } };
	uint32_t selectedCapture = 0;
#endif

	while (true)
	{
		MSG msg;

		while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			if (msg.message == WM_DESTROY) {
				PostQuitMessage(0);
				goto end;
			}
			if (msg.message == WM_QUIT) {
				goto end;
			}

			mouseKeyboard->processWndProc(msg.message, msg.wParam, msg.lParam);

			//if (msg.message == WM_KEYDOWN) return 0;
			DispatchMessageA(&msg);
		}

#if ENABLE_IMGUI
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame(); // this will reset our pretty cursor
		ImGui::NewFrame();
		SetCursor((HCURSOR)yellowCursor); // ahh, much better
#endif

		// cubePass.viewport ->
		viewport = {
			0.0f, 0.0f,
			static_cast<float>(renderContext->width),
			static_cast<float>(renderContext->height),
			0.0f, 1.0f };
		float w = viewport.Width / viewport.Height; // width (aspect ratio)

		///////////////////////////////////////////////////////////////////////////////////////////

		float4x4 rotateX   = { 1, 0, 0, 0, 0, static_cast<float>(cos(modelRotation.x)), -static_cast<float>(sin(modelRotation.x)), 0, 0, static_cast<float>(sin(modelRotation.x)), static_cast<float>(cos(modelRotation.x)), 0, 0, 0, 0, 1 };
		float4x4 rotateY   = { static_cast<float>(cos(modelRotation.y)), 0, static_cast<float>(sin(modelRotation.y)), 0, 0, 1, 0, 0, -static_cast<float>(sin(modelRotation.y)), 0, static_cast<float>(cos(modelRotation.y)), 0, 0, 0, 0, 1 };
		float4x4 rotateZ   = { static_cast<float>(cos(modelRotation.z)), -static_cast<float>(sin(modelRotation.z)), 0, 0, static_cast<float>(sin(modelRotation.z)), static_cast<float>(cos(modelRotation.z)), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		float4x4 scale     = { modelScale.x, 0, 0, 0, 0, modelScale.y, 0, 0, 0, 0, modelScale.z, 0, 0, 0, 0, 1 };
		float4x4 translate = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, modelTranslation.x, modelTranslation.y, modelTranslation.z, 1 };

		modelRotation.x += 0.005f;
		modelRotation.y += 0.009f;
		modelRotation.z += 0.001f;

		///////////////////////////////////////////////////////////////////////////////////////////

		float4x4 world = rotateZ * rotateY * rotateX;
		float4x4 project = { 2 * n / w, 0, 0, 0, 0, 2 * n / h, 0, 0, 0, 0, f / (f - n), 1, 0, 0, n * f / (n - f), 0 };

		// Constants constants;
		// constants.WorldViewProject = project * world;
		// constants.World = world;
		// constants.ViewProject = project;

		stateManager.setWorldMatrix(world);

		// memcpy(cdcConstantBuffer.data, &constants, sizeof(Constants));
		// cdcConstantBuffer.syncBuffer(deviceContext);

		renderDevice->resetRenderLists();
		renderDevice->beginRenderList(nullptr);
		auto *scene = renderDevice->createSubScene(
			&renderViewport,
			renderContext->renderTarget2C,
			renderContext->depthBuffer);
		scene->viewMatrix = translate;
		scene->projectMatrix = project;

		PCDX11MatrixState matrixState(renderDevice);
		matrixState.resize(1);
		auto *bottleWorldMatrix = reinterpret_cast<float4x4*>(matrixState.poseData->getMatrix(0));
		*bottleWorldMatrix = scale * world;

		// add drawables to the scene
		float backgroundColor[4] = {0.025f, 0.025f, 0.025f, 1.0f};
		// float lightAccumulation[4] = {0.9f, 0.9f, 0.9f, 1.0f};
		float lightAccumulation[4] = {0.5f, 0.5f, 0.5f, 0.0f};
		renderDevice->clearRenderTarget(10, /*mask=*/ 1, 0.0f, backgroundColor, 1.0f, 0);
		renderDevice->clearRenderTarget(2, /*mask=*/ 0x2000, 0.0f, lightAccumulation, 1.0f, 0); // deferred shading buffer
		renderDevice->recordDrawable(&cubeDrawable, /*mask=*/ 1, /*addToParent=*/ 0);
		static_cast<cdc::PCDX11RenderModelInstance*>(bottleRenderModelInstance)->baseMask = 0x1002; // normals & composite
		bottleRenderModelInstance->recordDrawables(&matrixState);
		renderDevice->recordDrawable(&imGuiDrawable, /*mask=*/ 0x100, /*addToParent=*/ 0);

		renderDevice->finishScene();
		renderDevice->endRenderList();

#if ENABLE_IMGUI
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Windows")) {
				if (ImGui::MenuItem("Capture frame")) {

					selectedCapture = captures.size();
					captures.push_back({renderDevice->captureRenderLists(), scene});
				}
				if (ImGui::MenuItem("Show drawables")) { showDrawablesWindow = true; }
				// if (ImGui::MenuItem("Show filesystem")) { showFilesystemWindow = true; }
				if (ImGui::MenuItem("Show objects")) { showObjectsWindow = true; }
				if (ImGui::MenuItem("Show DRMs")) { showDRMWindow = true; }
				if (ImGui::MenuItem("Show units")) { showUnitsWindow = true; }
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		imGuiDrawable.lastMinuteAdditions = [&]() {
			if (showDrawablesWindow) {
				CommonScene *xscene = captures[selectedCapture].second;
				if (!xscene)
					xscene = scene;
				ImGui::Begin("Scene drawables", &showDrawablesWindow);
				ImGui::BeginChild("capture list", ImVec2(0, 150), true);
				for (uint32_t i = 0; i < captures.size(); i++) {
					ImGui::PushID(i);
					const char *name = i ? "capture" : "live";
					if (ImGui::Selectable(name, i == selectedCapture)) {
						selectedCapture = i;
						renderDevice->revisitRenderLists(captures[i].first);
					}
					ImGui::PopID();
				}
				ImGui::EndChild();

				ImGui::Text("Note: 'Capture frame' is in main menu for now");
				// doesn't work within lastMinuteAdditions
				// if (ImGui::Button("Capture frame")) {
				// 	selectedCapture = captures.size();
				// 	captures.push_back({renderDevice->captureRenderLists(), scene});
				// }

				// buildUI(xscene->drawableListsAndMasks);
				buildUI(&renderDevice->renderPasses, xscene->drawableListsAndMasks);
				buildUI(xscene);
				ImGui::End();
			}
		};
		if (showFilesystemWindow) {
			// TODO
		}
		if (showObjectsWindow) {
			ImGui::Begin("Objects", &showObjectsWindow);
			buildObjectsUI();
			ImGui::End();
		}
		if (showDRMWindow) {
			ImGui::Begin("DRMs", &showDRMWindow);
			for (auto& entry : drmIndex.sectionHeaders) {
				if (ImGui::TreeNode(entry.first.c_str())) {
					uint32_t i=0;
					for (auto& section : entry.second) {
						const char *names[] = {
							"Generic",
							"Empty",
							"Animation",
							"",
							"",
							"RenderResource",
							"FMODSoundBank",
							"DTPData",
							"Script",
							"ShaderLib",
							"Material",
							"Object",
							"RenderMesh",
							"CollisionMesh",
							"StreamGroupList",
							"AnyType",
						};
						ImGui::Text("%3d: %04x %s unk6:%x (%d bytes)",
							i++, section.id, names[section.type], section.unknown06, section.payloadSize);
						if (section.type == 5) {
							ImGui::Text("    ");
							ImGui::SameLine();
							auto *resource = (cdc::RenderResource*)g_resolveSections[5]->getWrapped(section.id);
							if (auto tex = dynamic_cast<cdc::PCDX11Texture*>(resource)) {
								ImGui::Image(
									tex->createShaderResourceView(), ImVec2(256, 256));
							}
						}
					}
					ImGui::TreePop();
				}
			}
			ImGui::End();
		}
		if (showUnitsWindow) {
			ImGui::Begin("Units", &showUnitsWindow);
			buildUnitsUI();
			ImGui::End();
		}
#endif

		renderDevice->drawRenderLists();

		///////////////////////////////////////////////////////////////////////////////////////////

		renderContext->present();
	}
end:
#if ENABLE_IMGUI 
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
	return 0;
}

bool SpinnyCubePass::pre(
	cdc::CommonRenderDevice *renderDevice,
	uint32_t passId,
	uint32_t drawableCount,
	uint32_t priorPassesBitfield)
{
	auto *deviceContext = static_cast<cdc::PCDX11RenderDevice*>(cdc::g_renderDevice)->getD3DDeviceContext();

	deviceContext->RSSetViewports(1, viewport);
	deviceContext->RSSetState(rasterizerState);

	cdc::deviceManager->getStateManager()->setDepthState(D3D11_COMPARISON_LESS, true);
	deviceContext->OMSetBlendState(keepAlphaBlend, nullptr, 0xffffffff);

	return true;
}

void SpinnyCubePass::post(
	cdc::CommonRenderDevice *renderDevice,
	uint32_t passId)
{
	// empty
}

void SpinnyCubeDrawable::draw(uint32_t funcSetIndex, IRenderDrawable *other) {

	stateManager->setTextureAndSampler(0, texture, 0, 0.0f);

	stateManager->updateMatrices();
	stateManager->updateConstantBuffers();

	stateManager->setVertexShader(cdcVertexShader);
	// stateManager->setVsConstantBuffer(0, cdcConstantBuffer);
	stateManager->setCommonConstantBuffers();
	stateManager->setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	stateManager->setStreamDecl(streamDecl);
	stateManager->setVertexBuffer(cdcVertexBuffer);
	stateManager->setIndexBuffer(cdcIndexBuffer);

	if (false) {
		auto errorTable = static_cast<cdc::PCDX11PixelShaderTable*>(renderDevice->shlib_1->table);
		stateManager->setPixelShader(errorTable->pixelShaders[0]);
	} else {
		stateManager->setPixelShader(cdcPixelShader);
	}

	stateManager->updateRenderState();

	renderDevice->getD3DDeviceContext()->DrawIndexed(std::size(IndexData), 0, 0);
}

void ImGuiDrawable::draw(uint32_t funcSetIndex, IRenderDrawable *other) {
#if ENABLE_IMGUI
	if (lastMinuteAdditions)
		lastMinuteAdditions();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif
}
