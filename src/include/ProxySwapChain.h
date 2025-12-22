#pragma once

// enable/disable ImGui
#define ENABLE_IMGUI 0

// c++ includes
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <chrono>
#include <atomic>
#include <fstream>

// windows headers
#define NOMINMAX // disable min/max macros
#include <Windows.h>
#include <TlHelp32.h>

// directx headers
#include <d3d11.h> // staging creation / Map
#include <dxgi.h>
#include <dxgi1_2.h>     // IDXGIResource / IDXGISwapChain2
#include <d3dcompiler.h> // shader compilation
#pragma comment(lib, "d3dcompiler.lib")

#if ENABLE_IMGUI
// core imgui headers
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

// imgui win32 handler (forward decl – avoids C3861)
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// our imgui style
#include "imstyle.h"
#endif

// helper decls (implemented in d3d11.cpp)
extern std::string timeStamp();
extern void output();
extern int g_Width;                          // target width
extern int g_Height;                         // target height
extern ID3D11Texture2D *g_BackBufferTexture; // colour RT  (GetBuffer-0)
extern ID3D11Texture2D *g_DepthTexture;      // live depth RT  (exported)
extern ID3D11Device *g_Device;               // the real device

extern ID3D11Texture2D *g_BackBufferStaging; // CPU-readable copy (colour)
extern ID3D11Texture2D *g_DepthStaging;      // CPU-readable copy (depth)

// shared back buffer texture
extern ID3D11Texture2D *g_BackBufferShared;
extern HANDLE g_BackBufferSharedHandle;

// shared depth buffer texture
extern ID3D11Texture2D *g_DepthShared;
extern HANDLE g_DepthSharedHandle;

///////////////////////////////////////////////////////////////////////////////////////////
// helper function to create the shared texture for the back buffer and depth buffer
///////////////////////////////////////////////////////////////////////////////////////////

inline void createSharedBackBuffer(ID3D11Device *device, int w, int h)
{
    // create a shared texture for the back buffer
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = w;
    desc.Height = h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // standard back buffer format
    desc.SampleDesc.Count = 1;                   // no MSAA
    desc.SampleDesc.Quality = 0;                 // no MSAA quality
    desc.Usage = D3D11_USAGE_DEFAULT;            // GPU usage
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // minimal bind flags for sharing
    desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED; // basic sharing without keyed mutex

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, &g_BackBufferShared);
    if (FAILED(hr))
    {
#if DEBUG
        std::cout << timeStamp() << "Failed to create shared back buffer texture! HRESULT: " << std::hex << hr << std::dec << std::endl;
#endif
        return;
    }

    // get the shared handle
    IDXGIResource *resource = nullptr;
    hr = g_BackBufferShared->QueryInterface(__uuidof(IDXGIResource), (void **)&resource);
    if (SUCCEEDED(hr))
    {
        hr = resource->GetSharedHandle(&g_BackBufferSharedHandle);
        resource->Release();

        if (FAILED(hr))
        {
#if DEBUG
            std::cout << timeStamp() << "Failed to get shared handle for back buffer texture! HRESULT: " << std::hex << hr << std::dec << std::endl;
#endif
            return;
        }
    }
    else
    {
#if DEBUG
        std::cout << timeStamp() << "Failed to query IDXGIResource for back buffer texture! HRESULT: " << std::hex << hr << std::dec << std::endl;
#endif
        return;
    }

// print the shared handle address
#if DEBUG
    std::cout << timeStamp() << "Shared back buffer texture created! Handle: " << g_BackBufferSharedHandle << std::endl;
#endif
#if DEBUG
    std::cout << timeStamp() << "Shared back buffer texture address: " << g_BackBufferShared << std::endl;
#endif
#if DEBUG
    std::cout << timeStamp() << "Shared back buffer texture size: " << w << "x" << h << std::endl;
#endif
#if DEBUG
    std::cout << timeStamp() << "Shared back buffer texture format: " << desc.Format << std::endl;
#endif
}

inline void createSharedDepthBuffer(ID3D11Device *device, int w, int h)
{
    // create a shared texture for the depth buffer
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = w;
    desc.Height = h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R32_TYPELESS;      // depth format
    desc.SampleDesc.Count = 1;                   // no MSAA
    desc.SampleDesc.Quality = 0;                 // no MSAA quality
    desc.Usage = D3D11_USAGE_DEFAULT;            // GPU usage
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // minimal bind flags for sharing
    desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED; // basic sharing without keyed mutex

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, &g_DepthShared);
    if (FAILED(hr))
    {
#if DEBUG
        std::cout << timeStamp() << "Failed to create shared depth buffer texture! HRESULT: " << std::hex << hr << std::dec << std::endl;
#endif
        return;
    }

    // get the shared handle
    IDXGIResource *resource = nullptr;
    hr = g_DepthShared->QueryInterface(__uuidof(IDXGIResource), (void **)&resource);
    if (SUCCEEDED(hr))
    {
        hr = resource->GetSharedHandle(&g_DepthSharedHandle);
        resource->Release();

        if (FAILED(hr))
        {
#if DEBUG
            std::cout << timeStamp() << "Failed to get shared handle for depth buffer texture! HRESULT: " << std::hex << hr << std::dec << std::endl;
#endif
            return;
        }
    }
    else
    {
#if DEBUG
        std::cout << timeStamp() << "Failed to query IDXGIResource for depth buffer texture! HRESULT: " << std::hex << hr << std::dec << std::endl;
#endif
        return;
    }

// print the shared handle address
#if DEBUG
    std::cout << timeStamp() << "Shared depth buffer texture created! Handle: " << g_DepthSharedHandle << std::endl;
#endif
#if DEBUG
    std::cout << timeStamp() << "Shared depth buffer texture address: " << g_DepthShared << std::endl;
#endif
#if DEBUG
    std::cout << timeStamp() << "Shared depth buffer texture size: " << w << "x" << h << std::endl;
#endif
#if DEBUG
    std::cout << timeStamp() << "Shared depth buffer texture format: " << desc.Format << std::endl;
#endif
}

// Structure to send texture information with dimensions
struct TextureInfo
{
    HANDLE handle;
    UINT width;
    UINT height;
    DXGI_FORMAT format;
};

inline HANDLE createNamedPipe(const std::string &pipeName, bool isInbound = false, DWORD bufferSize = 32)
{
    // create the full pipe name with proper prefix
    std::string fullPipeName = "\\\\.\\pipe\\" + pipeName;

    // create a named pipe with proper settings for immediate use
    HANDLE hPipe = CreateNamedPipeA(
        fullPipeName.c_str(),
        isInbound ? PIPE_ACCESS_INBOUND : PIPE_ACCESS_OUTBOUND, // inbound for confirmation, outbound for data
        PIPE_TYPE_BYTE | PIPE_NOWAIT,                           // non-blocking byte mode
        1,                                                      // max instances
        bufferSize,                                             // output buffer size
        bufferSize,                                             // input buffer size
        0,                                                      // default timeout
        NULL);                                                  // default security

    if (hPipe == INVALID_HANDLE_VALUE)
    {
#if DEBUG
        std::cout << timeStamp() << "Failed to create named pipe: " << GetLastError() << std::endl;
#endif
        return INVALID_HANDLE_VALUE;
    }

// print the pipe name
#if DEBUG
    std::cout << timeStamp() << "Named pipe created: " << fullPipeName << " (direction: " << (isInbound ? "inbound" : "outbound") << ")" << std::endl;
#endif

    return hPipe;
}

inline int duplicateHandleToClientProcess()
{
    // static variables to maintain state between calls
    static DWORD lastFoundPID = 0;
    static HANDLE backBufferPipe = INVALID_HANDLE_VALUE;
    static HANDLE depthBufferPipe = INVALID_HANDLE_VALUE;
    static HANDLE confirmationPipe = INVALID_HANDLE_VALUE;
    static DWORD searchCooldown = 0; // wait a few frames between searches to avoid high CPU usage
    static TextureInfo lastSentBackInfo = {nullptr, 0, 0, DXGI_FORMAT_UNKNOWN};
    static TextureInfo lastSentDepthInfo = {nullptr, 0, 0, DXGI_FORMAT_UNKNOWN};
    static DWORD lastSendTime = 0;
    static const DWORD CONFIRMATION_TIMEOUT_MS = 2000; // 2 second timeout for confirmation
    static bool waitingForConfirmation = false;

    if (searchCooldown > 0)
    {
        searchCooldown--;

        // check for confirmation from client
        if (confirmationPipe != INVALID_HANDLE_VALUE && waitingForConfirmation)
        {
            DWORD available = 0;
            if (PeekNamedPipe(confirmationPipe, nullptr, 0, nullptr, &available, nullptr) && available >= 1)
            {
                BYTE confirmation = 0;
                DWORD bytesRead = 0;
                if (ReadFile(confirmationPipe, &confirmation, 1, &bytesRead, nullptr) && bytesRead == 1)
                {
#if DEBUG
                    std::cout << timeStamp() << "Received texture creation confirmation from client!" << std::endl;
#endif
                    waitingForConfirmation = false;
                    lastSendTime = 0; // reset timeout
                }
            }
        } // check if we need to resend due to timeout
        if (waitingForConfirmation && GetTickCount() - lastSendTime > CONFIRMATION_TIMEOUT_MS)
        {
#if DEBUG
            std::cout << timeStamp() << "Confirmation timeout - will resend texture info" << std::endl;
#endif
            waitingForConfirmation = false;
            // force resend by invalidating last sent info
            lastSentBackInfo.handle = nullptr;
            lastSentDepthInfo.handle = nullptr;
            // reduce search cooldown for faster retry
            searchCooldown = 5;
        }

        // prepare current texture info
        TextureInfo currentBackInfo = {nullptr, 0, 0, DXGI_FORMAT_UNKNOWN};
        TextureInfo currentDepthInfo = {nullptr, 0, 0, DXGI_FORMAT_UNKNOWN};

        if (g_BackBufferSharedHandle && g_BackBufferShared)
        {
            D3D11_TEXTURE2D_DESC desc;
            g_BackBufferShared->GetDesc(&desc);
            currentBackInfo = {g_BackBufferSharedHandle, desc.Width, desc.Height, desc.Format};
        }

        if (g_DepthSharedHandle && g_DepthShared)
        {
            D3D11_TEXTURE2D_DESC desc;
            g_DepthShared->GetDesc(&desc);
            currentDepthInfo = {g_DepthSharedHandle, desc.Width, desc.Height, desc.Format};
        }

        // check if we need to send new info
        bool shouldSendBack = (currentBackInfo.handle &&
                               (currentBackInfo.handle != lastSentBackInfo.handle ||
                                currentBackInfo.width != lastSentBackInfo.width ||
                                currentBackInfo.height != lastSentBackInfo.height ||
                                currentBackInfo.format != lastSentBackInfo.format));

        bool shouldSendDepth = (currentDepthInfo.handle &&
                                (currentDepthInfo.handle != lastSentDepthInfo.handle ||
                                 currentDepthInfo.width != lastSentDepthInfo.width ||
                                 currentDepthInfo.height != lastSentDepthInfo.height ||
                                 currentDepthInfo.format != lastSentDepthInfo.format));

        if (shouldSendBack || shouldSendDepth)
        {
            // send updated texture info
            if (shouldSendBack && backBufferPipe != INVALID_HANDLE_VALUE)
            {
                DWORD bytesWritten = 0;
                if (WriteFile(backBufferPipe, &currentBackInfo, sizeof(currentBackInfo), &bytesWritten, nullptr) &&
                    bytesWritten == sizeof(currentBackInfo))
                {
                    lastSentBackInfo = currentBackInfo;
#if DEBUG
                    std::cout << timeStamp() << "Sent back buffer info: handle=" << currentBackInfo.handle
                              << " size=" << currentBackInfo.width << "x" << currentBackInfo.height
                              << " format=" << currentBackInfo.format << std::endl;
#endif
                }
            }

            if (shouldSendDepth && depthBufferPipe != INVALID_HANDLE_VALUE)
            {
                DWORD bytesWritten = 0;
                if (WriteFile(depthBufferPipe, &currentDepthInfo, sizeof(currentDepthInfo), &bytesWritten, nullptr) &&
                    bytesWritten == sizeof(currentDepthInfo))
                {
                    lastSentDepthInfo = currentDepthInfo;
#if DEBUG
                    std::cout << timeStamp() << "Sent depth buffer info: handle=" << currentDepthInfo.handle
                              << " size=" << currentDepthInfo.width << "x" << currentDepthInfo.height
                              << " format=" << currentDepthInfo.format << std::endl;
#endif
                }
            }
            // start waiting for confirmation
            if (shouldSendBack || shouldSendDepth)
            {
                waitingForConfirmation = true;
                lastSendTime = GetTickCount();
#if DEBUG
                std::cout << timeStamp() << "Waiting for client confirmation..." << std::endl;
#endif
                // reduce cooldown for faster response
                searchCooldown = 5;
            }
        }
        else
        {
            return 0; // nothing to send during cooldown
        }
    }

    // reset cooldown (check every ~30 calls)
    if (searchCooldown == 0)
        searchCooldown = 30;

    // create pipes if they don't exist yet
    if (backBufferPipe == INVALID_HANDLE_VALUE && g_BackBufferSharedHandle)
    {
        backBufferPipe = createNamedPipe("dxpipe_backbuffer", false, sizeof(TextureInfo));
    }

    if (depthBufferPipe == INVALID_HANDLE_VALUE && g_DepthSharedHandle)
    {
        depthBufferPipe = createNamedPipe("dxpipe_depthbuffer", false, sizeof(TextureInfo));
    }

    if (confirmationPipe == INVALID_HANDLE_VALUE)
    {
        confirmationPipe = createNamedPipe("dxpipe_confirmation", true, 4); // inbound pipe for confirmation
    }

    // if we have no valid handles to share, wait until we do
    if (!g_BackBufferSharedHandle && !g_DepthSharedHandle)
    {
        return 0; // still waiting for valid handles
    }

    // create snapshot for process enumeration
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
#if DEBUG
        std::cout << timeStamp() << "Failed to create process snapshot: " << GetLastError() << std::endl;
#endif
        return -1; // error
    }

    // search for bloxshade.exe
    PROCESSENTRY32 processEntry = {};
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    BOOL hasNext = Process32First(snapshot, &processEntry);
    while (hasNext)
    {
        // case insensitive comparison for "bloxshade.exe" - handle both ANSI and Unicode
#ifdef UNICODE
        if (_wcsicmp(processEntry.szExeFile, L"bloxshade.exe") == 0)
#else
        if (_stricmp(processEntry.szExeFile, "bloxshade.exe") == 0)
#endif
        {
            HANDLE targetProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, processEntry.th32ProcessID);
            if (targetProcess != NULL)
            {
// found bloxshade.exe process - we can proceed with sending handles
#if DEBUG
                std::cout << timeStamp() << "Found bloxshade.exe process (PID: " << processEntry.th32ProcessID << ")" << std::endl;
#endif
                lastFoundPID = processEntry.th32ProcessID;
                CloseHandle(targetProcess);
                CloseHandle(snapshot);
                return 1; // client found
            }
        }

        hasNext = Process32Next(snapshot, &processEntry);
    }

    CloseHandle(snapshot);

    // bloxshade.exe not found, will try again next call
    return 0; // still searching
}

