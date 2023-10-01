#include <open.mp>

#if !defined MAX_TEAMS
    #define MAX_TEAMS 30
#endif

#if !defined MAX_RANKS
    #define MAX_RANKS 30
#endif

#if !defined MAX_RANK_NAME
    #define MAX_RANK_NAME 60
#endif

#if !defined MAX_TEAM_NAME
    #define MAX_TEAM_NAME 100
#endif

#define INVALID_TEAM -1
#define INVALID_TEAM_RANK -1

#define __TEAMS_VERSION__ "1.0.0"


#include <errors>
#include <logger>
#include <text>
#include <YSI_Coding\y_hooks>
#include <YSI_Data\y_iterate>

// --
// Header
// --

/** 
 * <library>messages</library>
 * <summary>Sends a message to a team.</summary>
 * <param name="color">Color.</param>
 * <param name="teamid">Team to send message to.</param>
 * <param name="message">Text to send.</param>
 * <returns>False if team is invalid, otherwise true.</returns>
 */
forward bool: Team_SendMessage(color, Team: teamid, const string: message[144], OPEN_MP_TAGS: ...);

/** 
 * <library>messages</library>
 * <summary>Gets player rank.</summary>
 * <param name="playerid">Player we get rank from.</param>
 * <returns>Player's rank.</returns>
 */
forward TeamRank: Team_GetPlayerRank(playerid);

/** 
 * <library>messages</library>
 * <summary>Creates a team.</summary>
 * <param name="name">Name of a team.</param>
 * <param name="maxmembers">Max members.</param>
 * <returns>Index of the created team.</returns>
 */
forward Team: Team_Create(const string: name[MAX_TEAM_NAME], maxmembers);

/** 
 * <library>messages</library>
 * <summary>Deletes a team.</summary>
 * <param name="teamid">Team to delete.</param>
 * <returns>True if function executed successfully, otherwise false (team does not exist).</returns>
 */
forward bool: Team_Delete(Team: teamid);

/** 
 * <library>messages</library>
 * <summary>Checks if a team is valid.</summary>
 * <param name="teamid">Team to check.</param>
 * <returns>True if valid, otherwise false.</returns>
 */
forward bool: Team_IsValid(Team: teamid);

/** 
 * <library>messages</library>
 * <summary>Adds a player to a group.</summary>
 * <param name="teamid">Team to add player to.</param>
 * <param name="playerid">Player to add.</param>
 * <returns>Error code 1 if team is invalid, error code 2 if player is already in some team, error code 3 if limit of members if reached, otherwise 0.</returns>
 */
forward Error: Team_AddMember(Team: teamid, playerid);

/** 
 * <library>messages</library>
 * <summary>Adds a player to a group.</summary>
 * <param name="teamid">Team to add player to.</param>
 * <param name="playerid">Player to add.</param>
 * <returns>Error code 1 if team is invalid, error code 2 if player is not in any team, otherwise 0.</returns>
 */
forward Error: Team_RemoveMember(Team: teamid, playerid);

/** 
 * <library>messages</library>
 * <summary>Gets player team.</summary>
 * <param name="playerid">Player to check.</param>
 * <returns>Player's team.</returns>
 */
forward Team: Team_GetPlayer(playerid);

/** 
 * <library>messages</library>
 * <summary>Checks if rank is valid in a team.</summary>
 * <param name="teamid">Team.</param>
 * <param name="rankid">Rank to check for.</param>
 * <returns>True  if valid, otherwise false.</returns>
 */
forward bool: Team_IsValidRank(Team: teamid, TeamRank: rankid);

/** 
 * <library>messages</library>
 * <summary>Adds a rank to a team.</summary>
 * <param name="teamid">Team.</param>
 * <param name="name">Rank name.</param>
 * <returns>True if added, otherwise false.</returns>
 */
forward TeamRank: Team_AddRank(Team: teamid, const name[MAX_RANK_NAME]);

/** 
 * <library>messages</library>
 * <summary>Removes a rank from a team.</summary>
 * <param name="teamid">Team.</param>
 * <param name="rankid">Rank to remove.</param>
 * <returns>True if removed, otherwise false.</returns>
 */
forward bool: Team_RemoveRank(Team: teamid, TeamRank: rankid);

