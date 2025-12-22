// c++ includes
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <future>
#include <thread>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

// windows headers
#define NOMINMAX // disable min/max macros
#include <Windows.h>
#include <TlHelp32.h>
#include <limits>
#include <shlobj.h>
#include <string.h>

// directx 11 headers
#include <d3d11.h>
#include <dxgi1_2.h>
#include <windowsx.h>

// our proxy wrappers
#include "ProxyDevice.h"
#include "ProxyDeviceContext.h"
#include "ProxyFactory.h"

// directx 11 libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// export controls
#pragma comment(linker, "/export:run=RobloxPlayerBeta_orig.run")

// jmp hook structures and utilities
struct HookInfo
{
    void *originalFunc;
    void *hookFunc;
    BYTE originalBytes[12]; // enough for x64 jmp
    SIZE_T patchSize;
    bool isPatched;
};

// function hooks
HookInfo g_D3D11CreateDeviceHook = {0};
HookInfo g_D3D11CreateDeviceAndSwapChainHook = {0};
HookInfo g_CreateDXGIFactoryHook = {0};
HookInfo g_CreateDXGIFactory1Hook = {0};
HookInfo g_CreateDXGIFactory2Hook = {0};

/////////////////////////////////////////////////////////////////////////////////////////
// helper functions to manage hooks
/////////////////////////////////////////////////////////////////////////////////////////

bool installHooks(HookInfo *specificHook = nullptr)
{
#if DEBUG
    std::cout << "Installing hooks..." << std::endl;
#endif

    // get the addresses of the original functions
    g_D3D11CreateDeviceHook.originalFunc = (void *)GetProcAddress(GetModuleHandleA("d3d11.dll"), "D3D11CreateDevice");
    // exclude the D3D11CreateDeviceAndSwapChain since it is not being used in this current context
    g_CreateDXGIFactoryHook.originalFunc = (void *)GetProcAddress(GetModuleHandleA("dxgi.dll"), "CreateDXGIFactory");
    g_CreateDXGIFactory1Hook.originalFunc = (void *)GetProcAddress(GetModuleHandleA("dxgi.dll"), "CreateDXGIFactory1");
    g_CreateDXGIFactory2Hook.originalFunc = (void *)GetProcAddress(GetModuleHandleA("dxgi.dll"), "CreateDXGIFactory2");

    // set our hook function pointers
    g_D3D11CreateDeviceHook.hookFunc = (void *)D3D11CreateDevice;
    g_CreateDXGIFactoryHook.hookFunc = (void *)CreateDXGIFactory;
    g_CreateDXGIFactory1Hook.hookFunc = (void *)CreateDXGIFactory1;
    g_CreateDXGIFactory2Hook.hookFunc = (void *)CreateDXGIFactory2; // print our original function addresses
#if DEBUG
    std::cout << "Original D3D11CreateDevice address: " << g_D3D11CreateDeviceHook.originalFunc << std::endl;
    std::cout << "Original CreateDXGIFactory address: " << g_CreateDXGIFactoryHook.originalFunc << std::endl;
    std::cout << "Original CreateDXGIFactory1 address: " << g_CreateDXGIFactory1Hook.originalFunc << std::endl;
    std::cout << "Original CreateDXGIFactory2 address: " << g_CreateDXGIFactory2Hook.originalFunc << std::endl;
#endif

    // check if we got valid addresses
    if (!g_D3D11CreateDeviceHook.originalFunc ||
        !g_CreateDXGIFactoryHook.originalFunc ||
        !g_CreateDXGIFactory1Hook.originalFunc ||
        !g_CreateDXGIFactory2Hook.originalFunc)
    {
        // if any of the original function addresses are null, we failed to get them
#if DEBUG
        std::cerr << "Failed to get original function addresses!" << std::endl;
#endif
        return false;
    }

#if DEBUG
    std::cout << "Original function addresses retrieved successfully." << std::endl;

    // print the addresss of our own functions
    std::cout << "Our D3D11CreateDevice address: " << (void *)D3D11CreateDevice << std::endl;
    std::cout << "Our CreateDXGIFactory address: " << (void *)CreateDXGIFactory << std::endl;
    std::cout << "Our CreateDXGIFactory1 address: " << (void *)CreateDXGIFactory1 << std::endl;
    std::cout << "Our CreateDXGIFactory2 address: " << (void *)CreateDXGIFactory2 << std::endl;
#endif

    // create shellcode to patch the original functions to our own
    auto installHook = [](HookInfo &hook) -> bool
    {
        if (!hook.originalFunc || !hook.hookFunc)
        {
#if DEBUG
            std::cerr << "Invalid hook configuration" << std::endl;
#endif
            return false;
        }

        // set up the shellcode: mov rax, [address] followed by jmp rax
        BYTE shellcode[12] = {
            0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, imm64
            0xFF, 0xE0                                                  // jmp rax
        };

        // insert the actual address into the shellcode
        *(uint64_t *)(&shellcode[2]) = (uint64_t)hook.hookFunc;

        // save original protection
        DWORD oldProtect;

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

#if DEBUG
        std::cout << "Successfully installed hook at " << hook.originalFunc << " -> " << hook.hookFunc << std::endl;
#endif
        return true;
    };

    // install specific hook or all hooks
    bool success = true;
    if (specificHook)
    {
        // install only the specified hook
#if DEBUG
        std::cout << "Installing specific hook only..." << std::endl;
#endif
        success = installHook(*specificHook);

        if (!success)
        {
#if DEBUG
            std::cerr << "Failed to install specific hook!" << std::endl;
#endif
            return false;
        }
    }
    else
    {
        // install all hooks
        success &= installHook(g_D3D11CreateDeviceHook);
        success &= installHook(g_CreateDXGIFactoryHook);
        success &= installHook(g_CreateDXGIFactory1Hook);
        success &= installHook(g_CreateDXGIFactory2Hook);

        if (!success)
        {
#if DEBUG
            std::cerr << "Some hooks failed to install!" << std::endl;
#endif
            return false;
        }
    }

#if DEBUG
    std::cout << "Hooks successfully installed!" << std::endl;
#endif
    return true;
}

