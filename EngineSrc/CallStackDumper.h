
/// CallStackDumper.h: interface for the CallStackDumper class.
//
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /

/************* Classe CallStackDumper ************

  Cette classe permet de dumper dynamiquement dans un fichier l'�tat de la
pile d'appels.
  Pour faire ce dump, d�clarer un objet CallStackDumper et appeler sa
m�thode DumpCallStack
  � l'endroit o� l'on veut voir la pile.

  Remarques :
  - La construction de l'objet est relativement lourde (chargement de DLL,
  chargement des infos de debogage, ...) Si on doit faire beaucoup de dumps,
il est recommand�
  de ne construire qu'un seul objet (si n�cessaire, en faire une variable
globale)

  - Si on en pr�cise pas le nom du fichier de dump dans le constructeur, la
classe cr��
  par d�faut un fichier dans le r�pertoire de l'executable du programme,
avec le nom de
  l'executable et une extension ".rpt"

  - Lors de la construction de l'objet et lors de l'appel � Set_nom_fichier,
le fichier de dump
  est ECRASE s'il existe d�j�. Par la suite, lors des appels successifs �
DumpCallStack,
  les nouveaux dumps sont rajout�s � la fin du fichier.

  - Le thread qui appelle DumpCallStack est interrompu le temps du dump. En
tenir compte
  dans le cas d'une application multithread.

  - L'appel � DumpCallStack ainsi que son fonctionnement interne n'apparait
pas dans le dump.

  - La classe utilise des objets de la STL. Elle n'est donc pas adapt�e aux
cas de crash
  de l'application, car la CRT et/ou la STL pourraient �tre mortes � ce
moment.

  - La classe essaie de r�cup�rer les noms symboliques des fonctions mais
s'ils ne sont pas
  disponibles (pas d'infos de debogage), elle dumpe l'adresse pyhysique dans
le module correspondant.
  Le chemin de recherche des infos de debogage peut �tre d�fini dans le
constructeur.
  Voir la documentation de SymInitialize pour le d�tail sur la recherche des
infos de debogage.
*/


#if !defined(AFX_ALLSTACKDUMPER_H__DE86F3E4_82F6_4D86_8DD9_E8568819200B__INCLUDED_)
#define AFX_ALLSTACKDUMPER_H__DE86F3E4_82F6_4D86_8DD9_E8568819200B__INCLUDED_

//#if _MSC_VER > 1000
//#pragma once
//#endif /// _MSC_VER > 1000

#include <windows.h>
#include <imagehlp.h>
#include <string>

/// Outputs a dmp file listing DLL call stack in case of a crash
class ENGINE_API CallStackDumper
{
public:
 CallStackDumper(char const* path_debug_infos=0);
 CallStackDumper::CallStackDumper(char const* nom_fichier_, char const* path_debug_infos=0);
 ~CallStackDumper();

 inline char const* Get_nom_fichier() const {return nom_fichier.c_str();}
 inline void Set_nom_fichier(char const* nom) {nom_fichier=nom;DeleteFile(nom);}

 void DumpCallStack(); //effectue le dump

private:
 //typedefs pour les diff�rentes m�thodes de ImageHlp
 typedef BOOL (__stdcall *SYMINITIALIZEPROC)(HANDLE, LPSTR, BOOL);
 typedef BOOL (__stdcall *SYMCLEANUPPROC)(HANDLE);
 typedef BOOL (__stdcall *STACKWALKPROC)(DWORD, HANDLE, HANDLE,LPSTACKFRAME, LPVOID, PREAD_PROCESS_MEMORY_ROUTINE,
	PFUNCTION_TABLE_ACCESS_ROUTINE, PGET_MODULE_BASE_ROUTINE,PTRANSLATE_ADDRESS_ROUTINE);
 typedef BOOL (__stdcall *SYMGETSYMFROMADDRPROC)(HANDLE, DWORD, PDWORD,
PIMAGEHLP_SYMBOL);
 typedef LPVOID (__stdcall *SYMFUNCTIONTABLEACCESSPROC)(HANDLE, DWORD);
 typedef DWORD (__stdcall *SYMGETMODULEBASEPROC)(HANDLE, DWORD);


 void init(); //initialisation de l'objet, appel� par les constructeurs

 static BOOL GetLogicalAdress(void* addr, char* module_name_buffer, DWORD
buffer_len, DWORD& section, DWORD& offset);

 static DWORD WINAPI ThreadFunc_DumpCallStack(void* param); //fonction de thread qui effectue le travail r�el

 string nom_fichier; //nom du fichier de dump
 string path_recherche_dbg; //chemin de recherche des infos de debogage
 HANDLE h_Event; //handle vers un �v�nement servant � synchronsier les thread
 HANDLE calling_thread; //handle du thread effectuant l'appel � DumpThreadCallStack

 HINSTANCE h_imagehlp;
 //pointeurs vers les m�thodes de ImageHlp
 static SYMINITIALIZEPROC SymInitialize;
 static SYMCLEANUPPROC SymCleanup;
 static STACKWALKPROC StackWalk;
 static SYMGETSYMFROMADDRPROC SymGetSymFromAddr;
 static SYMFUNCTIONTABLEACCESSPROC SymFunctionTableAccess;
 static SYMGETMODULEBASEPROC SymGetModuleBase;
};

#endif //!defined(AFX_ALLSTACKDUMPER_H__DE86F3E4_82F6_4D86_8DD9_E8568819200B__INCLUDED_)

