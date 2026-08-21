#include "CitizenRosterRegistry.h"

#include "CbLog.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "QueryResult.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <vector>

CitizenRosterRegistry& CitizenRosterRegistry::Instance()
{
    static CitizenRosterRegistry inst;
    return inst;
}

namespace
{
    struct AssignmentDef
    {
        char const* key;
        char const* homeKey;
        CitizenRole role;
        bool burstOnly;
        std::vector<uint32> poiIds;
    };

    std::vector<uint32> ParseGuidList(std::string value)
    {
        for (char& ch : value)
        {
            if (ch == ',' || ch == ';' || ch == '{' || ch == '}' ||
                ch == '[' || ch == ']' || std::isspace(static_cast<unsigned char>(ch)))
                ch = ' ';
        }

        std::vector<uint32> out;
        std::size_t pos = 0;
        while (pos < value.size())
        {
            while (pos < value.size() && value[pos] == ' ')
                ++pos;

            if (pos >= value.size())
                break;

            std::size_t end = pos;
            while (end < value.size() && value[end] != ' ')
                ++end;

            std::string token = value.substr(pos, end - pos);
            try
            {
                uint32 guid = static_cast<uint32>(std::stoul(token));
                if (guid)
                    out.push_back(guid);
            }
            catch (...)
            {
                CbLog::Warn("ignoring invalid GUID '{}' in city bot assignment list", token);
            }

            pos = end;
        }

        return out;
    }

    uint32 PickPoi(AssignmentDef const& def, std::size_t index)
    {
        if (def.poiIds.empty())
            return 0;

        return def.poiIds[index % def.poiIds.size()];
    }

    bool IsHordeRace(uint8 race)
    {
        return race == 2 || race == 5 || race == 6 || race == 8 || race == 10;
    }

    // 'a' = Alliance-only home, 'h' = Horde-only, 0 = mixed (Shattrath/Dalaran).
    char HomeFactionTag(char const* homeKey)
    {
        static char const* const alliance[] = { "Stormwind", "StormwindGate", "Goldshire",
                                                "Ironforge", "Darnassus", "Exodar" };
        static char const* const horde[] = { "Orgrimmar", "OrgrimmarGate", "Undercity",
                                             "ThunderBluff", "Silvermoon" };
        for (char const* key : alliance)
            if (!strcmp(homeKey, key))
                return 'a';
        for (char const* key : horde)
            if (!strcmp(homeKey, key))
                return 'h';
        return 0;
    }