bool removeHook(HookInfo *hookToRemove)
{
    // validate hook parameter
    if (!hookToRemove)
    {
#if DEBUG
        std::cerr << "No hook provided for removal!" << std::endl;
#endif return false;
    }

    // check if the hook is actually patched
    if (!hookToRemove->isPatched)
    {
#if DEBUG
        std::cout << "Hook at " << hookToRemove->originalFunc << " is not currently patched, nothing to remove." << std::endl;
#endif
        return true; // not an error, just nothing to do
    }

#if DEBUG
    std::cout << "Removing hook at address: " << hookToRemove->originalFunc << std::endl;
#endif

    // save original protection
    DWORD oldProtect;

    // make the memory writable
    if (!VirtualProtect(hookToRemove->originalFunc, hookToRemove->patchSize, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
#if DEBUG
        std::cerr << "Failed to change memory protection for hook removal at " << hookToRemove->originalFunc << std::endl;
#endif
        return false;
    }

    // restore original bytes from our backup
    memcpy(hookToRemove->originalFunc, hookToRemove->originalBytes, hookToRemove->patchSize);

    // restore protection
    VirtualProtect(hookToRemove->originalFunc, hookToRemove->patchSize, oldProtect, &oldProtect);

    // mark as no longer patched
    hookToRemove->isPatched = false; // flush instruction cache to ensure CPU picks up the changes
    FlushInstructionCache(GetCurrentProcess(), hookToRemove->originalFunc, hookToRemove->patchSize);

#if DEBUG
    std::cout << "Successfully removed hook at " << hookToRemove->originalFunc << std::endl;
#endif
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// code end
/////////////////////////////////////////////////////////////////////////////////////////

// debug info text file
#define ENABLE_TIMESTAMP 0 // timestamp disabled by default

#if DEBUG
// helper function
const char *dbgInfo = "C:\\Users\\hecker\\Desktop\\dxpipe_layer\\src\\dbgInfo.txt";
void output()
{
    static std::ofstream installFile(dbgInfo, std::ios::app);
    static std::streambuf *coutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(installFile.rdbuf());
}
#endif

// helper timestamp builder
static std::string timeStamp()
{
#if ENABLE_TIMESTAMP
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << "[" << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S") << "] ";
    return oss.str();
#else
    return "";
#endif
}

// --------------------------- D3D11 HOOKS ----------------------------------

// D3D11CreateDevice declaration
typedef HRESULT(WINAPI *D3D11CreateDevice_t)(
    IDXGIAdapter *pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    const D3D_FEATURE_LEVEL *pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    ID3D11Device **ppDevice,
    D3D_FEATURE_LEVEL *pFeatureLevel,
    ID3D11DeviceContext **ppImmediateContext);

// D3D11CreateDeviceAndSwapChain declaration (just forward, we don’t need extra logic)
typedef HRESULT(WINAPI *D3D11CreateDeviceAndSwapChain_t)(
    IDXGIAdapter *pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    const D3D_FEATURE_LEVEL *pFeatureLevels,
    UINT FeatureLevelCount,
    UINT SDKVersion,
    const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
    IDXGISwapChain **ppSwapChain,
    ID3D11Device **ppDevice,
    D3D_FEATURE_LEVEL *pFeatureLevel,
    ID3D11DeviceContext **ppImmediateContext);

// globals
static HMODULE g_realD3D11 = nullptr;
static D3D11CreateDevice_t g_real_D3D11CreateDevice = nullptr;
static D3D11CreateDeviceAndSwapChain_t g_real_D3D11CreateDeviceAndSwapChain = nullptr;
static ID3D11Device *g_Device = nullptr;
ID3D11Texture2D *g_BackBufferTexture = nullptr; // our back buffer texture

// staging copies
ID3D11Texture2D *g_BackBufferStaging = nullptr;
ID3D11Texture2D *g_DepthStaging = nullptr;

// our shareable copies
ID3D11Texture2D *g_BackBufferShared = nullptr;
ID3D11Texture2D *g_DepthShared = nullptr;
HANDLE g_BackBufferSharedHandle = nullptr;
HANDLE g_DepthSharedHandle = nullptr;

// global variable to store the device name
std::string g_DeviceName;

// dual GPU handling - track device creation calls
static bool g_FirstDeviceCreated = false;

// helper function to replace global pointers
void replaceGlobal(ID3D11Texture2D *&dst, ID3D11Texture2D *src)
{
    if (dst)
        dst->Release();
    dst = src;
    if (dst)
        dst->AddRef();
}

// helper function to update width and height from back buffer texture
void updateDimensionsFromBackBuffer()
{
    if (g_BackBufferTexture)
    {
        D3D11_TEXTURE2D_DESC desc;
        g_BackBufferTexture->GetDesc(&desc); // Only update if dimensions have changed to avoid unnecessary logs
        if (g_Width != (int)desc.Width || g_Height != (int)desc.Height)
        {
            g_Width = desc.Width;
            g_Height = desc.Height;
#if DEBUG
            std::cout << timeStamp() << "Updated dimensions from back buffer: "
                      << g_Width << "x" << g_Height << std::endl;
#endif
        }
    }
}

// --------------------------- DXGI HOOKS ------------------------------------

// CreateDXGIFactory1 declaration
typedef HRESULT(WINAPI *CreateDXGIFactory1_t)(
    REFIID riid,
    void **ppFactory);

// CreateDXGIFactory declaration (legacy version)
typedef HRESULT(WINAPI *CreateDXGIFactory_t)(
    REFIID riid,
    void **ppFactory);

// CreateDXGIFactory2 declaration
typedef HRESULT(WINAPI *CreateDXGIFactory2_t)(
    UINT Flags,
    REFIID riid,
    void **ppFactory);

static HMODULE g_realDXGI = nullptr; // handle to real dxgi.dll
static CreateDXGIFactory_t g_real_CreateDXGIFactory = nullptr;
static CreateDXGIFactory1_t g_real_CreateDXGIFactory1 = nullptr;
static CreateDXGIFactory2_t g_real_CreateDXGIFactory2 = nullptr;

// depth texture address
ID3D11Texture2D *g_DepthTexture = nullptr;

// other
int g_Width = 0;
int g_Height = 0;

// path to Bloxshade appdata folder
std::string appdataBloxshadePath;

// intercept D3D11CreateDevice
extern "C" HRESULT WINAPI D3D11CreateDevice(
    IDXGIAdapter *pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    const D3D_FEATURE_LEVEL *pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    ID3D11Device **ppDevice,
    D3D_FEATURE_LEVEL *pFeatureLevel,
    ID3D11DeviceContext **ppImmediateContext)
{
#if DEBUG
    output();
    std::cout << timeStamp() << "D3D11CreateDevice called!" << std::endl;
#endif // dual GPU handling: exclude the second device creation call (display out device)
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

    if (!g_real_D3D11CreateDevice)
    {
#if DEBUG
        std::cout << timeStamp() << "Original pointer not set" << std::endl;
#endif
        return E_FAIL;
    }

    // temporarily remove our hook to prevent infinite recursion
    removeHook(&g_D3D11CreateDeviceHook);

    HRESULT hr = g_real_D3D11CreateDevice(
        pAdapter, DriverType, Software, Flags,
        pFeatureLevels, FeatureLevels, SDKVersion,
        ppDevice, pFeatureLevel, ppImmediateContext);

    // reinstall our hook
    installHooks(&g_D3D11CreateDeviceHook);

    // wrap the immediate context with our proxy
    if (SUCCEEDED(hr) && ppImmediateContext && *ppImmediateContext)
    {
        ID3D11DeviceContext *realCtx = *ppImmediateContext;
        *ppImmediateContext = new ProxyDeviceContext(realCtx);
        realCtx->Release();
    }

    if (SUCCEEDED(hr) && ppDevice && *ppDevice)
    {
        // store the real device pointer the first time we see it
        if (!g_Device)
            g_Device = *ppDevice;

        // helper function to convert feature level to string
        auto flToStr = [](D3D_FEATURE_LEVEL lvl) -> std::string
        {
            switch (lvl)
            {
            case D3D_FEATURE_LEVEL_11_1:
                return "11.1";
            case D3D_FEATURE_LEVEL_11_0:
                return "11.0";
            case D3D_FEATURE_LEVEL_10_1:
                return "10.1";
            case D3D_FEATURE_LEVEL_10_0:
                return "10.0";
            case D3D_FEATURE_LEVEL_9_3:
                return "9.3";
            case D3D_FEATURE_LEVEL_9_2:
                return "9.2";
            case D3D_FEATURE_LEVEL_9_1:
                return "9.1";
            default:
                return "Unknown";
            } };

#if DEBUG
        std::cout << timeStamp() << "g_Device address: " << g_Device << std::endl;
#endif

        // retrieve device name
        IDXGIDevice *pDX = nullptr;
        if (SUCCEEDED((*ppDevice)->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDX)))
        {
            IDXGIAdapter *pAd = nullptr;
            if (SUCCEEDED(pDX->GetAdapter(&pAd)))
            {
                DXGI_ADAPTER_DESC desc;
                if (SUCCEEDED(pAd->GetDesc(&desc)))
                {
                    char name[128];
                    WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1,
                                        name, sizeof(name), nullptr, nullptr);
#if DEBUG
                    std::cout << timeStamp() << "Device name: " << name << std::endl;
#endif
                    g_DeviceName = name; // store the device name globally
                }
                pAd->Release();
            }
            pDX->Release();
        }
#if DEBUG
        std::cout << timeStamp() << "Feature level: " << flToStr((*ppDevice)->GetFeatureLevel()) << std::endl;
#endif

        // mark that we've successfully created the first device (game engine device)
        g_FirstDeviceCreated = true;
#if DEBUG
        std::cout << timeStamp() << "First device created - subsequent calls will be excluded" << std::endl;
#endif

        // wrap the real device with our proxy
        ID3D11Device *realDevice = *ppDevice;
        *ppDevice = new ProxyDevice(realDevice);
    }

    return hr;
}

