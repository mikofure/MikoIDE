#include "dx11_renderer.hpp"



#include "../../utils/logger.hpp"



#include <d3dcompiler.h>



#include <iomanip>



#include <sstream>



#include <algorithm>







#pragma comment(lib, "d3dcompiler.lib")







// Shader sources



namespace DX11Shaders {



const char *VERTEX_SHADER_SOURCE = R"(



        struct VSInput {



            float3 position : POSITION;



            float2 texCoord : TEXCOORD0;



        };



        



        struct PSInput {



            float4 position : SV_POSITION;



            float2 texCoord : TEXCOORD0;



        };



        



        PSInput main(VSInput input) {



            PSInput output;



            output.position = float4(input.position, 1.0f);



            output.texCoord = input.texCoord;



            return output;



        }



    )";







const char *PIXEL_SHADER_SOURCE = R"(



        Texture2D cefTexture : register(t0);



        SamplerState textureSampler : register(s0);



        



        struct PSInput {



            float4 position : SV_POSITION;



            float2 texCoord : TEXCOORD0;



        };



        



        float4 main(PSInput input) : SV_TARGET {



            return cefTexture.Sample(textureSampler, input.texCoord);



        }



    )";



} // namespace DX11Shaders







DX11Renderer::DX11Renderer()



    : hwnd_(nullptr), width_(0), height_(0), initialized_(false),



      vsyncEnabled_(true), multiSampleCount_(1),



      textureFormat_(DXGI_FORMAT_B8G8R8A8_UNORM), frameTime_(0.0),



      frameCount_(0), enablePartialUpdates_(false) {







  QueryPerformanceCounter(&lastFrameTime_);







  // Initialize buffer stats



  bufferStats_.totalUpdates = 0;



  bufferStats_.cacheHits = 0;



  bufferStats_.cacheMisses = 0;



  bufferStats_.avgUpdateTime = 0.0;







  // Initialize dirty region



  lastDirtyRegion_ = {0, 0, 0, 0};



}







DX11Renderer::~DX11Renderer() { Shutdown(); }







bool DX11Renderer::Initialize(void* window_handle, int width, int height) {



  HWND hwnd = static_cast<HWND>(window_handle);



  if (initialized_) {



    Logger::LogMessage("DX11Renderer: Already initialized");



    return true;



  }







  hwnd_ = hwnd;



  width_ = width;



  height_ = height;







  Logger::LogMessage("DX11Renderer: Initializing " + std::to_string(width) +



                     "x" + std::to_string(height));







  if (!CreateDevice()) {



    LogError("Failed to create D3D11 device");



    return false;



  }







  if (!CreateSwapChain()) {



    LogError("Failed to create swap chain");



    return false;



  }







  if (!CreateRenderTargets()) {



    LogError("Failed to create render targets");



    return false;



  }







  if (!CreateShaders()) {



    LogError("Failed to create shaders");



    return false;



  }







  if (!CreateGeometry()) {



    LogError("Failed to create geometry");



    return false;



  }







  if (!CreateStates()) {



    LogError("Failed to create render states");



    return false;



  }







  initialized_ = true;



  Logger::LogMessage("DX11Renderer: Successfully initialized");



  Logger::LogMessage("DX11Renderer: " + GetAdapterInfo());







  return true;



}







void DX11Renderer::Shutdown() {



  if (!initialized_)



    return;







  Logger::LogMessage("DX11Renderer: Shutting down");







  CleanupCEFTexture();



  CleanupRenderTargets();







  // Release all COM objects



  blendState_.Reset();



  rasterizerState_.Reset();



  depthStencilState_.Reset();



  vertexBuffer_.Reset();



  indexBuffer_.Reset();



  inputLayout_.Reset();



  vertexShader_.Reset();



  pixelShader_.Reset();



  samplerState_.Reset();



  swapChain_.Reset();



  context_.Reset();



  device_.Reset();



  factory_.Reset();







  initialized_ = false;



  Logger::LogMessage("DX11Renderer: Shutdown complete");



}







bool DX11Renderer::CreateDevice() {



  return CreateDeviceInternal(true);



}