/** 
 * <library>messages</library>
 * <summary>Sets a rank to the player.</summary>
 * <param name="id">Rank id.</param>
 * <param name="playerid">Player to set rank to.</param>
 * <returns>True if set, otherwise false.</returns>
 */
forward bool: Team_SetPlayerRank(TeamRank: id, playerid);

/** 
 * <library>messages</library>
 * <summary>Gets the rank of a player.</summary>
 * <param name="playerid">Player to get rank from.</param>
 * <returns>Player's rank.</returns>
 */
forward TeamRank: Team_GetPlayerRank(playerid);

/** 
 * <library>messages</library>
 * <summary>Gets a rank name to a destination string.</summary>
 * <param name="teamid">Team.</param>
 * <param name="rankid">Rank we get name of.</param>
 * <param name="dest">Destination string.</param>
 * <returns>True if executed successfully, otherwise false (invalid team or rank).</returns>
 */
forward bool: Team_GetRankName(Team: teamid, TeamRank: rankid, dest[MAX_RANK_NAME], len=sizeof(dest));

// --
// Definitions 
// --
static 
    Iterator: Team_gsList<MAX_TEAMS>,
    Iterator: Team_gsPlayerList[Team: MAX_TEAMS]<MAX_PLAYERS>,
    Iterator: Team_gsRanks[Team: MAX_TEAMS]<MAX_RANKS>;

static 
    Rank_gsName[Team: MAX_TEAMS][TeamRank: MAX_RANKS][MAX_RANK_NAME],
    Team_gsName[Team: MAX_TEAMS][MAX_TEAM_NAME],
    Team_gsMaxMembers[Team: MAX_TEAMS],
    Team: Player_gsTeam[MAX_PLAYERS],
    TeamRank: Player_gsRank[MAX_PLAYERS],
    Channel: Team_gsChatGroup[Team: MAX_TEAMS];

// --
// API
// --

stock void: Team_Log(bool: set)
{
    Logger_ToggleDebug("__TEAMS_DBGHANDLE", set);
}

stock Team: Team_Create(const string: name[MAX_TEAM_NAME], maxmembers)
{
    new Team: idx = Team: Iter_Alloc(Team_gsList);
    if(_:idx == INVALID_ITERATOR_SLOT)
    {
        Logger_Dbg("__TEAMS_DBGHANDLE", "Invalid iterator slot");
        return Team: INVALID_TEAM;
    }
    Team_gsMaxMembers[idx] = maxmembers;
    strcopy(Team_gsName[idx], name);

    Team_gsChatGroup[idx] = ChatGroup_Create(name, maxmembers);
    if(!ChatGroup_IsValid(Team_gsChatGroup[idx]))
    {
        Logger_Dbg("__TEAMS_DBGHANDLE", "Couldn't create chat group", Logger_I("teamid", _:idx), Logger_S("channel", name), Logger_I("maxmembers", maxmembers), Logger_I("MAX_CHAT_GROUP_MEMBERS", MAX_CHAT_GROUP_MEMBERS));
    }
    return idx;
}
    
stock bool: Team_Delete(Team: teamid)
{
    if(!Iter_Contains(Team_gsList, _:teamid))
    {
        Logger_Dbg("__TEAMS_DBGHANDLE", "Team is invalid", Logger_I("teamid", _:teamid));
        return false;
    }
    Team_gsMaxMembers[teamid] = 0;
    Team_gsName[teamid][0] = EOS;
    ChatGroup_Delete(Team_gsChatGroup[teamid]);
    
    return true;
}

stock bool: Team_IsValid(Team: teamid)
{
    return (Iter_Contains(Team_gsList, _:teamid));
}

stock Error: Team_AddMember(Team: teamid, playerid)
{
    if(!Team_IsValid(teamid))
    {
        Logger_Dbg("__TEAMS_DBGHANDLE", "Team is invalid", Logger_I("teamid", _:teamid));
        return Error(1, "Invalid team.");
    }
    if(Player_gsTeam[playerid] != Team: INVALID_TEAM)
    {
        Logger_Dbg("__TEAMS_DBGHANDLE", "Player is already on some team", Logger_I("playerid", playerid));
        return Error(2, "Player is already in some team");
    }
    if(Iter_Count(Team_gsPlayerList[teamid]) == Team_gsMaxMembers[teamid])
    {
        return Ok(3);
    }
    Iter_Add(Team_gsPlayerList[teamid], playerid);
    Player_gsTeam[playerid] = teamid;
    ChatGroup_AddMember(Team_gsChatGroup[teamid], playerid);
    return Ok();
}

