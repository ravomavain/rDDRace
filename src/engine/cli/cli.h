#ifndef ENGINE_CLI_CLI_H
#define ENGINE_CLI_CLI_H

class CCli : public ICli
{
	// needed interfaces
	IEngine *m_pEngine;
	IConsole *m_pConsole;
	IStorage *m_pStorage;

	int m_RconAuthed;

	// pinging
	int64 m_PingStartTime;

	class CJob m_ConsoleJob;
	CNetObjHandler m_NetObjHandler;

public:
	IEngine *Engine() { return m_pEngine; }
	IStorage *Storage() { return m_pStorage; }
	IConsole *Console() { return m_pConsole; }

	class CNetClient m_Net;
	
	NETADDR m_ServerAddress;

	CCli();

	int SendMsg(CMsgPacker *pMsg, int Flags, bool System=1);
	void SendInfo();
	void SendEnterGame();
	void SendReady();
	void SendStartInfo();
	void SendSwitchTeam(int Team);

	void DisconnectWithReason(const char *pReason);
	void Disconnect();
	void Connect(const char *pAddress);
	void Quit();

	void RconAuth(const char *pUsername, const char *pPassword);
	bool RconAuthed();
	void Rcon(const char *pLine);
	
	void ProcessServerPacket(CNetChunk *pPacket);
	void PumpNetwork();

	void Run();
	static int ConsoleLoop(void *pCli);

	void SetState(int s) { m_State = s; }

	void RegisterInterfaces();
	void InitInterfaces();
	
	static void Con_Connect(IConsole::IResult *pResult, void *pUserData, int ClientID);
	static void Con_Disconnect(IConsole::IResult *pResult, void *pUserData, int ClientID);
	static void Con_Quit(IConsole::IResult *pResult, void *pUserData, int ClientID);
	static void Con_Ping(IConsole::IResult *pResult, void *pUserData, int ClientID);
	static void Con_Rcon(IConsole::IResult *pResult, void *pUserData, int ClientID);
	static void Con_RconAuth(IConsole::IResult *pResult, void *pUserData, int ClientID);
	static void Con_Team(IConsole::IResult *pResult, void *pUserData, int ClientID);
	static void Con_State(IConsole::IResult *pResult, void *pUserData, int ClientID);

	void RegisterCommands();
};
#endif
