#pragma once
#include "CommonMaterial.h"
#include "PCDX11RenderExternalResource.h"
#include "surfaces/PCDX11BitmapTexture.h"
// #include "buffers/PCDX11StaticConstantBuffer.h"
#include "buffers/PCDX11UberConstantBuffer.h"

namespace cdc {

class PCDX11StreamDecl;
struct MaterialBlobSub;
struct MaterialInstanceData;
struct VertexAttributeLayoutA;

class PCDX11Material :
	public CommonMaterial,
	public PCDX11RenderExternalResource
{
	unsigned short numTextures = 0; // 1C
	unsigned short texBits = 0; // 1E
	// PCDX11BitmapTexture *texture[4]; // 20
	// PCDX11StaticConstantBuffer *constantBuffersPs[16]; // 30
	// PCDX11StaticConstantBuffer *constantBuffersVs[16]; // 70
	PCDX11UberConstantBuffer *constantBuffersPs[16]; // 30
	PCDX11UberConstantBuffer *constantBuffersVs[16]; // 70

	// mg = material globals
	static uint32_t mg_state; // 00B37BE0
	static uint32_t mg_vsSelectAndFlags;// 00B37BE4

	// these globals might be declared elsewhere
	static PCDX11StreamDecl *mg_streamDecl;     // 00EAAD18
	static VertexAttributeLayoutA *mg_layoutA;  // 00EAAD1C
	static PCDX11Material *mg_material;         // 00EAAD20
	static void *mg_cbdata;                     // 00EAAD24
	static MaterialInstanceData *mg_matInstance; // 00EAAD28
	static bool mg_tesselate;                   // 00EAAD2C

public:
	PCDX11Material(PCDX11RenderDevice *renderDevice) :
		PCDX11RenderExternalResource(renderDevice)
	{}

	void load(MaterialBlob *) override;
	void method_08() override;
	~PCDX11Material() = default;
	void method_18() override;

	void setupVertexResources(uint32_t, MaterialBlobSub*, MaterialInstanceData*, char*, bool);
	void setupPixelResources(uint32_t, MaterialBlobSub*, MaterialInstanceData*, char*, bool);

	void setupDepthBias(MaterialInstanceData*);
	void setupStencil(MaterialInstanceData*, bool, uint32_t);

	void setupSinglePassOpaque(PCDX11RenderDevice*, MaterialInstanceData*, uint32_t);
	void setupSinglePassTranslucent(PCDX11RenderDevice*, MaterialInstanceData*, uint32_t, float);
	static void invalidate();

	PCDX11StreamDecl *buildStreamDecl015(
		MaterialInstanceData*,
		void *drawableExtDword50,
		uint32_t vsSelect,
		bool arg4,
		VertexAttributeLayoutA *layout,
		uint8_t flags,
		float floatX,
		float floatY);

	PCDX11StreamDecl *buildStreamDecl01(
		MaterialInstanceData*,
		void *drawableExtDword50,
		uint8_t lightManager434_114,
		uint32_t vsSelect,
		VertexAttributeLayoutA *layout,
		uint8_t flags,
		float floatX,
		float floatY);

	PCDX11StreamDecl *buildStreamDecl4(
		MaterialInstanceData*,
		void *drawableExtDword50,
		uint32_t vsSelect,
		VertexAttributeLayoutA *layout,
		uint8_t flags,
		float floatX);

	PCDX11StreamDecl *buildStreamDecl038(
		MaterialInstanceData*,
		void *drawableExtDword50,
		void *drawableDword24,
		uint32_t vsSelect,
		VertexAttributeLayoutA *layout,
		uint8_t flags,
		bool flag,
		float floatX,
		float floatY);

	PCDX11StreamDecl *buildStreamDecl7(
		MaterialInstanceData*,
		void *drawableExtDword50,
		uint32_t vsSelect,
		VertexAttributeLayoutA *layout,
		uint8_t flags,
		float floatX,
		float floatY);
};

}
