

// CallStackDumper.cpp: implementation of the CallStackDumper class.
//
//////////////////////////////////////////////////////////////////////
#include <stdafx.h>
#include <fstream>
#include <iomanip>
#include <crtdbg.h>
#include "CallStackDumper.h"

//membres statiques
CallStackDumper::SYMINITIALIZEPROC CallStackDumper::SymInitialize=0;
CallStackDumper::SYMCLEANUPPROC CallStackDumper::SymCleanup=0;
CallStackDumper::STACKWALKPROC CallStackDumper::StackWalk=0;
CallStackDumper::SYMGETSYMFROMADDRPROC CallStackDumper::SymGetSymFromAddr=0;
CallStackDumper::SYMFUNCTIONTABLEACCESSPROC
CallStackDumper::SymFunctionTableAccess=0;
CallStackDumper::SYMGETMODULEBASEPROC CallStackDumper::SymGetModuleBase=0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CallStackDumper::CallStackDumper(char const* path_debug_infos)
{
 if (path_debug_infos!=NULL)
  path_recherche_dbg=path_debug_infos;

 //détermine le nom du fichier d'après le nom de l'exécutable
 char nom_executable [MAX_PATH];
 GetModuleFileName(0, nom_executable, MAX_PATH);
 nom_fichier=nom_executable;
 string::size_type position=nom_fichier.rfind('.'); //repère la position du '.' final
 nom_fichier.replace(position+1,3,"rpt"); //remplace le "exe" final par "rpt"

 DeleteFile(nom_fichier.c_str());

 init();
}

CallStackDumper::CallStackDumper(char const* nom_fichier_, char const*path_debug_infos)
: nom_fichier(nom_fichier_)
{
 if (path_debug_infos!=NULL)
  path_recherche_dbg=path_debug_infos;

 DeleteFile(nom_fichier.c_str());

 init();
}

CallStackDumper::~CallStackDumper()
{
 //nettoyer les infos de debogage
 SymCleanup(GetCurrentProcess());

 //détruire l'objet event
 CloseHandle(h_Event);

 //décharger le librairie ImageHlp.dll
 FreeLibrary(h_imagehlp);
}

//initialisation
void CallStackDumper::init()
{
 //charge la librairie IMAGEHLP.DLL
 h_imagehlp=LoadLibrary("ImageHlp.dll");
 _ASSERT(h_imagehlp!=0);

 //initialise les pointeurs vers les méthodes
 SymInitialize=(SYMINITIALIZEPROC)GetProcAddress(h_imagehlp,"SymInitialize");
 _ASSERT(SymInitialize!=0);
 SymCleanup=(SYMCLEANUPPROC)GetProcAddress(h_imagehlp, "SymCleanup");
 _ASSERT(SymCleanup!=0);
 StackWalk=(STACKWALKPROC)GetProcAddress(h_imagehlp, "StackWalk");
 _ASSERT(StackWalk!=0);
 SymGetSymFromAddr=(SYMGETSYMFROMADDRPROC)GetProcAddress(h_imagehlp,"SymGetSymFromAddr");
 _ASSERT(SymGetSymFromAddr!=0);

SymFunctionTableAccess=(SYMFUNCTIONTABLEACCESSPROC)GetProcAddress(h_imagehlp, "SymFunctionTableAccess");
 _ASSERT(SymFunctionTableAccess!=0);
 SymGetModuleBase=(SYMGETMODULEBASEPROC)GetProcAddress(h_imagehlp,"SymGetModuleBase");
 _ASSERT(SymGetModuleBase!=0);

 //charger les infos de debogage du processus courant (si disponibles)
 SymInitialize(GetCurrentProcess(), 0, TRUE);

 //créé l'événement qui servira à bloquer le thread de l'application pendant
//qu'on analysera sa call stack
 h_Event=CreateEvent(NULL, FALSE, FALSE, NULL);  //event auto-reset, non signalé, non nommé
 _ASSERT(h_Event!=0);
}

