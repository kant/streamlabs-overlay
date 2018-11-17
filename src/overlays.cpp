 
#include "stdafx.h"
#include "overlays.h"

#include <algorithm>
#include <iostream>

#include "web_view.h"
#include "settings.h"

#include "tlhelp32.h"
#include "..\include\overlays.h"
#pragma comment (lib, "uxtheme.lib")

wchar_t const g_szWindowClass[] = L"overlays";

void smg_overlays::process_hotkeys(MSG &msg)
{
	std::cout << "APP:"  << "process_hotkeys id " << msg.wParam << std::endl;
	switch (msg.wParam)
	{
	case HOTKEY_CATCH_APP:
	{
		//HWND top_window = GetTopWindow(NULL);
		HWND top_window = GetForegroundWindow();
		if (top_window != nullptr)
		{
			unsigned long process_id = 0;
			GetWindowThreadProcessId(top_window, &process_id);
			std::cout << "APP:"  << "APP:" << "catch app "<< process_id <<", "<< top_window << std::endl;
			EnumWindows(get_overlayed_windows, (LPARAM)&process_id);
		}
		create_windows_overlays();
	}
	break;
	case HOTKEY_SHOW_OVERLAYS:
	{
		if (showing_overlays)
		{
			//need to hide befor show. or show can be ignored. 
			showing_overlays = false;
			hide_overlays();
		}
		
		std::cout << "APP:"  << "show overlays " << std::endl;
		showing_overlays = true;
	}
	break;
	case HOTKEY_HIDE_OVERLAYS:
	{
		showing_overlays = false;
		
		hide_overlays();
	}
	break;
	case HOTKEY_UPDATE_OVERLAYS:
	{
		if (showing_overlays)
		{
			std::for_each(showing_windows.begin(), showing_windows.end(), [](std::shared_ptr<captured_window> &n) {
				n->update_window_screenshot();
				//InvalidateRect(n->overlay_hwnd, nullptr, TRUE);
			});
		}
	} break;
	case HOTKEY_ADD_WEB:
	{
		std::cout << "APP:" << "get HOTKEY_ADD_WEB " << web_views_thread_id<<  std::endl;
		if (in_standalone_mode)
		{
			std::cout << "APP:" << "create default web view" << std::endl;
			web_view_overlay_settings n;
			n.x = 100;
			n.y = 100;
			n.width = 400;
			n.height = 400;
			n.url = "http://mail.ru";
			create_empty_web_view_window(n);
		}
		PostThreadMessage((DWORD)web_views_thread_id, WM_HOTKEY, HOTKEY_ADD_WEB, 0);
	} break;

	case HOTKEY_QUIT:
	{
		PostQuitMessage(0);
	}break;
	};
}

int smg_overlays::create_empty_web_view_window( web_view_overlay_settings &n )
{
	std::shared_ptr<web_view_window> new_web_view_window = std::make_shared<web_view_window>();
	new_web_view_window->orig_handle = nullptr;
	new_web_view_window->use_method = window_grab_method::bitblt;
	new_web_view_window->x = n.x;
	new_web_view_window->y = n.y;
	new_web_view_window->width = n.width;
	new_web_view_window->height = n.height;
	new_web_view_window->url = n.url;
	showing_windows.push_back(new_web_view_window);

	if (n.url.find("http://") == 0 || n.url.find("https://") == 0 || n.url.find("file://") == 0)
	{
		new_web_view_window->url = n.url;
	} else {
		WCHAR buffer[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, buffer );
		std::wstring ws(buffer);
		std::string temp_path(ws.begin(), ws.end());
		//std::string::size_type pos = temp_path.find_last_of("\\/");

		new_web_view_window->url = "file:////";
		new_web_view_window->url += temp_path;// .substr(0, pos);
		new_web_view_window->url += "\\";
		new_web_view_window->url += n.url;
	}

	return new_web_view_window->id;
}

