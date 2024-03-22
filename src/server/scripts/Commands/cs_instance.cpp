/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

 /* ScriptData
 Name: instance_commandscript
 %Complete: 100
 Comment: All instance related commands
 Category: commandscripts
 EndScriptData */

#include "Chat.h"
#include "CommandScript.h"
#include "GameTime.h"
#include "Group.h"
#include "InstanceSaveMgr.h"
#include "InstanceScript.h"
#include "Language.h"
#include "MapMgr.h"
#include "ObjectAccessor.h"
#include "Player.h"

using namespace Acore::ChatCommands;

class instance_commandscript : public CommandScript
{
public:
    instance_commandscript() : CommandScript("instance_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable instanceCommandTable =
        {
            { "listbinds",    HandleInstanceListBindsCommand,    SEC_GAMEMASTER1,     Console::No },
            { "unbind",       HandleInstanceUnbindCommand,       SEC_GAMEMASTER2_L,    Console::No },
            { "stats",        HandleInstanceStatsCommand,        SEC_GAMEMASTER1,     Console::Yes },
            { "savedata",     HandleInstanceSaveDataCommand,     SEC_GAMEMASTER2_F, Console::No },
            { "setbossstate", HandleInstanceSetBossStateCommand, SEC_GAMEMASTER2_L,    Console::Yes },
            { "getbossstate", HandleInstanceGetBossStateCommand, SEC_GAMEMASTER1,     Console::Yes },
        };

        static ChatCommandTable commandTable =
        {
            { "instance", instanceCommandTable },
        };

        return commandTable;
    }

    static bool HandleInstanceListBindsCommand(ChatHandler* handler)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
            player = handler->GetSession()->GetPlayer();

        uint32 counter = 0;

        for (uint8 i = 0; i < MAX_DIFFICULTY; ++i)
        {
            for (auto const& [mapId, bind] : player->GetBoundInstances(Difficulty(i)))
            {
                InstanceSave* save = bind.save;
                std::string timeleft = secsToTimeString(save->GetResetTime() - time(nullptr));
                handler->PSendSysMessage(LANG_COMMAND_LIST_BIND_INFO, mapId, save->GetInstanceId(), bind.perm ? "yes" : "no", bind.extendState == EXTEND_STATE_EXPIRED ? "expired" : bind.extendState == EXTEND_STATE_EXTENDED ? "yes" : "no", save->GetDifficultyID(), save->CanReset() ? "yes" : "no", timeleft.c_str());
                counter++;

            }
        }
        handler->PSendSysMessage(LANG_COMMAND_LIST_BIND_PLAYER_BINDS, counter);

        counter = 0;
        if (Group* group = player->GetGroup())
        {
            for (uint8 i = 0; i < MAX_DIFFICULTY; ++i)
            {
                auto binds = group->GetBoundInstances(Difficulty(i));
                if (binds != group->GetBoundInstanceEnd())
                {
                    for (auto itr = binds->second.begin(); itr != binds->second.end(); ++itr)
                    {
                        InstanceSave* save = itr->second.save;
                        std::string timeleft = secsToTimeString(save->GetResetTime() - time(nullptr));
                        handler->PSendSysMessage(LANG_COMMAND_LIST_BIND_INFO, itr->first, save->GetInstanceId(), itr->second.perm ? "yes" : "no", "-", save->GetDifficultyID(), save->CanReset() ? "yes" : "no", timeleft.c_str());
                        counter++;
                    }
                }
            }
        }
        handler->PSendSysMessage(LANG_COMMAND_LIST_BIND_GROUP_BINDS, counter);