bool DX11Renderer::CreateDeviceInternal(bool preferHighPerformance) {



  UINT createDeviceFlags = 0;



#ifdef _DEBUG



  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;



#endif







  UINT factoryFlags = 0;



#ifdef _DEBUG



  factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;



#endif







  ComPtr<IDXGIFactory1> factory1;



  HRESULT hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory1));



  if (FAILED(hr)) {



    hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory1));



    if (FAILED(hr)) {



      LogError("CreateDevice: Failed to create DXGI factory", hr);



      return false;



    }



  }







  using_high_performance_adapter_ = false;



  ComPtr<IDXGIAdapter1> selectedAdapter;







  if (preferHighPerformance) {



    ComPtr<IDXGIFactory6> factory6;



    if (SUCCEEDED(factory1.As(&factory6))) {



      hr = factory6->EnumAdapterByGpuPreference(



          0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,



          IID_PPV_ARGS(&selectedAdapter));



      if (FAILED(hr)) {



        selectedAdapter.Reset();



      }



    }







    if (!selectedAdapter) {



      SIZE_T bestMemory = 0;



      for (UINT index = 0;; ++index) {



        ComPtr<IDXGIAdapter1> candidate;



        if (factory1->EnumAdapters1(index, &candidate) == DXGI_ERROR_NOT_FOUND) {



          break;



        }







        DXGI_ADAPTER_DESC1 desc;



        if (FAILED(candidate->GetDesc1(&desc))) {



          continue;



        }







        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {



          continue;



        }







        if (desc.DedicatedVideoMemory > bestMemory) {



          bestMemory = desc.DedicatedVideoMemory;



          selectedAdapter = candidate;



        }



      }



    }



  }







  ComPtr<IDXGIAdapter> adapter;



  if (selectedAdapter) {



    selectedAdapter.As(&adapter);



    using_high_performance_adapter_ = true;



  }







  D3D_FEATURE_LEVEL featureLevels[] = {



      D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,



      D3D_FEATURE_LEVEL_10_0};



  D3D_FEATURE_LEVEL featureLevel;







  hr = D3D11CreateDevice(



      adapter.Get(),



      adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,



      nullptr, createDeviceFlags, featureLevels, ARRAYSIZE(featureLevels),



      D3D11_SDK_VERSION, &device_, &featureLevel, &context_);







  if (FAILED(hr) && adapter) {



    Logger::LogMessage(



        "DX11Renderer: High-performance adapter initialization failed, "



        "retrying with system default");



    adapter.Reset();



    selectedAdapter.Reset();



    using_high_performance_adapter_ = false;



    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,



                           createDeviceFlags, featureLevels,



                           ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &device_,



                           &featureLevel, &context_);



  }







  if (FAILED(hr)) {



    LogError("D3D11CreateDevice failed", hr);



    return false;



  }







  if (using_high_performance_adapter_ && selectedAdapter) {



    DXGI_ADAPTER_DESC1 desc;



    if (SUCCEEDED(selectedAdapter->GetDesc1(&desc))) {



      std::wstring wname(desc.Description);



      std::string adapterName(wname.begin(), wname.end());



      auto vramMb = static_cast<unsigned long long>(desc.DedicatedVideoMemory /



                                                    (1024ull * 1024ull));



      Logger::LogMessage(



          "DX11Renderer: Using high-performance adapter '" + adapterName +



          "' with " + std::to_string(vramMb) + " MB VRAM");



    }



  } else {



    Logger::LogMessage("DX11Renderer: Using default hardware adapter");



  }







  Logger::LogMessage("DX11Renderer: Created device with feature level " +



                     std::to_string(featureLevel >> 12) + '.' +



                     std::to_string((featureLevel >> 8) & 0xF));







  factory_.Reset();



  if (FAILED(factory1.As(&factory_))) {



    ComPtr<IDXGIDevice> dxgiDevice;



    hr = device_.As(&dxgiDevice);



    if (FAILED(hr)) {



      LogError("Failed to get DXGI device", hr);



      return false;



    }







    ComPtr<IDXGIAdapter> parentAdapter;



    hr = dxgiDevice->GetAdapter(&parentAdapter);



    if (FAILED(hr)) {



      LogError("Failed to get DXGI adapter", hr);



      return false;



    }







    hr = parentAdapter->GetParent(IID_PPV_ARGS(&factory_));



    if (FAILED(hr)) {



      LogError("Failed to get DXGI factory", hr);



      return false;



    }



  }







  return true;



}