std::shared_ptr<web_view_window> smg_overlays::get_web_view_by_container(HWND container)
{
	std::shared_ptr<web_view_window> ret = nullptr;
	std::for_each(showing_windows.begin(), showing_windows.end(), [&ret, container](std::shared_ptr<captured_window> &n)
	{
		if (n->is_web_view())
		{
			std::shared_ptr<web_view_window> derived = std::static_pointer_cast<web_view_window> (n);
			if (derived->container_hwnd == container)
			{
				ret = derived;
			}

		}
	}
	);

	return ret;
}

bool web_view_window::check_orig_and_create_overlay()
{
	if (!overlay_crated && web_view_hwnd != nullptr)
	{
		overlay_crated = true;

		orig_handle = web_view_hwnd;
		use_method = window_grab_method::bitblt;
		
		return true;
	} else {
		return false;
	}
}

void smg_overlays::on_update_timer()
{
	create_windows_overlays();

	std::for_each(showing_windows.begin(), showing_windows.end(), [this](std::shared_ptr<captured_window> &n)
	{
		if(n->check_orig_and_create_overlay())
		{ 
			n->get_window_screenshot();
		}		
	}
	);

	if (showing_overlays)
	{
		std::for_each(showing_windows.begin(), showing_windows.end(), [](std::shared_ptr<captured_window> &n) {
			if (n->update_window_screenshot())
			{
				InvalidateRect(n->overlay_hwnd, nullptr, TRUE);
			}
		});
	}

}

void smg_overlays::deinit()
{
	update_settings();
	app_settings.write();

	std::cout << "APP:"  << "exit from app " << std::endl;
}

void  smg_overlays::hide_overlays()
{
	std::cout << "APP:"  << "hide_overlays " << std::endl;
	std::for_each(showing_windows.begin(), showing_windows.end(), [](std::shared_ptr<captured_window> &n) {
		ShowWindow(n->overlay_hwnd, SW_HIDE);
	});
	PostThreadMessage((DWORD)web_views_thread_id, WM_HOTKEY, HOTKEY_HIDE_OVERLAYS, 0);
}