    void ApplyConfigAssignments(std::vector<CitizenRosterEntry>& entries)
    {
        static std::vector<AssignmentDef> const assignments = {
            { "Assign.StormwindGate.Duelers", "StormwindGate", CITIZEN_ROLE_GATE_DUELIST, false,
              { 501, 502, 503, 504, 505, 506 } },
            { "Assign.OrgrimmarGate.Duelers", "OrgrimmarGate", CITIZEN_ROLE_GATE_DUELIST, false,
              { 601, 602, 603, 604, 605, 606 } },

            { "Assign.Goldshire.InnDancers", "Goldshire", CITIZEN_ROLE_INN_DANCER, false,
              { 404, 405 } },
            { "Assign.Goldshire.Duelers", "Goldshire", CITIZEN_ROLE_GOLDSHIRE_DUELIST, false,
              { 408, 409 } },
            { "Assign.Goldshire.Talkers", "Goldshire", CITIZEN_ROLE_PLAZA_TALKER, false,
              { 403, 407, 410, 411, 412, 413, 414, 415 } },
            { "Assign.Goldshire.Fishers", "Goldshire", CITIZEN_ROLE_FISHER, false,
              { 416, 417 } },
            { "Assign.Goldshire.FishersBurst", "Goldshire", CITIZEN_ROLE_FISHER, true,
              { 416, 417 } },
            { "Assign.Goldshire.Ambient", "Goldshire", CITIZEN_ROLE_FLEX, false,
              { 401, 402, 403, 407, 410, 411, 412, 413, 414, 415 } },
            { "Assign.Goldshire.AmbientBurst", "Goldshire", CITIZEN_ROLE_FLEX, true,
              { 401, 402, 403, 407, 410, 411, 412, 413, 414, 415 } },

            { "Assign.Stormwind.Fishers", "Stormwind", CITIZEN_ROLE_FISHER, false, { 7, 11 } },
            { "Assign.Stormwind.FishersBurst", "Stormwind", CITIZEN_ROLE_FISHER, true, { 7, 11 } },
            { "Assign.Stormwind.Ambient", "Stormwind", CITIZEN_ROLE_FLEX, false,
              { 1, 2, 3, 4, 5, 6, 8, 9, 10 } },
            { "Assign.Stormwind.AmbientBurst", "Stormwind", CITIZEN_ROLE_FLEX, true,
              { 1, 2, 3, 4, 5, 6, 8, 9, 10 } },
            { "Assign.Stormwind.CrowdBurst", "Stormwind", CITIZEN_ROLE_CROWD, true,
              { 26, 2 } },

            { "Assign.Ironforge.Fishers", "Ironforge", CITIZEN_ROLE_FISHER, false, { 205, 206 } },
            { "Assign.Ironforge.FishersBurst", "Ironforge", CITIZEN_ROLE_FISHER, true, { 205, 206 } },
            { "Assign.Ironforge.Ambient", "Ironforge", CITIZEN_ROLE_FLEX, false,
              { 201, 202, 203, 204 } },
            { "Assign.Ironforge.AmbientBurst", "Ironforge", CITIZEN_ROLE_FLEX, true,
              { 201, 202, 203, 204 } },

            { "Assign.Darnassus.Ambient", "Darnassus", CITIZEN_ROLE_FLEX, false,
              { 701, 702, 703 } },
            { "Assign.Darnassus.AmbientBurst", "Darnassus", CITIZEN_ROLE_FLEX, true,
              { 701, 702, 703 } },

            { "Assign.Exodar.Ambient", "Exodar", CITIZEN_ROLE_FLEX, false,
              { 801, 802, 803 } },
            { "Assign.Exodar.AmbientBurst", "Exodar", CITIZEN_ROLE_FLEX, true,
              { 801, 802, 803 } },

            { "Assign.Orgrimmar.Fishers", "Orgrimmar", CITIZEN_ROLE_FISHER, false, { 107, 119 } },
            { "Assign.Orgrimmar.FishersBurst", "Orgrimmar", CITIZEN_ROLE_FISHER, true, { 107, 119 } },
            { "Assign.Orgrimmar.Ambient", "Orgrimmar", CITIZEN_ROLE_FLEX, false,
              { 101, 102, 103, 104, 105, 106, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118 } },
            { "Assign.Orgrimmar.AmbientBurst", "Orgrimmar", CITIZEN_ROLE_FLEX, true,
              { 101, 102, 103, 104, 105, 106, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118 } },
            { "Assign.Orgrimmar.CrowdBurst", "Orgrimmar", CITIZEN_ROLE_CROWD, true,
              { 136, 102 } },

            { "Assign.Undercity.Ambient", "Undercity", CITIZEN_ROLE_FLEX, false,
              { 301, 302, 303, 304 } },
            { "Assign.Undercity.AmbientBurst", "Undercity", CITIZEN_ROLE_FLEX, true,
              { 301, 302, 303, 304 } },

            { "Assign.ThunderBluff.Ambient", "ThunderBluff", CITIZEN_ROLE_FLEX, false,
              { 901, 902, 903 } },
            { "Assign.ThunderBluff.AmbientBurst", "ThunderBluff", CITIZEN_ROLE_FLEX, true,
              { 901, 902, 903 } },

            { "Assign.Silvermoon.Ambient", "Silvermoon", CITIZEN_ROLE_FLEX, false,
              { 1001, 1002, 1003 } },
            { "Assign.Silvermoon.AmbientBurst", "Silvermoon", CITIZEN_ROLE_FLEX, true,
              { 1001, 1002, 1003 } },

            { "Assign.Shattrath.Ambient", "Shattrath", CITIZEN_ROLE_FLEX, false,
              { 1101, 1102, 1103 } },
            { "Assign.Shattrath.AmbientBurst", "Shattrath", CITIZEN_ROLE_FLEX, true,
              { 1101, 1102, 1103 } },

            { "Assign.Dalaran.Ambient", "Dalaran", CITIZEN_ROLE_FLEX, false,
              { 1201, 1202, 1203 } },
            { "Assign.Dalaran.AmbientBurst", "Dalaran", CITIZEN_ROLE_FLEX, true,
              { 1201, 1202, 1203 } },
        };

        std::unordered_map<uint32, CitizenRosterEntry*> byGuid;
        byGuid.reserve(entries.size());
        for (CitizenRosterEntry& entry : entries)
            byGuid[entry.guid] = &entry;

        std::unordered_set<uint32> assigned;
        bool anyAssignments = false;
        int32 priority = 0;
        uint32 applied = 0;

        for (AssignmentDef const& def : assignments)
        {
            std::string const fullKey = std::string("CitizenBots.") + def.key;
            std::vector<uint32> guids =
                ParseGuidList(sConfigMgr->GetOption<std::string>(fullKey, "", false));
            if (guids.empty())
                continue;

            anyAssignments = true;
            for (std::size_t i = 0; i < guids.size(); ++i)
            {
                uint32 const guid = guids[i];
                auto entryIt = byGuid.find(guid);
                if (entryIt == byGuid.end())
                {
                    CbLog::Warn("{} references unknown city bot GUID {}", fullKey, guid);
                    continue;
                }

                // A conf list can name any roster guid, including one whose race
                // belongs to the OTHER faction (found live 2026-08-16: four
                // Alliance Shattrath citizens padded into OrgrimmarGate.Duelers
                // -> open faction PvP at the duel hub). Refuse the assignment;
                // the citizen keeps its roster identity instead.
                if (char const tag = HomeFactionTag(def.homeKey))
                {
                    bool const horde = IsHordeRace(entryIt->second->race);
                    if ((horde && tag == 'a') || (!horde && tag == 'h'))
                    {
                        CbLog::Warn("{} assigns {} (guid {}, race {}) to {} -- wrong faction, "
                                    "skipping assignment",
                                    fullKey, entryIt->second->name, guid,
                                    entryIt->second->race, def.homeKey);
                        continue;
                    }
                }

                if (!assigned.insert(guid).second)
                {
                    CbLog::Warn("{} references GUID {} more than once; keeping first assignment",
                                fullKey, guid);
                    continue;
                }

                CitizenRosterEntry& entry = *entryIt->second;
                entry.homeKey = def.homeKey;
                entry.role = def.role;
                entry.poiId = PickPoi(def, i);
                entry.loginPriority = priority++;
                entry.enabled = true;
                entry.burstOnly = def.burstOnly;
                entry.assignmentConfigured = true;
                ++applied;
            }
        }

        if (!anyAssignments)
            return;

        bool const requireListed =
            sConfigMgr->GetOption<bool>("CitizenBots.Assign.RequireListed", true, false);
        if (requireListed)
        {
            entries.erase(std::remove_if(entries.begin(), entries.end(),
                                         [](CitizenRosterEntry const& entry)
                                         {
                                             return !entry.assignmentConfigured;
                                         }),
                          entries.end());
        }

        uint32 staticCount = 0;
        uint32 burstCount = 0;
        for (CitizenRosterEntry const& entry : entries)
        {
            if (entry.burstOnly)
                ++burstCount;
            else
                ++staticCount;
        }

        CbLog::Info("applied {} configured city bot assignments ({} static, {} burst, requireListed={})",
                    applied, staticCount, burstCount, requireListed ? 1 : 0);
    }
}

