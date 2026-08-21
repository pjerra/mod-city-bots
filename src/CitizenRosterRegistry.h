#ifndef MOD_CITYBOTS_CITIZENROSTERREGISTRY_H
#define MOD_CITYBOTS_CITIZENROSTERREGISTRY_H

#include "Ai/City/CityBots/CitizenInfo.h"

#include <string>
#include <vector>

struct CitizenRosterEntry
{
    uint32 guid{0};
    uint32 accountId{0};
    std::string name;
    uint8  race{0};
    uint8  cls{0};
    uint8  gender{0};
    uint8  level{1};
    std::string homeKey;
    CitizenRole role{CITIZEN_ROLE_FLEX};
    uint32 poiId{0};
    int32  loginPriority{100};
    bool   enabled{true};
    bool   burstOnly{false};
    bool   assignmentConfigured{false};
};

class CitizenRosterRegistry
{
public:
    static CitizenRosterRegistry& Instance();

    void LoadFromDatabase();
    bool IsLoaded() const { return _loaded; }

    std::vector<CitizenRosterEntry> const& All() const { return _entries; }

    CitizenRosterEntry const* FindByGuid(uint32 guid) const;
    CitizenRosterEntry const* FindByName(std::string const& name) const;

    std::vector<CitizenRosterEntry const*> ByHome(char const* homeKey) const;
    std::vector<CitizenRosterEntry const*> ByRole(CitizenRole role) const;
    std::vector<CitizenRosterEntry const*> LoginOrder() const;

    uint32 CountRosterAtHome(char const* homeKey) const;
    uint32 CountStaticRosterAtHome(char const* homeKey) const;
    bool IsRosterGuid(uint32 guid) const;

private:
    CitizenRosterRegistry() = default;

    bool _loaded{false};
    std::vector<CitizenRosterEntry> _entries;
};

#endif
