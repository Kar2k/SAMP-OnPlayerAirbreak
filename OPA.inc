/*
	OnPlayerAirbreak(playerid);

	This include is one of the only accurate airbreak detecting
	methods developed in SA-MP.

	I do not guarantee positive results with this include.
	There could be many false flags.

	Created by Emmet on Wednesday, November 6, 2013.
	Updated by Kar (bug fixes). Last update: February 12th, 2016.
*/

#if defined ac_OnPlayerAirbreak
	#endinput
#endif

#define ac_OnPlayerAirbreak


#if !defined FILTERSCRIPT

// How many times should airbreak be detected before OnPlayerAirbreak is finally called?
#if !defined MAX_FLAGGED_DETECTIONS
	#define MAX_FLAGGED_DETECTIONS	3
#endif

// Maximum distance a player must travel in less than a second before being flagged for airbreak (onfoot).
#if !defined ONFOOT_DISTANCE
	#define ONFOOT_DISTANCE 75.0
#endif

// Maximum distance a player must travel in less than a second before being flagged for airbreak (driver).
#if !defined VEHICLE_DISTANCE
	#define VEHICLE_DISTANCE 50.0
#endif

static
	// Last known coordinates of the player.
	Float:s_AirbreakLastCoords[MAX_PLAYERS][3],

	// Timestamp used to store the next second in a timestamp.
	s_AirbreakUpdateTick[MAX_PLAYERS],

	// Timestamp containing the next time to check for airbreak.
	s_AirbreakDetectImmunity[MAX_PLAYERS],

	// Timestamp containing the last detection.
	s_AirbreakLastDetection[MAX_PLAYERS],

	// Number of detections in the last 60 seconds.
	s_AirbreakDetections[MAX_PLAYERS]
;

forward OnPlayerAirbreak(playerid);

static AB_IsVehicleMoving(vehicleid)
{
	new
		Float:x,
		Float:y,
		Float:z;

	GetVehicleVelocity(vehicleid, x, y, z);

	return (!(floatabs(x) <= 0.001 && floatabs(y) <= 0.001 && floatabs(z) <= 0.005));
}

static AB_OnAirbreakDetected(playerid)
{
	// Called when the player is presumably airbreaking.
	// If the amount of detections exceeds MAX_FLAGGED_DETECTIONS, they are most likely airbreaking.

	new
		timestamp = gettime();

	if((++ s_AirbreakDetections[playerid]) >= MAX_FLAGGED_DETECTIONS && (timestamp - s_AirbreakLastDetection[playerid]) < 60)
	{
		CallLocalFunction("OnPlayerAirbreak", "i", playerid);
	}
	s_AirbreakLastDetection[playerid] = timestamp;
}

public OnPlayerConnect(playerid)
{
	s_AirbreakDetections[playerid] = 0;
	s_AirbreakLastDetection[playerid] = 0;
	s_AirbreakDetectImmunity[playerid] = 0;
	s_AirbreakUpdateTick[playerid] = gettime();

	#if defined AB_OnPlayerConnect
		return AB_OnPlayerConnect(playerid);
	#else
		return 1;
	#endif
}

public OnPlayerSpawn(playerid)
{
	s_AirbreakDetectImmunity[playerid] = gettime() + 3;

	GetPlayerPos(playerid, s_AirbreakLastCoords[playerid][0], s_AirbreakLastCoords[playerid][1], s_AirbreakLastCoords[playerid][2]);

	#if defined AB_OnPlayerSpawn
		return AB_OnPlayerSpawn(playerid);
	#else
		return 1;
	#endif
}

public OnPlayerDeath(playerid, killerid, reason)
{
	s_AirbreakDetectImmunity[playerid] = gettime() + 3;

	#if defined AB_OnPlayerDeath
		return AB_OnPlayerDeath(playerid, killerid, reason);
	#else
		return 1;
	#endif
}

