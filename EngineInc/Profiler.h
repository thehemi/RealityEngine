//=========== Copyright (c) 2004, Artificial Studios. All rights reserved. ===========
// Profiler - Engine Statistics
// Author: Tim Johnson
//====================================================================================
#pragma once

/// Average our frame states over this many seconds
static const float AveragingTime = 0.5f;

//-----------------------------------------------------------------------------
/// Profiler - Engine Statistics
//-----------------------------------------------------------------------------
class ENGINE_API Profiler
{
public:
	//
	// Counters
	//

	/// Mesh.DrawSubset() calls
	int	NumDraws;
	/// Shader file changes
	int	ShaderChanges;
	/// Material changes
	int MaterialChanges;
	/// Light changes
	int LightChanges;
	/// Shader technique changes
	int TechniqueChanges;
	/// Object TM changes
	int TransformChanges;
	/// Skinning state changes
	int SkinningChanges;
	/// Recursive Render Calls to Sort
	int	SortRenders;
	/// Has render tree been forced to grow this frame? And how many times
	int TreeGrow;
	/// How many calls to FlipTarget()
	int FlipTargets;
	/// How many SetHDR calls
	int SetHDRCalls;
	/// Occlusion queries
	int OcclusionQueries;
    /// C# Actors
    int ScriptActors;
    /// FX System batch draws
    int FXDraws;
    /// Lights
	int Lights;
    /// Actors (all types of anything)
    int Actors;

	//
	// Timers
	//
    /// Number of frames elapsed since counters were flushed (used to average)
    int NumFrames;
	/// MS spent batching items
	float BatchMS;
	/// MS spent in editor logic
	float EditorMS;
    /// MS spent on early rendering/prepping
    float RenderPreMS;
    /// MS spent in core rendering loop
    float RenderMS;
    /// Post rendering, including post-process
    float RenderPostMS;
    /// Water CPU time
    float WaterCPUMS;
    /// Water Rendering time
    float WaterRenderMS;
    /// Physics time
    float PhysicsMS;
    /// Game time
    float GameMS;
    /// Special effect time (game)
    float FXMS;
    /// Time spent in Actor::Tick functions
    float TickMS; 
    /// Time spent updating lights
    float LightMS;
    /// Time spent on python script processing
    float ScriptMS; 
    /// Engine->Update() subsystems call
    float SubsystemMS; 
    /// Audio MS
    float AudioMS;
    /// Time spent flipping the backbuffer. This is where you get a definite ms for graphics card processing time beyond what could be done in parallel
    float PresentMS;
    /// Total delta MS since last update
    float DeltaMS;

    /// Total MS of all timers. Can be used to check estimated FPS against actual FPS
    float GetTotalMS(){ 
        return AudioMS+BatchMS+EditorMS+
            RenderPreMS+RenderPostMS+RenderMS+PresentMS+
            WaterCPUMS+WaterRenderMS+PhysicsMS+FXMS+TickMS+ScriptMS+SubsystemMS+GameMS; 
    }

    // Initialization data timers, only used at world or game load
    /// Total time for all textures loaded this "frame"
    float TextureLoadSecs;
    /// Total time for meshes loaded this "frame"
    float MeshLoadSecs;
    /// Total time for mesh setup this "frame" (create VB/IB/Collision Data/Optimize)
    float MeshSetupSecs;
    /// Total time loading shaders
    float ShaderLoadSecs;

	/// Update all states
	void Update()
	{
        if(NumFrames*GDeltaTime > AveragingTime)
        {
            float oldPresent = PresentMS;
		    ZeroMemory(this,sizeof(Profiler));
            PresentMS = oldPresent;
            // Reset frame counter
            NumFrames = 0;
        }
        
        NumFrames++;
        DeltaMS += GDeltaTime*1000.0f;
	}

	string GetStats()
	{
		string stats;
		return stats;
	}

	/// Singleton
	static Profiler* Get();

protected:
    Profiler(){ ZeroMemory(this,sizeof(Profiler)); }
};
