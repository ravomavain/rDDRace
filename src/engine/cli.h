/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_CLIENT_H
#define ENGINE_CLIENT_H
#include "kernel.h"

#include "message.h"

class ICli : public IInterface
{
	MACRO_INTERFACE("cli", 0)
protected:
	// quick access to state of the client
	int m_State;
public:

	/* Constants: Client States
		STATE_OFFLINE - The client is offline.
		STATE_CONNECTING - The client is trying to connect to a server.
		STATE_LOADING - The client has connected to a server and is loading resources.
		STATE_ONLINE - The client is connected to a server and running the game.
		STATE_DEMOPLAYBACK - The client is playing a demo
		STATE_QUITING - The client is quiting.
	*/

	enum
	{
		STATE_OFFLINE=0,
		STATE_CONNECTING,
		STATE_LOADING,
		STATE_ONLINE,
		STATE_QUITING,
	};

	//
	inline int State() const { return m_State; }

	// actions
	virtual void Connect(const char *pAddress) = 0;
	virtual void Disconnect() = 0;
	virtual void Quit() = 0;

	// remote console
	virtual void RconAuth(const char *pUsername, const char *pPassword) = 0;
	virtual bool RconAuthed() = 0;
	virtual void Rcon(const char *pLine) = 0;

	virtual int SendMsg(CMsgPacker *pMsg, int Flags, bool System) = 0;
};
#endif
