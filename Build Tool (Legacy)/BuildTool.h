#pragma once


class BuildTool {
public:
	bool Build(string inFile, string outFile);
	void Initialize(HWND hwnd, string args);
	void Shutdown();

	bool m_PRTEnabled;
	bool model;
	bool strips;
	int RenderNodeSize;
	string inFile;
	string outFile;
};

extern BuildTool build;
