//=========== Copyright (c) 2004, Tim Johnson. All rights reserved. ===========
//
// Map Configuration Tab Window
//
//=============================================================================
#pragma once
#ifndef __MAPSETTINGS__H
#define __MAPSETTINGS__H


class MapSettings {
public:
	MapSettings();
	~MapSettings();

	void ShowDialog();
	void CloseDialog();
	void Init();

	Interface		*ip;
	IColorSwatch*	fogColor;
	HWND			hwnd; // Dialog 

	// Map settings
	SceneProperties sceneData;
	void CommitSettings();
	void ApplySettings();
	SceneProperties* GetSceneProperties();
};

extern MapSettings theMap;

#endif // __MAPSETTINGS__H