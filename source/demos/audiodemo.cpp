//************************************************************************************************
//
// CCL Demo Application
//
// This file is part of Crystal Class Library (R)
// Copyright (c) 2026 CCL Software Licensing GmbH.
// All Rights Reserved.
//
// Licensed for use under either:
//  1. a Commercial License provided by CCL Software Licensing GmbH, or
//  2. GNU Affero General Public License v3.0 (AGPLv3).
// 
// You must choose and comply with one of the above licensing options.
// For more information, please visit ccl.dev.
//
// Filename    : audiodemo.cpp
// Description : Audio Demo
//
//************************************************************************************************

#define DEBUG_LOG 1

#include "../demoitem.h"

#include "ccl/base/asyncoperation.h"

#include "ccl/app/params.h"
#include "ccl/app/components/bindingcomponent.h"

#include "ccl/extras/audio/runtime/audiodevicesession.h"
#include "ccl/extras/audio/runtime/audiodevicesetup.h"
#include "ccl/extras/audio/runtime/audioplayercomponent.h"
#include "ccl/extras/audio/runtime/audiorecordercomponent.h"
#include "ccl/extras/audio/runtime/audiotransport.h"
#include "ccl/extras/audio/runtime/audiotransportcomponent.h"
#include "ccl/extras/audio/meter/audiometercomponent.h"
#include "ccl/extras/audio/audiofileio.h"
#include "ccl/extras/audio/audiocurves.h"
#include "ccl/extras/audio/audioformatter.h"
#include "ccl/public/audio/audioparamids.h"

#include "ccl/public/gui/framework/ialert.h"
#include "ccl/public/gui/framework/ifileselector.h"

#include "ccl/public/system/formatter.h"
#include "ccl/public/plugservices.h"

using namespace CCL;

//////////////////////////////////////////////////////////////////////////////////////////////////

CCL_KERNEL_INIT_LEVEL (AudioDemo, kAppLevel+1)
{
	AudioFileBase::initCodecs ();
	return true;
}

