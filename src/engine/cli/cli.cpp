#include <stdio.h>
#include <stdlib.h>
#include <base/system.h>

#include <engine/cli.h>
#include <engine/config.h>
#include <engine/console.h>
#include <engine/engine.h>
#include <engine/storage.h>

#include <engine/shared/network.h>
#include <engine/shared/protocol.h>
#include <engine/shared/config.h>

#include <game/version.h>
#include <game/generated/protocol.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "cli.h"

CCli::CCli()
{
	m_pConsole = 0;
	m_RconAuthed = 0;
	m_State = ICli::STATE_OFFLINE;
}



int CCli::SendMsg(CMsgPacker *pMsg, int Flags, bool System)
{
	CNetChunk Packet;

	mem_zero(&Packet, sizeof(CNetChunk));

	Packet.m_ClientID = 0;
	Packet.m_pData = pMsg->Data();
	Packet.m_DataSize = pMsg->Size();

	// HACK: modify the message id in the packet and store the system flag
	if(*((unsigned char*)Packet.m_pData) == 1 && System && Packet.m_DataSize == 1)
		dbg_break();

	*((unsigned char*)Packet.m_pData) <<= 1;
	if(System)
		*((unsigned char*)Packet.m_pData) |= 1;

	if(Flags&MSGFLAG_VITAL)
		Packet.m_Flags |= NETSENDFLAG_VITAL;
	if(Flags&MSGFLAG_FLUSH)
		Packet.m_Flags |= NETSENDFLAG_FLUSH;
	if(!(Flags&MSGFLAG_NOSEND))
		m_Net.Send(&Packet);
	return 0;
}

void CCli::SendInfo()
{
	CMsgPacker Msg(NETMSG_INFO);
	Msg.AddString(GAME_NETVERSION, 128);
	Msg.AddString(g_Config.m_Password, 128);
	SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH);
}

void CCli::SendEnterGame()
{
	CMsgPacker Msg(NETMSG_ENTERGAME);
	SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH);
}

void CCli::SendReady()
{
	CMsgPacker Msg(NETMSG_READY);
	SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH);
}

void CCli::SendStartInfo()
{
	CMsgPacker Msg(NETMSGTYPE_CL_STARTINFO);
	Msg.AddString(g_Config.m_PlayerName, -1);
	Msg.AddString(g_Config.m_PlayerClan, -1);
	Msg.AddInt(g_Config.m_PlayerCountry);
	Msg.AddString(g_Config.m_PlayerSkin, -1);
	Msg.AddInt(g_Config.m_PlayerUseCustomColor);
	Msg.AddInt(g_Config.m_PlayerColorBody);
	Msg.AddInt(g_Config.m_PlayerColorFeet);
	SendMsg(&Msg, MSGFLAG_VITAL, 0);
}

void CCli::SendSwitchTeam(int Team)
{
	CMsgPacker Msg(NETMSGTYPE_CL_SETTEAM);
	Msg.AddInt(Team);
	SendMsg(&Msg, MSGFLAG_VITAL, 0);
}

void CCli::DisconnectWithReason(const char *pReason)
{
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "disconnecting. reason='%s'", pReason?pReason:"unknown");
	m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "client", aBuf);

	m_RconAuthed = 0;
	m_Net.Disconnect(pReason);
	SetState(ICli::STATE_OFFLINE);
}

void CCli::Disconnect()
{
	DisconnectWithReason(0);
}

void CCli::Connect(const char *pAddress)
{
	char aBuf[512];
	int Port = 8303;
	mem_zero(&m_ServerAddress, sizeof(m_ServerAddress));

	str_format(aBuf, sizeof(aBuf), "connecting to '%s'", pAddress);
	m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "client", aBuf);

	if(net_host_lookup(pAddress, &m_ServerAddress, NETTYPE_ALL) != 0)
	{
		char aBufMsg[256];
		str_format(aBufMsg, sizeof(aBufMsg), "could not find the address of %s, connecting to localhost", aBuf);
		m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "client", aBufMsg);
		net_host_lookup("localhost", &m_ServerAddress, NETTYPE_ALL);
	}

	m_RconAuthed = 0;
	if(m_ServerAddress.port == 0)
		m_ServerAddress.port = Port;
	m_Net.Connect(&m_ServerAddress);
	SetState(ICli::STATE_CONNECTING);
}

