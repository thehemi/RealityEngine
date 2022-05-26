// TODO: Implement static precaching for FX and all other actor classes?
// How will we know if they are going to be used? Precaching everything is overkill
#include "stdafx.h"
#include "Engine.h"
#include "FXManager.h"
#include "Profiler.h"


void QuickSortRecursive(FXSYSTEMVECTOR& pArr, int d, int h)
{
	int i,j;
	i = h;
	j = d;
	float dist = pArr[(d+h) / 2]->distanceFromCam;
	do {
		while (pArr[j]->distanceFromCam > dist)j++;
		while (pArr[i]->distanceFromCam < dist)i--;
		if ( i >= j )
		{
			if ( i != j ) 
			{
				FXSystem* zal = pArr[i];
				pArr[i] = pArr[j];
				pArr[j] = zal;
			}
			i--;j++;
		}
	}while (j <= i);
	if (d < i) QuickSortRecursive(pArr,d,i);
	if (j < h) QuickSortRecursive(pArr,j,h);
}










VertexIndicesBatch FXManager::vertexIndicesBatches[MAX_BATCHES];
vector<BatchedQuad> FXManager::batchedQuads;

// Returns a singleton instance
FXManager* FXManager::Instance () 
{
	static FXManager inst;
	return &inst;
}
void FXManager::PostRender(World* world,Camera& cam)
{
	for(int i = 0; i < MAX_BATCHES; i++)vertexIndicesBatches[i].used = false;

	FXSYSTEMVECTOR sortedSystems;

	for(int i=0;i<systems.size();i++)
	{
		FXSystem* sys = systems[i];
		if(sys->MyWorld == world && !sys->IsHidden)
		{
			sys->distanceFromCam = (sys->Location - cam.Location).Length();
			if(sys->distanceFromCam < cam.FarClip || sys->forceDistanceRender)
			{
				if(!sys->isSorted)
					sys->PostRender(cam);
				else 
					sortedSystems.push_back(sys);
			}
		}
	}
	DrawBatchedQuads();


	int sortedSize = sortedSystems.size();
	if(sortedSize > 0)
	{
		QuickSortRecursive(sortedSystems,0,sortedSize - 1);
		for(int i=0;i<sortedSize;i++)
			sortedSystems[i]->PostRender(cam);
	}
	DrawBatchedQuads();
}
void FXManager::Tick(World* world)
{
	if(Engine::Instance()->IsDedicated())
	{
		for(int i = 0; i < systems.size();i++)
		{
			if(systems[i]->LifeTime != -1)
			{
				delete systems[i];
				i--;
			}
		}
	}
	else
	{
		for(int i = 0; i < systems.size();i++)
		{
			if(systems[i]->MyWorld == world)
			{
				if(systems[i]->LifeTime != -1)
				{
					systems[i]->LifeTime -= GDeltaTime;
					if(systems[i]->LifeTime < 0)
					{
						delete systems[i];
						i--;
						continue;
					}
				}
				systems[i]->Tick();

				if(systems[i]->LifeTime == 0)
				{
					delete systems[i];
					i--;
				}
			}
		}
	}
}
void FXManager::Reset(World* world)
{
	for(int i = 0; i < systems.size();i++)
	{
		if(!world || systems[i]->MyWorld == world)
		{
			delete systems[i];
			i--;
		}
	}
	if(!world)systems.clear();
	FlushBatchedQuads();
}

void FXManager::ChangeSystemsWorld(World* oldWorld, World* newWorld)
{
	for(int i = 0; i < systems.size();i++)
	{
		if(systems[i]->MyWorld == oldWorld)
			systems[i]->MyWorld = newWorld;
	}
}

int FXManager::getBatchForWriting(Texture* texture,BlendMode destBlend)
{
	for(int i = 0; i < batchedQuads.size(); i++)
	{
		if(texture->GetTexture() == batchedQuads[i].texture->GetTexture() && destBlend == batchedQuads[i].destBlend)
		{
			if(batchedQuads[i].numQuads > 1499)//QUADS_PER_BATCH-1)
				continue; //look for different -- or new -- batch
			return i;
		}
	}

	return addNewBatch(texture, destBlend);
}