bool DX11Renderer::CreateSwapChain() {
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
  swapChainDesc.Width = width_;
  swapChainDesc.Height = height_;
  swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  swapChainDesc.Stereo = FALSE;
  swapChainDesc.SampleDesc.Count = multiSampleCount_;
  swapChainDesc.SampleDesc.Quality = 0;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.BufferCount = 2;
  swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
  swapChainDesc.Flags = 0;

  HRESULT hr = factory_->CreateSwapChainForHwnd(
      device_.Get(), hwnd_, &swapChainDesc, nullptr, nullptr, &swapChain_);

  if (FAILED(hr)) {
    LogError("CreateSwapChainForHwnd failed", hr);
    if (using_high_performance_adapter_) {
      Logger::LogMessage("DX11Renderer: Swap chain creation failed on high-performance adapter, retrying with default adapter");
      if (context_) {
        context_->ClearState();
      }
      swapChain_.Reset();
      factory_.Reset();
      context_.Reset();
      device_.Reset();
      if (!CreateDeviceInternal(false)) {
        return false;
      }
      return CreateSwapChain();
    }
    return false;
  }

  factory_->MakeWindowAssociation(hwnd_, DXGI_MWA_NO_ALT_ENTER);

  return true;
}

bool DX11Renderer::CreateRenderTargets() {



  // Get back buffer



  HRESULT hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer_));



  if (FAILED(hr)) {



    LogError("Failed to get back buffer", hr);



    return false;



  }







  // Create render target view



  hr = device_->CreateRenderTargetView(backBuffer_.Get(), nullptr,



                                       &renderTargetView_);



  if (FAILED(hr)) {



    LogError("Failed to create render target view", hr);



    return false;



  }







  // Create depth stencil buffer



  D3D11_TEXTURE2D_DESC depthDesc = {};



  depthDesc.Width = width_;



  depthDesc.Height = height_;



  depthDesc.MipLevels = 1;



  depthDesc.ArraySize = 1;



  depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;



  depthDesc.SampleDesc.Count = multiSampleCount_;



  depthDesc.SampleDesc.Quality = 0;



  depthDesc.Usage = D3D11_USAGE_DEFAULT;



  depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;



  depthDesc.CPUAccessFlags = 0;



  depthDesc.MiscFlags = 0;







  hr = device_->CreateTexture2D(&depthDesc, nullptr, &depthStencilBuffer_);



  if (FAILED(hr)) {



    LogError("Failed to create depth stencil buffer", hr);



    return false;



  }







  // Create depth stencil view



  hr = device_->CreateDepthStencilView(depthStencilBuffer_.Get(), nullptr,



                                       &depthStencilView_);



  if (FAILED(hr)) {



    LogError("Failed to create depth stencil view", hr);



    return false;



  }







  // Set viewport



  D3D11_VIEWPORT viewport = {};



  viewport.TopLeftX = 0.0f;



  viewport.TopLeftY = 0.0f;



  viewport.Width = static_cast<float>(width_);



  viewport.Height = static_cast<float>(height_);



  viewport.MinDepth = 0.0f;



  viewport.MaxDepth = 1.0f;







  context_->RSSetViewports(1, &viewport);







  return true;



}







