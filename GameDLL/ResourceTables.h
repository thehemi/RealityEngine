#ifndef RESOURCETABLES_H
#define RESOURCETABLES_H
#define ACTOR_TABLE_LENGTH	10

class ResourceTables
{
public:

string ActorTable[ACTOR_TABLE_LENGTH];
ResourceTables()
{
ActorTable[0] = "NetworkBox";
ActorTable[1] = "Player";
ActorTable[2] = "Buggy";
ActorTable[3] = "SkyController";
ActorTable[4] = "Heli";
}
int getActorTableIndex(const char* ActorName)
{
for(int i = 0; i < ACTOR_TABLE_LENGTH; i++)
{
if(!strcmp(ActorName,ActorTable[i].c_str()))
	return i;
}
Error("ResourceTables.h, Line 21: NetworkActor Table Index not found for %s",ActorName);
return -1;
}
string getActorClassName(unsigned char TableIndex)
{
	if(TableIndex >= ACTOR_TABLE_LENGTH)
		Error("ResourceTables.h, Line 27: Requested out of bounds NetworkActor class-name index.");

	return ActorTable[TableIndex];
}

};
#endif
