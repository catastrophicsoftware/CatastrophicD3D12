//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <winsdkver.h>
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <wrl/client.h>
#include <wrl/event.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include "DirectXHelpers.h"

#include "d3dx12.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <system_error>

#define LOCK_GUARD(__mutex_obj) std::lock_guard<std::mutex> __lock_guard(__mutex_obj);

// Off by default warnings
#pragma warning(disable : 4619 4061 4265 4355 4365 4571 4623 4625 4626 4628 4668 4710 4711 4746 4774 4820 4987 5026 5027 5031 5032 5039 5045 5219 26812)
// C4619 #pragma warning: there is no warning number 'X'
// C4061 enumerator 'X' in switch of enum 'X' is not explicitly handled by a case label
// C4265 class has virtual functions, but destructor is not virtual
// C4355 'this': used in base member initializer list
// C4365 signed/unsigned mismatch
// C4571 behavior change
// C4623 default constructor was implicitly defined as deleted
// C4625 copy constructor was implicitly defined as deleted
// C4626 assignment operator was implicitly defined as deleted
// C4628 digraphs not supported
// C4668 not defined as a preprocessor macro
// C4710 function not inlined
// C4711 selected for automatic inline expansion
// C4746 volatile access of '<expression>' is subject to /volatile:<iso|ms> setting
// C4774 format string expected in argument 3 is not a string literal
// C4820 padding added after data member
// C4987 nonstandard extension used
// C5026 move constructor was implicitly defined as deleted
// C5027 move assignment operator was implicitly defined as deleted
// C5031/5032 push/pop mismatches in windows headers
// C5039 pointer or reference to potentially throwing function passed to extern C function under - EHc
// C5045 Spectre mitigation warning
// C5219 implicit conversion from 'int' to 'float', possible loss of data
// 26812: The enum type 'x' is unscoped. Prefer 'enum class' over 'enum' (Enum.3).

// XBox One XDK related Off by default warnings
#pragma warning(disable : 4471 4643 4917 4986 5029 5043)
// C4471 forward declaration of an unscoped enumeration must have an underlying type
// C4643 Forward declaring in namespace std is not permitted by the C++ Standard
// C4917 a GUID can only be associated with a class, interface or namespace
// C4986 exception specification does not match previous declaration
// C5029 nonstandard extension used
// C5043 exception specification does not match previous declaration

#ifdef __INTEL_COMPILER
#pragma warning(disable : 161 2960 3280)
// warning #161: unrecognized #pragma
// message #2960: allocation may not satisfy the type's alignment; consider using <aligned_new> header
// message #3280: declaration hides member
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wc++98-compat"
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#pragma clang diagnostic ignored "-Wc++98-compat-local-type-template-args"
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wfloat-equal"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunused-const-variable"
#pragma clang diagnostic ignored "-Wunused-member-function"
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#pragma warning(push)
#pragma warning(disable : 4005)
#define NOMINMAX
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#pragma warning(pop)

#include <Windows.h>

#ifndef _WIN32_WINNT_WIN10
#define _WIN32_WINNT_WIN10 0x0A00
#endif

#ifdef _GAMING_XBOX
#include <gxdk.h>

#if _GXDK_VER < 0x4A610D2B /* GXDK Edition 200600 */
#error DirectX Tool Kit requires the June 2020 GDK or later
#endif

#ifdef _GAMING_XBOX_SCARLETT
#pragma warning(push)
#pragma warning(disable: 5204 5249)
#include <d3d12_xs.h>
#pragma warning(pop)
#include <d3dx12_xs.h>
#else
#pragma warning(push)
#pragma warning(disable: 5204)
#include <d3d12_x.h>
#pragma warning(pop)
#include <d3dx12_x.h>
#endif
#elif defined(_XBOX_ONE) && defined(_TITLE)
#include <xdk.h>

#if _XDK_VER < 0x295A044C /* XDK Edition 160200 */
#error DirectX Tool Kit for Direct3D 12 requires the February 2016 XDK or later
#endif

#include <d3d12_x.h>
#include <d3dx12_x.h>
#else

#ifdef _GAMING_DESKTOP
#include <grdk.h>

#if _GRDK_VER < 0x47BB2070 /* GDK Edition 191102 */
#error DirectX Tool Kit requires the November 2020 GDK QFE2 or later
#endif
#endif

#ifdef USING_DIRECTX_HEADERS
#include <directx/dxgiformat.h>
#include <directx/d3d12.h>
#else
#include <d3d12.h>
#endif

#include <dxgi1_4.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wtautological-type-limit-compare"
#endif

#define D3DX12_NO_STATE_OBJECT_HELPERS
#ifdef USING_DIRECTX_HEADERS
#include <directx/d3dx12.h>
#else
#include "d3dx12.h"
#endif
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#if (defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)) || (defined(_XBOX_ONE) && defined(_TITLE))
#pragma warning(push)
#pragma warning(disable: 4471 5204)
#include <Windows.UI.Core.h>
#pragma warning(pop)
#endif

#include <algorithm>
#include <atomic>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <exception>
#include <initializer_list>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <set>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>
#include <iostream>

#pragma warning(push)
#pragma warning(disable : 5204 5220)
#include <future>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4702)
#include <functional>
#pragma warning(pop)

#include <malloc.h>

#define _XM_NO_XMVECTOR_OVERLOADS_

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>

#if (DIRECTX_MATH_VERSION < 315)
#define XM_ALIGNED_STRUCT(x) __declspec(align(x)) struct
#endif

#pragma warning(push)
#pragma warning(disable : 4467 5038 5204 5220)
#include <wrl.h>
#pragma warning(pop)

#include <wincodec.h>

// DirectX Tool Kit for Audio is in all versions of DirectXTK12
#include <mmreg.h>
#include <Audioclient.h>

#ifndef XAUDIO2_HELPER_FUNCTIONS
#define XAUDIO2_HELPER_FUNCTIONS
#endif

#include <xaudio2.h>
#include <xaudio2fx.h>

#pragma warning(push)
#pragma warning(disable : 4619 4616 5246)
#include <x3daudio.h>
#pragma warning(pop)

#include <xapofx.h>

#if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
#include <apu.h>
#include <shapexmacontext.h>
#include <xma2defs.h>
#endif





#ifdef _DEBUG
#include <dxgidebug.h>
#endif

// To use graphics and CPU markup events with the latest version of PIX, change this to include <pix3.h>
// then add the NuGet package WinPixEventRuntime to the project.
#include <pix.h>

#include <XGameRuntime.h>

namespace DX
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) noexcept : result(hr) {}

        const char* what() const override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            // Set a breakpoint on this line to catch DirectX API errors
            throw com_exception(hr);
        }
    }
}

typedef UINT64 uint64;
typedef unsigned int uint32;

struct ScopedPerformanceTimer
{
    ScopedPerformanceTimer(const char* name)
    {
        start_time = std::chrono::high_resolution_clock::now();
        timerName = name;
    }
    ~ScopedPerformanceTimer()
    {
        end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << timerName << " -- Completed in: " << duration.count() << " milliseconds\n";
    }

    const char* timerName;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> end_time;
};
#define SCOPED_PERFORMANCE_TIMER(name) ScopedPerformanceTimer perfTimer(name);