stock Error: Team_RemoveMember(Team: teamid, playerid)
{
    if(!Team_IsValid(teamid))
    {
        Logger_Dbg("__TEAMS_DBGHANDLE", "Team is invalid", Logger_I("teamid", _:teamid));
        return Error(1, "Invalid team.");
    }
    if(Player_gsTeam[playerid] != teamid)
    {
        Logger_Dbg("__TEAMS_DBGHANDLE", "Player is not in this team", Logger_I("playerid", playerid), Logger_I("teamid", _:teamid));
        return Error(2, "Player is not in this team.");
    }
    Iter_Remove(Team_gsPlayerList[teamid], playerid);
    Player_gsTeam[playerid] = Team: INVALID_TEAM;
    ChatGroup_RemoveMember(Team_gsChatGroup[teamid], playerid);
    return Ok();
}

stock Team: Team_GetPlayer(playerid)
{
    return Player_gsTeam[playerid];
}

stock bool: Team_IsValidRank(Team: teamid, TeamRank: rankid)
{
    return (Team_IsValid(teamid) && Iter_Contains(Team_gsRanks[teamid], _:rankid));
}

stock TeamRank: Team_AddRank(Team: teamid, const name[MAX_RANK_NAME])
{
    new TeamRank: idx = TeamRank: Iter_Alloc(Team_gsRanks[teamid]);
    if(_:idx == INVALID_ITERATOR_SLOT)
    {
        Logger_Dbg("__TEAMS_DBGHANDLE", "Invalid iterator slot");
        return TeamRank: INVALID_TEAM_RANK;
    }
    strcopy(Rank_gsName[teamid][idx], name);
    return idx;
}

stock bool: Team_RemoveRank(Team: teamid, TeamRank: rankid)
{
    if(!Team_IsValidRank(teamid, rankid))
    {
        Logger_Dbg("__TEAMS_DBGHANDLE", "Rank is invalid", Logger_I("rankid", _:rankid));
        return false;
    }
    foreach(new i : Team_gsPlayerList[teamid])
    {
        if(Team_GetPlayerRank(i) == rankid)
        {
            Player_gsRank[i] = TeamRank: INVALID_TEAM_RANK;
        }
    }
    Iter_Remove(Team_gsRanks[teamid], _:rankid);
    Rank_gsName[teamid][rankid][0] = EOS;
    return true;
}

stock bool: Team_SetPlayerRank(TeamRank: id, playerid)
{
    if(!Team_IsValidRank(Player_gsTeam[playerid], id) && id != TeamRank: INVALID_TEAM_RANK)
    {
        Logger_Dbg("__TEAMS_DBGHANDLE", "Rank is invalid", Logger_I("rankid", _:id));
        return false;
    }
    Player_gsRank[playerid] = id;
    return true;
}

stock TeamRank: Team_GetPlayerRank(playerid)
{
    return Player_gsRank[playerid];
}

stock bool: Team_GetRankName(Team: teamid, TeamRank: rankid, dest[MAX_RANK_NAME], len=sizeof(dest))
{
    if(!Team_IsValidRank(teamid, rankid))
    {
        return false;
    }
    strcopy(dest, Rank_gsName[teamid][rankid], len);
    return true;
}

stock bool: Team_SendMessage(color, Team: teamid, const string: message[144], OPEN_MP_TAGS: ...)
{
    if(!Team_IsValid(teamid))
    {
        return false;
    }
    ChatGroup_SendMessage(ChatGroup: teamid, color, message, ___(3));
    return true;
}

// --
// Hooks
// --

hook OnPlayerConnect(playerid)
{
    Player_gsTeam[playerid] = Team: INVALID_TEAM;
    return Y_HOOKS_CONTINUE_RETURN_1;
}

hook OnScriptInit()
{
    printf("[warboot-team-module] Version %s successfully loaded.", __TEAMS_VERSION__);
    return Y_HOOKS_CONTINUE_RETURN_1;
}
