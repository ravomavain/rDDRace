/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_DDRACECOMMANDS_H
#define GAME_SERVER_DDRACECOMMANDS_H
#undef GAME_SERVER_DDRACECOMMANDS_H // this file can be included several times

#ifndef CONSOLE_COMMAND
#define CONSOLE_COMMAND(name, params, flags, callback, userdata, help)
#endif

CONSOLE_COMMAND("kill_pl", "v", CFGFLAG_SERVER, ConKillPlayer, this, "Kills player v and announces the kill")
CONSOLE_COMMAND("tele", "i", CFGFLAG_SERVER|CMDFLAG_TEST, ConTeleport, this, "Teleports you to player i")
CONSOLE_COMMAND("addweapon", "i", CFGFLAG_SERVER|CMDFLAG_TEST, ConAddWeapon, this, "Gives weapon with id i to you (all = -1, hammer = 0, gun = 1, shotgun = 2, grenade = 3, rifle = 4, ninja = 5)")
CONSOLE_COMMAND("removeweapon", "i", CFGFLAG_SERVER|CMDFLAG_TEST, ConRemoveWeapon, this, "removes weapon with id i from you (all = -1, hammer = 0, gun = 1, shotgun = 2, grenade = 3, rifle = 4)")
CONSOLE_COMMAND("shotgun", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConShotgun, this, "Gives a shotgun to you")
CONSOLE_COMMAND("grenade", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConGrenade, this, "Gives a grenade launcher to you")
CONSOLE_COMMAND("rifle", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConRifle, this, "Gives a rifle to you")
CONSOLE_COMMAND("weapons", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConWeapons, this, "Gives all weapons to you")
CONSOLE_COMMAND("unshotgun", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConUnShotgun, this, "Takes the shotgun from you")
CONSOLE_COMMAND("ungrenade", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConUnGrenade, this, "Takes the grenade launcher you")
CONSOLE_COMMAND("unrifle", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConUnRifle, this, "Takes the rifle from you")
CONSOLE_COMMAND("unweapons", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConUnWeapons, this, "Takes all weapons from you")
CONSOLE_COMMAND("ninja", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConNinja, this, "Makes you a ninja")
CONSOLE_COMMAND("super", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConSuper, this, "Makes you super")
CONSOLE_COMMAND("unsuper", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConUnSuper, this, "Removes super from you")
CONSOLE_COMMAND("left", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConGoLeft, this, "Makes you move 1 tile left")
CONSOLE_COMMAND("right", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConGoRight, this, "Makes you move 1 tile right")
CONSOLE_COMMAND("up", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConGoUp, this, "Makes you move 1 tile up")
CONSOLE_COMMAND("down", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConGoDown, this, "Makes you move 1 tile down")
CONSOLE_COMMAND("move", "ii", CFGFLAG_SERVER|CMDFLAG_TEST, ConMove, this, "Moves to the tile with x/y-number ii")
CONSOLE_COMMAND("move_raw", "ii", CFGFLAG_SERVER|CMDFLAG_TEST, ConMoveRaw, this, "Moves to the point with x/y-coordinates ii")
CONSOLE_COMMAND("credits", "", CFGFLAG_SERVER, ConCredits, this, "Shows the credits of the DDRace mod")
CONSOLE_COMMAND("emote", "?si", CFGFLAG_SERVER, ConEyeEmote, this, "Sets your tee's eye emote")
CONSOLE_COMMAND("broadmsg", "", CFGFLAG_SERVER, ConToggleBroadcast, this, "Toggles showing the server's broadcast message during race on/off")
CONSOLE_COMMAND("eyeemote", "", CFGFLAG_SERVER, ConEyeEmote, this, "Toggles use of standard eye-emotes on/off")
CONSOLE_COMMAND("settings", "?s", CFGFLAG_SERVER, ConSettings, this, "Shows gameplay information for this server")
CONSOLE_COMMAND("help", "?r", CFGFLAG_SERVER, ConHelp, this, "Shows help to command r, general help if left blank")
CONSOLE_COMMAND("info", "", CFGFLAG_SERVER, ConInfo, this, "Shows info about this server")
CONSOLE_COMMAND("me", "r", CFGFLAG_SERVER, ConMe, this, "Like the famous irc command '/me says hi' will display '<yourname> says hi'")
CONSOLE_COMMAND("pause", "", CFGFLAG_SERVER, ConTogglePause, this, "Toggles pause on/off (if activated on server)")
CONSOLE_COMMAND("force_pause", "vi", CFGFLAG_SERVER, ConForcePause, this, "Force v to pause for i seconds")
CONSOLE_COMMAND("force_unpause", "v", CFGFLAG_SERVER, ConForcePause, this, "Set force-pause timer of v to 0.")
CONSOLE_COMMAND("rank", "?r", CFGFLAG_SERVER, ConRank, this, "Shows the rank of player with name r (your rank by default)")
CONSOLE_COMMAND("rules", "", CFGFLAG_SERVER, ConRules, this, "Shows the server rules")
CONSOLE_COMMAND("team", "?i", CFGFLAG_SERVER, ConJoinTeam, this, "Lets you join team i (shows your team if left blank)")
CONSOLE_COMMAND("top5", "?i", CFGFLAG_SERVER, ConTop5, this, "Shows five ranks of the ladder beginning with rank i (1 by default)")
CONSOLE_COMMAND("showothers", "?i", CFGFLAG_SERVER, ConShowOthers, this, "Whether to showplayers from other teams or not (off by default), optional i = 0 for off else for on")

CONSOLE_COMMAND("mute", "", CFGFLAG_SERVER, ConMute, this, "");
CONSOLE_COMMAND("muteid", "vi", CFGFLAG_SERVER, ConMuteID, this, "");
CONSOLE_COMMAND("muteip", "si", CFGFLAG_SERVER, ConMuteIP, this, "");
CONSOLE_COMMAND("unmute", "v", CFGFLAG_SERVER, ConUnmute, this, "");
CONSOLE_COMMAND("mutes", "", CFGFLAG_SERVER, ConMutes, this, "");

CONSOLE_COMMAND("rescue", "", CFGFLAG_SERVER, ConRescue, this, "");
CONSOLE_COMMAND("r", "", CFGFLAG_SERVER, ConRescue, this, "");

#if defined(CONF_SQL)
CONSOLE_COMMAND("times", "?s?i", CFGFLAG_SERVER, ConTimes, this, "/times ?s?i shows last 5 times of the server or of a player beginning with name s starting with time i (i = 1 by default)")
#endif
#undef CONSOLE_COMMAND

#endif