void CCli::Quit()
{
	SetState(ICli::STATE_QUITING);
}

bool CCli::RconAuthed()
{
	return m_RconAuthed;
}

void CCli::RconAuth(const char *pName, const char *pPassword)
{
	if(RconAuthed())
		return;

	CMsgPacker Msg(NETMSG_RCON_AUTH);
	Msg.AddString(pName, 32);
	Msg.AddString(pPassword, 32);
	SendMsg(&Msg, MSGFLAG_VITAL);
}

void CCli::Rcon(const char *pCmd)
{
	CMsgPacker Msg(NETMSG_RCON_CMD);
	Msg.AddString(pCmd, 256);
	SendMsg(&Msg, MSGFLAG_VITAL);
}

void CCli::ProcessServerPacket(CNetChunk *pPacket)
{
	CUnpacker Unpacker;
	Unpacker.Reset(pPacket->m_pData, pPacket->m_DataSize);

	// unpack msgid and system flag
	int Msg = Unpacker.GetInt();
	int Sys = Msg&1;
	Msg >>= 1;

	if(Unpacker.Error())
		return;

	if(Sys)
	{
		// system message
		if(Msg == NETMSG_MAP_CHANGE)
		{
			SendReady();
		}
		else if(Msg == NETMSG_MAP_DATA)
		{
			SendReady();
		}
		else if(Msg == NETMSG_CON_READY)
		{
			SendStartInfo();
			//TODO: GameClient()->OnConnected();
		}
		else if(Msg == NETMSG_PING)
		{
			CMsgPacker Msg(NETMSG_PING_REPLY);
			SendMsg(&Msg, 0);
		}
		else if(Msg == NETMSG_RCON_AUTH_STATUS)
		{
			int Result = Unpacker.GetInt();
			if(Unpacker.Error() == 0)
				m_RconAuthed = Result;
		}
		else if(Msg == NETMSG_RCON_LINE)
		{
			const char *pLine = Unpacker.GetString();
			if(Unpacker.Error() == 0)
				m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rcon", pLine);
		}
		else if(Msg == NETMSG_PING_REPLY)
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "latency %.2f", (time_get() - m_PingStartTime)*1000 / (float)time_freq());
			m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "client/network", aBuf);
		}
		else if(Msg == NETMSG_INPUTTIMING)
		{
			//TODO:
		}
		else if(Msg == NETMSG_SNAP || Msg == NETMSG_SNAPSINGLE || Msg == NETMSG_SNAPEMPTY)
		{
			SetState(ICli::STATE_ONLINE);
		}
		else if(g_Config.m_Debug)
			dbg_msg("packet", "Got system message id=%d", Msg);
	}
	else
	{
		if(Msg == NETMSGTYPE_SV_READYTOENTER)
		{
			SendEnterGame();
			SendSwitchTeam(TEAM_SPECTATORS);
		}
		else if(Msg == NETMSGTYPE_SV_MOTD)
		{
			const char *pMotd = Unpacker.GetString();
			if(Unpacker.Error() == 0)
				m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "motd", pMotd);
		}
		else if(Msg == NETMSGTYPE_SV_CHAT)
		{
			int Team = Unpacker.GetInt();
			int ClientID = Unpacker.GetInt();
			const char *pMessage = Unpacker.GetString();
			if(Unpacker.Error() == 0)
			{
				char aBuf[1024];
				str_format(aBuf, sizeof(aBuf), "(%d|%d) %s", ClientID, Team, pMessage);
				m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chat", aBuf);
			}
		}
		else if(g_Config.m_Debug)
			dbg_msg("packet", "Got normal message id=%d : %s", Msg, m_NetObjHandler.GetMsgName(Msg));
	}
	
}

void CCli::PumpNetwork()
{
	m_Net.Update();
	if(State() != ICli::STATE_OFFLINE && State() != ICli::STATE_QUITING && m_Net.State() == NETSTATE_OFFLINE)
	{
		SetState(ICli::STATE_OFFLINE);
		Disconnect();
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "offline error='%s'", m_Net.ErrorString());
		m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "cli", aBuf);
	}

	if(State() == ICli::STATE_CONNECTING && m_Net.State() == NETSTATE_ONLINE)
	{
		// we switched to online
		m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "cli", "connected, sending info");
		SetState(ICli::STATE_LOADING);
		SendInfo();
	}

	// process packets
	CNetChunk Packet;
	while(m_Net.Recv(&Packet))
	{
		if(Packet.m_ClientID != -1)
			ProcessServerPacket(&Packet);
	}
}