void CitizenRosterRegistry::LoadFromDatabase()
{
    _entries.clear();
    _loaded = false;

    QueryResult result = PlayerbotsDatabase.Query(
        "SELECT guid, account_id, character_name, race, class, gender, level, "
        "home_key, role, poi_id, login_priority, enabled "
        "FROM citizen_roster ORDER BY login_priority, guid");

    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        CitizenRosterEntry entry;
        entry.guid = fields[0].Get<uint32>();
        entry.accountId = fields[1].Get<uint32>();
        entry.name = fields[2].Get<std::string>();
        entry.race = fields[3].Get<uint8>();
        entry.cls = fields[4].Get<uint8>();
        entry.gender = fields[5].Get<uint8>();
        entry.level = fields[6].Get<uint8>();
        entry.homeKey = fields[7].Get<std::string>();
        entry.role = static_cast<CitizenRole>(fields[8].Get<uint8>());
        entry.poiId = fields[9].Get<uint32>();
        entry.loginPriority = fields[10].Get<int32>();
        entry.enabled = fields[11].Get<uint8>() != 0;
        if (entry.enabled)
            _entries.push_back(std::move(entry));
    } while (result->NextRow());

    ApplyConfigAssignments(_entries);

    _loaded = !_entries.empty();
}

CitizenRosterEntry const* CitizenRosterRegistry::FindByGuid(uint32 guid) const
{
    for (CitizenRosterEntry const& entry : _entries)
        if (entry.guid == guid)
            return &entry;
    return nullptr;
}

