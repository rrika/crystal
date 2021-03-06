target_sources(dxhr PRIVATE
	drawables/PCDX11ClearDrawable.cpp
	drawables/PCDX11FastBlurDrawable.cpp
	drawables/PCDX11FXAADrawable.cpp
	drawables/PCDX11MLAADrawable.cpp
	drawables/PCDX11SetRTDrawable.cpp

	shaders/PCDX11PixelShader.cpp
	shaders/PCDX11ShaderLib.cpp
	shaders/PCDX11VertexShader.cpp

	surfaces/PCDX11BitmapTexture.cpp
	surfaces/PCDX11DefaultRenderTarget.cpp
	surfaces/PCDX11DepthBuffer.cpp
	surfaces/PCDX11RenderTexture.cpp
	surfaces/PCDX11Texture.cpp

	BuiltinResources.cpp
	CommonMaterial.cpp
	CommonRenderDevice.cpp
	LinearAllocator.cpp
	PCDX11DeviceManager.cpp
	PCDX11InternalResource.cpp
	PCDX11LightManager.cpp
	PCDX11Material.cpp
	PCDX11MatrixState.cpp
	PCDX11ModelDrawable.cpp
	PCDX11RenderContext.cpp
	PCDX11RenderDevice.cpp
	PCDX11RenderLight.cpp
	PCDX11RenderModel.cpp
	PCDX11RenderModelInstance.cpp
	PCDX11RenderPassCallbacks.cpp
	PCDX11Scene.cpp
	PCDX11StateManager.cpp
	PCDX11StreamDecl.cpp
	PCDX11StreamDeclCache.cpp
	RenderPasses.cpp
	RenderResource.cpp
	VertexAttribute.cpp)

if (ENABLE_IMGUI)
	target_sources(dxhr PRIVATE
		Inspector.cpp)
endif()
