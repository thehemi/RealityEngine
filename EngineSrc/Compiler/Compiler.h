//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Compiles the map into a game-ready format. This involves:
//
/// -Creating a collision quadtree
/// -Creating a view data octree
/// -Slicing geometry around lights
/// -Stripifying and cache coherency
/// -Adding tangent data
/// -Creating unique vertex and index pools and indices
//
/// Author: Tim Johnson, 30.8.2002
//====================================================================================
#pragma once
#ifndef COMPILER_INC
#define COMPILER_INC

class CompilerCallback
{
public:
    virtual bool OnCallback()
    {
        return false;
    }
};
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
class Compiler {
private:
	
	bool MeshNeedsRecompiling(Mesh* mesh, string file);
	void FindFrameAndActor(string name, Actor*& actor, ModelFrame*& pFrame);
	bool CompileMesh(ModelFrame* frame);
	bool CompileFrame(ModelFrame* frame);
	bool CompileActor(Actor* actor);
	void SetProgress(int percent);
	void UpdateProgress();
	PRTSettings TweakOptions(PRTSettings& o);

	string GetFile(string objectName, string ext);

	bool			m_bForceRecompile;
	bool			m_bCompiling;
	World*			m_pWorld;
	Model*			m_pModel;
	Actor*			m_CurrectActor;
public:
    CompilerCallback * m_Callback;
	bool bCompilePRT;
	bool bFast;
	float percentRays, percentBounces;

	Compiler(){
		bCompilePRT = true;
		m_pWorld = NULL; 
		m_pModel = NULL;
		m_bCompiling=false;
		m_bForceRecompile=false;
		bFast = false;
		percentRays = 100;
		percentBounces = 100;
		m_CurrectActor = NULL;
        m_Callback = NULL;
	}
	~Compiler();

	/// Compile an arbitrary set of actors. Doesn't use &vector, in case vector is modified
	/// during callback
	bool CompileActors(vector<Actor*> actors);
	bool CompileModel(Model* model);
	bool CompileWorld(World* world);
};

#endif