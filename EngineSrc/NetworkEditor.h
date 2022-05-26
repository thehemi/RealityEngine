//======= Copyright (c) 2004, Artificial Studios. All rights reserved. ========
/// Network Editing and CVS
//=============================================================================



//-----------------------------------------------------------------------------
/// Supports functionality for networked client/server CVS and client/server editing
//
//-----------------------------------------------------------------------------
class ENGINE_API NetworkEditor
{
public:
	/// Client/Server Messages
	const static int MSGID_CLIENT_REQUESTFILES = 0;
	const static int MSGID_CLIENT_LOADWORLD    = 1;

	static NetworkEditor* NetworkEditor::Instance ()
	{
		static NetworkEditor inst;
		return &inst;
	}

	//-----------------------------------------------------------------------------
	/// Client

	/// Called when the user wants to connect to a server
	bool	Connect(string server);


	//-----------------------------------------------------------------------------
	/// Server

	/// Sent by network subsystem to server upon connection
	void	ClientConnected(void* clientInfoHere);
	/// Sent when client disconnects
	void	ClientDisconnected(void* clientInfoHere);
	/// Message sent by client
	void	ClientMessage(void* messageInfoHere);
};