CitizenRosterEntry const* CitizenRosterRegistry::FindByName(std::string const& name) const
{
    for (CitizenRosterEntry const& entry : _entries)
        if (entry.name == name)
            return &entry;
    return nullptr;
}

std::vector<CitizenRosterEntry const*> CitizenRosterRegistry::ByHome(char const* homeKey) const
{
    std::vector<CitizenRosterEntry const*> out;
    if (!homeKey)
        return out;

    for (CitizenRosterEntry const& entry : _entries)
        if (entry.homeKey == homeKey)
            out.push_back(&entry);
    return out;
}

std::vector<CitizenRosterEntry const*> CitizenRosterRegistry::ByRole(CitizenRole role) const
{
    std::vector<CitizenRosterEntry const*> out;
    for (CitizenRosterEntry const& entry : _entries)
        if (entry.role == role)
            out.push_back(&entry);
    return out;
}

std::vector<CitizenRosterEntry const*> CitizenRosterRegistry::LoginOrder() const
{
    std::vector<CitizenRosterEntry const*> out;
    out.reserve(_entries.size());
    for (CitizenRosterEntry const& entry : _entries)
        out.push_back(&entry);

    std::sort(out.begin(), out.end(),
              [](CitizenRosterEntry const* a, CitizenRosterEntry const* b)
              {
                  if (a->loginPriority != b->loginPriority)
                      return a->loginPriority < b->loginPriority;
                  return a->guid < b->guid;
              });
    return out;
}

uint32 CitizenRosterRegistry::CountRosterAtHome(char const* homeKey) const
{
    uint32 count = 0;
    for (CitizenRosterEntry const& entry : _entries)
        if (entry.homeKey == homeKey)
            ++count;
    return count;
}

uint32 CitizenRosterRegistry::CountStaticRosterAtHome(char const* homeKey) const
{
    uint32 count = 0;
    for (CitizenRosterEntry const& entry : _entries)
        if (entry.homeKey == homeKey && !entry.burstOnly)
            ++count;
    return count;
}

bool CitizenRosterRegistry::IsRosterGuid(uint32 guid) const
{
    return FindByGuid(guid) != nullptr;
}
