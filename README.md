# Introduction
This is a module I developed back in June of 2025; it was intended to share both the back buffer and the depth texture with an external overlay through named pipes and a shared DirectX 11 resource handle. I’m releasing this solely as an educational resource, and its other components won’t be made available. Alongside that, this module is no longer functional.

## High-Level Overview
The original `RobloxPlayerBeta.dll` is renamed to `RobloxPlayerBeta_orig.dll` with the same export. This tricks Roblox into loading our module before loading their own. That allows us to initialize very early in the process, and lets the module proxy the relevant DXGI / D3D11 interfaces returned by `dxgi.dll` and `d3d11.dll`.

The export-forwarding itself is done directly in the project with:
```cpp
// export controls
#pragma comment(linker, "/export:run=RobloxPlayerBeta_orig.run")
```

## Runtime behavior
At the DLL entry point, the module only runs in the target executable and loads the real system DLLs for both `d3d11.dll` and `dxgi.dll` from the `System32` directory. This might be redundant behaviour, but I included it just in case.
```cpp
// entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
#if DEBUG
        output();
#endif
        // get the process path
        char szProcessPath[MAX_PATH] = {0};
        if (GetModuleFileNameA(nullptr, szProcessPath, MAX_PATH))
        {
            char *pFileName = strrchr(szProcessPath, '\\');
            pFileName = pFileName ? pFileName + 1 : szProcessPath;
            if (_stricmp(pFileName, "RobloxPlayerBeta.exe") != 0)
                return FALSE;
        }
        else
            return FALSE;

        // load the real d3d11.dll from System32
        char sysDir[MAX_PATH];
        GetSystemDirectoryA(sysDir, MAX_PATH);
        std::string realD3D = std::string(sysDir) + "\\d3d11.dll";
        g_realD3D11 = LoadLibraryA(realD3D.c_str());
        if (!g_realD3D11)
            return FALSE;
```

Once that’s done, the layer installs inline hooks by patching the real function entrypoints with a 12-byte x64 shellcode stub (`mov rax, imm64; jmp rax`). Those patches redirect execution into our local wrappers for `D3D11CreateDevice` and `CreateDXGIFactory*`.
```cpp
// set up the shellcode: mov rax, [address] followed by jmp rax
BYTE shellcode[12] = {
    0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, imm64
    0xFF, 0xE0                                                  // jmp rax
};

// insert the actual address into the shellcode
*(uint64_t *)(&shellcode[2]) = (uint64_t)hook.hookFunc;
```

The patching is done by making the target memory writable, saving the original bytes, writing the detour, restoring protections, and flushing the instruction cache:
```cpp
// make the memory writable
if (!VirtualProtect(hook.originalFunc, sizeof(shellcode), PAGE_EXECUTE_READWRITE, &oldProtect))
{
#if DEBUG
    std::cerr << "Failed to change memory protection for hook at " << hook.originalFunc << std::endl;
#endif
    return false;
}

// save original bytes before patching
memcpy(hook.originalBytes, hook.originalFunc, sizeof(shellcode));
hook.patchSize = sizeof(shellcode);

// write our shellcode
memcpy(hook.originalFunc, shellcode, sizeof(shellcode));

// restore protection
VirtualProtect(hook.originalFunc, sizeof(shellcode), oldProtect, &oldProtect);

// mark as patched
hook.isPatched = true; // flush instruction cache to ensure CPU picks up the changes
FlushInstructionCache(GetCurrentProcess(), hook.originalFunc, sizeof(shellcode));
```

When we need to call through to the original implementations, we temporarily remove the patch, call the real function, and then reinstall the hook. This avoids recursion without needing a separate trampoline stub.
```cpp
// temporarily remove our hook to prevent infinite recursion
removeHook(&g_D3D11CreateDeviceHook);

HRESULT hr = g_real_D3D11CreateDevice(
    pAdapter, DriverType, Software, Flags,
    pFeatureLevels, FeatureLevels, SDKVersion,
    ppDevice, pFeatureLevel, ppImmediateContext);

// reinstall our hook
installHooks(&g_D3D11CreateDeviceHook);
```

This also matters in some multi-GPU setups. For example, if your display output is on one adapter (say, an RX 6500 XT) but the game is running on another (say, an RTX 5070 Ti), you can see more than one D3D11 device creation path. This is also applicable for laptop setups using NVIDIA Optimus or vendor equivalents, where display composition and rendering may involve multiple adapters. In practice, the module treats the first `D3D11CreateDevice` call as the “game” device and intentionally avoids wrapping subsequent device creations to prevent unexpected state/NULL pointer issues.
```cpp
// dual GPU handling - track device creation calls
static bool g_FirstDeviceCreated = false;
```