//effectue le dump de la call stack dans le fichier
void CallStackDumper::DumpCallStack()
{
 //enregistrement du handle du thread courant, pour que le thread de travaily ait accès
 DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
GetCurrentProcess(), &calling_thread, 0, FALSE, DUPLICATE_SAME_ACCESS);

 //création du thread qui va faire le travail réel 'analyse de la callstack
 DWORD thread_id;
 HANDLE thread_handle=CreateThread(NULL, 0, ThreadFunc_DumpCallStack, this,
0, &thread_id);

 //puis on se bloque sur un événement (de façon à être sur de ne plus bouger
//tant que le thread d'analyse fait son travail)
 WaitForSingleObject(h_Event, INFINITE);

 //fermeture du handle sur le thread d'analyse
 CloseHandle(thread_handle);
 //fermeture du handle sur le thread courant
 CloseHandle(calling_thread);
}

//fonction de thread qui effectue le travail réel de dump de la call stack
//(pour pouvoir manipuler la call stack d'un thread, il faut que celui-ci soit
//suspendu)
DWORD WINAPI CallStackDumper::ThreadFunc_DumpCallStack(void* param)
{
 CallStackDumper* me=(CallStackDumper*)param;

 //suspendre le thread à analsyer
 SuspendThread(me->calling_thread);

 //ouverture du fichier de dump en mode append
 std::ofstream fichier(me->nom_fichier.c_str(), std::ios_base::out
|std::ios_base::app);

 //récupérer le contexte courant du thread
 CONTEXT thread_context;
 thread_context.ContextFlags=CONTEXT_CONTROL; //on s'intéresse aux registres de controle
 BOOL result=GetThreadContext(me->calling_thread, &thread_context);
 _ASSERT(result);

 //construction de la structure STACKFRAME initiale passée à StackWalk pour le premier appel
 STACKFRAME sf;
 memset(&sf, 0, sizeof(sf));
 sf.AddrPC.Offset=thread_context.Eip;  //pointeur d'instruction courant
 sf.AddrPC.Mode=AddrModeFlat;
 sf.AddrStack.Offset=thread_context.Esp; //addresse de la pile
 sf.AddrStack.Mode=AddrModeFlat;
 sf.AddrFrame.Offset=thread_context.Ebp;  //stack frame
 sf.AddrFrame.Mode=AddrModeFlat;

 int compteur_appels=0; //compteur du nombre de fonctions dans la pile
 //et on est parti pour remonter la pile!
 while (1)
 {
  if (!StackWalk(IMAGE_FILE_MACHINE_I386,
              GetCurrentProcess(),
        me->calling_thread,
        &sf,
        &thread_context,
        NULL,
        SymFunctionTableAccess,
        SymGetModuleBase,
        NULL))
   break; //fin de la boucle : on a remonté toute la call stack (ou bienalors erreur)

  if (0==sf.AddrFrame.Offset) //test simple pour vérifier que l'on est pas complètement dans les choux
   break;

/*Remarque : il y a une bizarrerie dans le fonctionnment de
WaitForSingleObject : apparemement,
il écrase la stack frame de la fonction qui l'appelle. En effet, la pile
d'appels "théorique"  au moment de l'analyse est :
....
trucmuche
CallStackDumper::DumpCallStack
WaitForSingleObject
<fonctions appelées par WaitForSingleObject>

Hors, le dump donne le résultat suivant :
...
trucmuche
<-- ici, il manque CallStackDumper::DumpCallStack et WaitForSingleObject -->
GlobalAddAtomA
ZwCancelTimer

Si on enlève le WaitForSingleObject dans DumpCallStack, on a bien la stack
normale (avec DumpCallStack en haut),
mais on ne peut pas fonctionner sans WaitForSIngleObject car il y a alors un
risque de deadlock
*/

  if (++compteur_appels > 2) //ca ne sert à rien d'afficher GlobalAddAtomAet ZwCancelTimer, et c'est plutôt troublant pour l'utilsateur, à cause de laremarque ci-dessus
  {
   //maintenant, obtenir les informations symboliques correspondantes à sf
   BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 512]; //buffer pour recevoir IMAGEHLP_SYMBOL + la chaine de caractères contenant le symbole lui même
   IMAGEHLP_SYMBOL* symbol=(IMAGEHLP_SYMBOL*)symbolBuffer;
   symbol->SizeOfStruct=sizeof(symbolBuffer);
   symbol->MaxNameLength=512;

   DWORD symDisplacement; //offset entre l'adresse d'entrée et l'adresse du symbole corespondant

   if (SymGetSymFromAddr(GetCurrentProcess(), sf.AddrPC.Offset,
&symDisplacement, symbol))
   {
    //informations de debogage disponible : on écrit directement le symbole dans le fichier
    fichier<<symbol->Name<<" + "<<std::hex<<symDisplacement<<"\r\n";
   }
   else
   {
    //informations de debogage non disponible : on calcule l'adresse logique de la fonction, et on l'écrit
    char ModuleName [MAX_PATH]; //nom du module contenant la fonction
    DWORD section=0, offset=0; //section et offset dans le module
    GetLogicalAdress((void*)sf.AddrPC.Offset, ModuleName, MAX_PATH, section,
offset);
    fichier<<std::setw(4)<<section<<":"<<std::setw(8)<<offset<<""<<ModuleName<<"\r\n";
   }
  }
 }
 fichier<<"\r\n";
 fichier.close(); //flush sur disque

 //laisser le thread à analyser continuer sa vie
 ResumeThread(me->calling_thread);
 SetEvent(me->h_Event);

 return 0; //fin du thread d'analyse
}