// --------------- forward D3D11CreateDeviceAndSwapChain (this also fixes an error on amd GPUs I had) -----------
extern "C" HRESULT WINAPI D3D11CreateDeviceAndSwapChain(
    IDXGIAdapter *pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    const D3D_FEATURE_LEVEL *pFeatureLevels,
    UINT FeatureLevelCount,
    UINT SDKVersion,
    const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
    IDXGISwapChain **ppSwapChain,
    ID3D11Device **ppDevice,
    D3D_FEATURE_LEVEL *pFeatureLevel,
    ID3D11DeviceContext **ppImmediateContext)
{
    // simply call the original – our factory/adapter hooks will still kick in
    if (!g_real_D3D11CreateDeviceAndSwapChain)
        return E_FAIL;
    return g_real_D3D11CreateDeviceAndSwapChain(
        pAdapter, DriverType, Software, Flags,
        pFeatureLevels, FeatureLevelCount, SDKVersion,
        pSwapChainDesc, ppSwapChain,
        ppDevice, pFeatureLevel, ppImmediateContext);
}

// ------------------ intercept CreateDXGIFactory -----------------------------
extern "C" HRESULT WINAPI CreateDXGIFactory(
    REFIID riid,
    void **ppFactory)
{
#if DEBUG
    output();
    std::cout << timeStamp() << "CreateDXGIFactory called!" << std::endl;
#endif

    if (!g_real_CreateDXGIFactory)
    {
#if DEBUG
        std::cout << timeStamp() << "CreateDXGIFactory original pointer not set!" << std::endl;
#endif
        return E_FAIL;
    }
    // temporarily remove our hook to prevent infinite recursion
    removeHook(&g_CreateDXGIFactoryHook);

    HRESULT hr = g_real_CreateDXGIFactory(riid, ppFactory);

    // reinstall our hook
    installHooks(&g_CreateDXGIFactoryHook);

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
    return hr;
}