```cpp
// dual GPU handling: exclude the second device creation call (display out device)
// in dual GPU setups, the first call is by the game engine, second is by windows
if (g_FirstDeviceCreated)
{
#if DEBUG
    std::cout << timeStamp() << "Excluding second D3D11CreateDevice call (dual GPU display device)" << std::endl;
#endif

    // temporarily remove our hook to prevent infinite recursion
    removeHook(&g_D3D11CreateDeviceHook);

    // call original function without wrapping
    HRESULT hr = g_real_D3D11CreateDevice(
        pAdapter, DriverType, Software, Flags,
        pFeatureLevels, FeatureLevels, SDKVersion,
        ppDevice, pFeatureLevel, ppImmediateContext);

    // reinstall our hook
    installHooks(&g_D3D11CreateDeviceHook);
    return hr;
}
```

After the device and swap chain are created, the layer replaces the real `ID3D11Device`, `ID3D11DeviceContext`, and relevant DXGI interfaces with proxy implementations (e.g., `ProxyDevice`, `ProxyDeviceContext`, `ProxyDXGIDevice`, `ProxyAdapter`, `ProxyFactory`, and `ProxySwapChain`).
The immediate context is wrapped like this:
```cpp
// wrap the immediate context with our proxy
if (SUCCEEDED(hr) && ppImmediateContext && *ppImmediateContext)
{
    ID3D11DeviceContext *realCtx = *ppImmediateContext;
    *ppImmediateContext = new ProxyDeviceContext(realCtx);
    realCtx->Release();
}
```

And the device is wrapped like this (and the “first device wins” rule is enforced here):
```cpp
// mark that we've successfully created the first device (game engine device)
g_FirstDeviceCreated = true;
#if DEBUG
std::cout << timeStamp() << "First device created - subsequent calls will be excluded" << std::endl;
#endif

// wrap the real device with our proxy
ID3D11Device *realDevice = *ppDevice;
*ppDevice = new ProxyDevice(realDevice);
```

On the DXGI side, the factory wrapping happens by upgrading to `IDXGIFactory2` and returning a proxy:
```cpp
if (SUCCEEDED(hr) && ppFactory && *ppFactory)
{
    IDXGIFactory2 *realFactory2 = nullptr;
    if (SUCCEEDED(((IUnknown *)*ppFactory)->QueryInterface(__uuidof(IDXGIFactory2), (void **)&realFactory2)) && realFactory2)
    {
#if DEBUG
        std::cout << timeStamp() << "Wrapping IDXGIFactory2 (via Factory)" << std::endl;
#endif
        *ppFactory = new ProxyFactory(realFactory2);
        realFactory2->Release();
    }
}
```

`ProxyFactory` then wraps the swap chain creation so everything funnels through `ProxySwapChain`:
```cpp
HRESULT hr = m_real->CreateSwapChain(pDev, d, ppSwapChain);
if (SUCCEEDED(hr) && ppSwapChain && *ppSwapChain)
{
    *ppSwapChain = new ProxySwapChain(reinterpret_cast<IDXGISwapChain2 *>(*ppSwapChain));
}
return hr;
```

`ProxySwapChain` captures `GetBuffer(0)` as the back buffer:
```cpp
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
}
```

The depth texture is tracked by the device proxy (see `ProxyDevice.h`): it watches `CreateTexture2D` and stores the depth texture pointer when it matches the expected dimensions/format.
```cpp
// intercept the call & log the address of the target texture
if (SUCCEEDED(hr) && ppTexture2D && *ppTexture2D &&
    pDesc && pDesc->SampleDesc.Count == 1 &&
    pDesc->Width == g_Width &&
    pDesc->Height == g_Height &&
    pDesc->Format == DXGI_FORMAT_R32_TYPELESS)
{
    // release previous depth surface (if any)
    if (g_DepthTexture)
        g_DepthTexture->Release();

    g_DepthTexture = *ppTexture2D;
    g_DepthTexture->AddRef(); // avoid premature release
#if DEBUG
    std::cout << timeStamp() << "Depth texture address: " << g_DepthTexture << std::endl;
#endif
}
```

During `Present()`, the swap chain proxy copies both the back buffer and the tracked depth texture into CPU-readable staging resources as well as GPU shared textures. The staging path is the part that enables deterministic dumping/debugging, while the shared path is what the overlay consumes.
```cpp
/* ------------ colour staging ------------ */
ensureStaging(g_BackBufferTexture, g_BackBufferStaging);
if (ctx && g_BackBufferTexture && g_BackBufferStaging)
    ctx->CopyResource(g_BackBufferStaging, g_BackBufferTexture);

/* ------------ depth staging  ------------ */
ensureStaging(g_DepthTexture, g_DepthStaging);
if (ctx && g_DepthTexture && g_DepthStaging)
    ctx->CopyResource(g_DepthStaging, g_DepthTexture);
```

