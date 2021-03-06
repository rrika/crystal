#pragma once
#include <d3d11.h>
#include "CommonRenderDevice.h"
#include "PCDX11InternalResource.h"
#include "PCDX11StreamDeclCache.h"
#include "shaders/PCDX11ShaderLib.h"
#include "shaders/PCDX11ShaderTable.h" // for PixelShaderTable and VertexShaderTable

namespace cdc {

class IRenderDrawable;
class PCDX11BaseTexture;
class PCDX11LightManager;
class PCDX11RenderContext;
class PCDX11RenderTarget;
class PCDX11Texture;
struct LightManagerSubB;

class PCDX11RenderDevice :
	public PCDX11InternalResource,
	public CommonRenderDevice
{
public:
	struct RenderList {
		RenderList(PCDX11RenderDevice *renderDevice, uint32_t *dimensions);

		uint32_t dword0;
		uint32_t dword4;
		uint32_t widthMaybe8;
		uint32_t heightMaybeC;
		PCDX11RenderTarget *renderTarget; // 10
		// uint32_t dword14;
		DrawableList drawableList; // 18
		LightManagerSubB *lightManagerSubB; // 28
		// uint8_t byte2C;
		// uint8_t byte2D;
		RenderList *next;
	};

	PCDX11RenderContext *renderContext_10CE8 = nullptr;
	PCDX11RenderContext *renderContext_10CEC;

	RenderList *renderList_current = nullptr; // 10CF8
	RenderList *renderList_processing = nullptr; // 10CFC
	RenderList *renderList_first = nullptr; // 10D00
	RenderList *renderList_last = nullptr; // 10D04
	RenderList *renderList_override = nullptr; // own addition

	PCDX11ShaderLib *shlib_22;
	PCDX11ShaderLib *shlib_21;
	PCDX11ShaderLib *shlib_20;
	PCDX11ShaderLib *shlib_19;
	PCDX11ShaderLib *shlib_18;
	PCDX11ShaderLib *shlib_17;
	PCDX11ShaderLib *shlib_16;
	PCDX11ShaderLib *shlib_15;
	PCDX11ShaderLib *shlib_14;
	PCDX11ShaderLib *shlib_13;
	PCDX11ShaderLib *shlib_12;
	PCDX11ShaderLib *shlib_11;
	PCDX11ShaderLib *shlib_10;
	PCDX11ShaderLib *shlib_9;
	PCDX11ShaderLib *shlib_8;
	PCDX11ShaderLib *shlib_7;
	PCDX11ShaderLib *shlib_6;
	PCDX11ShaderLib *shlib_5;
	PCDX11ShaderLib *shlib_4;
	PCDX11ShaderLib *shlib_3;
	PCDX11ShaderLib *shlib_2;
	PCDX11ShaderLib *shlib_1;
	PCDX11ShaderLib *shlib_0;
	PCDX11VertexShaderTable shtab_vs_wvp_1_0; // 10D1C
	PCDX11VertexShaderTable shtab_vs_ui;      // 10D38
	PCDX11VertexShaderTable shtab_vs_wvp;     // 10D54
	PCDX11PixelShaderTable shtab_ps_white_27; // 10D70
	PCDX11PixelShaderTable shtab_ps_passthrough; // 10D90
	PCDX11PixelShaderTable shtab_ps_white_25; // 10DB0
	PCDX11PixelShaderTable shtab_ps_fogColor; // 10DD0
	PCDX11PixelShaderTable shtab_ps_errorColor; // 10DF0

	PCDX11StreamDeclCache streamDeclCache; // 1112D4
	PCDX11Texture *missingTexture; // 1112F0

	ID3D11DeviceContext *deviceContext; // 111580

public:
	PCDX11RenderDevice(HWND hwnd, uint32_t width, uint32_t height);
	void createDefaultResources();
	void createDefaultVertexAttribLayouts();
	void setupPassCallbacks();
	void registerComparatorsAndDrawersModel();
	void registerComparatorsAndDrawersTerrain1();
	void registerComparatorsAndDrawersTerrain2();
	void setupShadowBuffer();

	// incomplete
	void refCountDec() override;
	void method_08() override;
	void resetRenderLists() override;
	void drawRenderLists() override;
	bool beginRenderList(float*) override;
	bool endRenderList() override;
	bool hasRenderList() override;
	void method_28() override;
	void method_2C() override;
	void method_30() override;
	CommonScene *createSubScene(
		RenderViewport *renderViewport,
		CommonRenderTarget *renderTarget,
		CommonDepthBuffer *depthBuffer) override;
	void finishScene() override;
	void getSceneRenderTarget() override;
	void method_64() override;
	void method_6C() override;
	void method_70() override;
	void method_A8() override;
	void clearRenderTarget(
		uint32_t flags,
		uint32_t unknown1,
		float unknown2,
		float *clearColor,
		float clearDepth,
		uint32_t clearStencil) override;
	void setRenderTarget() override;
	void method_C0() override;
	void method_C4() override;
	void method_C8() override;
	void method_CC() override;
	void method_D0() override;
	void method_D4() override;
	void method_D8() override;
	void method_DC() override;
	void method_EC() override;
	IMaterial *createMaterial() override;
	TextureMap *createTexture(uint32_t) override;
	void createProceduralTexture() override;
	IShaderLib *createShaderLib(uint32_t) override;
	RenderResource *createRenderModel(uint32_t) override;
	RenderModelInstance *createRenderModelInstance(RenderMesh*) override;
	RenderResource *createRenderTerrain(uint32_t) override;
	void createRenderTerrainInstance() override;
	void createRenderImage() override;
	void createWaterSurface() override;
	void createLightResource() override;
	void createRenderLight() override;
	void createMatrixState() override;
	void createVertexBuffer() override;
	void createIndexBuffer() override;
	IRenderTarget *createRenderTarget(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) override;
	IDepthBuffer *createDepthBuffer() override;
	void method_174() override;
	void getWindowSize(uint *width, uint *height) override;
	void method_17C() override;
	void method_180() override;

	// our own additions
	void *captureRenderLists() override;
	void revisitRenderLists(void*) override;
	void freeRenderLists(void*) override;

	bool internalCreate() override;
	void internalRelease() override;

	virtual void dx11_method_0();
	virtual void dx11_method_4();
	virtual void dx11_method_8();
	virtual void dx11_method_C();
	virtual void dx11_method_10();
	virtual void dx11_method_14();
	virtual void dx11_method_18();
	virtual void dx11_method_1C();
	virtual PCDX11RenderContext *getRenderContextAny();
	virtual void dx11_method_24();
	virtual void dx11_method_28();
	virtual void dx11_method_2C();
	virtual void dx11_method_30();
	virtual PCDX11RenderTarget *dx11_createRenderTarget(
		uint32_t width,
		uint32_t height,
		uint32_t dxgiFormat,
		uint32_t,
		uint32_t);
	virtual void dx11_method_38();
	virtual void dx11_method_3C();
	virtual void dx11_method_40();
	virtual void handleResize(int32_t width, int32_t height);
	virtual void dx11_method_48();

	ID3D11DeviceContext *d3dDeviceContext111580;
	ID3D11DeviceContext *getD3DDeviceContext() { return d3dDeviceContext111580; }
	PCDX11LightManager *getLightManager();
	void recordDrawable(IRenderDrawable *drawable, uint32_t mask, bool addToNextScene);
	void clearRenderTargetNow(char mode, float *color, float depth, uint32_t stencil);
	void setTexture(uint32_t slot, PCDX11BaseTexture *tex, uint32_t filter, float unknown);
	void drawRenderListsInternal(void *arg);
};

CommonRenderDevice *createPCDX11RenderDevice(HWND hwnd, uint width, uint height, bool unknown);

}