///////////////////////////////////////////////////////////////////////////////////////////
// code end
///////////////////////////////////////////////////////////////////////////////////////////

extern void replaceGlobal(ID3D11Texture2D *&dst, ID3D11Texture2D *src);
extern void updateDimensionsFromBackBuffer();

#if ENABLE_IMGUI
// Declare a global GPU texture and SRV for ImGui rendering
ID3D11Texture2D *g_BackBufferGPU = nullptr;
ID3D11ShaderResourceView *g_BackBufferGPU_SRV = nullptr;

// Declare a global GPU texture and SRV for the depth buffer
ID3D11Texture2D *g_DepthGPU = nullptr;
ID3D11ShaderResourceView *g_DepthGPU_SRV = nullptr;

// Extra resources for depth visualization
static ID3D11Texture2D *g_DepthVis = nullptr;
static ID3D11RenderTargetView *g_DepthVisRTV = nullptr;
static ID3D11ShaderResourceView *g_DepthVisSRV = nullptr;

// Extra resources for normal buffer visualization
static ID3D11Texture2D *g_NormalVis = nullptr;
static ID3D11RenderTargetView *g_NormalVisRTV = nullptr;
static ID3D11ShaderResourceView *g_NormalVisSRV = nullptr;

static ID3D11VertexShader *g_QuadVS = nullptr;
static ID3DBlob *g_QuadVSBlob = nullptr;

// Inline depth pixel shader code (one-time compile & reuse)
static ID3D11PixelShader *g_DepthPS = nullptr;
static ID3DBlob *g_DepthPSBlob = nullptr;

// Normal buffer pixel shader
static ID3D11PixelShader *g_NormalPS = nullptr;
static ID3DBlob *g_NormalPSBlob = nullptr;

// Depth shader constant buffer
static ID3D11Buffer *g_DepthConstantBuffer = nullptr;

// Depth shader parameters - configurable via ImGui
static float g_DepthFarPlane = 2000.0f;
static float g_DepthNearPlane = 25.0f;
static float g_DepthGamma = 0.5f;

struct DepthParams
{
    float FAR_PLANE;
    float NEAR_PLANE;
    float GAMMA;
    float padding; // D3D11 constant buffer alignment
};
#endif

#if ENABLE_IMGUI
// Simple shader to sample, linearize, and normalize depth, with brightness adjustment
static const char *s_depthShaderCode = R"(
Texture2D     tex : register(t0);
SamplerState  sam : register(s0);

cbuffer DepthParams : register(b0)
{
    float FAR_PLANE;
    float NEAR_PLANE;
    float GAMMA;
    float padding;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    // get the depth value at the texture coordinate
    float depth = tex.Sample(sam, input.uv).r;

    // linearize depth
    depth = lerp(NEAR_PLANE, FAR_PLANE, depth);

    // normalize depth
    depth = depth / (FAR_PLANE - NEAR_PLANE);

    // adjust brightness for better visibility using configurable gamma
    depth = pow(depth, GAMMA);

    return float4(depth, depth, depth, 1.0f);
}
)";

// derive normals from the depth buffer
static const char *s_depthNormals = R"(
Texture2D     tex : register(t0);
SamplerState  sam : register(s0);

cbuffer DepthParams : register(b0)
{
    float FAR_PLANE;
    float NEAR_PLANE;
    float GAMMA;
    float padding;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

// Function to get linearized depth (equivalent to ReShade's GetLinearizedDepth)
float GetDepth(float2 texcoord)
{
    float rawDepth = tex.Sample(sam, texcoord).r;
    // linearize depth using same method as our depth shader
    return lerp(NEAR_PLANE, FAR_PLANE, rawDepth);
}

float4 main(PSInput input) : SV_TARGET
{
    // get the linearized depth value at the texture coordinate (like ReShade GetDepth)
    float depth = GetDepth(input.uv);
    
    // buffer dimensions - get from texture size
    float2 dims;
    tex.GetDimensions(dims.x, dims.y);

    // horizontal differences (exactly like ReShade)
    float2 texOffset = float2(1, 0) / dims;
    float depthsX = depth - GetDepth(input.uv - texOffset);
    depthsX += (depth - GetDepth(input.uv + texOffset)) - depthsX;

    // vertical differences (exactly like ReShade)  
    texOffset = float2(0, 1) / dims;
    float depthsY = depth - GetDepth(input.uv - texOffset);
    depthsY += (depth - GetDepth(input.uv + texOffset)) - depthsY;

    // normalized normal (exactly like ReShade)
    float3 normal = normalize(float3(depthsX, depthsY, depth / FAR_PLANE));
    
    // Convert from [-1,1] to [0,1] range for display
    return float4(0.5 + 0.5 * normal, 1.0f);
}
)";

// Minimal "fullscreen triangle" VS
static const char *s_quadVS = R"(
struct VSInput
{
    uint vID : SV_VertexID;
};
struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};
PSInput main(VSInput input)
{
    PSInput outp;
    // generate a triangle covering [(-1,1)..(1,-1)]
    float2 verts[3] = {
        float2(-1.0f,  3.0f),
        float2(-1.0f, -1.0f),
        float2( 3.0f, -1.0f)
    };
    outp.position = float4(verts[input.vID], 0.0, 1.0);
    // remap XY to [0..1]
    outp.uv = (verts[input.vID] * float2(0.5f, -0.5f)) + float2(0.5f, 0.5f);
    return outp;
}
)";
#endif

#if ENABLE_IMGUI
// Compile the depth shader if not already compiled
inline void compileDepthShader(ID3D11Device *device)
{
    if (g_DepthPS)
        return; // Already compiled

    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob *shaderBlob = nullptr;
    ID3DBlob *errorBlob = nullptr;
    HRESULT hr = D3DCompile(s_depthShaderCode, strlen(s_depthShaderCode),
                            "DepthPS", nullptr, nullptr,
                            "main", "ps_5_0", flags, 0,
                            &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
#if DEBUG
        std::cout << timeStamp() << "Depth shader compile failed: "
                  << (char *)(errorBlob ? errorBlob->GetBufferPointer() : "")
                  << std::endl;
#endif
        if (errorBlob)
            errorBlob->Release();
        return;
    }

    hr = device->CreatePixelShader(shaderBlob->GetBufferPointer(),
                                   shaderBlob->GetBufferSize(),
                                   nullptr, &g_DepthPS);
    if (FAILED(hr))
    {
#if DEBUG
        std::cout << timeStamp() << "CreatePixelShader(depth) failed!"
                  << std::endl;
#endif
        shaderBlob->Release();
        return;
    }

    g_DepthPSBlob = shaderBlob; // Keep a reference
#if DEBUG
    std::cout << timeStamp() << "Depth shader compiled and created." << std::endl;
#endif
}

// Compile the normal buffer shader if not already compiled
inline void compileNormalShader(ID3D11Device *device)
{
    if (g_NormalPS)
        return; // Already compiled

    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob *shaderBlob = nullptr;
    ID3DBlob *errorBlob = nullptr;
    HRESULT hr = D3DCompile(s_depthNormals, strlen(s_depthNormals),
                            "NormalPS", nullptr, nullptr,
                            "main", "ps_5_0", flags, 0,
                            &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
#if DEBUG
        std::cout << timeStamp() << "Normal shader compile failed: "
                  << (char *)(errorBlob ? errorBlob->GetBufferPointer() : "")
                  << std::endl;
#endif
        if (errorBlob)
            errorBlob->Release();
        return;
    }

    hr = device->CreatePixelShader(shaderBlob->GetBufferPointer(),
                                   shaderBlob->GetBufferSize(),
                                   nullptr, &g_NormalPS);
    if (FAILED(hr))
    {
#if DEBUG
        std::cout << timeStamp() << "CreatePixelShader(normal) failed!"
                  << std::endl;
#endif
        shaderBlob->Release();
        return;
    }

    g_NormalPSBlob = shaderBlob; // Keep a reference
#if DEBUG
    std::cout << timeStamp() << "Normal shader compiled and created." << std::endl;
#endif
}

