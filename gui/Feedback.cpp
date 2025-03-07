#include "Feedback.hpp"
#include "MainDisplay.hpp"
#include "main.hpp"

#include "../libs/chesto/src/RootDisplay.hpp"

#ifndef NETWORK_MOCK
#include <curl/curl.h>
#include <curl/easy.h>
#endif

Feedback::Feedback(Package* package)
	: package(package)
	, title((std::string("Leaving feedback for: \"") + package->title + "\"").c_str(), 25)
	, icon(package->getIconUrl().c_str(), []{ return new ImageElement(RAMFS "res/default.png"); })
	, quit("Discard", Y_BUTTON, false, 20)
	, send("Submit", X_BUTTON, false, 20)
	, backspaceBtn("Del", B_BUTTON, false, 15)
	, capsBtn("Caps", L_BUTTON, false, 15)
	, response("If you need to send more detailed feedback, please email us at fight@fortheusers.org", 20, NULL, false, 460)
{
	title.position(50, 30);
	super::append(&title);

	icon.setScaleMode(SCALE_PROPORTIONAL_WITH_BG);
	icon.position(50, 100);
#if defined(_3DS) || defined(_3DS_MOCK)
  icon.resize(ICON_SIZE, ICON_SIZE);
#else
	icon.resize(256, ICON_SIZE);
#endif
	super::append(&icon);

	keyboard.typeAction = std::bind(&Feedback::keyboardInputCallback, this);
	keyboard.preventEnterAndTab = true;
	keyboard.updateSize();
	super::append(&keyboard);

	quit.position(895, 240);
	quit.action = std::bind(&Feedback::back, this);
	super::append(&quit);

	send.position(quit.x + quit.width + 15, quit.y);
	send.action = std::bind(&Feedback::submit, this);
	super::append(&send);

	backspaceBtn.position(quit.x - 15 - backspaceBtn.width, send.y);
	backspaceBtn.action = [this](void) {
		this->keyboard.backspace();
	};
	super::append(&backspaceBtn);

	capsBtn.position(backspaceBtn.x - 15 - capsBtn.width, send.y);
	capsBtn.action = [this](void) {
		this->keyboard.capsOn = !this->keyboard.capsOn;
		this->keyboard.updateSize();
	};
	super::append(&capsBtn);

	response.position(860, 20);
	super::append(&response);

	feedback.setSize(23);
	feedback.setWrappedWidth(730);
	feedback.position(390, 100);
	super::append(&feedback);
}

void Feedback::render(Element* parent)
{
	// draw a white background, 870 wiz
	CST_Color white = { 0xff, 0xff, 0xff, 0xff };

	  if (parent != NULL) {
    CST_SetDrawColor(RootDisplay::renderer, white);
  }

	return super::render(parent);
}

bool Feedback::process(InputEvents* event)
{
	bool ret = super::process(event);

	if (needsRefresh)
	{
		feedback.setText(keyboard.getTextInput());
		feedback.update();
		needsRefresh = false;
	}

	return ret;
}

void Feedback::keyboardInputCallback()
{
	needsRefresh = true;
}

void Feedback::submit()
{
#ifndef NETWORK_MOCK
	CURL* curl;
	CURLcode res;

#if defined(__WIIU__)
	const char* userKey = "wiiu_user";
#else
	const char* userKey = "switch_user";
#endif

	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "http://switchbru.com/appstore/feedback");
		std::string fields = std::string("name=") + userKey + "&package=" + package->pkg_name + "&message=" + keyboard.getTextInput();
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields.c_str());

		res = curl_easy_perform(curl);

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
#endif

	// close this window
	this->back();
}

void Feedback::back()
{
	RootDisplay::switchSubscreen(nullptr);
}
