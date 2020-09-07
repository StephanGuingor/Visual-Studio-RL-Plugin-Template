#include "FancyPlugin.h"
#include "pch.h"
#include "bakkesmod\wrappers\GameEvent\TutorialWrapper.h"
#include "bakkesmod\wrappers\GameObject\BallWrapper.h"
#include "bakkesmod\wrappers\GameObject\CarWrapper.h"
#include "utils\parser.h"
#include <random>

using namespace std;

BAKKESMOD_PLUGIN(FancyPlugin, "Fancy Plugin", "0.1", PLUGINTYPE_FREEPLAY)

/**
On Load:

Sets initial values for the bumps, and adds a start event.
And starts the CheckForBump routine.
*/
void FancyPlugin::onLoad()
{
	cvarManager->registerCvar("air_recovery_bumpspeed_angular", "(0, 6)", "How hard you will get thrown rotationally", true, true, -20.f, true, 20.f);
	cvarManager->registerCvar("air_recovery_bumpspeed_linear", "(800, 1100)", "How hard you will get thrown", true, true, 0.f, true, 3000.f);
	cvarManager->registerCvar("air_recovery_bumpspeed_linear_z", "(-100, 500)", "How hard you will get thrown (height)", true, true, -2000.f, true, 2000.f);
	cvarManager->registerCvar("air_recovery_cooldown", "(3000, 6000)", "Minimum time to wait after a bump", true, true, 0.f, true, 10000.f);

	cvarManager->registerNotifier("air_recovery_start", [this](std::vector<string> params) {
		if (!gameWrapper->IsInGame())
		{
			cvarManager->log("You need to be in freeplay to use this plugin. Sorry m8.");
			return;
		}
		lastCooldownTime = cvarManager->getCvar("air_recovery_cooldown").getFloatValue() / 1000;
		this->recoveryEnabled = true;
		this->CheckForBump();
		}, "Starts the recovery mode which will bump you with the given settings.", PERMISSION_FREEPLAY);

	cvarManager->registerNotifier("air_recovery_stop", [this](std::vector<string> params) {
		this->recoveryEnabled = false;
		}, "Stops the recovery plugin.", PERMISSION_ALL);
}


/**
CheckForBump:
Checks that it is a good enviorment to bump.
And calls GetBumpTimeout on loop
And has a callback to itself.

*/
void FancyPlugin::CheckForBump()
{
	if (!recoveryEnabled || !gameWrapper->IsInFreeplay())
		return; //Player stopped recovery training or left freeplay

	gameWrapper->SetTimeout([this](GameWrapper* gw) {
		// Callback
		this->CheckForBump();
		// Called on loop
		}, this->GetBumpTimeout());
}


/**
GetBumpTimeout: 
updates the last bump, if cooldown completes a bump executes.
*/
float FancyPlugin::GetBumpTimeout()
{
	if (!gameWrapper->IsInGame() || !recoveryEnabled)
		return .5f;
	ServerWrapper training = gameWrapper->GetGameEventAsServer();
	float lastBump = training.GetSecondsElapsed() - lastBumpTime;
	if (lastBump > lastCooldownTime)
	{
		auto gameCar = training.GetGameCar();

		if (!gameCar.IsOnGround() && !gameCar.IsOnWall()) //player is in the air
		{
			ExecuteBump();
			return lastCooldownTime;
		}
		return random(.2f, 1.f);
	}
	return lastCooldownTime - lastBump + 0.1f;
}

void FancyPlugin::ExecuteBump()
{
	if (!gameWrapper->IsInGame() || !recoveryEnabled)
		return;
	auto tutorial = gameWrapper->GetGameEventAsServer();



	Vector angularBump = {
		cvarManager->getCvar("air_recovery_bumpspeed_angular").getFloatValue(),
		cvarManager->getCvar("air_recovery_bumpspeed_angular").getFloatValue(),
		cvarManager->getCvar("air_recovery_bumpspeed_angular").getFloatValue()
	};
	Vector linearBump = {
		cvarManager->getCvar("air_recovery_bumpspeed_linear").getFloatValue(),
		cvarManager->getCvar("air_recovery_bumpspeed_linear").getFloatValue(),
		cvarManager->getCvar("air_recovery_bumpspeed_linear_z").getFloatValue()
	};


	angularBump.X = random(0, 1) == 1 ? angularBump.X : -angularBump.X;
	angularBump.Y = random(0, 1) == 1 ? angularBump.Y : -angularBump.Y;
	angularBump.Z = random(0, 1) == 1 ? angularBump.Z : -angularBump.Z;

	linearBump.X = random(0, 1) == 1 ? linearBump.X : -linearBump.X;
	linearBump.Y = random(0, 1) == 1 ? linearBump.Y : -linearBump.Y;

	auto car = tutorial.GetGameCar();
	car.SetAngularVelocity(angularBump, 0);
	car.SetVelocity(linearBump);


	lastBumpTime = tutorial.GetSecondsElapsed();
	lastCooldownTime = cvarManager->getCvar("air_recovery_cooldown").getFloatValue() / 1000;
}

void FancyPlugin::onUnload()
{
	this->recoveryEnabled = false;
}