#pragma once
#include <mutex>
#include "stdafx.h"

enum class overlay_status : int
{
	creating = 1,
	source_ready,
	working,
	destroing
};

class overlay_window
{
	protected:
	RECT rect;
	std::mutex rect_access;

	public:
	RECT get_rect();
	virtual bool set_rect(RECT& new_rect);
	virtual bool apply_new_rect(RECT& new_rect);

	sl_window_capture_method use_method;
	overlay_status status;

	bool get_window_screenshot();

	virtual bool save_state_to_settings();
	virtual std::string get_url();
	virtual void set_url(char* url);
	virtual void set_transparency(int transparency);
	virtual bool ready_to_create_overlay();
	HWND orig_handle;
	HBITMAP hbmp;
	HDC hdc;

	HWND overlay_hwnd;

	int id;

	virtual ~overlay_window();
	overlay_window();

	virtual void clean_resources();
};

class web_view_window : public overlay_window
{
	public:
	std::string url;
	bool overlay_crated = false;

	virtual bool save_state_to_settings();
	virtual bool ready_to_create_overlay();
	virtual void clean_resources();
	virtual std::string get_url();
	virtual void set_url(char* new_url);
	virtual bool apply_new_rect(RECT& new_rect);
};

class app_window : public overlay_window
{
	public:
};