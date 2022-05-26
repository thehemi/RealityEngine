#ifndef __AUTO_PATCHER_H
#define __AUTO_PATCHER_H

class RakPeerInterface;
class RakServerInterface;
class RakClientInterface;
struct Packet;
struct DownloadableFileDescriptor;

#include "NetworkTypes.h"
#include "ArrayList.h"
#include "SHA1.h"

enum SetFileDownloadableResult
{
	SET_FILE_DOWNLOADABLE_FAILED,
	SET_FILE_DOWNLOADABLE_FILE_NO_SIGNATURE_FILE,
	SET_FILE_DOWNLOADABLE_FILE_SIGNATURE_CHECK_FAILED,
	SET_FILE_DOWNLOADABLE_COMPRESSION_FAILED,
	SET_FILE_DOWNLOADABLE_SUCCESS,
};

class AutoPatcher
{
public:
	AutoPatcher();
	~AutoPatcher();

	// Frees all downloadable files.
	// Frees the download directory prefix.
	void Clear(void);

	// Set the ordering stream to send data on.  Defaults to 0.
	// Set this if you use ordered data for your game and don't want to hold up game data
	// because of autopatcher file data
	void SetOrderingStream(int streamIndex);

	// Call this to use RakPeer on sends.  Mutually exclusive with the other 2 overloads.
	void SetNetworkingSystem(RakPeerInterface *localSystem);
	// Call this to use RakClientInterface on sends.  Mutually exclusive with the other 2 overloads.
	void SetNetworkingSystem(RakClientInterface *localSystem);
	// Call this to use RakServerInterface on sends.  Mutually exclusive with the other 2 overloads.
	void SetNetworkingSystem(RakServerInterface *localSystem);

	// Set the value at which files larger than this will be compressed.
	// Files smaller than this will not be compressed.
	// Defaults to 1024.
	// Only changes files later passed to SetFileDownloadable, not files already processed
	void SetCompressionBoundary(unsigned boundary);

	// Creates a .sha file signature for a particular file.
	// This is used by SetFileDownloadable with checkFileSignature as true
	// Returns true on success, false on file does not exist.
	static bool CreateFileSignature(char *filename);

	// Makes a file downloadable
	// Returns true on success, false on can't open file
	// checkFileSignature - if true then check the associated .sha1 to make sure it describes our file.
	// checkFileSignature is useful to make sure a file wasn't externally modified by a hacker or a virus
	SetFileDownloadableResult SetFileDownloadable(char *filename, bool checkFileSignature);

	// Removes access to a file previously set as downloadable.
	bool UnsetFileDownloadable(char *filename);

	// Returns how many files are still in the download list.
	// Requires a previous call to OnAutopatcherFileList
	// If returns >=1 filename and fileLength will be filled in to match
	// the current file being downloaded
	// A value of 0 for compressedFileLength indicates unknown
	unsigned long GetDownloadStatus(char *filename, unsigned *fileLength, bool *fileDataIsCompressed, unsigned *compressedFileLength);

	// Sets a base directory to look for and put all downloaded files in.
	// For example, "Downloads"
	void SetDownloadedFileDirectoryPrefix(char *prefix);

	// Requests that the remote system send the directory of files that are downloadable.
	// The remote system should get ID_AUTOPATCHER_REQUEST_FILE_LIST.  When it does, it should call
	// SendDownloadableFileList with the playerID of the sender.
	// For the client, you can put UNASSIGNED_PLAYER_ID for remoteSystem
	void RequestDownloadableFileList(PlayerID remoteSystem);

	// If the packet identifier is ID_AUTOPATCHER_REQUEST_FILE_LIST, call this function with
	// packet->playerID
	// Sends a list of all downloadable files to the remote system
	// The remote system should get a packet with ID ID_AUTOPATCHER_FILE_LIST.
	// When it does, it should call
	// OnAutopatcherFileList with the packet from the network
	void SendDownloadableFileList(PlayerID remoteSystem);

	// If the packet identifier is ID_AUTOPATCHER_FILE_LIST, call this function with
	// the packet.
	// It will parse out all the files on the remote system and request to download
	// the ones we don't have or don't match.
	// The remote system should get a packet with ID 
	// ID_AUTOPATCHER_REQUEST_FILES for each file requested
	//
	// We can specify to only accept files if we previously requested them by a call
	// to RequestDownloadableFileList.  Set onlyAcceptFilesIfRequested to true to do this
	//
	// After this function is called you can call GetDownloadStatus(...)
	// To find out which, if any, files are currently downloading
	void OnAutopatcherFileList(Packet *packet, bool onlyAcceptFilesIfRequested);

	// If the packet identifier is ID_AUTOPATCHER_REQUEST_FILES, call this function with
	// the packet.
	// Reads the files from disk and sends them to the specified system
	void OnAutopatcherRequestFiles(Packet *packet);

	// If the packet identifier is ID_AUTOPATCHER_SET_DOWNLOAD_LIST, call this function with
	// the packet.
	// Finalizes the list of files that will be downloaded.
	void OnAutopatcherSetDownloadList(Packet *packet);

	// If the packet identifier is ID_AUTOPATCHER_WRITE_FILE, call this function with
	// the packet.
	// Writes a file to disk.  There is security to prevent writing files we
	// didn't ask for
	bool OnAutopatcherWriteFile(Packet *packet);

	// Used internally
	static bool GenerateSHA1(char *filename, char SHA1Code[SHA1_LENGTH]);
	static int GetFileLength(char *filename);
	static bool WriteFileWithDirectories(const char *path, char *data, unsigned dataLength);

protected:
	// One of these 3 must be set in order to send data
	RakPeerInterface *rakPeerInterface;
	RakClientInterface *rakClientInterface;
	RakServerInterface *rakServerInterface;

	// The directory to download into basically
	char *downloadPrefix;

	// Files of length >= this will be compressed
	unsigned compressionBoundary;

	bool downloadableFilesRequested;

	int orderingStream;

	// Directory of files that are downloadable
	BasicDataStructures::List<DownloadableFileDescriptor*> downloadableFiles, downloadingFiles;
};


#endif