bool DX11Renderer::CreateShaders() {



  ComPtr<ID3DBlob> vsBlob, psBlob;







  // Compile vertex shader



  if (!CompileShaderFromString(DX11Shaders::VERTEX_SHADER_SOURCE, "main",



                               "vs_5_0", &vsBlob)) {



    LogError("Failed to compile vertex shader");



    return false;



  }







  HRESULT hr = device_->CreateVertexShader(vsBlob->GetBufferPointer(),



                                           vsBlob->GetBufferSize(), nullptr,



                                           &vertexShader_);



  if (FAILED(hr)) {



    LogError("Failed to create vertex shader", hr);



    return false;



  }







  // Compile pixel shader



  if (!CompileShaderFromString(DX11Shaders::PIXEL_SHADER_SOURCE, "main",



                               "ps_5_0", &psBlob)) {



    LogError("Failed to compile pixel shader");



    return false;



  }







  hr = device_->CreatePixelShader(psBlob->GetBufferPointer(),



                                  psBlob->GetBufferSize(), nullptr,



                                  &pixelShader_);



  if (FAILED(hr)) {



    LogError("Failed to create pixel shader", hr);



    return false;



  }







  // Create input layout



  D3D11_INPUT_ELEMENT_DESC inputElements[] = {



      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,



       D3D11_INPUT_PER_VERTEX_DATA, 0},



      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,



       D3D11_INPUT_PER_VERTEX_DATA, 0}};







  hr = device_->CreateInputLayout(inputElements, ARRAYSIZE(inputElements),



                                  vsBlob->GetBufferPointer(),



                                  vsBlob->GetBufferSize(), &inputLayout_);



  if (FAILED(hr)) {



    LogError("Failed to create input layout", hr);



    return false;



  }







  return true;



}







bool DX11Renderer::CreateGeometry() {



  // Create fullscreen quad vertices



  QuadVertex vertices[] = {



      {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}, // Bottom-left



      {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},  // Top-left



      {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},   // Top-right



      {{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}   // Bottom-right



  };







  D3D11_BUFFER_DESC vertexBufferDesc = {};



  vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;



  vertexBufferDesc.ByteWidth = sizeof(vertices);



  vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;



  vertexBufferDesc.CPUAccessFlags = 0;







  D3D11_SUBRESOURCE_DATA vertexData = {};



  vertexData.pSysMem = vertices;







  HRESULT hr =



      device_->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer_);



  if (FAILED(hr)) {



    LogError("Failed to create vertex buffer", hr);



    return false;



  }







  // Create indices



  UINT indices[] = {0, 1, 2, 0, 2, 3};







  D3D11_BUFFER_DESC indexBufferDesc = {};



  indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;



  indexBufferDesc.ByteWidth = sizeof(indices);



  indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;



  indexBufferDesc.CPUAccessFlags = 0;







  D3D11_SUBRESOURCE_DATA indexData = {};



  indexData.pSysMem = indices;







  hr = device_->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer_);



  if (FAILED(hr)) {



    LogError("Failed to create index buffer", hr);



    return false;



  }







  return true;



}







bool DX11Renderer::CreateStates() {



  // Create blend state for alpha blending



  D3D11_BLEND_DESC blendDesc = {};



  blendDesc.RenderTarget[0].BlendEnable = TRUE;



  blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;



  blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;



  blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;



  blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;



  blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;



  blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;



  blendDesc.RenderTarget[0].RenderTargetWriteMask =



      D3D11_COLOR_WRITE_ENABLE_ALL;







  HRESULT hr = device_->CreateBlendState(&blendDesc, &blendState_);



  if (FAILED(hr)) {



    LogError("Failed to create blend state", hr);



    return false;



  }







  // Create rasterizer state



  D3D11_RASTERIZER_DESC rasterizerDesc = {};



  rasterizerDesc.FillMode = D3D11_FILL_SOLID;



  rasterizerDesc.CullMode = D3D11_CULL_BACK;



  rasterizerDesc.FrontCounterClockwise = FALSE;



  rasterizerDesc.DepthBias = 0;



  rasterizerDesc.SlopeScaledDepthBias = 0.0f;



  rasterizerDesc.DepthBiasClamp = 0.0f;



  rasterizerDesc.DepthClipEnable = TRUE;



  rasterizerDesc.ScissorEnable = FALSE;



  rasterizerDesc.MultisampleEnable = (multiSampleCount_ > 1);



  rasterizerDesc.AntialiasedLineEnable = FALSE;







  hr = device_->CreateRasterizerState(&rasterizerDesc, &rasterizerState_);



  if (FAILED(hr)) {



    LogError("Failed to create rasterizer state", hr);



    return false;



  }







  // Create depth stencil state



  D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};



  depthStencilDesc.DepthEnable =



      FALSE; // Disable depth testing for 2D rendering



  depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;



  depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;



  depthStencilDesc.StencilEnable = FALSE;







  hr = device_->CreateDepthStencilState(&depthStencilDesc, &depthStencilState_);



  if (FAILED(hr)) {



    LogError("Failed to create depth stencil state", hr);



    return false;



  }







  // Create sampler state



  D3D11_SAMPLER_DESC samplerDesc = {};



  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;



  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;



  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;



  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;



  samplerDesc.MipLODBias = 0.0f;



  samplerDesc.MaxAnisotropy = 1;



  samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;



  samplerDesc.MinLOD = 0.0f;



  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;







  hr = device_->CreateSamplerState(&samplerDesc, &samplerState_);



  if (FAILED(hr)) {



    LogError("Failed to create sampler state", hr);



    return false;



  }







  return true;



}







