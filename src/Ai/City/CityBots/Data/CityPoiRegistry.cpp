#include "Ai/City/CityBots/Data/CityPoiRegistry.h"

#include "Ai/City/CityBots/Data/CityLocationRegistry.h"
#include "CbLog.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "QueryResult.h"
#include "Random.h"
#include "SharedDefines.h"

#include <algorithm>
#include <cmath>
#include <initializer_list>

namespace
{
    bool SameDistrict(std::string const& left, std::string const& right)
    {
        if (left == right)
            return true;

        auto isStormwindTrade = [](std::string const& district)
        {
            return district == "Trade" || district == "TradeDistrict";
        };

        auto isOrgrimmarStrength = [](std::string const& district)
        {
            return district == "OrgrimmarHub" ||
                   district == "Valley" ||
                   district == "ValleyOfStrength" ||
                   district == "ValleyInn" ||
                   district == "ValleyMarket";
        };

        if (isStormwindTrade(left) && isStormwindTrade(right))
            return true;

        if (isOrgrimmarStrength(left) && isOrgrimmarStrength(right))
            return true;

        auto sameAliasGroup = [](std::string const& a, std::string const& b,
                                 char const* alias, std::initializer_list<char const*> districts)
        {
            auto matches = [&](std::string const& district)
            {
                if (district == alias)
                    return true;
                for (char const* candidate : districts)
                    if (district == candidate)
                        return true;
                return false;
            };

            return matches(a) && matches(b);
        };

        return sameAliasGroup(left, right, "IronforgeHub", { "GreatForge", "Commons" }) ||
               sameAliasGroup(left, right, "UndercityHub", { "MagicQuarter" }) ||
               sameAliasGroup(left, right, "DarnassusHub", { "Craftsmen", "TradesmensTerrace" }) ||
               sameAliasGroup(left, right, "ExodarHub", { "Seat", "CrystalHall" }) ||
               sameAliasGroup(left, right, "ThunderBluffHub", { "ElderRise", "LowerRise" }) ||
               sameAliasGroup(left, right, "SilvermoonHub", { "Sunfury", "Bazaar" }) ||
               sameAliasGroup(left, right, "ShattrathHub", { "Terrace", "CentralTerrace" }) ||
               sameAliasGroup(left, right, "DalaranHub", { "MagusCommerce", "CentralLoop", "PurpleParlor", "Sewers" });
    }

    void AddPoi(std::vector<CityPoi>& pois, uint32 id, char const* locationKey, uint32 zoneId,
                CityPoiType type, uint32 mapId, float x, float y, float z, float o,
                char const* district, uint32 weight)
    {
        CityPoi poi;
        poi.id = id;
        poi.locationKey = locationKey;
        poi.zoneId = zoneId;
        poi.poiType = type;
        poi.mapId = mapId;
        poi.x = x;
        poi.y = y;
        poi.z = z;
        poi.orientation = o;
        poi.district = district;
        poi.weight = weight;
        pois.push_back(poi);
    }
}

CityPoiRegistry& CityPoiRegistry::Instance()
{
    static CityPoiRegistry inst;
    return inst;
}

