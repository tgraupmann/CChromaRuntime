// stdafx.cpp : source file that includes just the standard includes
// *.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "Animation1D.h"
#include "Animation2D.h"
#include "ChromaSDKPlugin.h"
#include "ChromaThread.h"
#include <map>
#include <thread>

using namespace ChromaSDK;
using namespace std;

/* Setup log mechanism */
static DebugLogPtr _gDebugLogPtr;
void LogDebug(const char* format, ...)
{
	if (NULL == _gDebugLogPtr)
	{
		va_list args;
		va_start(args, format);
		vfprintf_s(stdout, format, args);
		va_end(args);
	}
	else if (NULL == format)
	{
		_gDebugLogPtr("");
	}
	else
	{
		char buffer[1024] = { 0 };
		va_list args;
		va_start(args, format);
		vsprintf_s(buffer, format, args);
		va_end(args);
		_gDebugLogPtr(&buffer[0]);
	}
}
void LogError(const char* format, ...)
{
	if (NULL == _gDebugLogPtr)
	{
		va_list args;
		va_start(args, format);
		vfprintf_s(stderr, format, args);
		va_end(args);
	}
	else if (NULL == format)
	{
		_gDebugLogPtr("");
	}
	else
	{
		char buffer[1024] = { 0 };
		va_list args;
		va_start(args, format);
		vsprintf_s(buffer, format, args);
		va_end(args);
		_gDebugLogPtr(&buffer[0]);
	}
}
/* End of setup log mechanism */

string _gPath = "";
int _gAnimationId = 0;
map<int, AnimationBase*> _gAnimations;
map<EChromaSDKDevice1DEnum, int> _gPlayMap1D;
map<EChromaSDKDevice2DEnum, int> _gPlayMap2D;

void SetupChromaThread()
{
	ChromaThread::Instance()->Start();
}

void StopChromaThread()
{
	ChromaThread::Instance()->Stop();
}

void StopAnimationByType(int animationId, AnimationBase* animation)
{
	if (animation == nullptr)
	{
		return;
	}

	switch (animation->GetDeviceType())
	{
	case EChromaSDKDeviceTypeEnum::DE_1D:
	{
		Animation1D* animation1D = dynamic_cast<Animation1D*>(animation);
		EChromaSDKDevice1DEnum device = animation1D->GetDevice();
		if (_gPlayMap1D.find(device) != _gPlayMap1D.end())
		{
			int prevAnimation = _gPlayMap1D[device];
			if (prevAnimation != -1)
			{
				PluginStopAnimation(prevAnimation);
			}
		}
		_gPlayMap1D[device] = animationId;
	}
	break;
	case EChromaSDKDeviceTypeEnum::DE_2D:
	{
		Animation2D* animation2D = dynamic_cast<Animation2D*>(animation);
		EChromaSDKDevice2DEnum device = animation2D->GetDevice();
		if (_gPlayMap2D.find(device) != _gPlayMap2D.end())
		{
			int prevAnimation = _gPlayMap2D[device];
			if (prevAnimation != -1)
			{
				PluginStopAnimation(prevAnimation);
			}
		}
		_gPlayMap2D[device] = animationId;
	}
	break;
	}
}