// ------------------ intercept CreateDXGIFactory1 ---------------------------
extern "C" HRESULT WINAPI CreateDXGIFactory1(
    REFIID riid,
    void **ppFactory)
{
#if DEBUG
    output();
    std::cout << timeStamp() << "CreateDXGIFactory1 called!" << std::endl;
#endif

    if (!g_real_CreateDXGIFactory1)
    {
#if DEBUG
        std::cout << timeStamp() << "CreateDXGIFactory1 original pointer not set!" << std::endl;
#endif
        return E_FAIL;
    }
    // temporarily remove our hook to prevent infinite recursion
    removeHook(&g_CreateDXGIFactory1Hook);

    HRESULT hr = g_real_CreateDXGIFactory1(riid, ppFactory);

    // reinstall our hook
    installHooks(&g_CreateDXGIFactory1Hook);

    if (SUCCEEDED(hr) && ppFactory && *ppFactory)
    {
        IDXGIFactory2 *realFactory2 = nullptr;
        if (SUCCEEDED(((IUnknown *)*ppFactory)->QueryInterface(__uuidof(IDXGIFactory2), (void **)&realFactory2)) && realFactory2)
        {
#if DEBUG
            std::cout << timeStamp() << "Wrapping IDXGIFactory2 (via Factory1)" << std::endl;
#endif
            *ppFactory = new ProxyFactory(realFactory2);
            realFactory2->Release();
        }
    }
    return hr;
}