void CityPoiRegistry::SeedFallbackData()
{
    if (!_pois.empty())
        return;

    AddPoi(_pois, 1, "Stormwind", 1519, CITY_POI_INN, 0, -8867.0f, 673.0f, 97.9f, 0.0f, "Trade", 100);
    AddPoi(_pois, 2, "Stormwind", 1519, CITY_POI_AUCTION_HOUSE, 0, -8813.0f, 663.0f, 95.0f, 0.0f, "Trade", 100);
    AddPoi(_pois, 3, "Stormwind", 1519, CITY_POI_MAILBOX, 0, -8815.0f, 667.0f, 95.0f, 0.0f, "Trade", 100);
    AddPoi(_pois, 4, "Stormwind", 1519, CITY_POI_TAVERN, 0, -8865.0f, 650.0f, 96.0f, 0.0f, "Trade", 100);
    AddPoi(_pois, 5, "Stormwind", 1519, CITY_POI_PLAZA, 0, -8835.0f, 625.0f, 94.0f, 0.0f, "Trade", 100);
    AddPoi(_pois, 6, "Stormwind", 1519, CITY_POI_TRAINER, 0, -8780.0f, 610.0f, 97.0f, 0.0f, "Trade", 100);
    AddPoi(_pois, 7, "Stormwind", 1519, CITY_POI_DOCK, 0, -8420.0f, 1320.0f, 5.0f, 0.0f, "Harbor", 100);
    AddPoi(_pois, 8, "Stormwind", 1519, CITY_POI_WALL_PATROL, 0, -8430.0f, 400.0f, 120.0f, 0.0f, "Walls", 100);
    AddPoi(_pois, 9, "Stormwind", 1519, CITY_POI_VENDOR, 0, -8795.0f, 640.0f, 97.0f, 0.0f, "Trade", 100);
    AddPoi(_pois, 10, "Stormwind", 1519, CITY_POI_PLAZA, 0, -8910.0f, 540.0f, 94.0f, 0.0f, "Cathedral", 80);
    AddPoi(_pois, 11, "Stormwind", 1519, CITY_POI_DOCK, 0, -8426.0f, 1325.0f, 5.0f, 0.0f, "Harbor", 100);
    AddPoi(_pois, 12, "Stormwind", 1519, CITY_POI_PLAZA, 0, -8752.0f, 780.0f, 97.0f, 0.0f, "OldTown", 90);
    AddPoi(_pois, 13, "Stormwind", 1519, CITY_POI_VENDOR, 0, -8727.0f, 647.0f, 99.0f, 0.0f, "OldTown", 80);
    AddPoi(_pois, 14, "Stormwind", 1519, CITY_POI_TRAINER, 0, -8705.0f, 812.0f, 97.0f, 0.0f, "OldTown", 80);
    AddPoi(_pois, 15, "Stormwind", 1519, CITY_POI_PLAZA, 0, -8392.0f, 687.0f, 95.0f, 0.0f, "DwarvenDistrict", 100);
    AddPoi(_pois, 16, "Stormwind", 1519, CITY_POI_VENDOR, 0, -8350.0f, 640.0f, 95.0f, 0.0f, "DwarvenDistrict", 80);
    AddPoi(_pois, 17, "Stormwind", 1519, CITY_POI_TRAINER, 0, -8425.0f, 610.0f, 95.0f, 0.0f, "DwarvenDistrict", 80);
    AddPoi(_pois, 18, "Stormwind", 1519, CITY_POI_PLAZA, 0, -9012.0f, 873.0f, 29.5f, 0.0f, "MageQuarter", 90);
    AddPoi(_pois, 19, "Stormwind", 1519, CITY_POI_TRAINER, 0, -9000.0f, 884.0f, 29.6f, 0.0f, "MageQuarter", 80);
    AddPoi(_pois, 20, "Stormwind", 1519, CITY_POI_VENDOR, 0, -8970.0f, 862.0f, 105.0f, 0.0f, "MageQuarter", 70);
    AddPoi(_pois, 21, "Stormwind", 1519, CITY_POI_PLAZA, 0, -8626.0f, 775.0f, 96.7f, 0.0f, "Canals", 90);
    AddPoi(_pois, 22, "Stormwind", 1519, CITY_POI_PLAZA, 0, -8685.0f, 944.0f, 97.0f, 0.0f, "CathedralSquare", 90);
    AddPoi(_pois, 23, "Stormwind", 1519, CITY_POI_TRAINER, 0, -8575.0f, 880.0f, 106.0f, 0.0f, "CathedralSquare", 80);
    AddPoi(_pois, 24, "Stormwind", 1519, CITY_POI_PLAZA, 0, -8981.78f, 397.36f, 75.9548f, 0.0f, "ValleyOfHeroes", 80);
    AddPoi(_pois, 25, "Stormwind", 1519, CITY_POI_PLAZA, 0, -8761.85f, 848.557f, 83.8052f, 0.0f, "Stockade", 70);
    AddPoi(_pois, 26, "Stormwind", 1519, CITY_POI_BANK, 0, -8918.0f, 624.0f, 99.5f, 0.0f, "Trade", 120);

    AddPoi(_pois, 101, "Orgrimmar", 1637, CITY_POI_INN, 1, 1634.0f, -4445.0f, 15.0f, 0.0f, "ValleyOfStrength", 100);
    AddPoi(_pois, 102, "Orgrimmar", 1637, CITY_POI_AUCTION_HOUSE, 1, 1679.0f, -4455.0f, 18.0f, 0.0f, "ValleyOfStrength", 100);
    AddPoi(_pois, 103, "Orgrimmar", 1637, CITY_POI_MAILBOX, 1, 1622.0f, -4390.0f, 10.0f, 0.0f, "ValleyOfStrength", 100);
    AddPoi(_pois, 104, "Orgrimmar", 1637, CITY_POI_TAVERN, 1, 1640.0f, -4430.0f, 15.0f, 0.0f, "ValleyOfStrength", 100);
    AddPoi(_pois, 105, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1600.0f, -4370.0f, 10.0f, 0.0f, "ValleyOfStrength", 100);
    AddPoi(_pois, 106, "Orgrimmar", 1637, CITY_POI_TRAINER, 1, 1720.0f, -4310.0f, 33.0f, 0.0f, "ValleyOfStrength", 100);
    AddPoi(_pois, 109, "Orgrimmar", 1637, CITY_POI_VENDOR, 1, 1660.0f, -4410.0f, 18.0f, 0.0f, "ValleyOfStrength", 100);
    AddPoi(_pois, 110, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1920.0f, -4130.0f, 40.0f, 0.0f, "Honor", 80);
    AddPoi(_pois, 111, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1565.0f, -4360.0f, 18.0f, 0.0f, "ValleyMarket", 100);
    AddPoi(_pois, 112, "Orgrimmar", 1637, CITY_POI_TAVERN, 1, 1615.0f, -4438.0f, 15.0f, 0.0f, "ValleyInn", 100);
    AddPoi(_pois, 113, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1705.0f, -4376.0f, 31.0f, 0.0f, "DragEntry", 100);
    AddPoi(_pois, 114, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1775.0f, -4285.0f, 8.0f, 0.0f, "Drag", 100);
    AddPoi(_pois, 115, "Orgrimmar", 1637, CITY_POI_VENDOR, 1, 1840.0f, -4395.0f, -15.0f, 0.0f, "Cleft", 100);
    AddPoi(_pois, 116, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1505.0f, -4415.0f, 22.0f, 0.0f, "GateWalk", 100);
    AddPoi(_pois, 117, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1470.0f, -4315.0f, 26.0f, 0.0f, "GateWalk", 100);
    AddPoi(_pois, 118, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1690.0f, -4270.0f, 45.0f, 0.0f, "Ridge", 80);
    AddPoi(_pois, 107, "Orgrimmar", 1637, CITY_POI_DOCK, 1, 1991.7499f, -4659.4850f, 27.375679f, 3.8521101f, "ValleyOfHonor", 100);
    AddPoi(_pois, 108, "Orgrimmar", 1637, CITY_POI_WALL_PATROL, 1, 1400.0f, -4400.0f, 55.0f, 0.0f, "Cleft", 100);
    AddPoi(_pois, 119, "Orgrimmar", 1637, CITY_POI_DOCK, 1, 1506.4592f, -4188.6763f, 41.288647f, 2.3166604f, "ValleyOfSpirits", 100);
    AddPoi(_pois, 120, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1485.0f, -4220.0f, 41.0f, 0.0f, "ValleyOfSpirits", 90);
    AddPoi(_pois, 121, "Orgrimmar", 1637, CITY_POI_VENDOR, 1, 1528.0f, -4190.0f, 42.0f, 0.0f, "ValleyOfSpirits", 80);
    AddPoi(_pois, 122, "Orgrimmar", 1637, CITY_POI_TRAINER, 1, 1450.0f, -4185.0f, 44.0f, 0.0f, "ValleyOfSpirits", 80);
    AddPoi(_pois, 123, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1900.0f, -4630.0f, 35.0f, 0.0f, "ValleyOfWisdom", 90);
    AddPoi(_pois, 124, "Orgrimmar", 1637, CITY_POI_TRAINER, 1, 1938.0f, -4675.0f, 32.0f, 0.0f, "ValleyOfWisdom", 80);
    AddPoi(_pois, 125, "Orgrimmar", 1637, CITY_POI_VENDOR, 1, 1855.0f, -4590.0f, 33.0f, 0.0f, "ValleyOfWisdom", 80);
    AddPoi(_pois, 126, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1772.0f, -4490.0f, 24.0f, 0.0f, "ValleyOfStrength", 100);
    AddPoi(_pois, 127, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1812.0f, -4345.0f, -12.0f, 0.0f, "CleftOfShadow", 80);
    AddPoi(_pois, 128, "Orgrimmar", 1637, CITY_POI_TRAINER, 1, 1835.0f, -4370.0f, -16.0f, 0.0f, "CleftOfShadow", 80);
    AddPoi(_pois, 129, "Orgrimmar", 1637, CITY_POI_VENDOR, 1, 1885.0f, -4280.0f, 24.0f, 0.0f, "TheDrag", 80);
    AddPoi(_pois, 130, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1990.0f, -4770.0f, 30.0f, 0.0f, "HallOfLegends", 70);
    AddPoi(_pois, 131, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1345.0f, -4660.0f, 28.0f, 0.0f, "WesternGate", 70);
    AddPoi(_pois, 132, "Orgrimmar", 1637, CITY_POI_DOCK, 1, 2090.0f, -4820.0f, 5.0f, 0.0f, "DurotarGate", 80);
    AddPoi(_pois, 133, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1580.0f, -4295.0f, 21.0f, 0.0f, "ValleyMarket", 100);
    AddPoi(_pois, 134, "Orgrimmar", 1637, CITY_POI_VENDOR, 1, 1715.0f, -4210.0f, 49.0f, 0.0f, "Ridge", 70);
    AddPoi(_pois, 135, "Orgrimmar", 1637, CITY_POI_PLAZA, 1, 1665.0f, -4240.0f, 49.0f, 0.0f, "Ridge", 70);
    AddPoi(_pois, 136, "Orgrimmar", 1637, CITY_POI_BANK, 1, 1627.0f, -4378.0f, 16.0f, 0.0f, "Valley", 120);

    AddPoi(_pois, 401, "Goldshire", 12, CITY_POI_INN, 0, -9469.0f, 62.0f, 56.0f, 0.0f, "LionsPride", 100);
    AddPoi(_pois, 402, "Goldshire", 12, CITY_POI_TAVERN, 0, -9466.0f, 58.0f, 56.0f, 0.0f, "LionsPride", 100);
    AddPoi(_pois, 403, "Goldshire", 12, CITY_POI_PLAZA, 0, -9458.0f, 68.0f, 56.0f, 0.0f, "Center", 100);
    AddPoi(_pois, 404, "Goldshire", 12, CITY_POI_INN_BED, 0, -9472.58f, 37.34f, 64.69f, 4.65f, "LionsPrideUp", 100);
    AddPoi(_pois, 405, "Goldshire", 12, CITY_POI_INN_BED, 0, -9474.35f, 35.85f, 64.69f, 4.65f, "LionsPrideUp", 80);
    AddPoi(_pois, 406, "Goldshire", 12, CITY_POI_INN_BED, 0, -9470.0f, 35.0f, 64.69f, 4.65f, "LionsPrideUp", 80);
    AddPoi(_pois, 407, "Goldshire", 12, CITY_POI_PLAZA, 0, -9462.0f, 72.0f, 56.0f, 0.0f, "Center", 100);
    AddPoi(_pois, 408, "Goldshire", 12, CITY_POI_DUEL_RING, 0, -9450.0f, 72.0f, 56.0f, 5.7f, "Center", 100);
    AddPoi(_pois, 409, "Goldshire", 12, CITY_POI_DUEL_RING, 0, -9469.0f, 78.0f, 56.0f, 3.8f, "Center", 100);
    AddPoi(_pois, 410, "Goldshire", 12, CITY_POI_PLAZA, 0, -9488.0f, 75.0f, 56.0f, 0.0f, "BlacksmithRoad", 100);
    AddPoi(_pois, 411, "Goldshire", 12, CITY_POI_PLAZA, 0, -9449.0f, 86.0f, 56.0f, 0.0f, "InnYard", 100);
    AddPoi(_pois, 412, "Goldshire", 12, CITY_POI_PLAZA, 0, -9498.0f, 38.0f, 56.0f, 0.0f, "WestRoad", 100);
    AddPoi(_pois, 413, "Goldshire", 12, CITY_POI_PLAZA, 0, -9435.0f, 60.0f, 56.0f, 0.0f, "EastRoad", 100);
    AddPoi(_pois, 414, "Goldshire", 12, CITY_POI_PLAZA, 0, -9474.0f, 42.0f, 64.69f, 4.65f, "UpstairsSpectator", 70);
    AddPoi(_pois, 415, "Goldshire", 12, CITY_POI_PLAZA, 0, -9468.0f, 31.0f, 64.69f, 4.65f, "UpstairsHall", 70);
    AddPoi(_pois, 416, "Goldshire", 12, CITY_POI_DOCK, 0, -9432.982f, 147.8207f, 56.400475f, 1.7737789f, "CrystalLake", 100);
    AddPoi(_pois, 417, "Goldshire", 12, CITY_POI_DOCK, 0, -9449.83f, 144.70833f, 56.253647f, 1.1336792f, "CrystalLake", 100);

    AddPoi(_pois, 501, "StormwindGate", 12, CITY_POI_DUEL_RING, 0, -9126.377f, 351.36536f, 93.62221f, 0.5799463f, "GateDuelRing", 100);
    AddPoi(_pois, 502, "StormwindGate", 12, CITY_POI_DUEL_RING, 0, -9108.679f, 316.851f, 93.39497f, 1.3260732f, "GateDuelRing", 100);
    AddPoi(_pois, 503, "StormwindGate", 12, CITY_POI_DUEL_RING, 0, -9129.0f, 348.8f, 93.62f, 0.5799463f, "GateDuelRing", 100);
    AddPoi(_pois, 504, "StormwindGate", 12, CITY_POI_DUEL_RING, 0, -9111.3f, 314.2f, 93.4f, 1.3260732f, "GateDuelRing", 100);
    AddPoi(_pois, 505, "StormwindGate", 12, CITY_POI_DUEL_RING, 0, -9123.8f, 348.9f, 93.62f, 0.5799463f, "GateDuelRing", 100);
    AddPoi(_pois, 506, "StormwindGate", 12, CITY_POI_DUEL_RING, 0, -9106.0f, 314.4f, 93.4f, 1.3260732f, "GateDuelRing", 100);

    AddPoi(_pois, 601, "OrgrimmarGate", 14, CITY_POI_DUEL_RING, 1, 1276.2615f, -4400.968f, 26.318865f, 0.40867978f, "GateDuelRing", 100);
    AddPoi(_pois, 602, "OrgrimmarGate", 14, CITY_POI_DUEL_RING, 1, 1283.0f, -4394.5f, 26.35f, 0.40867978f, "GateDuelRing", 100);
    AddPoi(_pois, 603, "OrgrimmarGate", 14, CITY_POI_DUEL_RING, 1, 1273.6f, -4403.2f, 26.32f, 0.40867978f, "GateDuelRing", 100);
    AddPoi(_pois, 604, "OrgrimmarGate", 14, CITY_POI_DUEL_RING, 1, 1280.5f, -4391.9f, 26.35f, 0.40867978f, "GateDuelRing", 100);
    AddPoi(_pois, 605, "OrgrimmarGate", 14, CITY_POI_DUEL_RING, 1, 1278.8f, -4403.0f, 26.32f, 0.40867978f, "GateDuelRing", 100);
    AddPoi(_pois, 606, "OrgrimmarGate", 14, CITY_POI_DUEL_RING, 1, 1285.6f, -4392.2f, 26.35f, 0.40867978f, "GateDuelRing", 100);

    AddPoi(_pois, 201, "Ironforge", 1537, CITY_POI_INN, 0, -4845.0f, -880.0f, 502.0f, 0.0f, "GreatForge", 100);
    AddPoi(_pois, 202, "Ironforge", 1537, CITY_POI_AUCTION_HOUSE, 0, -4960.0f, -910.0f, 503.0f, 0.0f, "GreatForge", 100);
    AddPoi(_pois, 203, "Ironforge", 1537, CITY_POI_TAVERN, 0, -4850.0f, -850.0f, 502.0f, 0.0f, "GreatForge", 100);
    AddPoi(_pois, 204, "Ironforge", 1537, CITY_POI_PLAZA, 0, -4920.0f, -990.0f, 501.0f, 0.0f, "MysticWard", 100);
    AddPoi(_pois, 205, "Ironforge", 1537, CITY_POI_DOCK, 0, -4638.0f, -1114.0f, 501.0f, 0.0f, "ForlornCavern", 100);
    AddPoi(_pois, 206, "Ironforge", 1537, CITY_POI_DOCK, 0, -4644.0f, -1119.0f, 501.0f, 0.0f, "ForlornCavern", 100);
    AddPoi(_pois, 207, "Ironforge", 1537, CITY_POI_PLAZA, 0, -4675.0f, -1230.0f, 501.0f, 0.0f, "HallOfExplorers", 80);
    AddPoi(_pois, 208, "Ironforge", 1537, CITY_POI_TRAINER, 0, -5010.0f, -1250.0f, 501.0f, 0.0f, "MilitaryWard", 80);
    AddPoi(_pois, 209, "Ironforge", 1537, CITY_POI_VENDOR, 0, -4925.0f, -955.0f, 501.0f, 0.0f, "Commons", 100);
    AddPoi(_pois, 210, "Ironforge", 1537, CITY_POI_PLAZA, 0, -4830.0f, -1260.0f, 501.0f, 0.0f, "TinkerTown", 90);
    AddPoi(_pois, 211, "Ironforge", 1537, CITY_POI_TRAINER, 0, -4650.0f, -960.0f, 501.0f, 0.0f, "MysticWard", 80);
    AddPoi(_pois, 212, "Ironforge", 1537, CITY_POI_PLAZA, 0, -4625.0f, -1100.0f, 501.0f, 0.0f, "ForlornCavern", 80);
    AddPoi(_pois, 213, "Ironforge", 1537, CITY_POI_PLAZA, 0, -4780.0f, -1000.0f, 502.0f, 0.0f, "GreatForge", 100);
    AddPoi(_pois, 214, "Ironforge", 1537, CITY_POI_PLAZA, 0, -5035.0f, -820.0f, 495.0f, 0.0f, "Gates", 70);
    AddPoi(_pois, 215, "Ironforge", 1537, CITY_POI_VENDOR, 0, -4865.0f, -1170.0f, 501.0f, 0.0f, "TinkerTown", 80);
    AddPoi(_pois, 216, "Ironforge", 1537, CITY_POI_BANK, 0, -4890.0f, -985.0f, 501.0f, 0.0f, "Commons", 120);
    AddPoi(_pois, 301, "Undercity", 1497, CITY_POI_INN, 0, 1633.0f, 240.0f, -43.0f, 0.0f, "MagicQuarter", 100);
    AddPoi(_pois, 302, "Undercity", 1497, CITY_POI_AUCTION_HOUSE, 0, 1600.0f, 170.0f, -56.0f, 0.0f, "MagicQuarter", 100);
    AddPoi(_pois, 303, "Undercity", 1497, CITY_POI_TAVERN, 0, 1630.0f, 200.0f, -43.0f, 0.0f, "MagicQuarter", 100);
    AddPoi(_pois, 304, "Undercity", 1497, CITY_POI_PLAZA, 0, 1580.0f, 260.0f, -43.0f, 0.0f, "MagicQuarter", 100);
    AddPoi(_pois, 305, "Undercity", 1497, CITY_POI_PLAZA, 0, 1590.0f, 330.0f, -43.0f, 0.0f, "TradeQuarter", 90);
    AddPoi(_pois, 306, "Undercity", 1497, CITY_POI_VENDOR, 0, 1560.0f, 430.0f, -43.0f, 0.0f, "RoguesQuarter", 80);
    AddPoi(_pois, 307, "Undercity", 1497, CITY_POI_TRAINER, 0, 1700.0f, 90.0f, -62.0f, 0.0f, "WarQuarter", 80);
    AddPoi(_pois, 308, "Undercity", 1497, CITY_POI_PLAZA, 0, 1460.0f, 170.0f, -62.0f, 0.0f, "Apothecarium", 80);
    AddPoi(_pois, 309, "Undercity", 1497, CITY_POI_VENDOR, 0, 1685.0f, 300.0f, -43.0f, 0.0f, "MagicQuarter", 80);
    AddPoi(_pois, 310, "Undercity", 1497, CITY_POI_PLAZA, 0, 1580.0f, 410.0f, -43.0f, 0.0f, "CanalRing", 80);
    AddPoi(_pois, 311, "Undercity", 1497, CITY_POI_TRAINER, 0, 1660.0f, 210.0f, -43.0f, 0.0f, "MagicQuarter", 80);
    AddPoi(_pois, 312, "Undercity", 1497, CITY_POI_PLAZA, 0, 1520.0f, 250.0f, -43.0f, 0.0f, "CentralRing", 90);
    AddPoi(_pois, 313, "Undercity", 1497, CITY_POI_PLAZA, 0, 1640.0f, 360.0f, -43.0f, 0.0f, "TradeQuarter", 80);
    AddPoi(_pois, 314, "Undercity", 1497, CITY_POI_VENDOR, 0, 1510.0f, 115.0f, -62.0f, 0.0f, "Apothecarium", 70);
    AddPoi(_pois, 315, "Undercity", 1497, CITY_POI_PLAZA, 0, 1715.0f, 245.0f, -43.0f, 0.0f, "InnerRing", 80);
    AddPoi(_pois, 316, "Undercity", 1497, CITY_POI_BANK, 0, 1595.0f, 230.0f, -43.0f, 0.0f, "MagicQuarter", 110);
    AddPoi(_pois, 701, "Darnassus", 1657, CITY_POI_INN, 1, 9947.0f, 2483.0f, 1316.0f, 0.0f, "Craftsmen", 100);
    AddPoi(_pois, 702, "Darnassus", 1657, CITY_POI_TAVERN, 1, 9940.0f, 2510.0f, 1316.0f, 0.0f, "Craftsmen", 100);
    AddPoi(_pois, 703, "Darnassus", 1657, CITY_POI_PLAZA, 1, 9920.0f, 2550.0f, 1316.0f, 0.0f, "Temple", 100);
    AddPoi(_pois, 704, "Darnassus", 1657, CITY_POI_AUCTION_HOUSE, 1, 9860.0f, 2330.0f, 1315.0f, 0.0f, "TradesmensTerrace", 100);
    AddPoi(_pois, 705, "Darnassus", 1657, CITY_POI_MAILBOX, 1, 9865.0f, 2355.0f, 1315.0f, 0.0f, "TradesmensTerrace", 90);
    AddPoi(_pois, 706, "Darnassus", 1657, CITY_POI_TRAINER, 1, 10040.0f, 2530.0f, 1316.0f, 0.0f, "WarriorsTerrace", 80);
    AddPoi(_pois, 707, "Darnassus", 1657, CITY_POI_VENDOR, 1, 10110.0f, 2570.0f, 1316.0f, 0.0f, "WarriorsTerrace", 80);
    AddPoi(_pois, 708, "Darnassus", 1657, CITY_POI_PLAZA, 1, 9650.0f, 2520.0f, 1336.0f, 0.0f, "TempleGardens", 80);
    AddPoi(_pois, 709, "Darnassus", 1657, CITY_POI_TRAINER, 1, 9705.0f, 2510.0f, 1335.0f, 0.0f, "TempleGardens", 80);
    AddPoi(_pois, 710, "Darnassus", 1657, CITY_POI_PLAZA, 1, 9790.0f, 2590.0f, 1316.0f, 0.0f, "Bridge", 70);
    AddPoi(_pois, 711, "Darnassus", 1657, CITY_POI_VENDOR, 1, 9910.0f, 2260.0f, 1328.0f, 0.0f, "CenarionEnclave", 80);
    AddPoi(_pois, 712, "Darnassus", 1657, CITY_POI_TRAINER, 1, 10080.0f, 2205.0f, 1328.0f, 0.0f, "CenarionEnclave", 80);
    AddPoi(_pois, 713, "Darnassus", 1657, CITY_POI_PLAZA, 1, 9945.0f, 2185.0f, 1328.0f, 0.0f, "CenarionEnclave", 80);
    AddPoi(_pois, 714, "Darnassus", 1657, CITY_POI_PLAZA, 1, 9750.0f, 2435.0f, 1316.0f, 0.0f, "Pool", 70);
    AddPoi(_pois, 715, "Darnassus", 1657, CITY_POI_VENDOR, 1, 10010.0f, 2425.0f, 1316.0f, 0.0f, "TerraceMarket", 80);
    AddPoi(_pois, 716, "Darnassus", 1657, CITY_POI_BANK, 1, 9865.0f, 2325.0f, 1315.0f, 0.0f, "TradesmensTerrace", 110);
    AddPoi(_pois, 801, "Exodar", 3557, CITY_POI_INN, 530, -3965.0f, -11653.0f, -139.0f, 0.0f, "Seat", 100);
    AddPoi(_pois, 802, "Exodar", 3557, CITY_POI_TAVERN, 530, -3950.0f, -11680.0f, -139.0f, 0.0f, "Seat", 100);
    AddPoi(_pois, 803, "Exodar", 3557, CITY_POI_PLAZA, 530, -4000.0f, -11800.0f, -100.0f, 0.0f, "Vault", 100);
    AddPoi(_pois, 804, "Exodar", 3557, CITY_POI_AUCTION_HOUSE, 530, -4020.0f, -11720.0f, -138.0f, 0.0f, "CrystalHall", 100);
    AddPoi(_pois, 805, "Exodar", 3557, CITY_POI_MAILBOX, 530, -4050.0f, -11710.0f, -138.0f, 0.0f, "CrystalHall", 90);
    AddPoi(_pois, 806, "Exodar", 3557, CITY_POI_TRAINER, 530, -3860.0f, -11695.0f, -137.0f, 0.0f, "TradersTier", 80);
    AddPoi(_pois, 807, "Exodar", 3557, CITY_POI_VENDOR, 530, -3905.0f, -11870.0f, -106.0f, 0.0f, "Vault", 80);
    AddPoi(_pois, 808, "Exodar", 3557, CITY_POI_PLAZA, 530, -4200.0f, -11580.0f, -130.0f, 0.0f, "SeatOfNaaru", 80);
    AddPoi(_pois, 809, "Exodar", 3557, CITY_POI_TRAINER, 530, -4215.0f, -11740.0f, -139.0f, 0.0f, "Trainers", 80);
    AddPoi(_pois, 810, "Exodar", 3557, CITY_POI_PLAZA, 530, -3840.0f, -11590.0f, -137.0f, 0.0f, "HallOfMystics", 80);
    AddPoi(_pois, 811, "Exodar", 3557, CITY_POI_VENDOR, 530, -4090.0f, -11620.0f, -137.0f, 0.0f, "CrystalHall", 80);
    AddPoi(_pois, 812, "Exodar", 3557, CITY_POI_PLAZA, 530, -3950.0f, -11570.0f, -137.0f, 0.0f, "SeatApproach", 80);
    AddPoi(_pois, 813, "Exodar", 3557, CITY_POI_TRAINER, 530, -4100.0f, -11830.0f, -106.0f, 0.0f, "Vault", 70);
    AddPoi(_pois, 814, "Exodar", 3557, CITY_POI_PLAZA, 530, -3875.0f, -11785.0f, -138.0f, 0.0f, "TradersTier", 80);
    AddPoi(_pois, 815, "Exodar", 3557, CITY_POI_VENDOR, 530, -4015.0f, -11505.0f, -137.0f, 0.0f, "SeatMarket", 80);
    AddPoi(_pois, 816, "Exodar", 3557, CITY_POI_BANK, 530, -3985.0f, -11730.0f, -138.0f, 0.0f, "CrystalHall", 110);
    AddPoi(_pois, 901, "ThunderBluff", 1638, CITY_POI_INN, 1, -1196.0f, 28.0f, 177.0f, 0.0f, "ElderRise", 100);
    AddPoi(_pois, 902, "ThunderBluff", 1638, CITY_POI_TAVERN, 1, -1210.0f, 50.0f, 177.0f, 0.0f, "ElderRise", 100);
    AddPoi(_pois, 903, "ThunderBluff", 1638, CITY_POI_PLAZA, 1, -1250.0f, 80.0f, 177.0f, 0.0f, "Pools", 100);
    AddPoi(_pois, 904, "ThunderBluff", 1638, CITY_POI_AUCTION_HOUSE, 1, -1265.0f, 125.0f, 132.0f, 0.0f, "LowerRise", 100);
    AddPoi(_pois, 905, "ThunderBluff", 1638, CITY_POI_MAILBOX, 1, -1278.0f, 118.0f, 132.0f, 0.0f, "LowerRise", 90);
    AddPoi(_pois, 906, "ThunderBluff", 1638, CITY_POI_TRAINER, 1, -1050.0f, 250.0f, 135.0f, 0.0f, "HunterRise", 80);
    AddPoi(_pois, 907, "ThunderBluff", 1638, CITY_POI_VENDOR, 1, -1010.0f, 210.0f, 134.0f, 0.0f, "HunterRise", 80);
    AddPoi(_pois, 908, "ThunderBluff", 1638, CITY_POI_PLAZA, 1, -1390.0f, -80.0f, 158.0f, 0.0f, "SpiritRise", 80);
    AddPoi(_pois, 909, "ThunderBluff", 1638, CITY_POI_TRAINER, 1, -1425.0f, -55.0f, 158.0f, 0.0f, "SpiritRise", 80);
    AddPoi(_pois, 910, "ThunderBluff", 1638, CITY_POI_PLAZA, 1, -1220.0f, -160.0f, 132.0f, 0.0f, "Bridge", 70);
    AddPoi(_pois, 911, "ThunderBluff", 1638, CITY_POI_VENDOR, 1, -1180.0f, 140.0f, 132.0f, 0.0f, "CentralRise", 80);
    AddPoi(_pois, 912, "ThunderBluff", 1638, CITY_POI_PLAZA, 1, -1295.0f, -10.0f, 132.0f, 0.0f, "CentralRise", 90);
    AddPoi(_pois, 913, "ThunderBluff", 1638, CITY_POI_TRAINER, 1, -1110.0f, -35.0f, 132.0f, 0.0f, "CentralRise", 80);
    AddPoi(_pois, 914, "ThunderBluff", 1638, CITY_POI_PLAZA, 1, -1090.0f, 95.0f, 132.0f, 0.0f, "Bridge", 70);
    AddPoi(_pois, 915, "ThunderBluff", 1638, CITY_POI_VENDOR, 1, -1330.0f, 170.0f, 132.0f, 0.0f, "Market", 80);
    AddPoi(_pois, 916, "ThunderBluff", 1638, CITY_POI_BANK, 1, -1255.0f, 125.0f, 132.0f, 0.0f, "LowerRise", 110);
    AddPoi(_pois, 1001, "Silvermoon", 3487, CITY_POI_INN, 530, 9480.0f, -7279.0f, 14.0f, 0.0f, "Sunfury", 100);
    AddPoi(_pois, 1002, "Silvermoon", 3487, CITY_POI_TAVERN, 530, 9460.0f, -7300.0f, 14.0f, 0.0f, "Sunfury", 100);
    AddPoi(_pois, 1003, "Silvermoon", 3487, CITY_POI_PLAZA, 530, 9500.0f, -7200.0f, 14.0f, 0.0f, "Court", 100);
    AddPoi(_pois, 1004, "Silvermoon", 3487, CITY_POI_AUCTION_HOUSE, 530, 9680.0f, -7500.0f, 15.0f, 0.0f, "Bazaar", 100);
    AddPoi(_pois, 1005, "Silvermoon", 3487, CITY_POI_MAILBOX, 530, 9650.0f, -7475.0f, 15.0f, 0.0f, "Bazaar", 90);
    AddPoi(_pois, 1006, "Silvermoon", 3487, CITY_POI_TRAINER, 530, 9605.0f, -7380.0f, 14.0f, 0.0f, "RoyalExchange", 80);
    AddPoi(_pois, 1007, "Silvermoon", 3487, CITY_POI_VENDOR, 530, 9580.0f, -7210.0f, 14.0f, 0.0f, "Court", 80);
    AddPoi(_pois, 1008, "Silvermoon", 3487, CITY_POI_PLAZA, 530, 9850.0f, -7285.0f, 14.0f, 0.0f, "MurderRow", 80);
    AddPoi(_pois, 1009, "Silvermoon", 3487, CITY_POI_TRAINER, 530, 9800.0f, -7335.0f, 14.0f, 0.0f, "MurderRow", 80);
    AddPoi(_pois, 1010, "Silvermoon", 3487, CITY_POI_PLAZA, 530, 9525.0f, -6860.0f, 16.0f, 0.0f, "WalkOfElders", 80);
    AddPoi(_pois, 1011, "Silvermoon", 3487, CITY_POI_VENDOR, 530, 9410.0f, -7100.0f, 15.0f, 0.0f, "ShepherdsGate", 70);
    AddPoi(_pois, 1012, "Silvermoon", 3487, CITY_POI_PLAZA, 530, 9750.0f, -7050.0f, 15.0f, 0.0f, "RoyalExchange", 90);
    AddPoi(_pois, 1013, "Silvermoon", 3487, CITY_POI_TRAINER, 530, 9900.0f, -7420.0f, 14.0f, 0.0f, "FarstridersSquare", 80);
    AddPoi(_pois, 1014, "Silvermoon", 3487, CITY_POI_PLAZA, 530, 9680.0f, -7150.0f, 14.0f, 0.0f, "Promenade", 80);
    AddPoi(_pois, 1015, "Silvermoon", 3487, CITY_POI_VENDOR, 530, 9550.0f, -7440.0f, 14.0f, 0.0f, "Bazaar", 80);
    AddPoi(_pois, 1016, "Silvermoon", 3487, CITY_POI_BANK, 530, 9640.0f, -7490.0f, 15.0f, 0.0f, "Bazaar", 110);
    AddPoi(_pois, 1101, "Shattrath", 3703, CITY_POI_INN, 530, -1838.0f, 5301.0f, -12.0f, 0.0f, "Aldor", 100);
    AddPoi(_pois, 1102, "Shattrath", 3703, CITY_POI_TAVERN, 530, -1900.0f, 5450.0f, -12.0f, 0.0f, "Scryer", 100);
    AddPoi(_pois, 1103, "Shattrath", 3703, CITY_POI_PLAZA, 530, -1850.0f, 5400.0f, -12.0f, 0.0f, "Terrace", 100);
    AddPoi(_pois, 1104, "Shattrath", 3703, CITY_POI_MAILBOX, 530, -1865.0f, 5410.0f, -12.0f, 0.0f, "Terrace", 90);
    AddPoi(_pois, 1105, "Shattrath", 3703, CITY_POI_VENDOR, 530, -1710.0f, 5490.0f, -12.0f, 0.0f, "LowerCity", 90);
    AddPoi(_pois, 1106, "Shattrath", 3703, CITY_POI_PLAZA, 530, -1600.0f, 5260.0f, -39.0f, 0.0f, "LowerCity", 80);
    AddPoi(_pois, 1107, "Shattrath", 3703, CITY_POI_TRAINER, 530, -2000.0f, 5350.0f, -8.0f, 0.0f, "ScryerRise", 80);
    AddPoi(_pois, 1108, "Shattrath", 3703, CITY_POI_PLAZA, 530, -2100.0f, 5295.0f, -8.0f, 0.0f, "ScryerRise", 80);
    AddPoi(_pois, 1109, "Shattrath", 3703, CITY_POI_TRAINER, 530, -1745.0f, 5760.0f, 130.0f, 0.0f, "AldorRise", 80);
    AddPoi(_pois, 1110, "Shattrath", 3703, CITY_POI_PLAZA, 530, -1800.0f, 5830.0f, 130.0f, 0.0f, "AldorRise", 80);
    AddPoi(_pois, 1111, "Shattrath", 3703, CITY_POI_VENDOR, 530, -1885.0f, 5200.0f, -12.0f, 0.0f, "Terrace", 80);
    AddPoi(_pois, 1112, "Shattrath", 3703, CITY_POI_PLAZA, 530, -1980.0f, 5560.0f, -12.0f, 0.0f, "Bridge", 70);
    AddPoi(_pois, 1113, "Shattrath", 3703, CITY_POI_PLAZA, 530, -1685.0f, 5350.0f, -12.0f, 0.0f, "LowerCityMarket", 80);
    AddPoi(_pois, 1114, "Shattrath", 3703, CITY_POI_VENDOR, 530, -1815.0f, 5515.0f, -12.0f, 0.0f, "TerraceMarket", 80);
    AddPoi(_pois, 1115, "Shattrath", 3703, CITY_POI_PLAZA, 530, -1915.0f, 5310.0f, -12.0f, 0.0f, "CentralTerrace", 90);
    AddPoi(_pois, 1116, "Shattrath", 3703, CITY_POI_BANK, 530, -1855.0f, 5425.0f, -12.0f, 0.0f, "Terrace", 100);
    AddPoi(_pois, 1201, "Dalaran", 4395, CITY_POI_INN, 571, 5807.0f, 588.0f, 661.0f, 0.0f, "Sewers", 100);
    AddPoi(_pois, 1202, "Dalaran", 4395, CITY_POI_TAVERN, 571, 5850.0f, 650.0f, 647.0f, 0.0f, "PurpleParlor", 100);
    AddPoi(_pois, 1203, "Dalaran", 4395, CITY_POI_PLAZA, 571, 5750.0f, 700.0f, 655.0f, 0.0f, "Krasus", 100);
    AddPoi(_pois, 1204, "Dalaran", 4395, CITY_POI_AUCTION_HOUSE, 571, 5890.0f, 620.0f, 650.0f, 0.0f, "MagusCommerce", 100);
    AddPoi(_pois, 1205, "Dalaran", 4395, CITY_POI_MAILBOX, 571, 5880.0f, 642.0f, 650.0f, 0.0f, "MagusCommerce", 90);
    AddPoi(_pois, 1206, "Dalaran", 4395, CITY_POI_VENDOR, 571, 5820.0f, 480.0f, 658.0f, 0.0f, "Runeweaver", 80);
    AddPoi(_pois, 1207, "Dalaran", 4395, CITY_POI_TRAINER, 571, 5775.0f, 620.0f, 650.0f, 0.0f, "Krasus", 80);
    AddPoi(_pois, 1208, "Dalaran", 4395, CITY_POI_PLAZA, 571, 5710.0f, 520.0f, 652.0f, 0.0f, "AllianceQuarter", 80);
    AddPoi(_pois, 1209, "Dalaran", 4395, CITY_POI_PLAZA, 571, 5905.0f, 470.0f, 652.0f, 0.0f, "HordeQuarter", 80);
    AddPoi(_pois, 1210, "Dalaran", 4395, CITY_POI_TRAINER, 571, 5810.0f, 760.0f, 640.0f, 0.0f, "VioletCitadel", 80);
    AddPoi(_pois, 1211, "Dalaran", 4395, CITY_POI_VENDOR, 571, 5730.0f, 770.0f, 640.0f, 0.0f, "VioletCitadel", 80);
    AddPoi(_pois, 1212, "Dalaran", 4395, CITY_POI_PLAZA, 571, 5680.0f, 650.0f, 647.0f, 0.0f, "Eventide", 80);
    AddPoi(_pois, 1213, "Dalaran", 4395, CITY_POI_PLAZA, 571, 5780.0f, 830.0f, 680.0f, 0.0f, "Landing", 70);
    AddPoi(_pois, 1214, "Dalaran", 4395, CITY_POI_VENDOR, 571, 5855.0f, 735.0f, 640.0f, 0.0f, "Market", 80);
    AddPoi(_pois, 1215, "Dalaran", 4395, CITY_POI_PLAZA, 571, 5810.0f, 690.0f, 647.0f, 0.0f, "CentralLoop", 90);
    AddPoi(_pois, 1216, "Dalaran", 4395, CITY_POI_BANK, 571, 5885.0f, 608.0f, 650.0f, 0.0f, "MagusCommerce", 110);

    _capitalZoneIds = { 1519, 1537, 1657, 3557, 1637, 1497, 1638, 3487, 3703, 4395, 12, 14 };
}