bool DX11Renderer::CreateCEFTexture(int width, int height) {



  CleanupCEFTexture();







  D3D11_TEXTURE2D_DESC textureDesc = {};



  textureDesc.Width = width;



  textureDesc.Height = height;



  textureDesc.MipLevels = 1;



  textureDesc.ArraySize = 1;



  textureDesc.Format = textureFormat_;



  textureDesc.SampleDesc.Count = 1;



  textureDesc.SampleDesc.Quality = 0;



  textureDesc.Usage = D3D11_USAGE_DYNAMIC;



  textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;



  textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;



  textureDesc.MiscFlags = 0;







  HRESULT hr = device_->CreateTexture2D(&textureDesc, nullptr, &cefTexture_);



  if (FAILED(hr)) {



    LogError("Failed to create CEF texture", hr);



    return false;



  }







  // Create shader resource view



  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};



  srvDesc.Format = textureFormat_;



  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;



  srvDesc.Texture2D.MipLevels = 1;



  srvDesc.Texture2D.MostDetailedMip = 0;







  hr = device_->CreateShaderResourceView(cefTexture_.Get(), &srvDesc,



                                         &cefTextureView_);



  if (FAILED(hr)) {



    LogError("Failed to create CEF texture view", hr);



    return false;



  }







  Logger::LogMessage("DX11Renderer: Created CEF texture " +



                     std::to_string(width) + "x" + std::to_string(height));



  return true;



}







bool DX11Renderer::CreateTextureFromBuffer(const void *buffer, int width,



                                           int height) {



  if (!CreateCEFTexture(width, height)) {



    return false;



  }







  return UpdateTexture(buffer, width, height);



}







bool DX11Renderer::CreateSharedTexture(int width, int height, ComPtr<ID3D11Texture2D>& sharedTexture) {



    if (!device_) {



        LogError("CreateSharedTexture: Device is null");



        return false;



    }







    // Optimized texture description for 120 FPS performance



    D3D11_TEXTURE2D_DESC desc = {};



    desc.Width = width;



    desc.Height = height;



    desc.MipLevels = 1;



    desc.ArraySize = 1;



    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;



    desc.SampleDesc.Count = 1;



    desc.SampleDesc.Quality = 0;



    desc.Usage = D3D11_USAGE_DEFAULT;  // Changed from STAGING for better GPU performance



    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;  // Enable GPU-only operations



    desc.CPUAccessFlags = 0;  // Remove CPU access to prevent GPU stalls



    desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;  // Enable DXGI sharing







    HRESULT hr = device_->CreateTexture2D(&desc, nullptr, &sharedTexture);



    if (FAILED(hr)) {



        LogError("CreateSharedTexture: Failed to create shared texture", hr);



        return false;



    }







    Logger::LogMessage("DX11Renderer: Created optimized shared texture " + 



                       std::to_string(width) + "x" + std::to_string(height));



    return true;



}