int CCli::ConsoleLoop(void *pCli)
{
	CCli *pSelf = (CCli *)pCli;
	IConsole *pConsole = pSelf->Console();
	while(1)
	{
		char* cmd;
		if(pSelf->State() == ICli::STATE_QUITING)
			break;
		if(pSelf->RconAuthed())
			cmd = readline("rcon> ");
		else
			cmd = readline("local> ");
		if (cmd && *cmd)
		{
			if(pSelf->RconAuthed())
			{
				pSelf->Rcon(cmd);
				add_history(cmd);
				free(cmd);
			}
			else
			{
				if(str_comp(cmd,"rcon")==0)
				{
					free(cmd);
					if(pSelf->State() == ICli::STATE_ONLINE)
					{
						cmd = readline("Password> ");
						if (cmd && *cmd)
						{
							pSelf->RconAuth("", cmd);
							free(cmd);
						}
					}
				}
				else
				{
					pConsole->ExecuteLine(cmd, -1, IConsole::CONSOLELEVEL_USER, 0, 0);
					add_history(cmd);
					free(cmd);
				}
			}
		}
		else if(cmd == (char*)NULL)
		{
			printf("\n");
			if(pSelf->State() == ICli::STATE_ONLINE)
			{
				pSelf->Disconnect();
			}
			else if(pSelf->State() == ICli::STATE_OFFLINE)
			{
				pSelf->Quit();
			}
		}
		// be nice to the CPU
		thread_sleep(1);
	}
	return 0;
}

void CCli::Run()
{
	// open socket
	{
		NETADDR BindAddr;
		mem_zero(&BindAddr, sizeof(BindAddr));
		BindAddr.type = NETTYPE_ALL;
		if(!m_Net.Open(BindAddr, 0))
		{
			dbg_msg("cli", "couldn't start network");
			return;
		}
	}
	
	Engine()->AddJob(&m_ConsoleJob, ConsoleLoop, this);
	
	//Connect("");
	
	m_pConsole->StoreCommands(false, -1);
	
	while(m_ConsoleJob.Status() != CJob::STATE_DONE)
	{
		PumpNetwork();
		
		// be nice to the CPU
		thread_sleep(1);
	}
	Disconnect();
}


void CCli::RegisterInterfaces() {}

void CCli::InitInterfaces()
{
	// fetch interfaces
	m_pEngine = Kernel()->RequestInterface<IEngine>();
	m_pStorage = Kernel()->RequestInterface<IStorage>();
}

void CCli::Con_Connect(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CCli *pSelf = (CCli *)pUserData;
	pSelf->Connect(pResult->GetString(0));
}

void CCli::Con_Disconnect(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CCli *pSelf = (CCli *)pUserData;
	pSelf->Disconnect();
}

void CCli::Con_Quit(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CCli *pSelf = (CCli *)pUserData;
	pSelf->Quit();
}

void CCli::Con_Ping(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CCli *pSelf = (CCli *)pUserData;

	CMsgPacker Msg(NETMSG_PING);
	pSelf->SendMsg(&Msg, 0);
	pSelf->m_PingStartTime = time_get();
}

void CCli::Con_Rcon(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CCli *pSelf = (CCli *)pUserData;
	pSelf->Rcon(pResult->GetString(0));
}

void CCli::Con_Team(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	((CCli*)pUserData)->SendSwitchTeam(pResult->GetInteger(0));
}

void CCli::Con_RconAuth(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CCli *pSelf = (CCli *)pUserData;
	pSelf->RconAuth("", pResult->GetString(0));
}

void CCli::Con_State(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CCli *pSelf = (CCli *)pUserData;
	switch(pSelf->State())
	{
		case STATE_OFFLINE:
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "state", "Offline");
			break;
		case STATE_CONNECTING:
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "state", "Connecting");
			break;
		case STATE_LOADING:
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "state", "Loading");
			break;
		case STATE_ONLINE:
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "state", "Online");
			break;
		case STATE_QUITING:
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "state", "Quiting");
			break;
	}
}