// ------------------ intercept CreateDXGIFactory2 ----------------------
extern "C" HRESULT WINAPI CreateDXGIFactory2(
    UINT Flags,
    REFIID riid,
    void **ppFactory)
{
#if DEBUG
    output();
    std::cout << timeStamp() << "CreateDXGIFactory2 called! Flags=" << Flags << std::endl;
#endif

    if (!g_real_CreateDXGIFactory2)
    {
#if DEBUG
        std::cout << timeStamp() << "CreateDXGIFactory2 original pointer not set!" << std::endl;
#endif return E_FAIL;
    }
    // temporarily remove our hook to prevent infinite recursion
    removeHook(&g_CreateDXGIFactory2Hook);

    HRESULT hr = g_real_CreateDXGIFactory2(Flags, riid, ppFactory);

    // reinstall our hook
    installHooks(&g_CreateDXGIFactory2Hook);

    if (SUCCEEDED(hr) && ppFactory && *ppFactory)
    {
        IDXGIFactory2 *realFactory2 = nullptr;
        if (SUCCEEDED(((IUnknown *)*ppFactory)->QueryInterface(__uuidof(IDXGIFactory2), (void **)&realFactory2)) && realFactory2)
        {
#if DEBUG
            std::cout << timeStamp() << "Wrapping IDXGIFactory2 (direct)" << std::endl;
#endif
            *ppFactory = new ProxyFactory(realFactory2);
            realFactory2->Release();
        }
    }
    return hr;
}

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

        g_real_D3D11CreateDevice = reinterpret_cast<D3D11CreateDevice_t>(
            GetProcAddress(g_realD3D11, "D3D11CreateDevice"));
        g_real_D3D11CreateDeviceAndSwapChain = reinterpret_cast<D3D11CreateDeviceAndSwapChain_t>(
            GetProcAddress(g_realD3D11, "D3D11CreateDeviceAndSwapChain"));
        if (!g_real_D3D11CreateDevice)
            return FALSE;

        // load the real dxgi.dll
        std::string realDXGI = std::string(sysDir) + "\\dxgi.dll";
        g_realDXGI = LoadLibraryA(realDXGI.c_str());
        if (!g_realDXGI)
            return FALSE;

        g_real_CreateDXGIFactory = reinterpret_cast<CreateDXGIFactory_t>(
            GetProcAddress(g_realDXGI, "CreateDXGIFactory"));
        g_real_CreateDXGIFactory1 = reinterpret_cast<CreateDXGIFactory1_t>(
            GetProcAddress(g_realDXGI, "CreateDXGIFactory1"));
        g_real_CreateDXGIFactory2 = reinterpret_cast<CreateDXGIFactory2_t>(
            GetProcAddress(g_realDXGI, "CreateDXGIFactory2"));
        if (!g_real_CreateDXGIFactory && !g_real_CreateDXGIFactory1 && !g_real_CreateDXGIFactory2)
            return FALSE;

        // install our hooks
        if (!installHooks())
        {
#if DEBUG
            std::cout << timeStamp() << "Failed to install hooks!" << std::endl;
#endif
            return FALSE;
        }
        // dimensions will be updated dynamically from back buffer texture
#if DEBUG
        std::cout << timeStamp() << "Dimensions will be detected from back buffer texture" << std::endl;

        std::cout << timeStamp() << "Hello from DxPipe!" << std::endl;
#endif

        // start our overlay with complete detachment
        char buf[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, buf)))
        {
            // construct path to Bloxshade executable in AppData
            appdataBloxshadePath = std::string(buf) + "\\Bloxshade\\bloxshade.exe";

            // get the directory path from the executable path
            std::string workingDir = appdataBloxshadePath.substr(0, appdataBloxshadePath.find_last_of("\\/"));

            // prepare process startup information
            STARTUPINFOA si = {sizeof(STARTUPINFOA)};
            PROCESS_INFORMATION pi = {0};

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

            if (success)
            {
                // immediately close process handles to prevent resource leaks
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
#if DEBUG
                std::cout << timeStamp() << "Started Bloxshade completely detached" << std::endl;
#endif
            }
#if DEBUG
            else
            {
                std::cout << timeStamp() << "Failed to start Bloxshade at: " << appdataBloxshadePath << std::endl;
            }
#endif
        }
    }
    return TRUE;
}