void CityPoiRegistry::LoadFromDatabase()
{
    _pois.clear();
    _capitalZoneIds.clear();

    QueryResult result = WorldDatabase.Query(
        "SELECT id, location_key, zone_id, poi_type, map_id, pos_x, pos_y, pos_z, "
        "orientation, district, weight FROM city_bot_poi ORDER BY id");

    if (!result)
    {
        CbLog::Warn("city_bot_poi missing; using fallback POIs");
        SeedFallbackData();
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        CityPoi poi;
        poi.id = fields[0].Get<uint32>();
        poi.locationKey = fields[1].Get<std::string>();
        poi.zoneId = fields[2].Get<uint32>();
        poi.poiType = static_cast<CityPoiType>(fields[3].Get<uint8>());
        poi.mapId = fields[4].Get<uint16>();
        poi.x = fields[5].Get<float>();
        poi.y = fields[6].Get<float>();
        poi.z = fields[7].Get<float>();
        poi.orientation = fields[8].Get<float>();
        poi.district = fields[9].Get<std::string>();
        poi.weight = fields[10].Get<uint32>();
        _pois.push_back(poi);

        if (std::find(_capitalZoneIds.begin(), _capitalZoneIds.end(), poi.zoneId) == _capitalZoneIds.end())
            _capitalZoneIds.push_back(poi.zoneId);
    } while (result->NextRow());

    if (_pois.empty())
        SeedFallbackData();

    CbLog::Info("loaded {} city POIs", _pois.size());
}