int FXManager::addNewBatch(Texture* texture,BlendMode destBlend)
{
	int batchArray = getFreeBatch();
	if(batchArray == -1)return -1;

	int theLast = batchedQuads.size();
	batchedQuads.resize(theLast + 1);

	batchedQuads[theLast].texture = texture;
	batchedQuads[theLast].destBlend = destBlend;
	batchedQuads[theLast].numQuads = 0;
	batchedQuads[theLast].myVertexIndicesBatch = batchArray;

	vertexIndicesBatches[batchArray].used = true;
	return theLast;
}

FXManager::FXManager()
{
	for(int k = 0; k < MAX_BATCHES;k++)
	{
		for(int v = 0; v < QUADS_PER_BATCH; v++)
		{
			vertexIndicesBatches[k].theIndices[(v*6)] = (v*4);
			vertexIndicesBatches[k].theIndices[(v*6)+1] = (v*4)+1;
			vertexIndicesBatches[k].theIndices[(v*6)+2] = (v*4)+2;
			vertexIndicesBatches[k].theIndices[(v*6)+3] = (v*4)+2;
			vertexIndicesBatches[k].theIndices[(v*6)+4] = (v*4)+1;
			vertexIndicesBatches[k].theIndices[(v*6)+5] = (v*4)+3;
		}
	}
}

void FXManager::DrawBatchedQuads()
{
	int size = batchedQuads.size();
	if(!size)return;
	RenderWrap::SetRS( D3DRS_SRCBLEND, BLEND_SRCALPHA); 

    // Disable slow anisotropic filtering for particles
    RenderWrap::dev->SetSamplerState( 0, D3DSAMP_MAXANISOTROPY  , 1);
	RenderWrap::dev->SetSamplerState( 0, D3DSAMP_MAGFILTER ,     D3DTEXF_LINEAR     );
	RenderWrap::dev->SetSamplerState( 0, D3DSAMP_MINFILTER,      D3DTEXF_LINEAR     );

	for(int i = 0; i < size; i++)
	{
		int index = batchedQuads[i].numQuads;
		RenderWrap::SetRS( D3DRS_DESTBLEND, batchedQuads[i].destBlend);
		batchedQuads[i].texture->Set(0);
		RenderDevice::Instance()->GetCanvas()->SimpleObject(index*4,&vertexIndicesBatches[batchedQuads[i].myVertexIndicesBatch].theVertices,index*6,&vertexIndicesBatches[batchedQuads[i].myVertexIndicesBatch].theIndices,sizeof(LVertex),false);
		batchedQuads[i].texture->UnSet(0);
        Profiler::Get()->FXDraws++;
	}

    RenderWrap::dev->SetSamplerState( 0, D3DSAMP_MAXANISOTROPY  , RenderDevice::Instance()->GetAnisotropyLevel());
    if(RenderDevice::Instance()->GetAnisotropyLevel() > 1)
	    RenderWrap::dev->SetSamplerState( 0, D3DSAMP_MAGFILTER ,     D3DTEXF_ANISOTROPIC     );

	batchedQuads.clear();
    
}
int FXManager::getFreeBatch()
{
	for(int i = 0; i < MAX_BATCHES; i++)if(!vertexIndicesBatches[i].used)return i;
	return -1;
}


void FXManager::AddFXSystemToHash(FXSystem* system)
{
	int hashIndex = 0;
	bool found = false;

	for(int i = 0; i < SystemHash.size(); i++)
	{
		if(SystemHash[i] == NULL)
		{
			hashIndex = i;
			found = true;
			break;
		}
	}

	if(!found)
	{
		hashIndex = SystemHash.size();
		SystemHash.resize(hashIndex + 1);
	}
	SystemHash[hashIndex] = system;
	system->HashIndex = hashIndex;
}

void	FXManager::RemoveSystem(FXSystem* sys)
{
	systems.erase( find(systems.begin(), systems.end(), sys) ); 
	if(sys->HashIndex != -1)
	{
		SystemHash[sys->HashIndex] = NULL;
		sys->HashIndex = -1;
	}
}

FXSystem* FXManager::GetFXSystemFromHash(DWORD index, FXSystem* system)
{
	if(index < SystemHash.size() && SystemHash[index] == system)
			return SystemHash[index];

	return NULL;
}