bool DX11Renderer::UpdateTexture(const void *buffer, int width, int height) {



    if (!buffer || !device_ || !context_) {



        return false;



    }







    // Performance optimization: Use texture cache to avoid recreation



    auto cacheKey = std::make_pair(width, height);



    ComPtr<ID3D11Texture2D> texture;



    ComPtr<ID3D11ShaderResourceView> textureView;







    auto it = textureCache_.find(cacheKey);



    if (it != textureCache_.end()) {



        texture = it->second.texture;



        textureView = it->second.view;



    } else {



        // Create new optimized texture for GPU-only operations



        D3D11_TEXTURE2D_DESC desc = {};



        desc.Width = width;



        desc.Height = height;



        desc.MipLevels = 1;



        desc.ArraySize = 1;



        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;



        desc.SampleDesc.Count = 1;



        desc.SampleDesc.Quality = 0;



        desc.Usage = D3D11_USAGE_DYNAMIC;  // Optimized for frequent updates



        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;



        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;  // Write-only for better performance



        desc.MiscFlags = 0;







        HRESULT hr = device_->CreateTexture2D(&desc, nullptr, &texture);



        if (FAILED(hr)) {



            LogError("UpdateTexture: Failed to create texture", hr);



            return false;



        }







        // Create shader resource view



        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};



        srvDesc.Format = desc.Format;



        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;



        srvDesc.Texture2D.MipLevels = 1;







        hr = device_->CreateShaderResourceView(texture.Get(), &srvDesc, &textureView);



        if (FAILED(hr)) {



            LogError("UpdateTexture: Failed to create shader resource view", hr);



            return false;



        }







        // Cache the texture for reuse



        TextureCache entry;



        entry.texture = texture;



        entry.view = textureView;



        entry.width = width;



        entry.height = height;



        entry.format = textureFormat_;



        entry.lastUsed = GetTickCount64();



        entry.isDirty = false;



        textureCache_[cacheKey] = entry;



        



        // Limit cache size to prevent memory bloat



        if (textureCache_.size() > 10) {



            textureCache_.erase(textureCache_.begin());



        }



    }







    // Fast GPU update using mapped resource



    D3D11_MAPPED_SUBRESOURCE mappedResource;



    HRESULT hr = context_->Map(texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, D3D11_MAP_FLAG_DO_NOT_WAIT, &mappedResource);



    



    if (hr == DXGI_ERROR_WAS_STILL_DRAWING) {



        // GPU is still using the resource, skip this frame to maintain 120 FPS



        return true;



    }



    



    if (FAILED(hr)) {



        LogError("UpdateTexture: Failed to map texture", hr);



        return false;



    }







    // Optimized memory copy with proper pitch handling



    const uint8_t* srcData = static_cast<const uint8_t*>(buffer);



    uint8_t* dstData = static_cast<uint8_t*>(mappedResource.pData);



    const int srcPitch = width * 4;  // BGRA format



    



    if (mappedResource.RowPitch == static_cast<UINT>(srcPitch)) {



        // Direct copy when pitches match



        memcpy(dstData, srcData, height * srcPitch);



    } else {



        // Row-by-row copy when pitches differ



        for (int y = 0; y < height; ++y) {



            memcpy(dstData + y * mappedResource.RowPitch, 



                   srcData + y * srcPitch, 



                   srcPitch);



        }



    }







    context_->Unmap(texture.Get(), 0);







    // Update the current CEF texture



    cefTexture_ = texture;



    cefTextureView_ = textureView;







    return true;



}







bool DX11Renderer::Present() {



    if (!swapChain_) {



        LogError("Present: SwapChain is null");



        return false;



    }







    // Frame pacing for 120 FPS



    static auto lastFrameTime = std::chrono::high_resolution_clock::now();



    static const auto targetFrameTime = std::chrono::microseconds(8333);  // 1/120 second in microseconds



    



    auto currentTime = std::chrono::high_resolution_clock::now();



    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastFrameTime);



    



    if (elapsed < targetFrameTime) {



        // Use precise timing for 120 FPS



        auto sleepTime = targetFrameTime - elapsed;



        if (sleepTime > std::chrono::microseconds(1000)) {



            std::this_thread::sleep_for(sleepTime - std::chrono::microseconds(500));



        }



        



        // Spin-wait for the remaining time for precision



        while (std::chrono::duration_cast<std::chrono::microseconds>(



                   std::chrono::high_resolution_clock::now() - lastFrameTime) < targetFrameTime) {



            std::this_thread::yield();



        }



    }



    



    lastFrameTime = std::chrono::high_resolution_clock::now();







    // Present with optimized settings for 120 FPS



    UINT presentFlags = 0;



    if (!vsyncEnabled_) {



        presentFlags = DXGI_PRESENT_DO_NOT_WAIT;  // Non-blocking present for high FPS



    }



    



    HRESULT hr = swapChain_->Present(vsyncEnabled_ ? 1 : 0, presentFlags);



    



    if (hr == DXGI_ERROR_WAS_STILL_DRAWING && !vsyncEnabled_) {



        // Skip frame if GPU is busy, maintaining 120 FPS target



        return true;



    }



    



    if (FAILED(hr)) {



        LogError("Present: Failed to present swap chain", hr);



        return false;



    }







    frameCount_++;



    



    // Performance monitoring for 120 FPS validation



    static int frameCounter = 0;



    static auto fpsStartTime = std::chrono::high_resolution_clock::now();



    



    frameCounter++;



    if (frameCounter >= 120) {  // Check every 120 frames (1 second at 120 FPS)



        auto now = std::chrono::high_resolution_clock::now();



        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - fpsStartTime);



        double fps = (frameCounter * 1000.0) / duration.count();



        



        Logger::LogMessage("DX11Renderer: Current FPS: " + std::to_string(fps));



        



        frameCounter = 0;



        fpsStartTime = now;



    }



    



    return true;



}