CCL_KERNEL_TERM_LEVEL (AudioDemo, kAppLevel+1)
{
	AudioFileBase::exitCodecs ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Tags/NodeIDs
//////////////////////////////////////////////////////////////////////////////////////////////////

namespace Tag
{
	enum AudioDemoTags
	{
		kCpuLoad = 100,
		kOverload,
		kRequestedSampleRate,
		kPlayerVolume,
		kPlayerPan,
		kOpenFile,
		kCloseFile,
		kDemoMode
	};
}

namespace NodeID
{
	const CStringPtr kGain = "gain";
	const CStringPtr kPan = "pan";
	const CStringPtr kOutputMeter = "outputMeter";
	const CStringPtr kInputMeter = "inputMeter";
	const CStringPtr kOscillator = "oscillator";
	const CStringPtr kNoiseGenerator = "noiseGenerator";
}

//************************************************************************************************
// AudioDemo
//************************************************************************************************

class AudioDemo: public DemoComponent,
				 private IdleClient
{
public:
	enum Mode
	{
		kDemoModePlayer,
		kDemoModeRecorder,
		kDemoModeOscillator,
		kDemoModeNoise,
		kDemoModeLast = kDemoModeNoise
	};

	AudioDemo ()
	: session (NEW AudioDeviceSession),
	  transport (NEW AudioTransport)
	{
		session->addObserver (this);

		addComponent (NEW AudioDeviceSetup (*session));
		addComponent (NEW AudioTransportComponent (*transport));
		addComponent (playerComponent = NEW AudioPlayerComponent);
		addComponent (recorderComponent = NEW AudioRecorderComponent);		
		addComponent (outputMeterComponent = NEW AudioMeterComponent ("OutputMeter"));
		addComponent (inputMeterComponent = NEW AudioMeterComponent ("InputMeter"));

		AudioPlayer& player = playerComponent->getPlayer ();
		transport->addClient (&player);
		playerComponent->setTransport (transport);

		AudioRecorder& recorder = recorderComponent->getRecorder ();
		transport->addClient (&recorder);
		recorderComponent->setTransport (transport);

		audioGraph = createAudioGraph (player, recorder);
		session->setProcessor (audioGraph->asProcessor ());

		outputMeterComponent->setMeterSource (&audioGraph->find<IAudioMeterNode> (NodeID::kOutputMeter)->getMeterSource ());
		inputMeterComponent->setMeterSource (&audioGraph->find<IAudioMeterNode> (NodeID::kInputMeter)->getMeterSource ());

		AutoPtr<AudioMeterSettings> meterSettings = NEW AudioMeterSettings;
		meterSettings->setClipEnabled (true);
		meterSettings->setRMSEnabled (true);
		meterSettings->setPeakHoldEnabled (true);

		outputMeterComponent->setMeterSettings (meterSettings);
		inputMeterComponent->setMeterSettings (meterSettings);

		UnknownPtr<IParameterProvider> oscillatorNode = audioGraph->find<IAudioNode> (NodeID::kOscillator);
		UnknownPtr<IParameterProvider> noiseGeneratorNode = audioGraph->find<IAudioNode> (NodeID::kNoiseGenerator);
		addComponent (oscillatorComponent = NEW ParameterBindingComponent (*oscillatorNode, "Oscillator"));
		addComponent (noiseGeneratorComponent = NEW ParameterBindingComponent (*noiseGeneratorNode, "NoiseGenerator"));

		IParameter* cpuLoadParam = paramList.addFloat (0., 1., CSTR ("cpuLoad"), Tag::kCpuLoad);
		cpuLoadParam->setFormatter (AutoPtr<IFormatter> (NEW Format::Percent));
		cpuLoadParam->setReadOnly ();
		paramList.addParam (CSTR ("overload"), Tag::kOverload)->setReadOnly ();

		ListParam* sampleRateList = unknown_cast<ListParam> (paramList.addList (CSTR ("requestedSampleRate"), Tag::kRequestedSampleRate));
		sampleRateList->setFormatter (AutoPtr<IFormatter> (NEW Format::SampleRate));
		constexpr SampleRate kSampleRates[] = {44100, 48000, 88200, 96000, 176400, 192000};
		for(int rate : kSampleRates)
			sampleRateList->appendValue (rate);
		session->setRequestedSampleRate (kSampleRates[0]);

		const double kMaxGain = AudioFaderGainCurve::getMaxFactor ();
		IParameter* volumeParam = paramList.addFloat (0, kMaxGain, CSTR ("volume"), Tag::kPlayerVolume);
		volumeParam->setCurve (AutoPtr<IParamCurve> (NEW AudioFaderGainCurve));
		volumeParam->setFormatter (AutoPtr<IFormatter> (NEW Format::GainDB));
		volumeParam->setDefaultValue (1.f);

		IParameter* panParam = paramList.addFloat (-1., 1., CSTR ("pan"), Tag::kPlayerPan);
		panParam->setBipolar (true);
		panParam->setFormatter (AutoPtr<IFormatter> (NEW Format::StereoPan));
		panParam->setDefaultValue (0.f);

		volumeParam->setValue (audioGraph->find<IAudioGainNode> (NodeID::kGain)->getGain ());
		panParam->setValue (audioGraph->find<IAudioPanNode> (NodeID::kPan)->getPan ());

		paramList.addParam (CSTR ("openFile"), Tag::kOpenFile);
		paramList.addParam (CSTR ("closeFile"), Tag::kCloseFile)->enable (false);

		paramList.addInteger (0, kDemoModeLast, CSTR ("demoMode"), Tag::kDemoMode);
		setDemoMode (kDemoModePlayer);
	}

	~AudioDemo ()
	{
		outputMeterComponent->setMeterSource (nullptr);
		inputMeterComponent->setMeterSource (nullptr);

		session->setProcessor (nullptr);
		session->removeObserver (this);

		transport->setActive (false);
		transport->setClock (nullptr);

		transport->removeClient (&playerComponent->getPlayer ());
		transport->removeClient (&recorderComponent->getRecorder ());		

		ccl_release (audioGraph);
	}

	void onSessionStateChanged ()
	{
		bool active = session->getState () == AudioDeviceSession::kActive;
		if(active != isTimerEnabled ())
		{
			CCL_PRINTF ("Audio session active: %s\n", active ? "true" : "false")

			transport->setActive (active);
			session->setPerformanceMeterActive (active);
			outputMeterComponent->setMeterActive (active);
			inputMeterComponent->setMeterActive (active);

			if(active)
				startTimer (kPerformanceUpdateInterval);
			else
			{
				stopTimer ();

				paramList.byTag (Tag::kCpuLoad)->setValue (0.);
				paramList.byTag (Tag::kOverload)->setValue (false);
			}
		}
	}

	void setDemoMode (Mode newMode)
	{
		CCL_PRINTF ("Switching to audio demo mode %d\n", int(newMode))

		// stop transport
		if(transport->isActive ())
			transport->stop ();

		// turn off generators
		if(lastDemoMode == kDemoModeOscillator || lastDemoMode == kDemoModeNoise)
		{
			if(IParameter* p = oscillatorComponent->findParameter (AudioParamID::Generator::kOn))
				p->setValue (0, true);
			if(IParameter* p = noiseGeneratorComponent->findParameter (AudioParamID::Generator::kOn))
				p->setValue (0, true);
		}

		if(newMode == kDemoModePlayer)
		{
			AudioPlayer& player = playerComponent->getPlayer ();
			transport->setClock (&player);

			// load last recorded take into player
			if(lastDemoMode == kDemoModeRecorder)
				if(const Url* recordedFile = recorderComponent->getLastRecordedFile ())
					loadFileForPlayback (*recordedFile);
		}
		else if(newMode == kDemoModeRecorder)
		{
			session->requestAudioCapturePermission ();

			AudioRecorder& recorder = recorderComponent->getRecorder ();
			transport->setClock (&recorder);
		}
		else
		{
			transport->setClock (nullptr);
		}

		paramList.byTag (Tag::kDemoMode)->setValue (int(newMode));
		lastDemoMode = newMode;
	}

	void loadFileForPlayback (UrlRef path, bool warn = false)
	{
		if(playerComponent->loadFile (path) == false)
		{
			if(warn == true)
				Promise (Alert::warnAsync ("Could not open audio file."));
		}

		paramList.byTag (Tag::kCloseFile)->enable (playerComponent->getPlayer ().hasSource ());
	}

	// IdleClient
	void onIdleTimer () override
	{
		IPerformanceMeter& meter = session->getPerformanceMeter ();
		paramList.byTag (Tag::kCpuLoad)->setValue (meter.getPerformance ());
		paramList.byTag (Tag::kOverload)->setValue (meter.isOverload ());
	}

	CLASS_INTERFACE (ITimerTask, DemoComponent)

	// DemoComponent
	void CCL_API notify (ISubject* subject, MessageRef msg) override
	{
		if(msg == kChanged && subject == session)
		{
			onSessionStateChanged ();
		}
	}

	tbool CCL_API paramChanged (IParameter* param) override
	{
		switch(param->getTag ())
		{
		case Tag::kOpenFile :
			{
				FileTypeFilter fileTypes;
				AudioFileBase::getFileTypes (fileTypes);

				if(!fileTypes.getContent ().isEmpty ())
				{
					AutoPtr<IFileSelector> fs = ccl_new<IFileSelector> (ClassID::FileSelector);
					for(auto fileType : fileTypes.getContent ())
						fs->addFilter (fileType);
					
					Promise p = fs->runAsync (IFileSelector::kOpenFile);
					p.then ([&, fs] (IAsyncOperation& op)
					{
						const IUrl* path = nullptr;
						if(op.getResult ().asBool ())
							path = fs->getPath (0);

						if(path)
							loadFileForPlayback (*path);
					});
				}
				else
				{
					Promise (Alert::infoAsync ("No audio codecs found."));
				}
			}
			break;

		case Tag::kCloseFile :
			playerComponent->unloadFile ();
			param->enable (false);
			break;

		case Tag::kRequestedSampleRate :
			session->setRequestedSampleRate (paramList.byTag<ListParam> (Tag::kRequestedSampleRate)->getSelectedValue ());
			break;

		case Tag::kPlayerVolume :
			audioGraph->find<IAudioGainNode> (NodeID::kGain)->setGain (param->getValue ());
			break;

		case Tag::kPlayerPan :
			audioGraph->find<IAudioPanNode> (NodeID::kPan)->setPan (param->getValue ());
			break;
		
		case Tag::kDemoMode :
			setDemoMode (static_cast<Mode> (param->getValue ().asInt ()));
			break;
		}
		return true;
	}

private:
	static constexpr int kPerformanceUpdateInterval = 250; // milliseconds

	Mode lastDemoMode = kDemoModePlayer;
	AutoPtr<AudioDeviceSession> session;
	AutoPtr<AudioTransport> transport;
	IAudioGraph* audioGraph = nullptr;
	AudioPlayerComponent* playerComponent = nullptr;
	AudioRecorderComponent* recorderComponent = nullptr;
	AudioMeterComponent* outputMeterComponent = nullptr;
	AudioMeterComponent* inputMeterComponent = nullptr;
	ParameterBindingComponent* oscillatorComponent = nullptr;
	ParameterBindingComponent* noiseGeneratorComponent = nullptr;

	static IAudioGraph* createAudioGraph (AudioPlayer& player, AudioRecorder& recorder)
	{
		IAudioGraph* graph = ccl_new<IAudioGraph> (ClassID::AudioGraph);
		ASSERT (graph != nullptr)

		ASSERT (AudioPlayer::kOutputChannelCount == AudioRecorder::kChannelCount)
		constexpr int kChannelCount = AudioPlayer::kOutputChannelCount;

		// 1:0 layout for endpoint
		AudioStreamLayout endpointLayout;
		endpointLayout.addChannelCount (kAudioInputDirection, kChannelCount);

		// 0:1 layout for sources
		AudioStreamLayout sourceLayout;
		sourceLayout.addChannelCount (kAudioOutputDirection, kChannelCount);
		
		// 1:1 layout for processors
		AudioStreamLayout processLayout;
		processLayout.addChannelCount (kAudioInputDirection, kChannelCount);
		processLayout.addChannelCount (kAudioOutputDirection, kChannelCount);

		// Playback : player -> gain -> output meter -> pan -> output

		auto* playerNode = ccl_new<IAudioProcessorNode> (ClassID::AudioProcessorNode);
		playerNode->setStreamLayout (player.getStreamLayout ());
		playerNode->setProcessor (&player);
		graph->addNode (playerNode);
		
		auto* gainNode = ccl_new<IAudioGainNode> (ClassID::AudioGainNode);
		gainNode->setID (NodeID::kGain);
		gainNode->setStreamLayout (processLayout);
		graph->addNode (gainNode);

		auto* outputMeterNode = ccl_new<IAudioMeterNode> (ClassID::AudioMeterNode);
		outputMeterNode->setID (NodeID::kOutputMeter);
		outputMeterNode->setStreamLayout (processLayout);
		graph->addNode (outputMeterNode);

		auto* panNode = ccl_new<IAudioPanNode> (ClassID::AudioPanNode);
		panNode->setID (NodeID::kPan);
		panNode->setStreamLayout (processLayout);
		graph->addNode (panNode);

		auto* outputNode = ccl_new<IAudioOutputNode> (ClassID::AudioOutputNode);
		outputNode->setStreamLayout (endpointLayout);
		outputNode->setOutputStream (0);
		graph->addNode (outputNode);
	
		playerNode->connect (gainNode);
		gainNode->connect (outputMeterNode);
		outputMeterNode->connect (panNode);
		panNode->connect (outputNode);

		// Recording: input -> input meter -> recorder -> output
		
		auto* inputNode = ccl_new<IAudioInputNode> (ClassID::AudioInputNode);
		inputNode->setStreamLayout (sourceLayout);
		inputNode->setInputStream (0);
		graph->addNode (inputNode);

		auto* inputMeterNode = ccl_new<IAudioMeterNode> (ClassID::AudioMeterNode);
		inputMeterNode->setID (NodeID::kInputMeter);
		inputMeterNode->setStreamLayout (processLayout);
		graph->addNode (inputMeterNode);

		auto* recorderNode = ccl_new<IAudioProcessorNode> (ClassID::AudioProcessorNode);
		recorderNode->setStreamLayout (recorder.getStreamLayout ());
		recorderNode->setProcessor (&recorder);
		graph->addNode (recorderNode);

		inputNode->connect (inputMeterNode);
		inputMeterNode->connect (recorderNode);
		recorderNode->connect (outputNode);

		// Generators - connected to output meter
		AutoPtr<IAudioProcessor> oscillator = ccl_new<IAudioProcessor> (ClassID::AudioOscillator);
		UnknownPtr<IAudioComponent> (oscillator)->setStreamLayout (sourceLayout);

		auto* oscillatorNode = ccl_new<IAudioProcessorNode> (ClassID::AudioProcessorNode);
		oscillatorNode->setID (NodeID::kOscillator);
		oscillatorNode->setStreamLayout (sourceLayout);
		oscillatorNode->setProcessor (oscillator);
		graph->addNode (oscillatorNode);

		oscillatorNode->connect (outputMeterNode);

		AutoPtr<IAudioProcessor> noiseGenerator = ccl_new<IAudioProcessor> (ClassID::AudioNoiseGenerator);
		UnknownPtr<IAudioComponent> (noiseGenerator)->setStreamLayout (sourceLayout);

		auto* noiseGeneratorNode = ccl_new<IAudioProcessorNode> (ClassID::AudioProcessorNode);
		noiseGeneratorNode->setID (NodeID::kNoiseGenerator);
		noiseGeneratorNode->setStreamLayout (sourceLayout);
		noiseGeneratorNode->setProcessor (noiseGenerator);
		graph->addNode (noiseGeneratorNode);

		noiseGeneratorNode->connect (outputMeterNode);

		return graph;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("System", "Audio", AudioDemo, DEMO_PAGE_SCROLLABLE, DOCLINK_DEFAULT)
