// simplewall
// Copyright (c) 2016-2021 Henry++

#include "global.h"

PICON_INFORMATION _app_icons_getdefault ()
{
	static R_INITONCE init_once = PR_INITONCE_INIT;
	static ICON_INFORMATION icon_info = {0};

	if (_r_initonce_begin (&init_once))
	{
		PR_STRING path;

		// load default icons
		path = _r_obj_concatstrings (2, _r_sys_getsystemdirectory ()->buffer, L"\\svchost.exe");

		_app_icons_loadfromfile (path, &icon_info.app_icon_id, &icon_info.app_hicon, FALSE);

		// load uwp icons
		if (_r_sys_isosversiongreaterorequal (WINDOWS_8))
		{
			_r_obj_movereference (&path, _r_obj_concatstrings (2, _r_sys_getsystemdirectory ()->buffer, L"\\wsreset.exe"));

			_app_icons_loadfromfile (path, &icon_info.uwp_icon_id, &icon_info.uwp_hicon, FALSE);
		}

		_r_obj_dereference (path);

		_r_initonce_end (&init_once);
	}

	return &icon_info;
}

_Ret_maybenull_
HICON _app_icons_getdefaultapp_hicon ()
{
	PICON_INFORMATION icon_info;

	icon_info = _app_icons_getdefault ();

	if (icon_info->app_hicon)
		return CopyIcon (icon_info->app_hicon);

	return NULL;
}

_Ret_maybenull_
HICON _app_icons_getdefaulttype_hicon (
	_In_ ENUM_TYPE_DATA type,
	_In_ PICON_INFORMATION icon_info
)
{
	if (type == DATA_APP_UWP)
	{
		if (icon_info->uwp_hicon)
			return CopyIcon (icon_info->uwp_hicon);
	}

	if (icon_info->app_hicon)
		return CopyIcon (icon_info->app_hicon);

	return NULL;
}

LONG _app_icons_getdefaultapp_id ()
{
	PICON_INFORMATION icon_info;

	icon_info = _app_icons_getdefault ();

	return icon_info->app_icon_id;
}

LONG _app_icons_getdefaultuwp_id ()
{
	PICON_INFORMATION icon_info;

	icon_info = _app_icons_getdefault ();

	return icon_info->uwp_icon_id;
}

HICON _app_icons_getsafeapp_hicon (
	_In_ ULONG_PTR app_hash
)
{
	PICON_INFORMATION icon_info;
	PITEM_APP ptr_app;
	HICON hicon;
	LONG icon_id;
	BOOLEAN is_iconshidded;

	icon_info = _app_icons_getdefault ();
	ptr_app = _app_getappitem (app_hash);

	if (!ptr_app)
	{
		if (icon_info->app_hicon)
			return CopyIcon (icon_info->app_hicon);

		return NULL;
	}

	is_iconshidded = _r_config_getboolean (L"IsIconsHidden", FALSE);

	if (!ptr_app->real_path || is_iconshidded || !_app_isappvalidbinary (ptr_app->type, ptr_app->real_path))
	{
		hicon = _app_icons_getdefaulttype_hicon (ptr_app->type, icon_info);

		_r_obj_dereference (ptr_app);

		return hicon;
	}

	_app_icons_loadfromfile (ptr_app->real_path, &icon_id, &hicon, TRUE);

	if (!icon_id || (ptr_app->type == DATA_APP_UWP && icon_id == icon_info->app_icon_id))
	{
		if (hicon)
			DestroyIcon (hicon);

		hicon = _app_icons_getdefaulttype_hicon (ptr_app->type, icon_info);
	}

	_r_obj_dereference (ptr_app);

	return hicon;
}

VOID _app_icons_loadfromfile (
	_In_ PR_STRING path,
	_Out_opt_ PLONG icon_id,
	_Out_opt_ HICON_PTR hicon,
	_In_ BOOLEAN is_loaddefaults
)
{
	SHFILEINFO shfi = {0};
	PICON_INFORMATION icon_info;
	UINT flags;

	flags = SHGFI_LARGEICON;

	if (icon_id)
		flags |= SHGFI_SYSICONINDEX;

	if (hicon)
		flags |= SHGFI_ICON;

	if (SHGetFileInfo (path->buffer, 0, &shfi, sizeof (shfi), flags))
	{
		if (icon_id)
			*icon_id = shfi.iIcon;

		if (hicon)
			*hicon = shfi.hIcon;

		return;
	}

	if (!icon_id && !hicon)
		return;

	if (is_loaddefaults)
	{
		icon_info = _app_icons_getdefault ();

		if (icon_id)
			*icon_id = icon_info->app_icon_id;

		if (hicon)
		{
			if (icon_info->app_hicon)
			{
				*hicon = CopyIcon (icon_info->app_hicon);
			}
			else
			{
				*hicon = NULL;
			}
		}

		return;
	}

	// set to null
	if (icon_id)
		*icon_id = 0;

	if (hicon)
		*hicon = NULL;
}