bool DX11Renderer::BeginFrame() {



  if (!initialized_) {



    LogError("BeginFrame: DX11Renderer not initialized");



    return false;



  }







  if (!context_ || !renderTargetView_) {



    LogError("BeginFrame: Render target not ready");



    return false;



  }







  context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());







  const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};



  context_->ClearRenderTargetView(renderTargetView_.Get(), clearColor);







  if (depthStencilView_) {



    context_->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);



  }







  return true;



}







bool DX11Renderer::EndFrame() {



  if (!initialized_) {



    LogError("EndFrame: DX11Renderer not initialized");



    return false;



  }







  if (context_) {



    ID3D11ShaderResourceView* nullSRV[1] = {nullptr};



    context_->PSSetShaderResources(0, 1, nullSRV);



  }







  return true;



}







bool DX11Renderer::Render() {



  if (!initialized_) {



    LogError("Render: DX11Renderer not initialized");



    return false;



  }







  if (!BeginFrame()) {



    return false;



  }







  // Set up the rendering pipeline



  context_->IASetInputLayout(inputLayout_.Get());



  context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);







  // Set vertex and index buffers



  UINT stride = sizeof(QuadVertex);



  UINT offset = 0;



  context_->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride,



                               &offset);



  context_->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R16_UINT, 0);







  // Set shaders



  context_->VSSetShader(vertexShader_.Get(), nullptr, 0);



  context_->PSSetShader(pixelShader_.Get(), nullptr, 0);







  // Set CEF texture and sampler



  if (cefTextureView_) {



    context_->PSSetShaderResources(0, 1, cefTextureView_.GetAddressOf());



  }



  context_->PSSetSamplers(0, 1, samplerState_.GetAddressOf());







  // Set render states



  float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};



  context_->OMSetBlendState(blendState_.Get(), blendFactor, 0xffffffff);



  context_->RSSetState(rasterizerState_.Get());



  context_->OMSetDepthStencilState(depthStencilState_.Get(), 1);







  // Draw the quad



  context_->DrawIndexed(6, 0, 0);







  if (!EndFrame()) {



    return false;



  }







  return Present();



}







bool DX11Renderer::ResizeTextures(int width, int height) {



  if (width_ == width && height_ == height) {



    return true;



  }







  width_ = width;



  height_ = height;







  Logger::LogMessage("DX11Renderer: Resizing to " + std::to_string(width) +



                     "x" + std::to_string(height));







  CleanupRenderTargets();







  HRESULT hr =



      swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);



  if (FAILED(hr)) {



    LogError("Failed to resize swap chain buffers", hr);



    return false;



  }







  return CreateRenderTargets();



}







void DX11Renderer::EnableVSync(bool enable) {



  vsyncEnabled_ = enable;



  Logger::LogMessage("DX11Renderer: VSync " +



                     std::string(enable ? "enabled" : "disabled"));



}







void DX11Renderer::SetMultiSampleCount(int samples) {



  if (samples != multiSampleCount_) {



    multiSampleCount_ = samples;



    Logger::LogMessage("DX11Renderer: Multisampling set to " +



                       std::to_string(samples) + "x");



    // Note: Requires recreation of swap chain and render targets



  }



}