void CCli::RegisterCommands()
{
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	// register server dummy commands for tab completion
	m_pConsole->Register("kick", "i?r", CFGFLAG_SERVER, 0, 0, "Kick player with specified id for any reason", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("ban", "s?ir", CFGFLAG_SERVER, 0, 0, "Ban player with ip/id for x minutes for any reason", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("unban", "s", CFGFLAG_SERVER, 0, 0, "Unban ip", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("bans", "", CFGFLAG_SERVER, 0, 0, "Show banlist", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("status", "", CFGFLAG_SERVER, 0, 0, "List players", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("shutdown", "", CFGFLAG_SERVER, 0, 0, "Shut down", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("record", "?s", CFGFLAG_SERVER, 0, 0, "Record to a file", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("stoprecord", "", CFGFLAG_SERVER, 0, 0, "Stop recording", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("reload", "", CFGFLAG_SERVER, 0, 0, "Reload the map", IConsole::CONSOLELEVEL_USER);

	
	m_pConsole->Register("team", "i", CFGFLAG_CLI, Con_Team, this, "Switch team", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("quit", "", CFGFLAG_CLI|CFGFLAG_STORE, Con_Quit, this, "Quit Teeworlds", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("exit", "", CFGFLAG_CLI|CFGFLAG_STORE, Con_Quit, this, "Quit Teeworlds", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("connect", "s", CFGFLAG_CLI, Con_Connect, this, "Connect to the specified host/ip", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("disconnect", "", CFGFLAG_CLI, Con_Disconnect, this, "Disconnect from the server", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("ping", "", CFGFLAG_CLI, Con_Ping, this, "Ping the current server", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("rcon", "r", CFGFLAG_CLI, Con_Rcon, this, "Send specified command to rcon", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("rcon_auth", "s", CFGFLAG_CLI, Con_RconAuth, this, "Authenticate to rcon", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("state", "", CFGFLAG_CLI, Con_State, this, "Get cli state", IConsole::CONSOLELEVEL_USER);

	// DDRace

	m_pConsole->Register("login", "?s", CFGFLAG_SERVER, 0, 0, "Allows you access to rcon if no password is given, or changes your level if a password is given", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("auth", "?s", CFGFLAG_SERVER, 0, 0, "Allows you access to rcon if no password is given, or changes your level if a password is given", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("vote", "r", CFGFLAG_SERVER, 0, 0, "Forces the current vote to result in r (Yes/No)", IConsole::CONSOLELEVEL_USER);
	m_pConsole->Register("cmdlist", "", CFGFLAG_SERVER, 0, 0, "Shows the list of all commands", IConsole::CONSOLELEVEL_USER);

	#define CONSOLE_COMMAND(name, params, flags, callback, userdata, help, level) m_pConsole->Register(name, params, flags, 0, 0, help, level);
	#include <game/ddracecommands.h>
}


static CCli m_Cli;

int main(int argc, const char **argv) // ignore_convention
{
	IKernel *pKernel = IKernel::Create();
	pKernel->RegisterInterface(&m_Cli);
	m_Cli.RegisterInterfaces();

	// create the components
	IEngine *pEngine = CreateEngine("Teeworlds");
	IConsole *pConsole = CreateConsole(CFGFLAG_CLI);
	IStorage *pStorage = CreateStorage("Teeworlds", argc, argv); // ignore_convention
	IConfig *pConfig = CreateConfig();

	{
		bool RegisterFail = false;

		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pEngine);
		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pConsole);
		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pConfig);
		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pStorage);

		if(RegisterFail)
			return -1;
	}
	
	
	pEngine->Init();
	pConfig->Init();
	str_copy(g_Config.m_PlayerName, "Administrator", sizeof("Administrator"));
	
	// register all console commands
	m_Cli.RegisterCommands();

	// init client's interfaces
	m_Cli.InitInterfaces();

	// execute autoexec file
	pConsole->ExecuteFile("cli.cfg", -1, IConsole::CONSOLELEVEL_CONFIG, 0, 0);

	// parse the command line arguments
	if(argc > 1) // ignore_convention
		pConsole->ParseArguments(argc-1, &argv[1]); // ignore_convention
	
	// restore empty config strings to their defaults
	pConfig->RestoreStrings();

	pEngine->InitLogfile();
	
	m_Cli.Run();
	
	return 0;
}
