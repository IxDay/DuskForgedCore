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

#include "NPCPackets.h"

void WorldPackets::NPC::Hello::Read()
{
    _worldPacket >> Unit;
}

WorldPacket const* WorldPackets::NPC::TrainerList::Write()
{
    _worldPacket << TrainerGUID;
    _worldPacket << int32(TrainerType);

    _worldPacket << int32(Spells.size());
    for (TrainerListSpell const& spell : Spells)
    {
        _worldPacket << int32(spell.SpellID);
        _worldPacket << uint8(spell.Usable);
        _worldPacket << int32(spell.MoneyCost);
        _worldPacket << int32(spell.ProfessionDialog);
        _worldPacket << int32(spell.ProfessionButton);
        _worldPacket << uint8(spell.ReqLevel);
        _worldPacket << int32(spell.ReqSkillLine);
        _worldPacket << int32(spell.ReqSkillRank);
        _worldPacket.append(spell.ReqAbility.data(), spell.ReqAbility.size());
    }

    _worldPacket << Greeting;

    return &_worldPacket;
}

void WorldPackets::NPC::TrainerBuySpell::Read()
{
    _worldPacket >> TrainerGUID;
    _worldPacket >> SpellID;
}

WorldPacket const* WorldPackets::NPC::TrainerBuyFailed::Write()
{
    _worldPacket << TrainerGUID;
    _worldPacket << int32(SpellID);
    _worldPacket << int32(TrainerFailedReason);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::NPC::TrainerBuySucceeded::Write()
{
    _worldPacket << TrainerGUID;
    _worldPacket << int32(SpellID);

    return &_worldPacket;
}