        return true;
    }

    static bool HandleInstanceUnbindCommand(ChatHandler* handler, Variant<uint16, EXACT_SEQUENCE("all")> mapArg, Optional<uint8> difficultyArg)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
            player = handler->GetSession()->GetPlayer();

        uint16 counter = 0;
        uint16 mapId = 0;

        if (mapArg.holds_alternative<uint16>())
        {
            mapId = mapArg.get<uint16>();
            if (!mapId)
                return false;
        }

        for (uint8 i = 0; i < MAX_DIFFICULTY; ++i)
        {
            Player::BoundInstancesMap& binds = player->GetBoundInstances(Difficulty(i));
            for (Player::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end();)
            {
                InstanceSave const* save = itr->second.save;
                if (itr->first != player->GetMapId() && (!mapId || mapId == itr->first) && (!difficultyArg || difficultyArg == save->GetDifficultyID()))
                {
                    std::string timeleft = secsToTimeString(save->GetResetTime() - time(nullptr));
                    handler->PSendSysMessage(LANG_COMMAND_INST_UNBIND_UNBINDING, itr->first, save->GetInstanceId(), itr->second.perm ? "yes" : "no", save->GetDifficultyID(), save->CanReset() ? "yes" : "no", timeleft.c_str());
                    player->UnbindInstance(itr, Difficulty(i));
                    counter++;
                }
                else
                    ++itr;

            }
        }

        handler->PSendSysMessage("instances unbound: %d", counter);

        return true;
    }

    static bool HandleInstanceStatsCommand(ChatHandler* handler)
    {
        uint32 dungeon = 0, battleground = 0, arena = 0, spectators = 0;
        sMapMgr->GetNumInstances(dungeon, battleground, arena);
        handler->PSendSysMessage("instances loaded: dungeons (%d), battlegrounds (%d), arenas (%d)", dungeon, battleground, arena);
        dungeon = 0;
        battleground = 0;
        arena = 0;
        spectators = 0;
        sMapMgr->GetNumPlayersInInstances(dungeon, battleground, arena, spectators);
        handler->SendErrorMessage("players in instances: dungeons (%d), battlegrounds (%d), arenas (%d + %d spect)", dungeon, battleground, arena, spectators);
        return false;
    }

    static bool HandleInstanceSaveDataCommand(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Map* map = player->GetMap();
        if (!map->IsDungeon())
        {
            handler->SendErrorMessage("Map is not a dungeon.");
            return false;
        }

        if (!map->ToInstanceMap()->GetInstanceScript())
        {
            handler->SendErrorMessage("Map has no instance data.");
            return false;
        }

        map->ToInstanceMap()->GetInstanceScript()->SaveToDB();

        return true;
    }

    static bool HandleInstanceSetBossStateCommand(ChatHandler* handler, uint32 encounterId, uint32 state, Optional<PlayerIdentifier> player)
    {
        // Character name must be provided when using this from console.
        if (!player && !handler->GetSession())
        {
            handler->SendErrorMessage(LANG_CMD_SYNTAX);
            return false;
        }

        if (!player)
            player = PlayerIdentifier::FromSelf(handler);

        if (!player->IsConnected())
        {
            handler->SendErrorMessage(LANG_PLAYER_NOT_FOUND);
            return false;
        }

        InstanceMap* map = player->GetConnectedPlayer()->GetMap()->ToInstanceMap();
        if (!map)
        {
            handler->SendErrorMessage(LANG_NOT_DUNGEON);
            return false;
        }

        if (!map->GetInstanceScript())
        {
            handler->SendErrorMessage(LANG_NO_INSTANCE_DATA);
            return false;
        }

        // Reject improper values.
        if (encounterId > map->GetInstanceScript()->GetEncounterCount())
        {
            handler->SendErrorMessage(LANG_BAD_VALUE);
            return false;
        }

        map->GetInstanceScript()->SetBossState(encounterId, EncounterState(state));
        std::string stateName = InstanceScript::GetBossStateName(state);
        handler->PSendSysMessage(LANG_COMMAND_INST_SET_BOSS_STATE, encounterId, state, stateName.c_str());
        return true;
    }

    static bool HandleInstanceGetBossStateCommand(ChatHandler* handler, uint32 encounterId, Optional<PlayerIdentifier> player)
    {
        // Character name must be provided when using this from console.
        if (!player && !handler->GetSession())
        {
            handler->SendErrorMessage(LANG_CMD_SYNTAX);
            return false;
        }

        if (!player)
            player = PlayerIdentifier::FromSelf(handler);

        if (!player->IsConnected())
        {
            handler->SendErrorMessage(LANG_PLAYER_NOT_FOUND);
            return false;
        }

        InstanceMap* map = player->GetConnectedPlayer()->GetMap()->ToInstanceMap();
        if (!map)
        {
            handler->SendErrorMessage(LANG_NOT_DUNGEON);
            return false;
        }

        if (!map->GetInstanceScript())
        {
            handler->SendErrorMessage(LANG_NO_INSTANCE_DATA);
            return false;
        }

        if (encounterId > map->GetInstanceScript()->GetEncounterCount())
        {
            handler->SendErrorMessage(LANG_BAD_VALUE);
            return false;
        }

        uint32 state = map->GetInstanceScript()->GetBossState(encounterId);
        std::string stateName = InstanceScript::GetBossStateName(state);
        handler->PSendSysMessage(LANG_COMMAND_INST_GET_BOSS_STATE, encounterId, state, stateName.c_str());
        return true;
    }
};

void AddSC_instance_commandscript()
{
    new instance_commandscript();
}
