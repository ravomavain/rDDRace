/* copyright (c) 2008 rajh and gregwar. Score stuff */

#ifndef GAME_SERVER_FILESCORE_H
#define GAME_SERVER_FILESCORE_H

#include <base/tl/sorted_array.h>

#include "../score.h"

class CFileScore : public IScore
{
	CGameContext *m_pGameServer;
	IServer *m_pServer;

	class CPlayerScore
	{
	public:
		char m_aName[MAX_NAME_LENGTH];
		float m_Score;
		float m_aCpTime[NUM_CHECKPOINTS];

		CPlayerScore() {};
		CPlayerScore(const char *pName, float Score, float aCpTime[NUM_CHECKPOINTS]);

		bool operator<(const CPlayerScore& other) { return (this->m_Score < other.m_Score); }
	};

	sorted_array<CPlayerScore> m_Top;

	CGameContext *GameServer() { return m_pGameServer; }
	IServer *Server() { return m_pServer; }

	CPlayerScore *SearchScore(int ID, int *pPosition){ return SearchName(Server()->ClientName(ID), pPosition, 0 );};

	CPlayerScore *SearchName(const char *pName, int *pPosition, bool MatchCase);
	void UpdatePlayer(int ID, float Score, float aCpTime[NUM_CHECKPOINTS]);

	void Init();
	void Save();
	static void SaveScoreThread(void *pUser);

public:

	CFileScore(CGameContext *pGameServer);
	~CFileScore();

	virtual void LoadScore(int ClientID);
	virtual void SaveScore(int ClientID, float Time, CCharacter *pChar);

	virtual void ShowTop5(IConsole::IResult *pResult, int ClientID, int Debut=1);
	virtual void ShowRank(int ClientID, const char* pName, bool Search=false);
};

#endif
