// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// TODO: reference additional headers your program requires here

#if _MSC_VER // this is defined when compiling with Visual Studio
#define EXPORT_API __declspec(dllexport) // Visual Studio needs annotating exported functions with this
#else
#define EXPORT_API // XCode does not need annotating exported functions, so define is empty
#endif

/* Setup log mechanism */
typedef void(*DebugLogPtr)(const char *);
void LogDebug(const char* text, ...);
void LogError(const char* text, ...);
/* End of setup log mechanism */

extern "C"
{
	//GameMaker: Only has double and char* types

	EXPORT_API void PluginSetLogDelegate(DebugLogPtr fp);

	EXPORT_API bool PluginIsPlatformSupported();
	EXPORT_API double PluginIsPlatformSupportedD();

	EXPORT_API bool PluginIsInitialized();
	EXPORT_API double PluginIsInitializedD();

	EXPORT_API int PluginOpenAnimation(char* path);
	EXPORT_API double PluginOpenAnimationD(char* path);

	EXPORT_API int PluginLoadAnimation(int animationId);
	EXPORT_API double PluginLoadAnimationD(double animationId);

	EXPORT_API int PluginUnloadAnimation(int animationId);
	EXPORT_API double PluginUnloadAnimationD(double animationId);

	EXPORT_API int PluginPlayAnimation(int animationId);
	EXPORT_API double PluginPlayAnimationD(double animationId);

	EXPORT_API bool PluginIsPlaying(int animationId);
	EXPORT_API double PluginIsPlayingD(double animationId);

	EXPORT_API int PluginStopAnimation(int animationId);
	EXPORT_API double PluginStopAnimationD(double animationId);

	EXPORT_API int PluginCloseAnimation(int animationId);
	EXPORT_API double PluginCloseAnimationD(double animationId);

	EXPORT_API int PluginUninit();
	EXPORT_API double PluginUninitD();

	EXPORT_API int PluginCreateAnimation(char* path, int deviceType, int device);
	EXPORT_API int PluginSaveAnimation(int animationId, char* path);
	EXPORT_API int PluginResetAnimation(int animationId);
	EXPORT_API int PluginGetDeviceType(int animationId);
	EXPORT_API int PluginGetDevice(int animationId);
	EXPORT_API int PluginSetDevice(int animationId, int deviceType, int device);
	EXPORT_API int PluginGetMaxLeds(int device);
	EXPORT_API int PluginGetMaxRow(int device);
	EXPORT_API int PluginGetMaxColumn(int device);
	EXPORT_API int PluginGetFrameCount(int animationId);
	EXPORT_API int PluginAddFrame(int animationId, float duration, int* colors, int length);
	EXPORT_API int PluginUpdateFrame(int animationId, int frameIndex, float duration, int* colors, int length);
	EXPORT_API int PluginGetFrame(int animationId, int frameIndex, float* duration, int* colors, int length);
	EXPORT_API int PluginPreviewFrame(int animationId, int frameIndex);
	EXPORT_API int PluginOverrideFrameDuration(int animationId, float duration);

	EXPORT_API int PluginReverse(int animationId);
	EXPORT_API int PluginMirrorHorizontally(int animationId);
	EXPORT_API int PluginMirrorVertically(int animationId);
}