public OnPlayerUpdate(playerid)
{
	if(!IsPlayerNPC(playerid))
	{
		new
			vehicleid = GetPlayerVehicleID(playerid),
			timestamp = gettime(),
			Float:x,
			Float:y,
			Float:z,
			Float:distance;

		if(timestamp > s_AirbreakUpdateTick[playerid])
		{
			if(timestamp > s_AirbreakDetectImmunity[playerid] && GetPlayerSurfingVehicleID(playerid) == INVALID_VEHICLE_ID && GetPlayerSurfingObjectID(playerid) == INVALID_OBJECT_ID && GetPlayerSpecialAction(playerid) != SPECIAL_ACTION_ENTER_VEHICLE && GetPlayerSpecialAction(playerid) != SPECIAL_ACTION_EXIT_VEHICLE)
			{
				if(GetPlayerState(playerid) == PLAYER_STATE_ONFOOT)
				{
					distance = GetPlayerDistanceFromPoint(playerid, s_AirbreakLastCoords[playerid][0], s_AirbreakLastCoords[playerid][1], s_AirbreakLastCoords[playerid][2]);

					GetPlayerPos(playerid, x, y, z);

					if((floatabs(s_AirbreakLastCoords[playerid][2] - z) < 1.0 && floatabs(distance) >= ONFOOT_DISTANCE) && (floatabs(s_AirbreakLastCoords[playerid][1] - y) >= 50.0 || floatabs(s_AirbreakLastCoords[playerid][0] - x) >= 50.0))
					{
						AB_OnAirbreakDetected(playerid);
					}
				}
				else if(GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
				{
					distance = GetVehicleDistanceFromPoint(vehicleid, s_AirbreakLastCoords[playerid][0], s_AirbreakLastCoords[playerid][1], s_AirbreakLastCoords[playerid][2]);

					GetVehiclePos(vehicleid, x, y, z);

					if((!AB_IsVehicleMoving(vehicleid) && floatabs(distance) >= VEHICLE_DISTANCE) && (floatabs(s_AirbreakLastCoords[playerid][1] - y) >= 40.0 || floatabs(s_AirbreakLastCoords[playerid][0] - x) >= 40.0))
					{
						AB_OnAirbreakDetected(playerid);
					}
				}
			}

			if(GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
			{
				GetVehiclePos(vehicleid, s_AirbreakLastCoords[playerid][0], s_AirbreakLastCoords[playerid][1], s_AirbreakLastCoords[playerid][2]);
			}
			else
			{
				GetPlayerPos(playerid, s_AirbreakLastCoords[playerid][0], s_AirbreakLastCoords[playerid][1], s_AirbreakLastCoords[playerid][2]);
			}

			s_AirbreakUpdateTick[playerid] = timestamp;
		}
	}
	#if defined AB_OnPlayerUpdate
		return AB_OnPlayerUpdate(playerid);
	#else
		return 1;
	#endif
}

forward AB_SetSpawnInfo(playerid, team, skin, Float:x, Float:y, Float:z, Float:rotation, weapon1, weapon1_ammo, weapon2, weapon2_ammo, weapon3, weapon3_ammo);
public AB_SetSpawnInfo(playerid, team, skin, Float:x, Float:y, Float:z, Float:rotation, weapon1, weapon1_ammo, weapon2, weapon2_ammo, weapon3, weapon3_ammo)
{
	new
		ret = SetSpawnInfo(playerid, team, skin, x, y, z, rotation, weapon1, weapon1_ammo, weapon2, weapon2_ammo, weapon3, weapon3_ammo);

	if(ret)
	{
		switch(GetPlayerState(playerid))
		{
			case PLAYER_STATE_NONE, PLAYER_STATE_WASTED:
			{
				s_AirbreakDetectImmunity[playerid] = gettime() + 3;

				s_AirbreakLastCoords[playerid][0] = x;
				s_AirbreakLastCoords[playerid][1] = y;
				s_AirbreakLastCoords[playerid][2] = z;
			}
		}
	}

	return ret;
}

forward AB_SetPlayerPos(playerid, Float:x, Float:y, Float:z);
public AB_SetPlayerPos(playerid, Float:x, Float:y, Float:z)
{
	new
		ret = SetPlayerPos(playerid, x, y, z);

	if(ret)
	{
		s_AirbreakDetectImmunity[playerid] = gettime() + 3;

		s_AirbreakLastCoords[playerid][0] = x;
		s_AirbreakLastCoords[playerid][1] = y;
		s_AirbreakLastCoords[playerid][2] = z;
	}

	return ret;
}

forward AB_SetPlayerPosFindZ(playerid, Float:x, Float:y, Float:z);
public AB_SetPlayerPosFindZ(playerid, Float:x, Float:y, Float:z)
{
	new
		ret = SetPlayerPosFindZ(playerid, x, y, z);

	if(ret)
	{
		s_AirbreakDetectImmunity[playerid] = gettime() + 3;

		s_AirbreakLastCoords[playerid][0] = x;
		s_AirbreakLastCoords[playerid][1] = y;
		s_AirbreakLastCoords[playerid][2] = z;
	}

	return ret;
}

forward AB_PutPlayerInVehicle(playerid, vehicleid, seatid);
public AB_PutPlayerInVehicle(playerid, vehicleid, seatid)
{
	new
		ret = PutPlayerInVehicle(playerid, vehicleid, seatid);

	if(ret)
	{
		s_AirbreakDetectImmunity[playerid] = gettime() + 3;
	}

	return ret;
}

forward AB_SetVehiclePos(vehicleid, Float:x, Float:y, Float:z);
public AB_SetVehiclePos(vehicleid, Float:x, Float:y, Float:z)
{
	for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++)
	{
		if(!IsPlayerConnected(playerid)) continue;
		if(GetPlayerState(i) == PLAYER_STATE_DRIVER && IsPlayerInVehicle(i, vehicleid))
		{
			s_AirbreakDetectImmunity[i] = gettime() + 3;

			s_AirbreakLastCoords[i][0] = x;
			s_AirbreakLastCoords[i][1] = y;
			s_AirbreakLastCoords[i][2] = z;

			break;
		}
	}
	return SetVehiclePos(vehicleid, x, y, z);
}

#if defined _ALS_OnPlayerConnect
	#undef OnPlayerConnect
#else
	#define _ALS_OnPlayerConnect
#endif

#if defined _ALS_OnPlayerSpawn
	#undef OnPlayerSpawn
 #else
	#define _ALS_OnPlayerSpawn
#endif

#if defined _ALS_OnPlayerDeath
	#undef OnPlayerDeath
#else
	#define _ALS_OnPlayerDeath
#endif

#if defined _ALS_OnPlayerUpdate
	#undef OnPlayerUpdate
#else
	#define _ALS_OnPlayerUpdate
#endif

#if defined _ALS_SetSpawnInfo
	#undef SetSpawnInfo
#else
	#define _ALS_SetSpawnInfo
#endif

#if defined _ALS_SetPlayerPos
	#undef SetPlayerPos
#else
	#define _ALS_SetPlayerPos
#endif

#if defined _ALS_SetVehiclePos
	#undef SetVehiclePos
#else
	#define _ALS_SetVehiclePos
#endif

#if defined _ALS_SetPlayerPosFindZ
	#undef SetPlayerPosFindZ
#else
	#define _ALS_SetPlayerPosFindZ
#endif

#if defined _ALS_PutPlayerInVehicle
	#undef PutPlayerInVehicle
#else
	#define _ALS_PutPlayerInVehicle
#endif

#define OnPlayerConnect 	AB_OnPlayerConnect
#define OnPlayerSpawn 		AB_OnPlayerSpawn
#define OnPlayerDeath 		AB_OnPlayerDeath
#define OnPlayerUpdate 		AB_OnPlayerUpdate

#define SetSpawnInfo        AB_SetSpawnInfo
#define SetPlayerPos        AB_SetPlayerPos
#define SetPlayerPosFindZ   AB_SetPlayerPosFindZ
#define PutPlayerInVehicle  AB_PutPlayerInVehicle
#define SetVehiclePos       AB_SetVehiclePos

#if defined AB_OnPlayerConnect
	forward AB_OnPlayerConnect(playerid);
#endif

#if defined AB_OnPlayerSpawn
	forward AB_OnPlayerSpawn(playerid);
#endif

#if defined AB_OnPlayerUpdate
	forward AB_OnPlayerUpdate(playerid);
#endif

#if defined AB_OnPlayerDeath
	forward AB_OnPlayerDeath(playerid, killerid, reason);
#endif


#else //if it's a FS

stock AB_FPutPlayerInVehicle(playerid, vehicleid, seatid)
	return CallRemoteFunction("AB_PutPlayerInVehicle", "iii", playerid, vehicleid, seatid);

stock AB_FSetPlayerPosFindZ(playerid, Float:x, Float:y, Float:z)
	return CallRemoteFunction("AB_SetPlayerPosFindZ", "ifff", playerid, x, y, z);

stock AB_FSetVehiclePos(vehicleid, Float:x, Float:y, Float:z)
	return CallRemoteFunction("AB_SetVehiclePos", "ifff", vehicleid x, y, z);

stock AB_FSetPlayerPos(playerid, Float:x, Float:y, Float:z)
	return CallRemoteFunction("AB_SetPlayerPos", "ifff", playerid, x, y, z);

stock AB_FSetSpawnInfo(playerid, team, skin, Float:x, Float:y, Float:z, Float:rotation, weapon1, weapon1_ammo, weapon2, weapon2_ammo, weapon3, weapon3_ammo)
	return CallRemoteFunction("AB_SetSpawnInfo", "iiiffffiiiiii", playerid, team, skin, Float:x, Float:y, Float:z, Float:rotation, weapon1, weapon1_ammo, weapon2, weapon2_ammo, weapon3, weapon3_ammo);

#if defined _ALS_SetSpawnInfo
	#undef SetSpawnInfo
#else
	#define _ALS_SetSpawnInfo
#endif

#if defined _ALS_SetPlayerPos
	#undef SetPlayerPos
#else
	#define _ALS_SetPlayerPos
#endif

#if defined _ALS_SetVehiclePos
	#undef SetVehiclePos
#else
	#define _ALS_SetVehiclePos
#endif

#if defined _ALS_SetPlayerPosFindZ
	#undef SetPlayerPosFindZ
#else
	#define _ALS_SetPlayerPosFindZ
#endif

#if defined _ALS_PutPlayerInVehicle
	#undef PutPlayerInVehicle
#else
	#define _ALS_PutPlayerInVehicle
#endif

#define SetSpawnInfo        AB_FSetSpawnInfo
#define SetPlayerPos        AB_FSetPlayerPos
#define SetPlayerPosFindZ   AB_FSetPlayerPosFindZ
#define PutPlayerInVehicle  AB_FPutPlayerInVehicle
#define SetVehiclePos       AB_FSetVehiclePos

#endif