void  smg_overlays::create_overlay_window_class()
{
	WNDCLASSEX wcex = { sizeof(wcex) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = g_szWindowClass;

	RegisterClassEx(&wcex);
}

void  smg_overlays::create_windows_overlays()
{
	std::for_each(showing_windows.begin(), showing_windows.end(), [this](std::shared_ptr<captured_window> &n) 
	{
		if (n->overlay_hwnd == nullptr && n->ready_to_create_overlay())
		{
			DWORD const dwStyle = WS_POPUP;     // no border or title bar
			DWORD const dwStyleEx = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT;   // transparent, topmost, with no taskbar item
			n->overlay_hwnd = CreateWindowEx(dwStyleEx, g_szWindowClass, NULL, dwStyle, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);

			if (n->overlay_hwnd)
			{
				if (app_settings.use_color_key)
				{
					SetLayeredWindowAttributes(n->overlay_hwnd, RGB(0xFF, 0xFF, 0xFF), 0xD0, LWA_COLORKEY);
				} else {
					SetLayeredWindowAttributes(n->overlay_hwnd, RGB(0xFF, 0xFF, 0xFF), app_settings.transparency, LWA_ALPHA);
				}

				SetWindowPos(n->overlay_hwnd, HWND_TOPMOST, n->x, n->y, n->width, n->height, SWP_NOREDRAW);
				if (showing_overlays)
				{
					ShowWindow(n->overlay_hwnd, SW_SHOW);
				} else {
					ShowWindow(n->overlay_hwnd, SW_HIDE);
				}
				
			}
		}
	});
}

void  smg_overlays::register_hotkeys()
{
	RegisterHotKey(NULL, HOTKEY_SHOW_OVERLAYS, MOD_ALT, 0x53);  //'S'how
	RegisterHotKey(NULL, HOTKEY_HIDE_OVERLAYS, MOD_ALT, 0x48);  //'H'ide all
	RegisterHotKey(NULL, HOTKEY_ADD_WEB, MOD_ALT, 0x57);  //add 'W'ebview
	RegisterHotKey(NULL, HOTKEY_UPDATE_OVERLAYS, MOD_ALT, 0x55);  //'U'pdate
	RegisterHotKey(NULL, HOTKEY_QUIT, MOD_ALT, 0x51);  //'Q'uit
	RegisterHotKey(NULL, HOTKEY_CATCH_APP, MOD_ALT, 0x50);  //catch a'P'p window
}

void  smg_overlays::create_windows_for_apps()
{
	//EnumWindows(get_overlayed_windows, 0);
	std::for_each(app_settings.apps_names.begin(), app_settings.apps_names.end(),  [](std::string &n)
		{
			WCHAR * process_name = new wchar_t[n.size() + 1];
			mbstowcs(&process_name[0], n.c_str(), n.size()+1);
			
			FindRunningProcess(process_name);
		}
		);		
}

 size_t smg_overlays::get_count()
{
	return showing_windows.size();
}

std::shared_ptr<captured_window> smg_overlays::get_overlay_by_id(int overlay_id)
{
	std::shared_ptr<captured_window> ret;

	std::list< std::shared_ptr<captured_window> >::iterator findIter = std::find_if(showing_windows.begin(), showing_windows.end(),
		 [&overlay_id](std::shared_ptr<captured_window> &n) { return overlay_id == n->id; }
	);

	if (findIter != showing_windows.end())
	{
		 ret = *findIter;
	}

	return ret;
}

std::vector<int > smg_overlays::get_ids()
{
	std::vector<int > ret;
	int i = 0;
	ret.resize(showing_windows.size());
	std::for_each(showing_windows.begin(), showing_windows.end(), [&ret, &i](std::shared_ptr<captured_window> &n) {
		ret[i] = n->id;
		i++;
	});

	return ret;
}

smg_overlays::smg_overlays()
{
	showing_overlays = false;

	std::cout << "APP:"  << "start application " << std::endl;

	if (!app_settings.read())
	{
		app_settings.default_init();
		//app_settings.test_init();
	}

}

void smg_overlays::init()
{
	create_overlay_window_class();

	create_windows_for_apps();

	if (in_standalone_mode)
	{
		register_hotkeys();
	}

	create_windows_overlays();
	
	std::for_each(app_settings.web_pages.begin(), app_settings.web_pages.end(), [this](web_view_overlay_settings &n)
	{
		create_empty_web_view_window(n);
	}
	);
}

bool FindRunningProcess(const WCHAR * process_name_part) 
{
	bool procRunning = false;

	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap == INVALID_HANDLE_VALUE) 
	{
		procRunning = false;
	} else {
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hProcessSnap, &pe32)) 
		{ 
			while (true)
			{
				if (StrStrW(pe32.szExeFile, process_name_part) != nullptr)
				{
					unsigned long process_id = pe32.th32ProcessID;
					EnumWindows(get_overlayed_windows, (LPARAM)&process_id);
				}
				if (!Process32Next(hProcessSnap, &pe32))
				{ 
					break;
				}
			}
			CloseHandle(hProcessSnap);
		}
	}

	return procRunning;
}

