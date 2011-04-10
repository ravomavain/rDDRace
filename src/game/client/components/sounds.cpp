/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/engine.h>
#include <engine/sound.h>
#include <engine/shared/config.h>
#include <game/generated/client_data.h>
#include <game/client/gameclient.h>
#include <game/client/components/camera.h>
#include <game/client/components/menus.h>
#include "sounds.h"


struct CUserData
{
	CGameClient *m_pGameClient;
	bool m_Render;
} g_UserData;

static int LoadSoundsThread(void *pUser)
{
	CUserData *pData = static_cast<CUserData *>(pUser);
	
	for(int s = 0; s < g_pData->m_NumSounds; s++)
	{
		for(int i = 0; i < g_pData->m_aSounds[s].m_NumSounds; i++)
		{
			int Id = pData->m_pGameClient->Sound()->LoadWV(g_pData->m_aSounds[s].m_aSounds[i].m_pFilename);
			g_pData->m_aSounds[s].m_aSounds[i].m_Id = Id;
		}

		if(pData->m_Render)
			pData->m_pGameClient->m_pMenus->RenderLoading();
	}

	return 0;
}
void CSounds::OnInit()
{
	// setup sound channels
	Sound()->SetChannel(CSounds::CHN_GUI, 1.0f, 0.0f);
	Sound()->SetChannel(CSounds::CHN_MUSIC, 1.0f, 0.0f);
	Sound()->SetChannel(CSounds::CHN_WORLD, 0.9f, 1.0f);
	Sound()->SetChannel(CSounds::CHN_GLOBAL, 1.0f, 0.0f);

	Sound()->SetListenerPos(0.0f, 0.0f);

	ClearQueue();

	// load sounds
	if(g_Config.m_ClThreadsoundloading)
	{
		g_UserData.m_pGameClient = m_pClient;
		g_UserData.m_Render = false;
		m_pClient->Engine()->AddJob(&m_SoundJob, LoadSoundsThread, &g_UserData);
		m_WaitForSoundJob = true;
	}
	else
	{
		g_UserData.m_pGameClient = m_pClient;
		g_UserData.m_Render = true;
		LoadSoundsThread(&g_UserData);
		m_WaitForSoundJob = false;
	}
}

void CSounds::OnReset()
{
	Sound()->StopAll();
	ClearQueue();
}

void CSounds::OnRender()
{
	// check for sound initialisation
	if(m_WaitForSoundJob && m_SoundJob.Status() == CJob::STATE_DONE)
		m_WaitForSoundJob = false;

	// set listner pos
	Sound()->SetListenerPos(m_pClient->m_pCamera->m_Center.x, m_pClient->m_pCamera->m_Center.y);

	// play sound from queue
	if(m_QueuePos > 0)
	{
		int64 Now =  time_get();
		if(m_QueueWaitTime <= Now)
		{
			Play(CHN_GLOBAL, m_aQueue[0], 1.0f, vec2(0,0));
			m_QueueWaitTime = Now+time_freq()*3/10; // wait 300ms before playing the next one
			if(--m_QueuePos > 0)
				mem_move(m_aQueue, m_aQueue+1, m_QueuePos*sizeof(int));
		}
	}
}

void CSounds::ClearQueue()
{
	mem_zero(m_aQueue, sizeof(m_aQueue));
	m_QueuePos = 0;
	m_QueueWaitTime = time_get();
}

void CSounds::Enqueue(int SetId)
{
	// add sound to the queue
	if(!g_Config.m_ClEditor && m_QueuePos < QUEUE_SIZE)
		m_aQueue[m_QueuePos++] = SetId;
}

void CSounds::PlayAndRecord(int Chn, int SetId, float Vol, vec2 Pos)
{
	CNetMsg_Sv_SoundGlobal Msg;
	Msg.m_SoundID = SetId;
	Client()->SendPackMsg(&Msg, MSGFLAG_NOSEND|MSGFLAG_RECORD);
	
	Play(Chn, SetId, Vol, Pos);
}

void CSounds::Play(int Chn, int SetId, float Vol, vec2 Pos)
{
	if(m_WaitForSoundJob || SetId < 0 || SetId >= g_pData->m_NumSounds)
		return;

	SOUNDSET *pSet = &g_pData->m_aSounds[SetId];

	if(!pSet->m_NumSounds)
		return;

	if(pSet->m_NumSounds == 1)
	{
		Sound()->PlayAt(Chn, pSet->m_aSounds[0].m_Id, 0, Pos.x, Pos.y);
		return;
	}

	// play a random one
	int Id;
	do
	{
		Id = rand() % pSet->m_NumSounds;
	}
	while(Id == pSet->m_Last);
	Sound()->PlayAt(Chn, pSet->m_aSounds[Id].m_Id, 0, Pos.x, Pos.y);
	pSet->m_Last = Id;
}