extern "C"
{
	EXPORT_API void PluginSetLogDelegate(DebugLogPtr fp)
	{
		_gDebugLogPtr = fp;
		LogDebug("PluginSetLogDelegate:");
	}

	EXPORT_API bool PluginIsPlatformSupported()
	{
		return true;
	}

	EXPORT_API double PluginIsPlatformSupportedD()
	{
		if (PluginIsPlatformSupported())
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	EXPORT_API bool PluginIsInitialized()
	{
		// Chroma thread plays animations
		SetupChromaThread();

		return ChromaSDKPlugin::GetInstance()->IsInitialized();
	}

	EXPORT_API double PluginIsInitializedD()
	{
		if (PluginIsInitialized())
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	EXPORT_API int PluginOpenAnimation(char* path)
	{
		try
		{
			// Chroma thread plays animations
			SetupChromaThread();

			if (!PluginIsInitialized())
			{
				LogError("PluginOpenAnimation: Plugin is not initialized!\r\n");
				return -1;
			}

			//return animation id
			AnimationBase* animation = ChromaSDKPlugin::GetInstance()->OpenAnimation(path);
			if (animation == nullptr)
			{
				LogError("PluginOpenAnimation: Animation is null! name=%s\r\n", path);
				return -1;
			}
			else
			{
				int id = _gAnimationId;
				_gAnimations[id] = animation;
				++_gAnimationId;
				return id;
			}
		}
		catch (exception)
		{
			LogError("PluginOpenAnimation: Exception path=%s\r\n", path);
			return -1;
		}
	}

	EXPORT_API double PluginOpenAnimationD(char* path)
	{
		return PluginOpenAnimation(path);
	}

	EXPORT_API int PluginLoadAnimation(int animationId)
	{
		try
		{
			// Chroma thread plays animations
			SetupChromaThread();

			if (!PluginIsInitialized())
			{
				LogError("PluginLoadAnimation: Plugin is not initialized!\r\n");
				return -1;
			}

			if (_gAnimations.find(animationId) != _gAnimations.end())
			{
				AnimationBase* animation = _gAnimations[animationId];
				if (animation == nullptr)
				{
					LogError("PluginLoadAnimation: Animation is null! id=%d", animationId);
					return -1;
				}
				animation->Load();
				return animationId;
			}
		}
		catch (exception)
		{
			LogError("PluginLoadAnimation: Exception animationId=%d\r\n", (int)animationId);
		}
		return -1;
	}

	EXPORT_API double PluginLoadAnimationD(double animationId)
	{
		return (double)PluginLoadAnimation((int)animationId);
	}

	EXPORT_API int PluginUnloadAnimation(int animationId)
	{
		try
		{
			// Chroma thread plays animations
			SetupChromaThread();

			if (!PluginIsInitialized())
			{
				LogError("PluginUnloadAnimation: Plugin is not initialized!\r\n");
				return -1;
			}

			if (_gAnimations.find(animationId) != _gAnimations.end())
			{
				AnimationBase* animation = _gAnimations[animationId];
				if (animation == nullptr)
				{
					LogError("PluginUnloadAnimation: Animation is null! id=%d", animationId);
					return -1;
				}
				animation->Unload();
				return animationId;
			}
		}
		catch (exception)
		{
			LogError("PluginUnloadAnimation: Exception animationId=%d\r\n", (int)animationId);
		}
		return -1;
	}

	EXPORT_API double PluginUnloadAnimationD(double animationId)
	{
		return (double)PluginUnloadAnimation((int)animationId);
	}

	EXPORT_API int PluginPlayAnimation(int animationId)
	{
		try
		{
			// Chroma thread plays animations
			SetupChromaThread();

			if (!PluginIsInitialized())
			{
				LogError("PluginPlayAnimation: Plugin is not initialized!\r\n");
				return -1;
			}

			if (_gAnimations.find(animationId) != _gAnimations.end())
			{
				AnimationBase* animation = _gAnimations[animationId];
				if (animation == nullptr)
				{
					LogError("PluginPlayAnimation: Animation is null! id=%d", animationId);
					return -1;
				}
				StopAnimationByType(animationId, animation);
				animation->Play();
				return animationId;
			}
		}
		catch (exception)
		{
			LogError("PluginPlayAnimation: Exception animationId=%d\r\n", (int)animationId);
		}
		return -1;
	}

	EXPORT_API double PluginPlayAnimationD(double animationId)
	{
		return (double)PluginPlayAnimation((int)animationId);
	}

	EXPORT_API bool PluginIsPlaying(int animationId)
	{
		try
		{
			// Chroma thread plays animations
			SetupChromaThread();

			if (!PluginIsInitialized())
			{
				LogError("PluginIsPlaying: Plugin is not initialized!\r\n");
				return false;
			}

			if (_gAnimations.find(animationId) != _gAnimations.end())
			{
				AnimationBase* animation = _gAnimations[animationId];
				if (animation == nullptr)
				{
					LogError("PluginIsPlaying: Animation is null! id=%d", animationId);
					return false;
				}
				return animation->IsPlaying();
			}
		}
		catch (exception)
		{
			LogError("PluginIsPlaying: Exception animationId=%d\r\n", (int)animationId);
		}
		return false;
	}

	EXPORT_API double PluginIsPlayingD(double animationId)
	{
		if (PluginIsPlaying((int)animationId))
		{
			return 1.0;
		}
		else
		{
			return 0.0;
		}
	}

	EXPORT_API int PluginStopAnimation(int animationId)
	{
		try
		{
			// Chroma thread plays animations
			SetupChromaThread();

			if (!PluginIsInitialized())
			{
				LogError("PluginStopAnimation: Plugin is not initialized!\r\n");
				return -1;
			}

			if (_gAnimations.find(animationId) != _gAnimations.end())
			{
				AnimationBase* animation = _gAnimations[animationId];
				if (animation == nullptr)
				{
					LogError("PluginStopAnimation: Animation is null! id=%d", animationId);
					return -1;
				}
				animation->Stop();
				return animationId;
			}
		}
		catch (exception)
		{
			LogError("PluginStopAnimation: Exception animationId=%d\r\n", (int)animationId);
		}
		return -1;
	}

	EXPORT_API double PluginStopAnimationD(double animationId)
	{
		return (double)PluginStopAnimation((int)animationId);
	}

	EXPORT_API int PluginCloseAnimation(int animationId)
	{
		try
		{
			// Chroma thread plays animations
			SetupChromaThread();

			if (!PluginIsInitialized())
			{
				LogError("PluginCloseAnimation: Plugin is not initialized!\r\n");
				return -1;
			}

			if (_gAnimations.find(animationId) != _gAnimations.end())
			{
				AnimationBase* animation = _gAnimations[animationId];
				if (animation == nullptr)
				{
					LogError("PluginCloseAnimation: Animation is null! id=%d", animationId);
					return -1;
				}
				animation->Stop();
				delete _gAnimations[animationId];
				_gAnimations.erase(animationId);
				return animationId;
			}
		}
		catch (exception)
		{
			LogError("PluginCloseAnimation: Exception animationId=%d\r\n", (int)animationId);
		}
		return -1;
	}

	EXPORT_API double PluginCloseAnimationD(double animationId)
	{
		return (double)PluginCloseAnimation((int)animationId);
	}

	EXPORT_API int PluginInit()
	{
		// Chroma thread plays animations
		SetupChromaThread();

		return ChromaSDKPlugin::GetInstance()->ChromaSDKInit();
	}

	EXPORT_API double PluginInitD()
	{
		return (double)PluginInit();
	}

	EXPORT_API int PluginUninit()
	{
		// Chroma thread plays animations
		StopChromaThread();

		int result = ChromaSDKPlugin::GetInstance()->ChromaSDKUnInit();
		if (PluginIsInitialized())
		{
			for (auto iter = _gAnimations.begin(); iter != _gAnimations.end(); ++iter)
			{
				PluginCloseAnimation(iter->first);
			}
		}
		_gAnimations.clear();
		_gAnimationId = 0;
		return result;
	}

	EXPORT_API double PluginUninitD()
	{
		return (double)PluginUninit();
	}

	EXPORT_API int PluginCreateAnimation(char* path, int deviceType, int device)
	{
		switch ((EChromaSDKDeviceTypeEnum)deviceType)
		{
		case EChromaSDKDeviceTypeEnum::DE_1D:
			{
				Animation1D animation1D = Animation1D();
				animation1D.SetDevice((EChromaSDKDevice1DEnum)device);
				vector<FChromaSDKColorFrame1D>& frames = animation1D.GetFrames();
				frames.clear();
				FChromaSDKColorFrame1D frame = FChromaSDKColorFrame1D();
				frame.Colors = ChromaSDKPlugin::GetInstance()->CreateColors1D((EChromaSDKDevice1DEnum)device);
				frames.push_back(frame);
				animation1D.Save(path);
				return PluginOpenAnimation(path);
			}
			break;
		case EChromaSDKDeviceTypeEnum::DE_2D:
			{
				Animation2D animation2D = Animation2D();
				animation2D.SetDevice((EChromaSDKDevice2DEnum)device);
				vector<FChromaSDKColorFrame2D>& frames = animation2D.GetFrames();
				frames.clear();
				FChromaSDKColorFrame2D frame = FChromaSDKColorFrame2D();
				frame.Colors = ChromaSDKPlugin::GetInstance()->CreateColors2D((EChromaSDKDevice2DEnum)device);
				frames.push_back(frame);
				animation2D.Save(path);
				return PluginOpenAnimation(path);
			}
			break;
		}
		return -1;
	}

	EXPORT_API int PluginSaveAnimation(int animationId, char* path)
	{
		PluginStopAnimation(animationId);

		// Chroma thread plays animations
		SetupChromaThread();

		if (!PluginIsInitialized())
		{
			LogError("PluginSaveAnimation: Plugin is not initialized!\r\n");
			return -1;
		}

		if (_gAnimations.find(animationId) != _gAnimations.end())
		{
			AnimationBase* animation = _gAnimations[animationId];
			if (animation == nullptr)
			{
				LogError("PluginSaveAnimation: Animation is null! id=%d", animationId);
				return -1;
			}
			animation->Save(path);
			return animationId;
		}
		return -1;
	}

	EXPORT_API int PluginResetAnimation(int animationId)
	{
		PluginStopAnimation(animationId);

		// Chroma thread plays animations
		SetupChromaThread();

		if (!PluginIsInitialized())
		{
			LogError("PluginResetAnimation: Plugin is not initialized!\r\n");
			return -1;
		}

		if (_gAnimations.find(animationId) != _gAnimations.end())
		{
			AnimationBase* animation = _gAnimations[animationId];
			if (animation == nullptr)
			{
				LogError("PluginResetAnimation: Animation is null! id=%d", animationId);
				return -1;
			}
			animation->ResetFrames();
			return animationId;
		}

		return -1;
	}

	EXPORT_API int PluginGetFrameCount(int animationId)
	{
		if (_gAnimations.find(animationId) != _gAnimations.end())
		{
			AnimationBase* animation = _gAnimations[animationId];
			if (animation == nullptr)
			{
				LogError("PluginGetFrameCount: Animation is null! id=%d", animationId);
				return -1;
			}
			return animation->GetFrameCount();
		}

		return -1;
	}

	EXPORT_API int PluginGetDeviceType(int animationId)
	{
		if (_gAnimations.find(animationId) != _gAnimations.end())
		{
			AnimationBase* animation = _gAnimations[animationId];
			if (animation == nullptr)
			{
				LogError("PluginGetDeviceType: Animation is null! id=%d", animationId);
				return -1;
			}
			return (int)animation->GetDeviceType();
		}

		return -1;
	}

	EXPORT_API int PluginGetDevice(int animationId)
	{
		if (_gAnimations.find(animationId) != _gAnimations.end())
		{
			AnimationBase* animation = _gAnimations[animationId];
			if (animation == nullptr)
			{
				LogError("PluginGetDevice: Animation is null! id=%d", animationId);
				return -1;
			}
			switch (animation->GetDeviceType())
			{
			case EChromaSDKDeviceTypeEnum::DE_1D:
			{
				Animation1D* animation1D = dynamic_cast<Animation1D*>(animation);
				return (int)animation1D->GetDevice();
			}
			break;
			case EChromaSDKDeviceTypeEnum::DE_2D:
			{
				Animation2D* animation2D = dynamic_cast<Animation2D*>(animation);
				return (int)animation2D->GetDevice();
			}
			break;
			}
		}

		return -1;
	}

	EXPORT_API int PluginSetDevice(int animationId, int deviceType, int device)
	{
		PluginStopAnimation(animationId);

		// Chroma thread plays animations
		SetupChromaThread();

		if (!PluginIsInitialized())
		{
			LogError("PluginSetDevice: Plugin is not initialized!\r\n");
			return -1;
		}

		PluginCloseAnimation(animationId);

		switch ((EChromaSDKDeviceTypeEnum)deviceType)
		{
		case EChromaSDKDeviceTypeEnum::DE_1D:
		{
			Animation1D* animation1D = new Animation1D();
			animation1D->SetDevice((EChromaSDKDevice1DEnum)device);
			_gAnimations[animationId] = animation1D;
			return animationId;
		}
		break;
		case EChromaSDKDeviceTypeEnum::DE_2D:
		{
			Animation2D* animation2D = new Animation2D();
			animation2D->SetDevice((EChromaSDKDevice2DEnum)device);
			_gAnimations[animationId] = animation2D;
			return animationId;
		}
		break;
		}

		return -1;
	}

	EXPORT_API int PluginGetMaxLeds(int device)
	{
		return ChromaSDKPlugin::GetInstance()->GetMaxLeds((EChromaSDKDevice1DEnum)device);
	}

	EXPORT_API int PluginGetMaxRow(int device)
	{
		return ChromaSDKPlugin::GetInstance()->GetMaxRow((EChromaSDKDevice2DEnum)device);
	}

	EXPORT_API int PluginGetMaxColumn(int device)
	{
		return ChromaSDKPlugin::GetInstance()->GetMaxColumn((EChromaSDKDevice2DEnum)device);
	}

	EXPORT_API int PluginAddFrame(int animationId, float duration, int* colors, int length)
	{
		PluginStopAnimation(animationId);

		// Chroma thread plays animations
		SetupChromaThread();

		if (!PluginIsInitialized())
		{
			LogError("PluginAddFrame: Plugin is not initialized!\r\n");
			return -1;
		}

		if (_gAnimations.find(animationId) != _gAnimations.end())
		{
			AnimationBase* animation = _gAnimations[animationId];
			if (animation == nullptr)
			{
				LogError("PluginAddFrame: Animation is null! id=%d", animationId);
				return -1;
			}
			switch (animation->GetDeviceType())
			{
				case EChromaSDKDeviceTypeEnum::DE_1D:
				{
					Animation1D* animation1D = dynamic_cast<Animation1D*>(animation);
					int maxLeds = ChromaSDKPlugin::GetInstance()->GetMaxLeds(animation1D->GetDevice());
					vector<FChromaSDKColorFrame1D>& frames = animation1D->GetFrames();
					FChromaSDKColorFrame1D frame = FChromaSDKColorFrame1D();
					frame.Duration = duration;
					vector<COLORREF> newColors = ChromaSDKPlugin::GetInstance()->CreateColors1D(animation1D->GetDevice());
					for (int i = 0; i < maxLeds && i < length; ++i)
					{
						newColors[i] = colors[i];
					}
					frame.Colors = newColors;
					frames.push_back(frame);
				}
				break;
			case EChromaSDKDeviceTypeEnum::DE_2D:
				{
					Animation2D* animation2D = dynamic_cast<Animation2D*>(animation);
					int maxRow = ChromaSDKPlugin::GetInstance()->GetMaxRow(animation2D->GetDevice());
					int maxColumn = ChromaSDKPlugin::GetInstance()->GetMaxColumn(animation2D->GetDevice());
					vector<FChromaSDKColorFrame2D>& frames = animation2D->GetFrames();
					FChromaSDKColorFrame2D frame = FChromaSDKColorFrame2D();
					frame.Duration = duration;
					vector<FChromaSDKColors> newColors = ChromaSDKPlugin::GetInstance()->CreateColors2D(animation2D->GetDevice());
					int index = 0;
					for (int i = 0; i < maxRow && index < length; ++i)
					{
						std::vector<COLORREF>& row = newColors[i].Colors;
						for (int j = 0; j < maxColumn && index < length; ++j)
						{
							row[j] = colors[index];
							++index;
						}
					}
					frame.Colors = newColors;
					frames.push_back(frame);
				}
				break;
			}
			return animationId;
		}

		return -1;
	}

	EXPORT_API int PluginUpdateFrame(int animationId, int frameIndex, float duration, int* colors, int length)
	{
		PluginStopAnimation(animationId);

		// Chroma thread plays animations
		SetupChromaThread();

		if (!PluginIsInitialized())
		{
			LogError("PluginUpdateFrame: Plugin is not initialized!\r\n");
			return -1;
		}

		if (_gAnimations.find(animationId) != _gAnimations.end())
		{
			AnimationBase* animation = _gAnimations[animationId];
			if (animation == nullptr)
			{
				LogError("PluginUpdateFrame: Animation is null! id=%d", animationId);
				return -1;
			}
			switch (animation->GetDeviceType())
			{
			case EChromaSDKDeviceTypeEnum::DE_1D:
			{
				Animation1D* animation1D = dynamic_cast<Animation1D*>(animation);
				int maxLeds = ChromaSDKPlugin::GetInstance()->GetMaxLeds(animation1D->GetDevice());
				vector<FChromaSDKColorFrame1D>& frames = animation1D->GetFrames();
				if (frameIndex < 0 || frameIndex >= frames.size())
				{
					LogError("PluginUpdateFrame: frame index is invalid! %d of %d", frameIndex, frames.size());
					return -1;
				}
				FChromaSDKColorFrame1D& frame = frames[frameIndex];
				frame.Duration = duration;
				vector<COLORREF> newColors = ChromaSDKPlugin::GetInstance()->CreateColors1D(animation1D->GetDevice());
				for (int i = 0; i < maxLeds && i < length; ++i)
				{
					newColors[i] = colors[i];
				}
				frame.Colors = newColors;
			}
			break;
			case EChromaSDKDeviceTypeEnum::DE_2D:
			{
				Animation2D* animation2D = dynamic_cast<Animation2D*>(animation);
				int maxRow = ChromaSDKPlugin::GetInstance()->GetMaxRow(animation2D->GetDevice());
				int maxColumn = ChromaSDKPlugin::GetInstance()->GetMaxColumn(animation2D->GetDevice());
				vector<FChromaSDKColorFrame2D>& frames = animation2D->GetFrames();
				if (frameIndex < 0 || frameIndex >= frames.size())
				{
					LogError("PluginUpdateFrame: frame index is invalid! %d of %d", frameIndex, frames.size());
					return -1;
				}
				FChromaSDKColorFrame2D& frame = frames[frameIndex];
				frame.Duration = duration;
				vector<FChromaSDKColors> newColors = ChromaSDKPlugin::GetInstance()->CreateColors2D(animation2D->GetDevice());
				int index = 0;
				for (int i = 0; i < maxRow && index < length; ++i)
				{
					std::vector<COLORREF>& row = newColors[i].Colors;
					for (int j = 0; j < maxColumn && index < length; ++j)
					{
						row[j] = colors[index];
						++index;
					}
				}
				frame.Colors = newColors;
			}
			break;
			}
			return animationId;
		}

		return -1;
	}

	EXPORT_API int PluginGetFrame(int animationId, int frameIndex, float* duration, int* colors, int length)
	{
		PluginStopAnimation(animationId);

		// Chroma thread plays animations
		SetupChromaThread();

		if (!PluginIsInitialized())
		{
			LogError("PluginGetFrame: Plugin is not initialized!\r\n");
			return -1;
		}

		if (_gAnimations.find(animationId) != _gAnimations.end())
		{
			AnimationBase* animation = _gAnimations[animationId];
			if (animation == nullptr)
			{
				LogError("PluginGetFrame: Animation is null! id=%d", animationId);
				return -1;
			}
			switch (animation->GetDeviceType())
			{
			case EChromaSDKDeviceTypeEnum::DE_1D:
			{
				Animation1D* animation1D = dynamic_cast<Animation1D*>(animation);
				int maxLeds = ChromaSDKPlugin::GetInstance()->GetMaxLeds(animation1D->GetDevice());
				vector<FChromaSDKColorFrame1D>& frames = animation1D->GetFrames();
				if (frameIndex < 0 || frameIndex >= frames.size())
				{
					LogError("PluginGetFrame: frame index is invalid! %d of %d", frameIndex, frames.size());
					return -1;
				}
				FChromaSDKColorFrame1D& frame = frames[frameIndex];
				*duration = frame.Duration;
				vector<COLORREF>& frameColors = frame.Colors;
				for (int i = 0; i < maxLeds && i < length; ++i)
				{
					colors[i] = frameColors[i];
				}
			}
			break;
			case EChromaSDKDeviceTypeEnum::DE_2D:
			{
				Animation2D* animation2D = dynamic_cast<Animation2D*>(animation);
				int maxRow = ChromaSDKPlugin::GetInstance()->GetMaxRow(animation2D->GetDevice());
				int maxColumn = ChromaSDKPlugin::GetInstance()->GetMaxColumn(animation2D->GetDevice());
				vector<FChromaSDKColorFrame2D>& frames = animation2D->GetFrames();
				if (frameIndex < 0 || frameIndex >= frames.size())
				{
					LogError("PluginGetFrame: frame index is invalid! %d of %d", frameIndex, frames.size());
					return -1;
				}
				FChromaSDKColorFrame2D& frame = frames[frameIndex];
				*duration = frame.Duration;
				vector<FChromaSDKColors>& frameColors = frame.Colors;
				int index = 0;
				for (int i = 0; i < maxRow && index < length; ++i)
				{
					std::vector<COLORREF>& row = frameColors[i].Colors;
					for (int j = 0; j < maxColumn && index < length; ++j)
					{
						colors[index] = row[j];
						++index;
					}
				}
			}
			break;
			}
			return animationId;
		}

		return -1;
	}

	EXPORT_API int PluginPreviewFrame(int animationId, int frameIndex)
	{
		//LogDebug("PluginPreviewFrame: animationId=%d frameIndex=%d\r\n", animationId, frameIndex);

		PluginStopAnimation(animationId);

		// Chroma thread plays animations
		SetupChromaThread();

		if (!PluginIsInitialized())
		{
			LogError("PluginPreviewFrame: Plugin is not initialized!\r\n");
			return -1;
		}

		if (_gAnimations.find(animationId) != _gAnimations.end())
		{
			AnimationBase* animation = _gAnimations[animationId];
			if (animation == nullptr)
			{
				LogError("PluginPreviewFrame: Animation is null! id=%d", animationId);
				return -1;
			}
			switch (animation->GetDeviceType())
			{
			case EChromaSDKDeviceTypeEnum::DE_1D:
			{
				Animation1D* animation1D = dynamic_cast<Animation1D*>(animation);
				int maxLeds = ChromaSDKPlugin::GetInstance()->GetMaxLeds(animation1D->GetDevice());
				vector<FChromaSDKColorFrame1D>& frames = animation1D->GetFrames();
				if (frameIndex < 0 || frameIndex >= frames.size())
				{
					LogError("PluginPreviewFrame: frame index is invalid! %d of %d", frameIndex, frames.size());
					return -1;
				}
				FChromaSDKColorFrame1D frame = frames[frameIndex];
				vector<COLORREF>& colors = frame.Colors;
				FChromaSDKEffectResult result = ChromaSDKPlugin::GetInstance()->CreateEffectCustom1D(animation1D->GetDevice(), colors);
				if (result.Result == 0)
				{
					ChromaSDKPlugin::GetInstance()->SetEffect(result.EffectId);
					ChromaSDKPlugin::GetInstance()->DeleteEffect(result.EffectId);
				}
			}
			break;
			case EChromaSDKDeviceTypeEnum::DE_2D:
			{
				Animation2D* animation2D = dynamic_cast<Animation2D*>(animation);
				int maxRow = ChromaSDKPlugin::GetInstance()->GetMaxRow(animation2D->GetDevice());
				int maxColumn = ChromaSDKPlugin::GetInstance()->GetMaxColumn(animation2D->GetDevice());
				vector<FChromaSDKColorFrame2D>& frames = animation2D->GetFrames();
				if (frameIndex < 0 || frameIndex >= frames.size())
				{
					LogError("PluginPreviewFrame: frame index is invalid! %d of %d", frameIndex, frames.size());
					return -1;
				}
				FChromaSDKColorFrame2D frame = frames[frameIndex];
				vector<FChromaSDKColors>& colors = frame.Colors;
				FChromaSDKEffectResult result = ChromaSDKPlugin::GetInstance()->CreateEffectCustom2D(animation2D->GetDevice(), colors);
				if (result.Result == 0)
				{
					ChromaSDKPlugin::GetInstance()->SetEffect(result.EffectId);
					ChromaSDKPlugin::GetInstance()->DeleteEffect(result.EffectId);
				}
			}
			break;
			}
			return animationId;
		}

		return -1;
	}

	EXPORT_API int PluginOverrideFrameDuration(int animationId, float duration)
	{
		PluginStopAnimation(animationId);

		// Chroma thread plays animations
		SetupChromaThread();

		if (!PluginIsInitialized())
		{
			LogError("PluginOverrideFrameDuration: Plugin is not initialized!\r\n");
			return -1;
		}

		if (_gAnimations.find(animationId) != _gAnimations.end())
		{
			AnimationBase* animation = _gAnimations[animationId];
			if (animation == nullptr)
			{
				LogError("PluginOverrideFrameDuration: Animation is null! id=%d", animationId);
				return -1;
			}
			switch (animation->GetDeviceType())
			{
			case EChromaSDKDeviceTypeEnum::DE_1D:
			{
				Animation1D* animation1D = dynamic_cast<Animation1D*>(animation);
				vector<FChromaSDKColorFrame1D>& frames = animation1D->GetFrames();
				for (int i = 0; i < frames.size(); ++i)
				{
					FChromaSDKColorFrame1D& frame = frames[i];
					frame.Duration = duration;
				}
			}
			break;
			case EChromaSDKDeviceTypeEnum::DE_2D:
			{
				Animation2D* animation2D = dynamic_cast<Animation2D*>(animation);
				vector<FChromaSDKColorFrame2D>& frames = animation2D->GetFrames();
				for (int i = 0; i < frames.size(); ++i)
				{
					FChromaSDKColorFrame2D& frame = frames[i];
					frame.Duration = duration;
				}
			}
			break;
			}
			return animationId;
		}

		return -1;
	}

	EXPORT_API int PluginReverse(int animationId)
	{
		PluginStopAnimation(animationId);

		// Chroma thread plays animations
		SetupChromaThread();

		if (!PluginIsInitialized())
		{
			LogError("PluginReverse: Plugin is not initialized!\r\n");
			return -1;
		}

		if (_gAnimations.find(animationId) != _gAnimations.end())
		{
			AnimationBase* animation = _gAnimations[animationId];
			if (animation == nullptr)
			{
				LogError("PluginReverse: Animation is null! id=%d", animationId);
				return -1;
			}
			switch (animation->GetDeviceType())
			{
			case EChromaSDKDeviceTypeEnum::DE_1D:
			{
				Animation1D* animation1D = dynamic_cast<Animation1D*>(animation);
				vector<FChromaSDKColorFrame1D>& frames = animation1D->GetFrames();
				vector<FChromaSDKColorFrame1D> copy = vector<FChromaSDKColorFrame1D>();
				for (int i = frames.size() - 1; i >= 0; --i)
				{
					FChromaSDKColorFrame1D& frame = frames[i];
					copy.push_back(frame);
				}
				frames.clear();
				for (int i = 0; i < copy.size(); ++i)
				{
					FChromaSDKColorFrame1D& frame = copy[i];
					frames.push_back(frame);
				}
			}
			break;
			case EChromaSDKDeviceTypeEnum::DE_2D:
			{
				Animation2D* animation2D = dynamic_cast<Animation2D*>(animation);
				vector<FChromaSDKColorFrame2D>& frames = animation2D->GetFrames();
				vector<FChromaSDKColorFrame2D> copy = vector<FChromaSDKColorFrame2D>();
				for (int i = frames.size() - 1; i >= 0; --i)
				{
					FChromaSDKColorFrame2D& frame = frames[i];
					copy.push_back(frame);
				}
				frames.clear();
				for (int i = 0; i < copy.size(); ++i)
				{
					FChromaSDKColorFrame2D& frame = copy[i];
					frames.push_back(frame);
				}
			}
			break;
			}
			return animationId;
		}

		return -1;
	}

	EXPORT_API int PluginMirrorHorizontally(int animationId)
	{
		PluginStopAnimation(animationId);

		// Chroma thread plays animations
		SetupChromaThread();

		if (!PluginIsInitialized())
		{
			LogError("PluginMirrorHorizontally: Plugin is not initialized!\r\n");
			return -1;
		}

		if (_gAnimations.find(animationId) != _gAnimations.end())
		{
			AnimationBase* animation = _gAnimations[animationId];
			if (animation == nullptr)
			{
				LogError("PluginMirrorHorizontally: Animation is null! id=%d", animationId);
				return -1;
			}
			switch (animation->GetDeviceType())
			{
			case EChromaSDKDeviceTypeEnum::DE_1D:
			{
				Animation1D* animation1D = dynamic_cast<Animation1D*>(animation);
				vector<FChromaSDKColorFrame1D>& frames = animation1D->GetFrames();
				int maxLeds = ChromaSDKPlugin::GetInstance()->GetMaxLeds(animation1D->GetDevice());
				for (int index = 0; index < frames.size(); ++index)
				{
					FChromaSDKColorFrame1D& frame = frames[index];
					std::vector<COLORREF>& colors = frame.Colors;
					std::vector<COLORREF> newColors = ChromaSDKPlugin::GetInstance()->CreateColors1D(animation1D->GetDevice());
					for (int i = 0; i < maxLeds; ++i)
					{
						int reverse = maxLeds - 1 - i;
						newColors[i] = colors[reverse];
					}
					frame.Colors = newColors;
				}
			}
			break;
			case EChromaSDKDeviceTypeEnum::DE_2D:
			{
				Animation2D* animation2D = dynamic_cast<Animation2D*>(animation);
				vector<FChromaSDKColorFrame2D>& frames = animation2D->GetFrames();
				int maxRow = ChromaSDKPlugin::GetInstance()->GetMaxRow(animation2D->GetDevice());
				int maxColumn = ChromaSDKPlugin::GetInstance()->GetMaxColumn(animation2D->GetDevice());
				for (int index = 0; index < frames.size(); ++index)
				{
					FChromaSDKColorFrame2D& frame = frames[index];
					std::vector<FChromaSDKColors>& colors = frame.Colors;
					std::vector<FChromaSDKColors> newColors = ChromaSDKPlugin::GetInstance()->CreateColors2D(animation2D->GetDevice());
					for (int i = 0; i < maxRow; ++i)
					{
						std::vector<COLORREF>& row = colors[i].Colors;
						std::vector<COLORREF>& newRow = newColors[i].Colors;
						for (int j = 0; j < maxColumn; ++j)
						{
							int reverse = maxColumn - 1 - j;
							newRow[j] = row[reverse];
						}
					}
					frame.Colors = newColors;
				}
			}
			break;
			}
			return animationId;
		}

		return -1;
	}

	EXPORT_API int PluginMirrorVertically(int animationId)
	{
		PluginStopAnimation(animationId);

		// Chroma thread plays animations
		SetupChromaThread();

		if (!PluginIsInitialized())
		{
			LogError("PluginMirrorVertically: Plugin is not initialized!\r\n");
			return -1;
		}

		if (_gAnimations.find(animationId) != _gAnimations.end())
		{
			AnimationBase* animation = _gAnimations[animationId];
			if (animation == nullptr)
			{
				LogError("PluginMirrorVertically: Animation is null! id=%d", animationId);
				return -1;
			}
			switch (animation->GetDeviceType())
			{
			case EChromaSDKDeviceTypeEnum::DE_1D:
				//skip, only 1 high
				break;
			case EChromaSDKDeviceTypeEnum::DE_2D:
			{
				Animation2D* animation2D = dynamic_cast<Animation2D*>(animation);
				vector<FChromaSDKColorFrame2D>& frames = animation2D->GetFrames();
				int maxRow = ChromaSDKPlugin::GetInstance()->GetMaxRow(animation2D->GetDevice());
				for (int index = 0; index < frames.size(); ++index)
				{
					FChromaSDKColorFrame2D& frame = frames[index];
					std::vector<FChromaSDKColors>& colors = frame.Colors;
					std::vector<FChromaSDKColors> newColors = ChromaSDKPlugin::GetInstance()->CreateColors2D(animation2D->GetDevice());
					for (int i = 0; i < maxRow; ++i)
					{
						int reverse = maxRow - 1 - i;
						newColors[reverse].Colors = colors[i].Colors;
					}
					frame.Colors = newColors;
				}
			}
			break;
			}
			return animationId;
		}

		return -1;
	}
}