BOOL smg_overlays::process_found_window(HWND hwnd, LPARAM param)
{
	char buffer[128];
	bool window_ok = false;
	if (param != NULL)
	{
		unsigned long process_id = 0;
		GetWindowThreadProcessId(hwnd, &process_id);
		if ( *((unsigned long*)param) == process_id  && (GetWindow(hwnd, GW_OWNER) == (HWND)nullptr && IsWindowVisible(hwnd) ) )
		{
			window_ok = true;
		}
	} else {
		int written = GetWindowTextA(hwnd, buffer, 128);
		if (written && strstr(buffer, "Notepad.") != nullptr)
		{
			window_ok = true;
		}
	}
	
	if (window_ok)
	{
		WINDOWINFO wi;
		GetWindowInfo(hwnd, &wi);
		int y = wi.rcWindow.bottom - wi.rcWindow.top;
		int x = wi.rcWindow.left - wi.rcWindow.right;

		int written = GetWindowTextA(hwnd, buffer, 128);
	}

	if(window_ok )
	{
		bool we_have_it = false;
		std::for_each(showing_windows.begin(), showing_windows.end(), [&hwnd, &we_have_it](std::shared_ptr<captured_window> &n) {
			if (n->orig_handle == hwnd)
			{
				we_have_it = true;
			}
		});

		if (!we_have_it)
		{
			std::shared_ptr<captured_window> found_window = std::make_shared<captured_window>();
			found_window->orig_handle = hwnd;
			found_window->get_window_screenshot();
			showing_windows.push_back(found_window);

			//add process file name to settings 
			TCHAR nameProcess[MAX_PATH];
			HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, *((unsigned long*)param));
			DWORD file_name_size = MAX_PATH;
			QueryFullProcessImageName( processHandle, 0, nameProcess, &file_name_size);
			CloseHandle(processHandle);
			std::wstring ws(nameProcess);
			std::string temp_path(ws.begin(), ws.end());
			std::string::size_type pos = temp_path.find_last_of("\\/");
			std::string file_name = temp_path.substr(pos+1, temp_path.size());

			std::list<std::string>::iterator findIter = std::find_if(app_settings.apps_names.begin(), app_settings.apps_names.end(), 
				[&file_name](const std::string &v) { return v.compare(file_name) == 0; }
				);
			if (findIter == app_settings.apps_names.end())
			{
				app_settings.apps_names.push_back(file_name);
			}
		
		}
	}
	return TRUE;
}

void smg_overlays::draw_overlay_gdi(HWND & hWnd, bool g_bDblBuffered)
{
	PAINTSTRUCT     ps;
	HPAINTBUFFER    hBufferedPaint = NULL;
	RECT            rc;

	GetClientRect(hWnd, &rc);
	HDC hdc = BeginPaint(hWnd, &ps);

	if (g_bDblBuffered)
	{
		// Get doublebuffered DC
		HDC hdcMem;
		hBufferedPaint = BeginBufferedPaint(hdc, &rc, BPBF_COMPOSITED, NULL, &hdcMem);
		if (hBufferedPaint)
		{
			hdc = hdcMem;
		}
	}

	std::for_each(showing_windows.begin(), showing_windows.end(), [&hdc, &hWnd](std::shared_ptr<captured_window> &n) {
		if (hWnd == n->overlay_hwnd)
		{
			BOOL ret = BitBlt(hdc, 0, 0, n->width, n->height, n->hdc, 0, 0, SRCCOPY);
			if (!ret)
			{
				std::cout << "APP:"  << "draw_overlay_gdi had issue " << GetLastError() << std::endl;
			}
		}
	});

	if (hBufferedPaint)
	{
		// end painting
		BufferedPaintMakeOpaque(hBufferedPaint, nullptr);
		EndBufferedPaint(hBufferedPaint, TRUE);
	}

	EndPaint(hWnd, &ps);
}

bool web_view_window::save_state_to_settings()
{
	web_view_overlay_settings wnd_settings;
	wnd_settings.url = url;

	RECT client_rect = { 0 };
	GetWindowRect(container_hwnd, &client_rect);
	wnd_settings.x = client_rect.left;
	wnd_settings.y = client_rect.top;
	wnd_settings.width = client_rect.right - client_rect.left;
	wnd_settings.height = client_rect.bottom - client_rect.top;

	app_settings.web_pages.push_back(wnd_settings);
	return true;
}