The shared textures themselves are created with `D3D11_RESOURCE_MISC_SHARED`, and the shared handle is obtained via `IDXGIResource::GetSharedHandle`:
```cpp
desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // standard back buffer format
desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED; // basic sharing without keyed mutex
```

```cpp
IDXGIResource *resource = nullptr;
hr = g_BackBufferShared->QueryInterface(__uuidof(IDXGIResource), (void **)&resource);
if (SUCCEEDED(hr))
{
    hr = resource->GetSharedHandle(&g_BackBufferSharedHandle);
    resource->Release();
```

Once we have the textures we need, named pipes are used to communicate the shared handle plus basic texture metadata (dimensions/format) so the external overlay can open the shared resources without copying raw pixels over the pipe. The payload is a simple struct:
```cpp
// Structure to send texture information with dimensions
struct TextureInfo
{
    HANDLE handle;
    UINT width;
    UINT height;
    DXGI_FORMAT format;
};
```

And the pipes are created like this:
```cpp
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
```

The `Present()` path calls the helper that discovers the overlay process and sends the DXGI shared handles plus texture metadata over named pipes:
```cpp
/* ------------ handle duplication to client process ------------ */
// attempt to duplicate handles to external client process
duplicateHandleToClientProcess();
```

Inside that helper, the module sends updated `TextureInfo` whenever the handle/metadata changes:
```cpp
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
```

Finally, the swap chain proxy can also render ImGui directly inside the application (in the `Present()` path). This was mainly used during development as a debugging resource and is currently disabled by default in the code:
```cpp
// enable/disable ImGui
#define ENABLE_IMGUI 0
```

One last behavior worth calling out is that the module also attempts to start the overlay process from AppData with detachment flags:
```cpp
// attempt to create process with detachment flags
BOOL success = CreateProcessA(
    appdataBloxshadePath.c_str(),                                    // Application path
    NULL,                                                            // Command line
    NULL,                                                            // Process handle not inheritable
    NULL,                                                            // Thread handle not inheritable
    FALSE,                                                           // Set handle inheritance to FALSE
    DETACHED_PROCESS | CREATE_NO_WINDOW | CREATE_BREAKAWAY_FROM_JOB, // Complete detachment flags
    NULL,                                                            // Use parent's environment block
    workingDir.c_str(),                                              // Use Bloxshade's directory as working directory
    &si,                                                             // Pointer to STARTUPINFO structure
    &pi                                                              // Pointer to PROCESS_INFORMATION structure
);
```

## Flowchart
```text
┌──────────────────────────────┐
│ RobloxPlayerBeta.exe starts  │
└──────────────┬───────────────┘
               │
               v
┌──────────────────────────────────────────────┐
│ Roblox loads RobloxPlayerBeta.dll            │
│ (actually this module; original renamed      │
│  to RobloxPlayerBeta_orig.dll)               │
└──────────────┬───────────────────────────────┘
               │
               v
┌──────────────────────────────────────────────┐
│ DllMain (PROCESS_ATTACH)                     │
│  - verify target process                     │
│  - LoadLibrary System32\\d3d11.dll + dxgi.dll│
│  - GetProcAddress original exports           │
│  - patch entrypoints with 12B detours        │
└──────────────┬───────────────────────────────┘
               │
               v
┌────────────────────────────────────────────────┐
│ App calls D3D11CreateDevice / CreateDXGIFactory│
└──────────────┬─────────────────────────────────┘
               │ detoured
               v
┌──────────────────────────────────────────────┐
│ Hook wrapper runs                            │
│  - unpatch → call real function → re-patch   │
│  - first device: wrap Device + ImmediateCtx  │
│  - later devices: pass-through (multi-GPU)   │
└──────────────┬───────────────────────────────┘
               │
               v
┌──────────────────────────────────────────────┐
│ ProxyFactory intercepts CreateSwapChain*     │
│  - returns ProxySwapChain                    │
└──────────────┬───────────────────────────────┘
               │
               v
┌──────────────────────────────────────────────┐
│ ProxySwapChain::GetBuffer(0)                 │
│  - capture back buffer texture pointer       │
└──────────────┬───────────────────────────────┘
               │
               v
┌──────────────────────────────────────────────┐
│ ProxyDevice::CreateTexture2D                 │
│  - detect/store candidate depth texture      │
└──────────────┬───────────────────────────────┘
               │
               v
┌──────────────────────────────────────────────┐
│ ProxySwapChain::Present() (each frame)       │
│  - ensure staging textures                   │
│  - CopyResource: back/depth → staging        │
│  - ensure shared textures + GetSharedHandle  │
│  - CopyResource: back/depth → shared         │
│  - send TextureInfo(handle,w,h,fmt) via pipes│
│  - (optional) ImGui debug rendering          │
└──────────────────────────────────────────────┘
```

## Authors & contact
Made by [Extravi](https://extravi.dev/).

Email: dante@extravi.dev