// Compile our simple fullscreen VS once
inline void compileQuadVS(ID3D11Device *device)
{
    if (g_QuadVS)
        return; // Already compiled
    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    ID3DBlob *vsBlob = nullptr;
    ID3DBlob *err = nullptr;
    HRESULT hr = D3DCompile(s_quadVS, strlen(s_quadVS),
                            "QuadVS", nullptr, nullptr,
                            "main", "vs_5_0", flags, 0,
                            &vsBlob, &err);
    if (FAILED(hr))
    {
#if DEBUG
        std::cout << timeStamp() << "Quad VS compile failed: "
                  << (char *)(err ? err->GetBufferPointer() : "")
                  << std::endl;
#endif
        if (err)
            err->Release();
        return;
    }
    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(),
                                    vsBlob->GetBufferSize(),
                                    nullptr, &g_QuadVS);
    if (FAILED(hr))
    {
#if DEBUG
        std::cout << timeStamp() << "CreateVertexShader(quad) failed!"
                  << std::endl;
#endif
        vsBlob->Release();
        return;
    }
    g_QuadVSBlob = vsBlob;
#if DEBUG
    std::cout << timeStamp() << "Quad VS compiled and created." << std::endl;
#endif
}

// Create depth constant buffer
inline void ensureDepthConstantBuffer(ID3D11Device *device)
{
    if (!device)
        return;

    if (g_DepthConstantBuffer)
        return; // Already created

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(DepthParams);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(device->CreateBuffer(&desc, nullptr, &g_DepthConstantBuffer)))
    {
#if DEBUG
        std::cout << timeStamp() << "CreateBuffer(DepthConstantBuffer) failed!" << std::endl;
#endif
    }
}