//A partir d'une adresse linéaire, retourne le module contenant cette
//adresse, le section dans le module et l'offset dans la section
BOOL CallStackDumper::GetLogicalAdress(void* addr, char* module_name_buffer,
DWORD buffer_len, DWORD& section, DWORD& offset)
{
 MEMORY_BASIC_INFORMATION mbi;

 if (!VirtualQuery(addr, &mbi, sizeof(mbi)))
  return FALSE;

 DWORD hMod=(DWORD)mbi.AllocationBase;

 //nom du module
 if (!GetModuleFileName((HMODULE) hMod, module_name_buffer, buffer_len))
//le pointeur HMODULE est en fait l'adresse (virtuelle) du début de la(les) page(s) de code
  return FALSE;

 //pointeur vers le header DOS en mémoire du module
 PIMAGE_DOS_HEADER pDosHdr=(PIMAGE_DOS_HEADER)hMod; //le header DOS est la première chose dans la(les) page(s) de code

 //à partir du header DOS, trouver le pointeur vers le header PE
 PIMAGE_NT_HEADERS pNtHdr=(PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

 PIMAGE_SECTION_HEADER pSection=IMAGE_FIRST_SECTION(pNtHdr);

 DWORD rva=(DWORD) addr-hMod; //RVA = offset de l'adresse de chargement du module

 //on parcourt la table des sections, à la recherche d'une section quinclus l'adresse linéaire
 for (unsigned i=0; i< pNtHdr->FileHeader.NumberOfSections; ++i)
 {
  DWORD sectionStart=pSection->VirtualAddress;
  DWORD sectionEnd=sectionStart + max(pSection->SizeOfRawData,
pSection->Misc.VirtualSize);

  //est ce que l'adresse est dans cette section?
  if ( ( rva >= sectionStart) && (rva<=sectionEnd) )
  {
   //on a trouvé la section, reste plus qu'à calculer l'offset
   section=i+1; //numéro de la section
   offset=rva-sectionStart;
   return TRUE;
  }
 } return FALSE;
}