bool DX11Renderer::CheckFeatureSupport() {



  if (!device_)



    return false;







  D3D11_FEATURE_DATA_THREADING threadingSupport = {};



  device_->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingSupport,



                               sizeof(threadingSupport));







  Logger::LogMessage(



      "DX11Renderer: Driver concurrent creates: " +



      std::string(threadingSupport.DriverConcurrentCreates ? "Yes" : "No"));



  Logger::LogMessage(



      "DX11Renderer: Driver command lists: " +



      std::string(threadingSupport.DriverCommandLists ? "Yes" : "No"));







  return true;



}







std::string DX11Renderer::GetAdapterInfo() {



  if (!factory_)



    return "No adapter info available";







  ComPtr<IDXGIAdapter1> adapter;



  HRESULT hr = factory_->EnumAdapters1(0, &adapter);



  if (FAILED(hr))



    return "Failed to enumerate adapter";







  DXGI_ADAPTER_DESC1 desc;



  hr = adapter->GetDesc1(&desc);



  if (FAILED(hr))



    return "Failed to get adapter description";







  std::wstring wDescription(desc.Description);



  std::string description(wDescription.begin(), wDescription.end());







  std::stringstream ss;



  ss << "Adapter: " << description;



  ss << ", VRAM: " << (desc.DedicatedVideoMemory / 1024 / 1024) << " MB";



  ss << ", System RAM: " << (desc.DedicatedSystemMemory / 1024 / 1024) << " MB";



  ss << ", Shared RAM: " << (desc.SharedSystemMemory / 1024 / 1024) << " MB";







  return ss.str();



}







void DX11Renderer::LogPerformanceStats() {



  if (frameCount_ > 0) {



    Logger::LogMessage(



        "DX11Renderer: Frame time: " + std::to_string(frameTime_) +



        "ms, FPS: " + std::to_string(1000.0 / frameTime_));



  }



}







void DX11Renderer::CleanupRenderTargets() {



  renderTargetView_.Reset();



  depthStencilView_.Reset();



  backBuffer_.Reset();



  depthStencilBuffer_.Reset();



}







void DX11Renderer::CleanupCEFTexture() {



  cefTextureView_.Reset();



  cefTexture_.Reset();



}







bool DX11Renderer::CompileShaderFromString(const std::string &shaderSource,



                                           const std::string &entryPoint,



                                           const std::string &target,



                                           ID3DBlob **blob) {



  ComPtr<ID3DBlob> errorBlob;



  HRESULT hr = D3DCompile(shaderSource.c_str(), shaderSource.length(), nullptr,



                          nullptr, nullptr, entryPoint.c_str(), target.c_str(),



                          D3DCOMPILE_ENABLE_STRICTNESS, 0, blob, &errorBlob);







  if (FAILED(hr)) {



    if (errorBlob) {



      std::string error(



          static_cast<const char *>(errorBlob->GetBufferPointer()),



          errorBlob->GetBufferSize());



      LogError("Shader compilation failed: " + error, hr);



    } else {



      LogError("Shader compilation failed", hr);



    }



    return false;



  }







  return true;



}







void DX11Renderer::LogError(const std::string &message, HRESULT hr) {



  std::string fullMessage = "DX11Renderer: " + message;



  if (hr != S_OK) {



    fullMessage += " (HRESULT: " + HResultToString(hr) + ")";



  }



  Logger::LogMessage(fullMessage);



}







std::string DX11Renderer::HResultToString(HRESULT hr) {



  std::stringstream ss;



  ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(8)



     << hr;



  return ss.str();



}







// Performance optimization methods



void DX11Renderer::EnablePartialUpdates(bool enable) {



  enablePartialUpdates_ = enable;



  Logger::LogMessage("DX11Renderer: Partial updates " +



                     std::string(enable ? "enabled" : "disabled"));



}







void DX11Renderer::ClearTextureCache() {



  textureCache_.clear();



  bufferStats_.cacheHits = 0;



  bufferStats_.cacheMisses = 0;



  Logger::LogMessage("DX11Renderer: Texture cache cleared");



}







void DX11Renderer::SetDirtyRegion(int x, int y, int width, int height) {



  lastDirtyRegion_.x = x;



  lastDirtyRegion_.y = y;



  lastDirtyRegion_.width = width;



  lastDirtyRegion_.height = height;



}