bool CityPoiRegistry::IsCitizenZone(uint32 zoneId) const
{
    return CityLocationRegistry::Instance().IsCitizenAllowedZone(zoneId) ||
           std::find(_capitalZoneIds.begin(), _capitalZoneIds.end(), zoneId) != _capitalZoneIds.end();
}

CityPoi const* CityPoiRegistry::GetPoi(uint32 poiId) const
{
    for (CityPoi const& poi : _pois)
        if (poi.id == poiId)
            return &poi;
    return nullptr;
}

bool CityPoiRegistry::LowestPoiZWithin(std::string const& locationKey, float x, float y,
                                       float radius, float* outMinZ) const
{
    bool any = false;
    float minZ = 0.0f;
    float const r2 = radius * radius;
    for (CityPoi const& poi : _pois)
    {
        if (poi.locationKey != locationKey)
            continue;
        float const dx = poi.x - x;
        float const dy = poi.y - y;
        if (dx * dx + dy * dy > r2)
            continue;
        if (!any || poi.z < minZ)
        {
            minZ = poi.z;
            any = true;
        }
    }
    if (any && outMinZ)
        *outMinZ = minZ;
    return any;
}

CityPoi const* CityPoiRegistry::PickPoi(std::string const& locationKey, CityPoiType type,
                                          uint32 excludePoiId,
                                          std::string const& excludeDistrict) const
{
    std::vector<uint32> weighted;
    for (CityPoi const& poi : _pois)
    {
        if (poi.locationKey != locationKey || poi.poiType != type || poi.id == excludePoiId || !poi.weight)
            continue;
        if (!excludeDistrict.empty() && SameDistrict(poi.district, excludeDistrict))
            continue;
        for (uint32 i = 0; i < poi.weight; ++i)
            weighted.push_back(poi.id);
    }

    if (weighted.empty())
        return nullptr;

    return GetPoi(weighted[urand(0, weighted.size() - 1)]);
}