void  smg_overlays::update_settings()
{
	app_settings.web_pages.clear();

	std::for_each(showing_windows.begin(), showing_windows.end(), [this](std::shared_ptr<captured_window> &n)
	{
		n->save_state_to_settings();
	}
	);
	std::cout << "APP:"  << "update_settings finished " << std::endl;
}

bool captured_window::update_window_screenshot()
{
	return get_window_screenshot();
}

captured_window::~captured_window()
{
	clean_resources();
}

captured_window::captured_window()
{
	static int id_counter = 128;
	id = id_counter++;
	use_method = window_grab_method::print;
	orig_handle = nullptr;
	overlay_hwnd = nullptr;
	hdc = nullptr;
	hbmp = nullptr;
}

void captured_window::clean_resources()
{
	DeleteDC(hdc);
	DeleteObject(hbmp);
}

bool captured_window::get_window_screenshot() 
{
	bool updated = false;
	BOOL ret = false;
	RECT client_rect = { 0 };
	HDC hdcScreen = GetDC(orig_handle);

	ret = GetWindowRect(orig_handle, &client_rect);
	if (ret && hdcScreen != nullptr)
	{
		int new_x = client_rect.left;
		int new_y = client_rect.top;
		int new_width = client_rect.right - client_rect.left;
		int new_height = client_rect.bottom - client_rect.top;
			
		HDC new_hdc = nullptr;
		HBITMAP new_hbmp = nullptr;
		bool keep_gdi = false;;

		if (new_width == width && new_height == height)
		{
			keep_gdi = true;
			new_hdc = hdc;
			new_hbmp = hbmp;
		} else {
			new_hdc = CreateCompatibleDC(hdcScreen);
			new_hbmp = CreateCompatibleBitmap(hdcScreen, new_width, new_height);
			SelectObject(new_hdc, new_hbmp);
		}

		if ( new_hdc == nullptr || new_hbmp == nullptr)
		{
			DeleteDC(new_hdc);
			DeleteObject(new_hbmp);
		} else {

			switch (use_method)
			{
			case window_grab_method::bitblt:
				ret = BitBlt(new_hdc, 0, 0, new_width, new_height, hdcScreen, 0, 0, SRCCOPY);
				break;
			case window_grab_method::print:
				ret = PrintWindow(orig_handle, new_hdc, 0);
				break;
			case window_grab_method::message_print:
				LRESULT msg_ret = SendMessage(orig_handle, WM_PAINT, (WPARAM)new_hdc, PRF_CHILDREN | PRF_CLIENT | PRF_ERASEBKGND | PRF_NONCLIENT | PRF_OWNED);
				ret = (msg_ret == S_OK);
				break;
			};
			
			if (ret)
			{
				if (!keep_gdi)
				{
					clean_resources();

					hdc = new_hdc;
					hbmp = new_hbmp;
				}
				
				if (x == new_x && y == new_y && width == new_width && height == new_height)
				{
				} else {
					x = new_x;
					y = new_y;
					width = new_width;
					height = new_height;

					if (overlay_hwnd)
					{
						MoveWindow(overlay_hwnd, x, y, width, height, FALSE);
					}
				}
				updated = true;
			} else {
				std::cout << "APP:"  << "get_window_screenshot failed to get bitmap from orig window " << GetLastError() << std::endl;
				if (!keep_gdi)
				{

					DeleteDC(new_hdc);
					DeleteObject(new_hbmp);
				}
			}
		}
	}

	if (overlay_hwnd)
	{
		if (!updated)
		{
			if (IsWindow(orig_handle))
			{
				//it is still a window we can show it later 
				ShowWindow(overlay_hwnd, SW_HIDE);
			}
			else {
				//it is not a window anymore . should close our overlay 
				ShowWindow(overlay_hwnd, SW_HIDE);
			}
		}
		else {
			if (!IsWindowVisible(overlay_hwnd))
			{
				ShowWindow(overlay_hwnd, SW_SHOWNA);
			}
		}
	}
	ReleaseDC(NULL, hdcScreen);

	return updated;
}
 