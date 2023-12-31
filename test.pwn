// generated by "sampctl package init"
#include <open.mp>

#include "teams.inc"

main() 
{
	// write code here and run "sampctl package build" to compile
	// then run "sampctl package run" to run it
}

new Team: police,
	Team: gang;
public OnGameModeInit()
{
	Team_Log(true);
	police = Team_Create("police", 30);
	gang = Team_Create("gang", 30);
	return 1;
}

public OnGameModeExit()
{
	Team_Delete(police);
	Team_Delete(gang);
	return 1;
}

public OnPlayerConnect(playerid)
{
	printf("Player team %d", _:Team_GetPlayer(playerid));
	new Error: e = Team_AddMember(police, playerid);
	if(IsError(e))
	{
		if(e == Error:1)
		{
			print("Invalid team 1");
		}
		else if(e == Error:2)
		{
			print("Player is in some team 1");
		}
		Handled(true);
	}
	new Error: e1 = Team_AddMember(gang, playerid);
	if(IsError(e1))
	{
		if(e1 == Error:1)
		{
			print("Invalid team");
		}
		else if(e1 == Error:2)
		{
			print("Player is in some team");
		}
		Handled(true);
	}
	printf("3. Player team %d & rank %d", _:Team_GetPlayer(playerid), _:Team_GetPlayerRank(playerid));

	new TeamRank: first = Team_AddRank(police, "private lol");
	Team_SetPlayerRank(first, playerid);
	new rank[MAX_RANK_NAME];
	Team_GetRankName(police, first, rank);
	printf("rank name %s", rank);
	printf("2. Player team %d & rank %d", _:Team_GetPlayer(playerid), _:Team_GetPlayerRank(playerid));
	Team_SetPlayerRank(TeamRank: INVALID_TEAM_RANK, playerid);
	printf("1. Player team %d & rank %d", _:Team_GetPlayer(playerid), _:Team_GetPlayerRank(playerid));

	Team_SetPlayerRank(first, playerid);
	Team_RemoveRank(police, first);

	printf("0. Player team %d & rank %d", _:Team_GetPlayer(playerid), _:Team_GetPlayerRank(playerid));
	Team_SendMessage(-1, police, "Test %d", 3);
	Team_RemoveMember(police, playerid);
	

	return 1;
}