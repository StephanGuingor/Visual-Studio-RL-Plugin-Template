#pragma once
#include "pch.h"
#pragma comment( lib, "bakkesmod.lib" )
#include "bakkesmod/plugin/bakkesmodplugin.h"

class FancyPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
private:
		bool recoveryEnabled = false;
		float lastBumpTime = 0.0f;
		float lastCooldownTime = 0.0f;
public:
		virtual void onLoad();
		virtual void onUnload();
		void CheckForBump();
		float GetBumpTimeout();
		void ExecuteBump();
	};