CityPoi const* CityPoiRegistry::PickPoi(uint32 zoneId, CityPoiType type, uint32 excludePoiId) const
{
    std::vector<uint32> weighted;
    for (CityPoi const& poi : _pois)
    {
        if (poi.zoneId != zoneId || poi.poiType != type || poi.id == excludePoiId || !poi.weight)
            continue;
        for (uint32 i = 0; i < poi.weight; ++i)
            weighted.push_back(poi.id);
    }

    if (weighted.empty())
        return nullptr;

    return GetPoi(weighted[urand(0, weighted.size() - 1)]);
}

CityPoi const* CityPoiRegistry::PickRoamPoi(std::string const& locationKey, uint32 seed,
                                             uint32 excludePoiId, bool includeHubs,
                                             std::string const& excludeDistrict) const
{
    uint32 totalWeight = 0;
    for (CityPoi const& poi : _pois)
    {
        if (poi.locationKey != locationKey || poi.id == excludePoiId || !poi.weight ||
            poi.poiType == CITY_POI_DUEL_RING || poi.poiType == CITY_POI_INN_BED)
            continue;

        if (!excludeDistrict.empty() && SameDistrict(poi.district, excludeDistrict))
            continue;

        if (!includeHubs && (poi.poiType == CITY_POI_INN ||
                             poi.poiType == CITY_POI_AUCTION_HOUSE ||
                             poi.poiType == CITY_POI_BANK))
            continue;

        totalWeight += poi.weight;
    }

    if (!totalWeight)
        return nullptr;

    uint32 roll = seed % totalWeight;
    for (CityPoi const& poi : _pois)
    {
        if (poi.locationKey != locationKey || poi.id == excludePoiId || !poi.weight ||
            poi.poiType == CITY_POI_DUEL_RING || poi.poiType == CITY_POI_INN_BED)
            continue;

        if (!excludeDistrict.empty() && SameDistrict(poi.district, excludeDistrict))
            continue;

        if (!includeHubs && (poi.poiType == CITY_POI_INN ||
                             poi.poiType == CITY_POI_AUCTION_HOUSE ||
                             poi.poiType == CITY_POI_BANK))
            continue;

        if (roll < poi.weight)
            return &poi;

        roll -= poi.weight;
    }

    return nullptr;
}

WorldPosition CityPoiRegistry::GetWorldPosition(CityPoi const& poi) const
{
    return WorldPosition(poi.mapId, poi.x, poi.y, poi.z, poi.orientation);
}