// Update depth constant buffer with current parameters
inline void updateDepthConstantBuffer(ID3D11DeviceContext *ctx)
{
    if (!ctx || !g_DepthConstantBuffer)
        return;
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(ctx->Map(g_DepthConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        DepthParams *params = static_cast<DepthParams *>(mapped.pData);
        params->FAR_PLANE = g_DepthFarPlane;
        params->NEAR_PLANE = g_DepthNearPlane;
        params->GAMMA = g_DepthGamma;
        params->padding = 0.0f;
        ctx->Unmap(g_DepthConstantBuffer, 0);
    }
}

// Create or resize our "visible" RGBA texture & RTV/SRV for depth rendering
inline void ensureDepthVis(ID3D11Device *device, int w, int h)
{
    if (!device || w < 1 || h < 1)
        return;

    // Check if we need to recreate
    bool recreate = false;
    D3D11_TEXTURE2D_DESC cur{};
    if (!g_DepthVis)
        recreate = true;
    else
    {
        g_DepthVis->GetDesc(&cur);
        if (cur.Width != UINT(w) || cur.Height != UINT(h))
            recreate = true;
    }
    if (!recreate)
        return;

    // Release old
    if (g_DepthVisRTV)
    {
        g_DepthVisRTV->Release();
        g_DepthVisRTV = nullptr;
    }
    if (g_DepthVisSRV)
    {
        g_DepthVisSRV->Release();
        g_DepthVisSRV = nullptr;
    }
    if (g_DepthVis)
    {
        g_DepthVis->Release();
        g_DepthVis = nullptr;
    }

    // Create a standard RGBA UNORM texture to hold our grayscale output
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = w;
    desc.Height = h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // normal color
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    if (FAILED(device->CreateTexture2D(&desc, nullptr, &g_DepthVis)))
    {
#if DEBUG
        std::cout << timeStamp() << "CreateTexture2D(depthVis) failed!" << std::endl;
#endif
        return;
    }

    // RTV
    if (FAILED(device->CreateRenderTargetView(g_DepthVis, nullptr, &g_DepthVisRTV)))
    {
#if DEBUG
        std::cout << timeStamp() << "CreateRenderTargetView(depthVis) failed!" << std::endl;
#endif
    }

    // SRV for ImGui
    if (FAILED(device->CreateShaderResourceView(g_DepthVis, nullptr, &g_DepthVisSRV)))
    {
#if DEBUG
        std::cout << timeStamp() << "CreateShaderResourceView(depthVis) failed!" << std::endl;
#endif
    }
}

// Create or resize our "visible" RGBA texture & RTV/SRV for normal buffer rendering
inline void ensureNormalVis(ID3D11Device *device, int w, int h)
{
    if (!device || w < 1 || h < 1)
        return;

    // Check if we need to recreate
    bool recreate = false;
    D3D11_TEXTURE2D_DESC cur{};
    if (!g_NormalVis)
        recreate = true;
    else
    {
        g_NormalVis->GetDesc(&cur);
        if (cur.Width != UINT(w) || cur.Height != UINT(h))
            recreate = true;
    }
    if (!recreate)
        return;

    // Release old
    if (g_NormalVisRTV)
    {
        g_NormalVisRTV->Release();
        g_NormalVisRTV = nullptr;
    }
    if (g_NormalVisSRV)
    {
        g_NormalVisSRV->Release();
        g_NormalVisSRV = nullptr;
    }
    if (g_NormalVis)
    {
        g_NormalVis->Release();
        g_NormalVis = nullptr;
    }

    // Create a standard RGBA UNORM texture to hold our normal buffer output
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = w;
    desc.Height = h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // normal color
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    if (FAILED(device->CreateTexture2D(&desc, nullptr, &g_NormalVis)))
    {
#if DEBUG
        std::cout << timeStamp() << "CreateTexture2D(normalVis) failed!" << std::endl;
#endif
        return;
    }

    // RTV
    if (FAILED(device->CreateRenderTargetView(g_NormalVis, nullptr, &g_NormalVisRTV)))
    {
#if DEBUG
        std::cout << timeStamp() << "CreateRenderTargetView(normalVis) failed!" << std::endl;
#endif
    }

    // SRV for ImGui
    if (FAILED(device->CreateShaderResourceView(g_NormalVis, nullptr, &g_NormalVisSRV)))
    {
#if DEBUG
        std::cout << timeStamp() << "CreateShaderResourceView(normalVis) failed!" << std::endl;
#endif
    }
}
#endif

/* ------------------------------------------------------------------------
   ImGui helpers (local to this translation unit)                          */
#if ENABLE_IMGUI
static bool s_imguiInit = false; // one-time guard
static HWND s_hwnd = nullptr;
static WNDPROC s_origProc = nullptr;
static ID3D11RenderTargetView *g_Rtv = nullptr;

static bool showImGui = false;                                // toggled with <Home>
static bool vk_gFps_c = true;                                 // fps overlay
static ImVec4 fpsColor = ImVec4(1.0f, 1.0f, 0.784314f, 1.0f); // #fff7c8

// dxpipe version
#define VERSION "0.0.0"

// Forwarded input for ImGui + <Home> hot-key
static LRESULT CALLBACK WndProcHook(HWND hWnd, UINT msg,
                                    WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return TRUE;

    if (msg == WM_KEYDOWN && wParam == VK_HOME)
    {
        showImGui = !showImGui;
        return 0;
    }
    return CallWindowProc(s_origProc, hWnd, msg, wParam, lParam);
}
#endif

/* helper to print a texture’s basic info */
inline void printBufferDetails(const char *name, ID3D11Texture2D *tex)
{
    if (tex)
    {
        D3D11_TEXTURE2D_DESC d{};
        tex->GetDesc(&d);
#if DEBUG
        std::cout << timeStamp() << name << ": "
                  << d.Width << "x" << d.Height
                  << " format=" << std::hex << d.Format
                  << std::dec << std::endl;
#endif
    }
    else
    {
#if DEBUG
        std::cout << timeStamp() << name << ": nullptr" << std::endl;
#endif
    }
}

// IDXGISwapChain proxy wrapper
//  • logs Present / ResizeBuffers / GetBuffer(0)
//  • keeps CPU-readable staging copies
//  • renders ImGui in-place
class ProxySwapChain : public IDXGISwapChain2
{
public:
    explicit ProxySwapChain(IDXGISwapChain2 *real)
        : m_real(real), m_ref(1)
    {
        m_real->AddRef();
    }

    // -------- IUnknown ---------------------------------------------------
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) override
    {
        if (riid == __uuidof(IDXGISwapChain) ||
            riid == __uuidof(IDXGISwapChain1) ||
            riid == __uuidof(IDXGISwapChain2) ||
            riid == __uuidof(IUnknown))
        {
            *ppv = static_cast<IDXGISwapChain2 *>(this);
            AddRef();
            return S_OK;
        }
        return m_real->QueryInterface(riid, ppv);
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return static_cast<ULONG>(++m_ref); }
    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG c = static_cast<ULONG>(--m_ref);
        if (!c)
        {
            m_real->Release();
            delete this;
        }
        return c;
    }

    // -------- IDXGIObject ------------------------------------------------
    HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID g, UINT s, const void *d) override { return m_real->SetPrivateData(g, s, d); }
    HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID g, const IUnknown *u) override { return m_real->SetPrivateDataInterface(g, u); }
    HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID g, UINT *s, void *d) override { return m_real->GetPrivateData(g, s, d); }
    HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **pp) override { return m_real->GetParent(riid, pp); }

    // -------- IDXGIDeviceSubObject ---------------------------------------
    HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void **ppv) override
    {
        return m_real->GetDevice(riid, ppv);
    }

    // -------- IDXGISwapChain ---------------------------------------------
    HRESULT STDMETHODCALLTYPE Present(UINT si, UINT f) override
    {
#if DEBUG
        output();
#endif
#if DEBUG
// std::cout << timeStamp() << "IDXGISwapChain::Present" << std::endl;
#endif
#if DEBUG
// std::cout << timeStamp() << "Present(" << si << ", " << f << ")" << std::endl;
#endif

        // Update dimensions from back buffer if available
        updateDimensionsFromBackBuffer();

        /* direct access to global variables */
        // g_DepthTexture and g_Device are now directly accessible
        // get the real device from d3d11.cpp global variables
        ID3D11Device *realDevice = g_Device;

        // log current buffers
        // printBufferDetails("Back buffer", g_BackBufferTexture);
        // printBufferDetails("Depth buffer", g_DepthTexture);

        /* helper: create / resize CPU staging texture */
        auto ensureStaging = [&](ID3D11Texture2D *src, ID3D11Texture2D *&dst)
        {
            if (!src || !realDevice)
                return;

            D3D11_TEXTURE2D_DESC d{};
            src->GetDesc(&d);
            bool recreate = false;

            if (!dst)
                recreate = true;
            else
            {
                D3D11_TEXTURE2D_DESC cur{};
                dst->GetDesc(&cur);
                recreate = (cur.Width != d.Width) ||
                           (cur.Height != d.Height) ||
                           (cur.Format != d.Format);
            }

            if (!recreate)
                return;

            replaceGlobal(dst, nullptr);

            d.BindFlags = 0;
            d.Usage = D3D11_USAGE_STAGING;
            d.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            d.MiscFlags = 0;
            d.ArraySize = 1;
            d.MipLevels = 1;

            if (FAILED(realDevice->CreateTexture2D(&d, nullptr, &dst)))
            {
#if DEBUG
                std::cout << timeStamp() << "CreateTexture2D(STAGING) failed!" << std::endl;
#endif
            }
        };

        /* helper: create / resize GPU texture + SRV */
        auto ensureGPUTexture = [&](ID3D11Texture2D *src, ID3D11Texture2D *&dst, ID3D11ShaderResourceView *&srv)
        {
            if (!src || !realDevice)
                return;

            D3D11_TEXTURE2D_DESC d{};
            src->GetDesc(&d);
            bool recreate = false;

            if (!dst)
                recreate = true;
            else
            {
                D3D11_TEXTURE2D_DESC cur{};
                dst->GetDesc(&cur);
                recreate = (cur.Width != d.Width) ||
                           (cur.Height != d.Height) ||
                           (cur.Format != d.Format);
            }

            if (!recreate)
                return;

            /* release old texture + SRV */
            replaceGlobal(dst, nullptr);
            if (srv)
            {
                srv->Release();
                srv = nullptr;
            }

            d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            d.Usage = D3D11_USAGE_DEFAULT;
            d.CPUAccessFlags = 0;
            d.MiscFlags = 0;
            d.ArraySize = 1;
            d.MipLevels = 1;

            ID3D11Texture2D *tmp = nullptr;
            if (FAILED(realDevice->CreateTexture2D(&d, nullptr, &tmp)))
            {
#if DEBUG
                std::cout << timeStamp() << "CreateTexture2D(GPU) failed!" << std::endl;
#endif
                return;
            }
            replaceGlobal(dst, tmp);

            /* create SRV */
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            srvDesc.Format = (d.Format == DXGI_FORMAT_R32_TYPELESS)
                                 ? DXGI_FORMAT_R32_FLOAT
                                 : d.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = d.MipLevels;

            if (FAILED(realDevice->CreateShaderResourceView(dst, &srvDesc, &srv)))
            {
#if DEBUG
                std::cout << timeStamp() << "CreateShaderResourceView failed!" << std::endl;
#endif
            }
        };

        // immediate context once
        ID3D11DeviceContext *ctx = nullptr;
        if (realDevice)
            realDevice->GetImmediateContext(&ctx);

        /* ------------ colour staging ------------ */
        ensureStaging(g_BackBufferTexture, g_BackBufferStaging);
        if (ctx && g_BackBufferTexture && g_BackBufferStaging)
            ctx->CopyResource(g_BackBufferStaging, g_BackBufferTexture);

        /* ------------ depth staging  ------------ */
        ensureStaging(g_DepthTexture, g_DepthStaging);
        if (ctx && g_DepthTexture && g_DepthStaging)
            ctx->CopyResource(g_DepthStaging, g_DepthTexture);

#if ENABLE_IMGUI
        /* re-create GPU texture (default usage + SRV) for the new back buffer */
        ensureGPUTexture(g_BackBufferTexture, g_BackBufferGPU, g_BackBufferGPU_SRV);
        if (ctx && g_BackBufferTexture && g_BackBufferGPU)
            ctx->CopyResource(g_BackBufferGPU, g_BackBufferTexture); /* re-create GPU texture (default usage + SRV) for the depth buffer */
        ensureGPUTexture(g_DepthTexture, g_DepthGPU, g_DepthGPU_SRV);
        if (ctx && g_DepthTexture && g_DepthGPU)
            ctx->CopyResource(g_DepthGPU, g_DepthTexture);
#endif /* ------------ shared buffer management ------------ */
        // create/update shared back buffer if needed
        if (g_BackBufferTexture && realDevice)
        {
            D3D11_TEXTURE2D_DESC desc{};
            g_BackBufferTexture->GetDesc(&desc);

            // check if shared buffer needs recreation (size/format change or doesn't exist)
            bool needsRecreation = !g_BackBufferShared;
            if (g_BackBufferShared)
            {
                D3D11_TEXTURE2D_DESC sharedDesc{};
                g_BackBufferShared->GetDesc(&sharedDesc);
                if (sharedDesc.Width != desc.Width || sharedDesc.Height != desc.Height)
                {
#if DEBUG
                    std::cout << timeStamp() << "Back buffer size changed - recreating shared buffer\n";
#endif
                    needsRecreation = true;
                }
            }

            if (needsRecreation)
            {
                // release old shared buffer and handle
                if (g_BackBufferShared)
                {
#if DEBUG
                    std::cout << timeStamp() << "Releasing old back buffer shared texture\n";
#endif
                    g_BackBufferShared->Release();
                    g_BackBufferShared = nullptr;
                }
                g_BackBufferSharedHandle = nullptr;

// create new shared back buffer
#if DEBUG
                std::cout << timeStamp() << "Creating new shared back buffer (" << desc.Width << "x" << desc.Height << ")\n";
#endif
                createSharedBackBuffer(realDevice, desc.Width, desc.Height);
            }

            // copy current back buffer to shared buffer
            if (g_BackBufferShared && ctx)
                ctx->CopyResource(g_BackBufferShared, g_BackBufferTexture);
        }

        // create/update shared depth buffer if needed (prioritize depth buffer updates)
        if (g_DepthTexture && realDevice)
        {
            D3D11_TEXTURE2D_DESC desc{};
            g_DepthTexture->GetDesc(&desc);

            // check if shared buffer needs recreation (size/format change or doesn't exist)
            bool needsRecreation = !g_DepthShared;
            if (g_DepthShared)
            {
                D3D11_TEXTURE2D_DESC sharedDesc{};
                g_DepthShared->GetDesc(&sharedDesc);
                if (sharedDesc.Width != desc.Width || sharedDesc.Height != desc.Height)
                {
#if DEBUG
                    std::cout << timeStamp() << "Depth buffer size changed - recreating shared buffer\n";
#endif
                    needsRecreation = true;
                }
            }

            if (needsRecreation)
            {
                // release old shared buffer and handle
                if (g_DepthShared)
                {
#if DEBUG
                    std::cout << timeStamp() << "Releasing old depth buffer shared texture\n";
#endif
                    g_DepthShared->Release();
                    g_DepthShared = nullptr;
                }
                g_DepthSharedHandle = nullptr;

// create new shared depth buffer
#if DEBUG
                std::cout << timeStamp() << "Creating new shared depth buffer (" << desc.Width << "x" << desc.Height << ")\n";
#endif
                createSharedDepthBuffer(realDevice, desc.Width, desc.Height);
            }

            // copy current depth buffer to shared buffer
            if (g_DepthShared && ctx)
                ctx->CopyResource(g_DepthShared, g_DepthTexture);
        }

        /* ------------ handle duplication to client process ------------ */
        // attempt to duplicate handles to external client process
        duplicateHandleToClientProcess();

#if ENABLE_IMGUI
        // Compile depth shader and quad VS if needed
        if (realDevice)
        {
            compileDepthShader(realDevice);
            compileNormalShader(realDevice);
            compileQuadVS(realDevice);
            ensureDepthConstantBuffer(realDevice);
        }

        // If we have a valid depth SRV, do a small pass to fill g_DepthVis
        if (ctx && g_DepthPS && g_QuadVS && g_DepthGPU_SRV)
        {
            // Make sure our "vis" texture is allocated
            ensureDepthVis(realDevice, g_Width, g_Height);
            if (g_DepthVisRTV)
            {
                // Update depth parameters in constant buffer
                updateDepthConstantBuffer(ctx);

                // Save old pipeline
                ID3D11RenderTargetView *oldRTV = nullptr;
                ctx->OMGetRenderTargets(1, &oldRTV, nullptr);

                // Set ours
                ctx->OMSetRenderTargets(1, &g_DepthVisRTV, nullptr);

                // Optional: clear black
                float clr[4] = {0, 0, 0, 0};
                ctx->ClearRenderTargetView(g_DepthVisRTV, clr);

                // Setup minimal pipeline to draw a triangle
                ctx->PSSetShader(g_DepthPS, nullptr, 0);
                ctx->VSSetShader(g_QuadVS, nullptr, 0);

                // Bind constant buffer to pixel shader
                ctx->PSSetConstantBuffers(0, 1, &g_DepthConstantBuffer);

                // Provide the depth SRV
                ctx->PSSetShaderResources(0, 1, &g_DepthGPU_SRV);

                // No input layout needed if we rely on SV_VertexID inside the VS
                ctx->IASetInputLayout(nullptr);
                ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                // Draw 3 verts for fullscreen coverage
                ctx->Draw(3, 0);

                // Restore old RT
                ctx->OMSetRenderTargets(1, &oldRTV, nullptr);
                if (oldRTV)
                    oldRTV->Release();
            }
        }

        // If we have a valid depth SRV, do a small pass to fill g_NormalVis
        if (ctx && g_NormalPS && g_QuadVS && g_DepthGPU_SRV)
        {
            // Make sure our "normal vis" texture is allocated
            ensureNormalVis(realDevice, g_Width, g_Height);
            if (g_NormalVisRTV)
            {
                // Update depth parameters in constant buffer (same buffer as depth shader)
                updateDepthConstantBuffer(ctx);

                // Save old pipeline
                ID3D11RenderTargetView *oldRTV = nullptr;
                ctx->OMGetRenderTargets(1, &oldRTV, nullptr);

                // Set ours
                ctx->OMSetRenderTargets(1, &g_NormalVisRTV, nullptr);

                // Optional: clear black
                float clr[4] = {0, 0, 0, 0};
                ctx->ClearRenderTargetView(g_NormalVisRTV, clr);

                // Setup minimal pipeline to draw a triangle
                ctx->PSSetShader(g_NormalPS, nullptr, 0);
                ctx->VSSetShader(g_QuadVS, nullptr, 0);

                // Bind constant buffer to pixel shader
                ctx->PSSetConstantBuffers(0, 1, &g_DepthConstantBuffer);

                // Provide the depth SRV
                ctx->PSSetShaderResources(0, 1, &g_DepthGPU_SRV);

                // No input layout needed if we rely on SV_VertexID inside the VS
                ctx->IASetInputLayout(nullptr);
                ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                // Draw 3 verts for fullscreen coverage
                ctx->Draw(3, 0);

                // Restore old RT
                ctx->OMSetRenderTargets(1, &oldRTV, nullptr);
                if (oldRTV)
                    oldRTV->Release();
            }
        }
#endif /* ---------- ImGui one-time initialisation ---------- */
#if ENABLE_IMGUI
        if (!s_imguiInit && realDevice && ctx)
        {
            m_real->GetHwnd(&s_hwnd);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(s_hwnd);
            ImGui_ImplDX11_Init(realDevice, ctx);
            setImStyleTheme();

            // build RTV for back buffer
            ID3D11Texture2D *bb = nullptr;
            if (SUCCEEDED(m_real->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                            reinterpret_cast<void **>(&bb))) &&
                bb)
            {
                realDevice->CreateRenderTargetView(bb, nullptr, &g_Rtv);
                bb->Release();
            }

            // subclass window → ImGui + <Home> toggle
            s_origProc = (WNDPROC)SetWindowLongPtrW(
                s_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProcHook);

            s_imguiInit = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // create an interal imgui overlay that matches the back buffer size
        ////////////////////////////////////////////////////////////////////////

        if (s_imguiInit && g_BackBufferTexture)
        { // get back buffer size
            D3D11_TEXTURE2D_DESC d{};
            g_BackBufferTexture->GetDesc(&d);
            g_Width = int(d.Width);
            g_Height = int(d.Height);

            // set ImGui display size
            ImGuiIO &io = ImGui::GetIO();
            io.DisplaySize = ImVec2(float(g_Width), float(g_Height));

            // recreate overlay if back buffer is lost or size changes
            if (!g_Rtv || d.Width != g_Width || d.Height != g_Height)
            {
                if (g_Rtv)
                {
                    g_Rtv->Release();
                    g_Rtv = nullptr;
                }

                ID3D11Texture2D *bb = nullptr;
                if (SUCCEEDED(m_real->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&bb))) && bb)
                {
                    realDevice->CreateRenderTargetView(bb, nullptr, &g_Rtv);
                    bb->Release();
                }
            }

            if (g_Rtv && ctx)
            {
                // debug clear to black
                // float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f}; // black color
                // ctx->OMSetRenderTargets(1, &g_Rtv, nullptr);
                // ctx->ClearRenderTargetView(g_Rtv, clearColor);

                // render shader effects
            }
        }

        /* ---------- per-frame ImGui ----------------------- */
        if (s_imguiInit)
        {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            /* fps overlay ------------------------------------------------ */
            static auto prevTime = std::chrono::high_resolution_clock::now();
            static int counter = 0;
            static std::string gFps_c;

            if (vk_gFps_c)
            {
                auto cur = std::chrono::high_resolution_clock::now();
                double diff = std::chrono::duration<double>(cur - prevTime).count();
                ++counter;
                if (diff >= (1.0 / 30.0))
                {
                    gFps_c = std::to_string((1.0 / diff) * counter) + "fps / " +
                             std::to_string((diff / counter) * 1000) + "ms";
                    prevTime = cur;
                    counter = 0;
                }

                float windowHeight = 30.0f;
                ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x,
                                                windowHeight),
                                         ImGuiCond_Always);

                ImGuiStyle &style = ImGui::GetStyle();
                ImVec4 prevBg = style.Colors[ImGuiCol_WindowBg];
                float prevBorder = style.WindowBorderSize;
                style.Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0);
                style.WindowBorderSize = 0.0f;

                if (ImGui::Begin("DxPipeFps", nullptr,
                                 ImGuiWindowFlags_NoTitleBar |
                                     ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoInputs))
                {
                    ImVec2 ws = ImGui::GetWindowSize();
                    ImVec2 pos = ImGui::GetCursorPos();
                    float lh = ImGui::GetTextLineHeight();
                    pos.y += ((ws.y - pos.y) / lh - 1) * lh;
                    pos.x = ws.x - ImGui::CalcTextSize(gFps_c.c_str()).x - 10;
                    ImGui::SetCursorPos(pos);
                    ImGui::PushStyleColor(ImGuiCol_Text, fpsColor);
                    ImGui::Text("%s", gFps_c.c_str());
                    ImGui::PopStyleColor();
                    ImGui::End();
                } // restore defaults
                style.Colors[ImGuiCol_WindowBg] = prevBg;
                style.WindowBorderSize = prevBorder;
            } /* settings window ------------------------------------------- */
            if (showImGui)
            { // Set initial position to left edge of screen with 3px padding
                static bool firstOpen = true;
                if (firstOpen)
                {
                    ImGui::SetNextWindowPos(ImVec2(3, 3), ImGuiCond_Always);
                    // Default width of 760, full height minus 6px padding
                    ImGui::SetNextWindowSize(ImVec2(760, ImGui::GetIO().DisplaySize.y - 6), ImGuiCond_Always);
                    firstOpen = false;
                }

                // Track if window is being dragged
                static bool wasDragging = false;
                static ImVec2 dragStartPos = ImVec2(0, 0);
                static ImVec2 dragStartSize = ImVec2(0, 0);
                static bool showSnapIndicators = false;
                static bool wasSnapped = false;
                static int snapTarget = -1; // -1 = none, 0 = left, 1 = right, 2 = top, 3 = bottom, 4+ = corners

                // Draw snap zones when not dragging to indicate possible docking positions
                ImVec2 displaySize = ImGui::GetIO().DisplaySize;
                ImVec4 accent = ImGui::GetStyle().Colors[ImGuiCol_ResizeGrip];
                ImU32 snapZoneColor = IM_COL32(accent.x * 255, accent.y * 255, accent.z * 255, 40);

                // Draw snap zones at edges to indicate where windows can be snapped
                const float zoneSize = 30.0f;
                const float cornerSize = 60.0f;
                const float rounding = ImGui::GetStyle().WindowRounding;

                // Left zone
                ImGui::GetBackgroundDrawList()->AddRectFilled(
                    ImVec2(0, cornerSize),
                    ImVec2(zoneSize, displaySize.y - cornerSize),
                    snapZoneColor, rounding, ImDrawFlags_RoundCornersRight);

                // Right zone
                ImGui::GetBackgroundDrawList()->AddRectFilled(
                    ImVec2(displaySize.x - zoneSize, cornerSize),
                    ImVec2(displaySize.x, displaySize.y - cornerSize),
                    snapZoneColor, rounding, ImDrawFlags_RoundCornersLeft);

                // Top zone
                ImGui::GetBackgroundDrawList()->AddRectFilled(
                    ImVec2(cornerSize, 0),
                    ImVec2(displaySize.x - cornerSize, zoneSize),
                    snapZoneColor, rounding, ImDrawFlags_RoundCornersBottom);

                // Bottom zone
                ImGui::GetBackgroundDrawList()->AddRectFilled(
                    ImVec2(cornerSize, displaySize.y - zoneSize),
                    ImVec2(displaySize.x - cornerSize, displaySize.y),
                    snapZoneColor, rounding, ImDrawFlags_RoundCornersTop);

                // Corner zones - slightly darker to show they're special snap points
                ImU32 cornerColor = IM_COL32(accent.x * 255, accent.y * 255, accent.z * 255, 60);

                // Top-left corner
                ImGui::GetBackgroundDrawList()->AddRectFilled(
                    ImVec2(0, 0),
                    ImVec2(cornerSize, cornerSize),
                    cornerColor, rounding, ImDrawFlags_RoundCornersBottomRight);

                // Top-right corner
                ImGui::GetBackgroundDrawList()->AddRectFilled(
                    ImVec2(displaySize.x - cornerSize, 0),
                    ImVec2(displaySize.x, cornerSize),
                    cornerColor, rounding, ImDrawFlags_RoundCornersBottomLeft);

                // Bottom-left corner
                ImGui::GetBackgroundDrawList()->AddRectFilled(
                    ImVec2(0, displaySize.y - cornerSize),
                    ImVec2(cornerSize, displaySize.y),
                    cornerColor, rounding, ImDrawFlags_RoundCornersTopRight);

                // Bottom-right corner
                ImGui::GetBackgroundDrawList()->AddRectFilled(
                    ImVec2(displaySize.x - cornerSize, displaySize.y - cornerSize),
                    ImVec2(displaySize.x, displaySize.y),
                    cornerColor, rounding, ImDrawFlags_RoundCornersTopLeft);
                // Create a non-modal window with standard decorations
                if (ImGui::Begin("DxPipe Settings", nullptr))
                {
                    // Edge snapping functionality
                    ImVec2 windowPos = ImGui::GetWindowPos();
                    ImVec2 windowSize = ImGui::GetWindowSize();

                    // Define snap distance threshold (in pixels)
                    const float snapDistance = 15.0f; // Amount of pixels to trigger snap
                    const float edgePadding = 3.0f;   // Padding from screen edge when snapped
                    bool snapped = false;

                    // Detect dragging state
                    bool isDragging = ImGui::IsWindowFocused() && ImGui::IsMouseDragging(0) &&
                                      ImGui::IsMouseHoveringRect(ImVec2(windowPos.x, windowPos.y),
                                                                 ImVec2(windowPos.x + windowSize.x, windowPos.y + 30.0f));

                    // Handle drag start
                    if (isDragging && !wasDragging)
                    {
                        dragStartPos = windowPos;
                        dragStartSize = windowSize;
                        showSnapIndicators = true;
                    }
                    // Handle drag end - apply snap when mouse is released
                    if (!isDragging && wasDragging)
                    {
                        showSnapIndicators = false;

                        // Apply snapping based on stored target
                        if (snapTarget >= 0)
                        {
                            wasSnapped = true;

                            switch (snapTarget)
                            {
                            case 0: // Left edge
                                ImGui::SetWindowPos(ImVec2(edgePadding, windowPos.y));
                                if (ImGui::GetIO().MousePos.y < snapDistance * 3)
                                {
                                    // Top-left corner
                                    ImGui::SetWindowPos(ImVec2(edgePadding, edgePadding));
                                    ImGui::SetWindowSize(ImVec2(windowSize.x, displaySize.y - (edgePadding * 2)));
                                }
                                else if (ImGui::GetIO().MousePos.y > displaySize.y - snapDistance * 3)
                                {
                                    // Bottom-left corner
                                    ImGui::SetWindowPos(ImVec2(edgePadding, displaySize.y - windowSize.y - edgePadding));
                                    ImGui::SetWindowSize(ImVec2(windowSize.x, displaySize.y - (edgePadding * 2)));
                                }
                                else
                                {
                                    // Middle-left
                                    ImGui::SetWindowPos(ImVec2(edgePadding, windowPos.y));
                                    ImGui::SetWindowSize(ImVec2(windowSize.x, displaySize.y - (edgePadding * 2)));
                                }
                                break;

                            case 1: // Right edge
                                if (ImGui::GetIO().MousePos.y < snapDistance * 3)
                                {
                                    // Top-right corner
                                    ImGui::SetWindowPos(ImVec2(displaySize.x - windowSize.x - edgePadding, edgePadding));
                                    ImGui::SetWindowSize(ImVec2(windowSize.x, displaySize.y - (edgePadding * 2)));
                                }
                                else if (ImGui::GetIO().MousePos.y > displaySize.y - snapDistance * 3)
                                {
                                    // Bottom-right corner
                                    ImGui::SetWindowPos(ImVec2(displaySize.x - windowSize.x - edgePadding, displaySize.y - windowSize.y - edgePadding));
                                    ImGui::SetWindowSize(ImVec2(windowSize.x, displaySize.y - (edgePadding * 2)));
                                }
                                else
                                {
                                    // Middle-right
                                    ImGui::SetWindowPos(ImVec2(displaySize.x - windowSize.x - edgePadding, windowPos.y));
                                    ImGui::SetWindowSize(ImVec2(windowSize.x, displaySize.y - (edgePadding * 2)));
                                }
                                break;

                            case 2: // Top edge
                                if (ImGui::GetIO().MousePos.x > displaySize.x * 0.3f && ImGui::GetIO().MousePos.x < displaySize.x * 0.7f)
                                {
                                    // Center top
                                    ImGui::SetWindowPos(ImVec2(edgePadding, edgePadding));
                                    ImGui::SetWindowSize(ImVec2(displaySize.x - (edgePadding * 2), windowSize.y));
                                }
                                else
                                {
                                    // Normal top
                                    ImGui::SetWindowPos(ImVec2(windowPos.x, edgePadding));
                                }
                                break;

                            case 3: // Bottom edge
                                if (ImGui::GetIO().MousePos.x > displaySize.x * 0.3f && ImGui::GetIO().MousePos.x < displaySize.x * 0.7f)
                                {
                                    // Center bottom
                                    ImGui::SetWindowPos(ImVec2(edgePadding, displaySize.y - windowSize.y - edgePadding));
                                    ImGui::SetWindowSize(ImVec2(displaySize.x - (edgePadding * 2), windowSize.y));
                                }
                                else
                                {
                                    // Normal bottom
                                    ImGui::SetWindowPos(ImVec2(windowPos.x, displaySize.y - windowSize.y - edgePadding));
                                }
                                break;
                            }
                        }
                    }

                    wasDragging = isDragging; // Draw snap indicators when dragging
                    if (showSnapIndicators)
                    {
                        // Get theme accent color from style
                        ImVec4 accent = ImGui::GetStyle().Colors[ImGuiCol_ResizeGrip];
                        ImU32 accentColor = IM_COL32(accent.x * 255, accent.y * 255, accent.z * 255, 100);
                        ImU32 highlightColor = IM_COL32(accent.x * 255, accent.y * 255, accent.z * 255, 180);
                        ImU32 glowColor = IM_COL32(accent.x * 255, accent.y * 255, accent.z * 255, 60);

                        // Get mouse position to create dynamic indicators
                        ImVec2 mousePos = ImGui::GetIO().MousePos;

                        // Dynamic highlight based on mouse position - more stylish glow effect
                        float animRatio = ImFmod(ImGui::GetTime() * 2.0f, 6.28318f);
                        float glowIntensity = (ImSin(animRatio) * 0.5f) + 0.5f; // 0.0 to 1.0 pulsing

                        // Reset snap target when mouse moves to new zone
                        snapTarget = -1;

                        // Determine which edge is closest to mouse
                        bool nearLeftEdge = mousePos.x < snapDistance * 3;
                        bool nearRightEdge = mousePos.x > displaySize.x - snapDistance * 3;
                        bool nearTopEdge = mousePos.y < snapDistance * 3;
                        bool nearBottomEdge = mousePos.y > displaySize.y - snapDistance * 3;
                        // Left edge indicator with glow effect
                        if (nearLeftEdge)
                        {
                            // Set snap target for left edge
                            snapTarget = 0;

                            // Animated glow effect
                            float glowSize = 5.0f + (glowIntensity * 4.0f);

                            // Basic edge indicator
                            ImGui::GetForegroundDrawList()->AddRectFilled(
                                ImVec2(0, 0),
                                ImVec2(edgePadding + 2, displaySize.y),
                                accentColor);

                            // Add a glow effect
                            for (int i = 0; i < 8; i++)
                            {
                                float alpha = 120 - (i * 15);
                                if (alpha < 0)
                                    alpha = 0;
                                ImGui::GetForegroundDrawList()->AddRectFilled(
                                    ImVec2(-glowSize + (i * 1.0f), 0),
                                    ImVec2(edgePadding + 2 + glowSize - (i * 1.0f), displaySize.y),
                                    IM_COL32(accent.x * 255, accent.y * 255, accent.z * 255, (int)(alpha * glowIntensity)));
                            }

                            // Show full-height outline for left edge docking
                            ImGui::GetForegroundDrawList()->AddRect(
                                ImVec2(edgePadding, edgePadding),
                                ImVec2(windowSize.x + edgePadding, displaySize.y - edgePadding),
                                highlightColor,
                                ImGui::GetStyle().WindowRounding);
                        }
                        // Right edge indicator with glow effect
                        if (nearRightEdge)
                        {
                            // Set snap target for right edge
                            snapTarget = 1;

                            // Animated glow effect
                            float glowSize = 5.0f + (glowIntensity * 4.0f);

                            // Basic edge indicator
                            ImGui::GetForegroundDrawList()->AddRectFilled(
                                ImVec2(displaySize.x - edgePadding - 2, 0),
                                ImVec2(displaySize.x, displaySize.y),
                                accentColor);

                            // Add a glow effect
                            for (int i = 0; i < 8; i++)
                            {
                                float alpha = 120 - (i * 15);
                                if (alpha < 0)
                                    alpha = 0;
                                ImGui::GetForegroundDrawList()->AddRectFilled(
                                    ImVec2(displaySize.x - edgePadding - 2 - glowSize + (i * 1.0f), 0),
                                    ImVec2(displaySize.x + glowSize - (i * 1.0f), displaySize.y),
                                    IM_COL32(accent.x * 255, accent.y * 255, accent.z * 255, (int)(alpha * glowIntensity)));
                            }

                            // Show full-height outline for right edge docking
                            ImGui::GetForegroundDrawList()->AddRect(
                                ImVec2(displaySize.x - windowSize.x - edgePadding, edgePadding),
                                ImVec2(displaySize.x - edgePadding, displaySize.y - edgePadding),
                                highlightColor,
                                ImGui::GetStyle().WindowRounding);
                        }
                        // Top edge indicator with glow effect
                        if (nearTopEdge)
                        {
                            // Set snap target for top edge
                            snapTarget = 2;

                            // Animated glow effect
                            float glowSize = 5.0f + (glowIntensity * 4.0f);

                            // Basic edge indicator
                            ImGui::GetForegroundDrawList()->AddRectFilled(
                                ImVec2(0, 0),
                                ImVec2(displaySize.x, edgePadding + 2),
                                accentColor);

                            // Add a glow effect
                            for (int i = 0; i < 8; i++)
                            {
                                float alpha = 120 - (i * 15);
                                if (alpha < 0)
                                    alpha = 0;
                                ImGui::GetForegroundDrawList()->AddRectFilled(
                                    ImVec2(0, -glowSize + (i * 1.0f)),
                                    ImVec2(displaySize.x, edgePadding + 2 + glowSize - (i * 1.0f)),
                                    IM_COL32(accent.x * 255, accent.y * 255, accent.z * 255, (int)(alpha * glowIntensity)));
                            }

                            // Show full-width outline for top edge docking
                            ImGui::GetForegroundDrawList()->AddRect(
                                ImVec2(edgePadding, edgePadding),
                                ImVec2(displaySize.x - edgePadding, windowSize.y + edgePadding),
                                highlightColor,
                                ImGui::GetStyle().WindowRounding);
                        }
                        // Bottom edge indicator with glow effect
                        if (nearBottomEdge)
                        {
                            // Set snap target for bottom edge
                            snapTarget = 3;

                            // Animated glow effect
                            float glowSize = 5.0f + (glowIntensity * 4.0f);

                            // Basic edge indicator
                            ImGui::GetForegroundDrawList()->AddRectFilled(
                                ImVec2(0, displaySize.y - edgePadding - 2),
                                ImVec2(displaySize.x, displaySize.y),
                                accentColor);

                            // Add a glow effect
                            for (int i = 0; i < 8; i++)
                            {
                                float alpha = 120 - (i * 15);
                                if (alpha < 0)
                                    alpha = 0;
                                ImGui::GetForegroundDrawList()->AddRectFilled(
                                    ImVec2(0, displaySize.y - edgePadding - 2 - glowSize + (i * 1.0f)),
                                    ImVec2(displaySize.x, displaySize.y + glowSize - (i * 1.0f)),
                                    IM_COL32(accent.x * 255, accent.y * 255, accent.z * 255, (int)(alpha * glowIntensity)));
                            }

                            // Show full-width outline for bottom edge docking
                            ImGui::GetForegroundDrawList()->AddRect(
                                ImVec2(edgePadding, displaySize.y - windowSize.y - edgePadding),
                                ImVec2(displaySize.x - edgePadding, displaySize.y - edgePadding),
                                highlightColor,
                                ImGui::GetStyle().WindowRounding);
                        }

                        // Show corner indicators with pulsing effect
                        // Top-left corner
                        if (nearLeftEdge && nearTopEdge)
                        {
                            ImGui::GetForegroundDrawList()->AddRectFilled(
                                ImVec2(0, 0),
                                ImVec2(snapDistance, snapDistance),
                                IM_COL32(accent.x * 255, accent.y * 255, accent.z * 255, (int)(180 * glowIntensity)));
                        }

                        // Top-right corner
                        if (nearRightEdge && nearTopEdge)
                        {
                            ImGui::GetForegroundDrawList()->AddRectFilled(
                                ImVec2(displaySize.x - snapDistance, 0),
                                ImVec2(displaySize.x, snapDistance),
                                IM_COL32(accent.x * 255, accent.y * 255, accent.z * 255, (int)(180 * glowIntensity)));
                        }

                        // Bottom-left corner
                        if (nearLeftEdge && nearBottomEdge)
                        {
                            ImGui::GetForegroundDrawList()->AddRectFilled(
                                ImVec2(0, displaySize.y - snapDistance),
                                ImVec2(snapDistance, displaySize.y),
                                IM_COL32(accent.x * 255, accent.y * 255, accent.z * 255, (int)(180 * glowIntensity)));
                        }

                        // Bottom-right corner
                        if (nearRightEdge && nearBottomEdge)
                        {
                            ImGui::GetForegroundDrawList()->AddRectFilled(
                                ImVec2(displaySize.x - snapDistance, displaySize.y - snapDistance),
                                ImVec2(displaySize.x, displaySize.y),
                                IM_COL32(accent.x * 255, accent.y * 255, accent.z * 255, (int)(180 * glowIntensity)));
                        }
                    }
                    // Check for left edge snap - requires close proximity to left edge specifically
                    if (windowPos.x < snapDistance && ImGui::GetIO().MousePos.x < snapDistance * 3)
                    {
                        ImGui::SetWindowPos(ImVec2(edgePadding, windowPos.y));

                        // When near left edge, also check if we need to resize vertically
                        if (windowPos.y < snapDistance && ImGui::GetIO().MousePos.y < snapDistance * 3)
                        {
                            // Snap to top-left corner
                            ImGui::SetWindowPos(ImVec2(edgePadding, edgePadding));
                            ImGui::SetWindowSize(ImVec2(windowSize.x, displaySize.y - (edgePadding * 2)));
                            snapped = true;
                        }
                        else if (windowPos.y + windowSize.y > displaySize.y - snapDistance &&
                                 ImGui::GetIO().MousePos.y > displaySize.y - snapDistance * 3)
                        {
                            // Snap to bottom-left corner
                            ImGui::SetWindowPos(ImVec2(edgePadding, displaySize.y - windowSize.y - edgePadding));
                            ImGui::SetWindowSize(ImVec2(windowSize.x, displaySize.y - (edgePadding * 2)));
                            snapped = true;
                        }
                    }
                    // Check for right edge snap - requires close proximity to right edge specifically
                    if (!snapped && windowPos.x + windowSize.x > displaySize.x - snapDistance &&
                        ImGui::GetIO().MousePos.x > displaySize.x - snapDistance * 3)
                    {
                        ImGui::SetWindowPos(ImVec2(displaySize.x - windowSize.x - edgePadding, windowPos.y));

                        // When near right edge, also check if we need to resize vertically
                        if (windowPos.y < snapDistance && ImGui::GetIO().MousePos.y < snapDistance * 3)
                        {
                            // Snap to top-right corner
                            ImGui::SetWindowPos(ImVec2(displaySize.x - windowSize.x - edgePadding, edgePadding));
                            ImGui::SetWindowSize(ImVec2(windowSize.x, displaySize.y - (edgePadding * 2)));
                            snapped = true;
                        }
                        else if (windowPos.y + windowSize.y > displaySize.y - snapDistance &&
                                 ImGui::GetIO().MousePos.y > displaySize.y - snapDistance * 3)
                        {
                            // Snap to bottom-right corner
                            ImGui::SetWindowPos(ImVec2(displaySize.x - windowSize.x - edgePadding, displaySize.y - windowSize.y - edgePadding));
                            ImGui::SetWindowSize(ImVec2(windowSize.x, displaySize.y - (edgePadding * 2)));
                            snapped = true;
                        }
                    }
                    // Check for top edge snap (if not already snapped to a corner)
                    if (!snapped && windowPos.y < snapDistance && ImGui::GetIO().MousePos.y < snapDistance * 3)
                    {
                        ImGui::SetWindowPos(ImVec2(windowPos.x, edgePadding));

                        // If we're at the top center region, consider full width
                        if (windowPos.x > displaySize.x * 0.3f && windowPos.x + windowSize.x < displaySize.x * 0.7f &&
                            ImGui::GetIO().MousePos.x > displaySize.x * 0.3f && ImGui::GetIO().MousePos.x < displaySize.x * 0.7f)
                        {
                            ImGui::SetWindowPos(ImVec2(edgePadding, edgePadding));
                            ImGui::SetWindowSize(ImVec2(displaySize.x - (edgePadding * 2), windowSize.y));
                            snapped = true;
                        }
                    }

                    // Check for bottom edge snap (if not already snapped to a corner)
                    if (!snapped && windowPos.y + windowSize.y > displaySize.y - snapDistance &&
                        ImGui::GetIO().MousePos.y > displaySize.y - snapDistance * 3)
                    {
                        ImGui::SetWindowPos(ImVec2(windowPos.x, displaySize.y - windowSize.y - edgePadding));

                        // If we're at the bottom center region, consider full width
                        if (windowPos.x > displaySize.x * 0.3f && windowPos.x + windowSize.x < displaySize.x * 0.7f &&
                            ImGui::GetIO().MousePos.x > displaySize.x * 0.3f && ImGui::GetIO().MousePos.x < displaySize.x * 0.7f)
                        {
                            ImGui::SetWindowPos(ImVec2(edgePadding, displaySize.y - windowSize.y - edgePadding));
                            ImGui::SetWindowSize(ImVec2(displaySize.x - (edgePadding * 2), windowSize.y));
                            snapped = true;
                        }
                    }
                    if (ImGui::BeginTabBar("Tabs"))
                    {
                        // New RTV Tab combining Back Buffer, Depth View, and Normal Buffer
                        if (ImGui::BeginTabItem("RTV"))
                        {
                            // --- Back Buffer View Section ---
                            ImGui::Text("Back Buffer View");
                            ImGui::Separator();

                            if (g_BackBufferGPU_SRV)
                            {
                                ImGui::Text("Back Buffer:");
                                // buffer width / height
                                ImGui::SameLine();
                                ImGui::Text("%dx%d", g_Width, g_Height);
                                // back buffer
                                D3D11_TEXTURE2D_DESC d_bb{}; // Renamed variable
                                g_BackBufferTexture->GetDesc(&d_bb);
                                ImGui::Text("Format: %08X", d_bb.Format);

                                // image scaling and positioning for back buffer
                                float horizontalEdgePadding_BB = 2.0f;
                                ImVec2 contentRegionAvail_BB = ImGui::GetContentRegionAvail();

                                float displayableWidth_BB = contentRegionAvail_BB.x - (2.0f * horizontalEdgePadding_BB);
                                // Height allocation: 30% for back buffer
                                float displayableHeight_BB = contentRegionAvail_BB.y * 0.30f;

                                float imageOriginalW_BB = float(g_Width);
                                float imageOriginalH_BB = float(g_Height);
                                float finalImageW_BB = 0.0f;
                                float finalImageH_BB = 0.0f;

                                if (imageOriginalW_BB > 0 && imageOriginalH_BB > 0 && displayableWidth_BB > 0 && displayableHeight_BB > 0)
                                {
                                    float aspectRatio_BB = imageOriginalW_BB / imageOriginalH_BB;
                                    finalImageW_BB = displayableWidth_BB;
                                    finalImageH_BB = finalImageW_BB / aspectRatio_BB;
                                    if (finalImageH_BB > displayableHeight_BB)
                                    {
                                        finalImageH_BB = displayableHeight_BB;
                                        finalImageW_BB = finalImageH_BB * aspectRatio_BB;
                                    }
                                }

                                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + horizontalEdgePadding_BB);
                                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
                                if (ImGui::BeginChild("BackBufferImageFrame", ImVec2(finalImageW_BB, finalImageH_BB), true, ImGuiWindowFlags_None))
                                {
                                    ImGui::Image((ImTextureID)g_BackBufferGPU_SRV, ImGui::GetContentRegionAvail());
                                }
                                ImGui::EndChild();
                                ImGui::PopStyleVar(2);
                            }
                            else
                            {
                                ImGui::Text("Back buffer not available!");
                            }

                            ImGui::Spacing();   // Add some vertical space
                            ImGui::Separator(); // Visually separate the two views
                            ImGui::Spacing();   // Add some vertical space

                            // --- Depth Buffer View Section ---
                            ImGui::Text("Depth Buffer View");
                            ImGui::Separator();

                            if (g_DepthVisSRV)
                            {
                                D3D11_TEXTURE2D_DESC d_depth{}; // Renamed variable
                                g_DepthTexture->GetDesc(&d_depth);
                                ImGui::Text("Depth Buffer:");
                                ImGui::SameLine();
                                ImGui::Text("%dx%d", d_depth.Width, d_depth.Height);
                                ImGui::Text("Format: %08X", d_depth.Format);

                                // image scaling and positioning for Depth Buffer
                                float horizontalEdgePadding_D = 2.0f;
                                ImVec2 contentRegionAvail_D = ImGui::GetContentRegionAvail();

                                float displayableWidth_D = contentRegionAvail_D.x - (2.0f * horizontalEdgePadding_D);
                                // Height allocation: 35% for depth buffer
                                float displayableHeight_D = contentRegionAvail_D.y * 0.50f;

                                float imageOriginalW_D = float(d_depth.Width);
                                float imageOriginalH_D = float(d_depth.Height);
                                float finalImageW_D = 0.0f;
                                float finalImageH_D = 0.0f;

                                if (imageOriginalW_D > 0 && imageOriginalH_D > 0 && displayableWidth_D > 0 && displayableHeight_D > 0)
                                {
                                    float aspectRatio_D = imageOriginalW_D / imageOriginalH_D;
                                    finalImageW_D = displayableWidth_D;
                                    finalImageH_D = finalImageW_D / aspectRatio_D;
                                    if (finalImageH_D > displayableHeight_D)
                                    {
                                        finalImageH_D = displayableHeight_D;
                                        finalImageW_D = finalImageH_D * aspectRatio_D;
                                    }
                                }

                                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + horizontalEdgePadding_D);
                                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
                                if (ImGui::BeginChild("DepthImageFrame", ImVec2(finalImageW_D, finalImageH_D), true, ImGuiWindowFlags_None))
                                {
                                    ImGui::Image((ImTextureID)g_DepthVisSRV, ImGui::GetContentRegionAvail());
                                }
                                ImGui::EndChild();
                                ImGui::PopStyleVar(2);
                            }
                            else
                            {
                                ImGui::Text("Depth buffer not available!");
                            }

                            ImGui::Spacing();   // Add some vertical space
                            ImGui::Separator(); // Visually separate the views
                            ImGui::Spacing();   // Add some vertical space

                            // --- Normal Buffer View Section ---
                            ImGui::Text("Normal Buffer View");
                            ImGui::Separator();

                            if (g_NormalVisSRV)
                            {
                                D3D11_TEXTURE2D_DESC d_normal{};
                                g_DepthTexture->GetDesc(&d_normal); // Normal buffer uses same size as depth
                                ImGui::Text("Normal Buffer:");
                                ImGui::SameLine();
                                ImGui::Text("%dx%d", d_normal.Width, d_normal.Height);
                                ImGui::Text("Format: RGBA8_UNORM (derived from depth)");

                                // image scaling and positioning for Normal Buffer
                                float horizontalEdgePadding_N = 2.0f;
                                ImVec2 contentRegionAvail_N = ImGui::GetContentRegionAvail();

                                float displayableWidth_N = contentRegionAvail_N.x - (2.0f * horizontalEdgePadding_N);
                                // Use remaining available height for normal buffer
                                float displayableHeight_N = contentRegionAvail_N.y;

                                float imageOriginalW_N = float(d_normal.Width);
                                float imageOriginalH_N = float(d_normal.Height);
                                float finalImageW_N = 0.0f;
                                float finalImageH_N = 0.0f;

                                if (imageOriginalW_N > 0 && imageOriginalH_N > 0 && displayableWidth_N > 0 && displayableHeight_N > 0)
                                {
                                    float aspectRatio_N = imageOriginalW_N / imageOriginalH_N;
                                    finalImageW_N = displayableWidth_N;
                                    finalImageH_N = finalImageW_N / aspectRatio_N;
                                    if (finalImageH_N > displayableHeight_N)
                                    {
                                        finalImageH_N = displayableHeight_N;
                                        finalImageW_N = finalImageH_N * aspectRatio_N;
                                    }
                                }

                                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + horizontalEdgePadding_N);
                                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
                                if (ImGui::BeginChild("NormalImageFrame", ImVec2(finalImageW_N, finalImageH_N), true, ImGuiWindowFlags_None))
                                {
                                    ImGui::Image((ImTextureID)g_NormalVisSRV, ImGui::GetContentRegionAvail());
                                }
                                ImGui::EndChild();
                                ImGui::PopStyleVar(2);
                            }
                            else
                            {
                                ImGui::Text("Normal buffer not available!");
                            }

                            // Add a small section showing shared depth shader controls that affect both depth and normal views
                            if (g_DepthVisSRV || g_NormalVisSRV)
                            {
                                ImGui::Spacing();
                                ImGui::Text("Depth Shader Controls (affects both depth and normal views)");
                                ImGui::Separator();

                                // Far Plane slider (completely independent)
                                ImGui::SliderFloat("Far Plane", &g_DepthFarPlane, 1.1f, 3000.0f, "%.1f");

                                // Near Plane slider (completely independent)
                                ImGui::SliderFloat("Near Plane", &g_DepthNearPlane, 1.1f, 1000.0f, "%.1f");

                                // Gamma slider (affects depth view only)
                                ImGui::SliderFloat("Gamma (depth only)", &g_DepthGamma, 0.1f, 3.0f, "%.2f");
                                ImGui::Spacing();

                                // Reset button
                                if (ImGui::Button("Reset to Defaults"))
                                {
                                    g_DepthFarPlane = 2000.0f;
                                    g_DepthNearPlane = 25.0f;
                                    g_DepthGamma = 0.5f;
                                }

                                ImGui::SameLine();

                                // Dump Buffers button
                                if (ImGui::Button("Dump Buffers"))
                                {
                                    // Get module directory
                                    char modulePath[MAX_PATH];
                                    HMODULE hModule = GetModuleHandle(NULL);
                                    GetModuleFileNameA(hModule, modulePath, MAX_PATH);
                                    std::string moduleDir = std::string(modulePath);
                                    size_t lastSlash = moduleDir.find_last_of("\\/");
                                    if (lastSlash != std::string::npos)
                                    {
                                        moduleDir = moduleDir.substr(0, lastSlash + 1);
                                    }

                                    // Generate timestamp for unique filenames
                                    auto now = std::chrono::system_clock::now();
                                    auto time_t = std::chrono::system_clock::to_time_t(now);
                                    std::stringstream ss;
                                    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
                                    std::string timestamp = ss.str();

                                    // Dump back buffer
                                    if (g_BackBufferStaging && ctx)
                                    {
                                        D3D11_MAPPED_SUBRESOURCE mapped;
                                        if (SUCCEEDED(ctx->Map(g_BackBufferStaging, 0, D3D11_MAP_READ, 0, &mapped)))
                                        {
                                            D3D11_TEXTURE2D_DESC desc;
                                            g_BackBufferStaging->GetDesc(&desc);

                                            std::string filename = moduleDir + "backbuffer_" + timestamp + ".bin";
                                            std::ofstream file(filename, std::ios::binary);
                                            if (file.is_open())
                                            {
                                                file.write(static_cast<const char *>(mapped.pData), mapped.RowPitch * desc.Height);
                                                file.close();
#if DEBUG
                                                std::cout << timeStamp() << "Back buffer dumped to: " << filename << std::endl;
#endif
                                            }
                                            ctx->Unmap(g_BackBufferStaging, 0);
                                        }
                                    }

                                    // Dump depth buffer
                                    if (g_DepthStaging && ctx)
                                    {
                                        D3D11_MAPPED_SUBRESOURCE mapped;
                                        if (SUCCEEDED(ctx->Map(g_DepthStaging, 0, D3D11_MAP_READ, 0, &mapped)))
                                        {
                                            D3D11_TEXTURE2D_DESC desc;
                                            g_DepthStaging->GetDesc(&desc);

                                            std::string filename = moduleDir + "depthbuffer_" + timestamp + ".bin";
                                            std::ofstream file(filename, std::ios::binary);
                                            if (file.is_open())
                                            {
                                                file.write(static_cast<const char *>(mapped.pData), mapped.RowPitch * desc.Height);
                                                file.close();
#if DEBUG
                                                std::cout << timeStamp() << "Depth buffer dumped to: " << filename << std::endl;
#endif
                                            }
                                            ctx->Unmap(g_DepthStaging, 0);
                                        }
                                    }
                                }

                                ImGui::Spacing();

                                // Show current values
                                ImGui::Text("Current Values:");
                                ImGui::Text("Near: %.1f | Far: %.1f | Gamma: %.2f", g_DepthNearPlane, g_DepthFarPlane, g_DepthGamma);

                                // Show warning if values might cause issues (but don't auto-correct)
                                if (g_DepthNearPlane > g_DepthFarPlane)
                                {
                                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f)); // Yellow warning
                                    ImGui::Text("Warning: Near plane > Far plane may cause unexpected results");
                                    ImGui::PopStyleColor();
                                }
                            }
                            ImGui::EndTabItem(); // End RTV Tab
                        } // general settings
                        if (ImGui::BeginTabItem("Settings"))
                        {
                            ImGui::Text("General Settings");
                            ImGui::Separator();
                            ImGui::Spacing();

                            ImGui::Text("DxPipe Version");
                            ImGui::Separator();
                            ImGui::Text("%s", VERSION);
                            ImGui::Spacing();

                            ImGui::Text("FPS Counter Settings");
                            ImGui::Separator();
                            ImGui::Checkbox("Show FPS Counter", &vk_gFps_c);
                            ImGui::ColorEdit3("FPS Counter Colour", (float *)&fpsColor);
                            ImGui::Spacing();

                            ImGui::EndTabItem();
                        }

                        ImGui::EndTabBar();
                    }
                    ImGui::End();
                }
            } /* render ImGui ---------------------------------------------- */
            ImGui::Render();

            // RTV lost? recreate (e.g., after ResizeBuffers)
            if (!g_Rtv && realDevice)
            {
                ID3D11Texture2D *bb = nullptr;
                if (SUCCEEDED(m_real->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                                reinterpret_cast<void **>(&bb))) &&
                    bb)
                {
                    realDevice->CreateRenderTargetView(bb, nullptr, &g_Rtv);
                    bb->Release();
                }
            }

            if (g_Rtv)
                ctx->OMSetRenderTargets(1, &g_Rtv, nullptr);

            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }
#endif

        /* ------------ release & present ------------------- */
        if (ctx)
            ctx->Release();
        return m_real->Present(si, f);
    }

    // capture colour RT on GetBuffer(0)
    HRESULT STDMETHODCALLTYPE GetBuffer(UINT i, REFIID riid, void **ppv) override
    {
        HRESULT hr = m_real->GetBuffer(i, riid, ppv);
        if (SUCCEEDED(hr) && i == 0 && ppv && *ppv &&
            riid == __uuidof(ID3D11Texture2D))
        {
#if DEBUG
            output();
#endif
#if DEBUG
            std::cout << timeStamp()
                      << "IDXGISwapChain::GetBuffer(0) → back-buffer"
                      << std::endl;
#endif
            replaceGlobal(g_BackBufferTexture,
                          reinterpret_cast<ID3D11Texture2D *>(*ppv));

            // Update dimensions from the back buffer texture
            updateDimensionsFromBackBuffer();
        }
        return hr;
    } // intercepted – clear staging + RTV on resize
    HRESULT STDMETHODCALLTYPE ResizeBuffers(UINT n, UINT w, UINT h,
                                            DXGI_FORMAT fmt, UINT fl) override
    {
#if DEBUG
        output();
#endif
#if DEBUG
        std::cout << timeStamp() << "IDXGISwapChain::ResizeBuffers → "
                  << w << "x" << h << std::endl;
#endif

        // Update global dimensions from resize parameters
        g_Width = w;
        g_Height = h;
#if DEBUG
        std::cout << timeStamp() << "Updated dimensions from ResizeBuffers: "
                  << g_Width << "x" << g_Height << std::endl;
#endif

// print the g_BackBufferTexture address
#if DEBUG
        std::cout << timeStamp() << "g_BackBufferTexture: "
                  << g_BackBufferTexture << std::endl;
#endif

        /* release colour */
        replaceGlobal(g_BackBufferTexture, nullptr);
        replaceGlobal(g_BackBufferStaging, nullptr); /* depth – staging is released elsewhere, shared copy handled here */

        /* release RTV so it is recreated on next Present */
#if ENABLE_IMGUI
        if (g_Rtv)
            g_Rtv->Release(), g_Rtv = nullptr;

        /* release GPU texture + SRV */
        replaceGlobal(g_BackBufferGPU, nullptr);
        if (g_BackBufferGPU_SRV)
            g_BackBufferGPU_SRV->Release(), g_BackBufferGPU_SRV = nullptr;

        replaceGlobal(g_DepthGPU, nullptr);
        if (g_DepthGPU_SRV)
            g_DepthGPU_SRV->Release(), g_DepthGPU_SRV = nullptr;
#endif

        /* release shared buffers and handles - they will be recreated */
        if (g_BackBufferShared)
        {
            g_BackBufferShared->Release();
            g_BackBufferShared = nullptr;
        }
        g_BackBufferSharedHandle = nullptr;

        if (g_DepthShared)
        {
            g_DepthShared->Release();
            g_DepthShared = nullptr;
        }
        g_DepthSharedHandle = nullptr;

#if ENABLE_IMGUI
        /* release depth visualization resources */
        if (g_DepthVisRTV)
        {
            g_DepthVisRTV->Release();
            g_DepthVisRTV = nullptr;
        }
        if (g_DepthVisSRV)
        {
            g_DepthVisSRV->Release();
            g_DepthVisSRV = nullptr;
        }
        if (g_DepthVis)
        {
            g_DepthVis->Release();
            g_DepthVis = nullptr;
        }

        /* release normal visualization resources */
        if (g_NormalVisRTV)
        {
            g_NormalVisRTV->Release();
            g_NormalVisRTV = nullptr;
        }
        if (g_NormalVisSRV)
        {
            g_NormalVisSRV->Release();
            g_NormalVisSRV = nullptr;
        }
        if (g_NormalVis)
        {
            g_NormalVis->Release();
            g_NormalVis = nullptr;
        }
#endif

        HRESULT hr = m_real->ResizeBuffers(n, w, h, fmt, fl);

        if (SUCCEEDED(hr) && w && h)
        {
            g_Width = int(w);
            g_Height = int(h);
        }

        // reacquire new back buffer
        if (SUCCEEDED(hr))
        {
            ID3D11Texture2D *tmp = nullptr;
            if (SUCCEEDED(m_real->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                            reinterpret_cast<void **>(&tmp))) &&
                tmp)
            {
                replaceGlobal(g_BackBufferTexture, tmp);
                tmp->Release();
            }
        }
        return hr;
    }

    /* -------- boiler-plate forwards unchanged (IDXGISwapChain1 / 2) ----- */
    HRESULT STDMETHODCALLTYPE SetFullscreenState(BOOL f, IDXGIOutput *o) override { return m_real->SetFullscreenState(f, o); }
    HRESULT STDMETHODCALLTYPE GetFullscreenState(BOOL *f, IDXGIOutput **o) override { return m_real->GetFullscreenState(f, o); }
    HRESULT STDMETHODCALLTYPE GetDesc(DXGI_SWAP_CHAIN_DESC *d) override { return m_real->GetDesc(d); }
    HRESULT STDMETHODCALLTYPE ResizeTarget(const DXGI_MODE_DESC *m) override { return m_real->ResizeTarget(m); }
    HRESULT STDMETHODCALLTYPE GetContainingOutput(IDXGIOutput **o) override { return m_real->GetContainingOutput(o); }
    HRESULT STDMETHODCALLTYPE GetFrameStatistics(DXGI_FRAME_STATISTICS *s) override { return m_real->GetFrameStatistics(s); }
    HRESULT STDMETHODCALLTYPE GetLastPresentCount(UINT *c) override { return m_real->GetLastPresentCount(c); }

    HRESULT STDMETHODCALLTYPE GetDesc1(DXGI_SWAP_CHAIN_DESC1 *d) override { return m_real->GetDesc1(d); }
    HRESULT STDMETHODCALLTYPE GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC *d) override { return m_real->GetFullscreenDesc(d); }
    HRESULT STDMETHODCALLTYPE GetHwnd(HWND *hwnd) override { return m_real->GetHwnd(hwnd); }
    HRESULT STDMETHODCALLTYPE GetCoreWindow(REFIID riid, void **ppv) override { return m_real->GetCoreWindow(riid, ppv); }
    HRESULT STDMETHODCALLTYPE Present1(UINT si, UINT f, const DXGI_PRESENT_PARAMETERS *p) override
    {
#if DEBUG
        output();
#endif
#if DEBUG
        std::cout << timeStamp() << "IDXGISwapChain1::Present1" << std::endl;
#endif
        return m_real->Present1(si, f, p);
    }
    BOOL STDMETHODCALLTYPE IsTemporaryMonoSupported() override { return m_real->IsTemporaryMonoSupported(); }
    HRESULT STDMETHODCALLTYPE GetRestrictToOutput(IDXGIOutput **o) override { return m_real->GetRestrictToOutput(o); }
    HRESULT STDMETHODCALLTYPE SetBackgroundColor(const DXGI_RGBA *c) override { return m_real->SetBackgroundColor(c); }
    HRESULT STDMETHODCALLTYPE GetBackgroundColor(DXGI_RGBA *c) override { return m_real->GetBackgroundColor(c); }
    HRESULT STDMETHODCALLTYPE SetRotation(DXGI_MODE_ROTATION r) override { return m_real->SetRotation(r); }
    HRESULT STDMETHODCALLTYPE GetRotation(DXGI_MODE_ROTATION *r) override { return m_real->GetRotation(r); }

    HRESULT STDMETHODCALLTYPE SetSourceSize(UINT w, UINT h) override { return m_real->SetSourceSize(w, h); }
    HRESULT STDMETHODCALLTYPE GetSourceSize(UINT *w, UINT *h) override { return m_real->GetSourceSize(w, h); }
    HRESULT STDMETHODCALLTYPE SetMaximumFrameLatency(UINT l) override { return m_real->SetMaximumFrameLatency(l); }
    HRESULT STDMETHODCALLTYPE GetMaximumFrameLatency(UINT *l) override { return m_real->GetMaximumFrameLatency(l); }
    HANDLE STDMETHODCALLTYPE GetFrameLatencyWaitableObject() override { return m_real->GetFrameLatencyWaitableObject(); }
    HRESULT STDMETHODCALLTYPE SetMatrixTransform(const DXGI_MATRIX_3X2_F *m) override { return m_real->SetMatrixTransform(m); }
    HRESULT STDMETHODCALLTYPE GetMatrixTransform(DXGI_MATRIX_3X2_F *m) override { return m_real->GetMatrixTransform(m); }

private:
    IDXGISwapChain2 *m_real;
    std::atomic<uint32_t> m_ref;
};
