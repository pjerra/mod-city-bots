#ifndef MOD_CITYBOTS_CROWDDIALOGUES_H
#define MOD_CITYBOTS_CROWDDIALOGUES_H

// Crowd-scene dialogue data (living-crowd design), shared between the
// action-layer CitizenCrowdAction and the CbStrategyGate direct-drive port so
// the two paths can never drift apart on content.

#include "Ai/City/CityBots/CitizenInfo.h"

namespace CrowdDialogues
{
    struct CrowdDialogue
    {
        CityPoiType pool;           // CITY_POI_BANK, CITY_POI_AUCTION_HOUSE, or CITY_POI_PLAZA (= generic)
        char const* lines[3];       // alternating: initiator, partner, initiator
        uint8 lineCount;            // 2 or 3
    };

    inline CrowdDialogue const kCrowdDialogues[] =
    {
        { CITY_POI_BANK, { "This queue gets longer every day.", "Tell me about it. I just need one deposit box.", nullptr }, 2 },
        { CITY_POI_BANK, { "Guild vault's full again.", "Stop hoarding shoulder armor then.", "It's a collection!" }, 3 },
        { CITY_POI_BANK, { "They should open a second counter.", "They've been saying that since the war ended.", nullptr }, 2 },
        { CITY_POI_BANK, { "Forgot my vault key again.", "They can open it from their side, you know.", nullptr }, 2 },
        { CITY_POI_BANK, { "I swear this line was shorter yesterday.", "Everything is slower on auction day.", nullptr }, 2 },
        { CITY_POI_BANK, { "Half my vault is old holiday clothes.", "Sell them. Someone always buys.", "Never. They are memories." }, 3 },
        { CITY_POI_BANK, { "The goblins charge less, I hear.", "And count your coins twice when you leave.", nullptr }, 2 },
        { CITY_POI_BANK, { "Thirty slots and every one of them full.", "Buy another tab then.", "With what gold? The tab money is in the vault." }, 3 },
        { CITY_POI_BANK, { "You ever count it all? Just to be sure?", "Every week. The tellers hate me.", nullptr }, 2 },
        { CITY_POI_BANK, { "Someone tried to deposit a live chicken earlier.", "Again? That is the third time this month.", nullptr }, 2 },
        { CITY_POI_BANK, { "Interest rates. Now there is a scam.", "Better than keeping it under the mattress.", nullptr }, 2 },
        { CITY_POI_BANK, { "My guild wants a vault tab just for fish.", "That is oddly specific.", "You have not met our cook." }, 3 },
        { CITY_POI_BANK, { "I only came in to break a gold piece.", "And the line still took half the morning.", nullptr }, 2 },
        { CITY_POI_BANK, { "They say the vaults go three floors down.", "Deeper. My cousin cleans them.", nullptr }, 2 },
        { CITY_POI_BANK, { "Lost my deposit slip somewhere.", "The teller remembers everyone. You will be fine.", nullptr }, 2 },
        { CITY_POI_BANK, { "One day I will have enough for that epic mount.", "Keep saving. The vault does not spend itself.", nullptr }, 2 },
        { CITY_POI_AUCTION_HOUSE, { "Saronite prices are madness this week.", "Buy now. They only go up before raid night.", nullptr }, 2 },
        { CITY_POI_AUCTION_HOUSE, { "Someone undercut me by one copper. One!", "That's just how the game is played, friend.", "Then I'll undercut by two." }, 3 },
        { CITY_POI_AUCTION_HOUSE, { "Any good deals today?", "Glyphs are cheap. Everything else, forget it.", nullptr }, 2 },
        { CITY_POI_AUCTION_HOUSE, { "Netherweave is finally moving again.", "Tailors stocking up before the weekend.", nullptr }, 2 },
        { CITY_POI_AUCTION_HOUSE, { "I posted forty stacks of ore last night.", "And undercut every one of us doing it.", "Business is business." }, 3 },
        { CITY_POI_AUCTION_HOUSE, { "Enchanting mats are robbery right now.", "Roll a gatherer like the rest of us.", nullptr }, 2 },
        { CITY_POI_AUCTION_HOUSE, { "The auctioneer called my bid insulting.", "Was it?", "Deeply. That is the point." }, 3 },
        { CITY_POI_AUCTION_HOUSE, { "Glyph market crashed again.", "Scribes flooding it every reset.", nullptr }, 2 },
        { CITY_POI_AUCTION_HOUSE, { "Somebody bought my sword in under a minute.", "Then you priced it too low, friend.", nullptr }, 2 },
        { CITY_POI_AUCTION_HOUSE, { "Frostweave never stays listed long.", "Bandages. The war eats them by the crate.", nullptr }, 2 },
        { CITY_POI_AUCTION_HOUSE, { "I bid on a pet for my daughter.", "Which one?", "The little dragon. She will love it." }, 3 },
        { CITY_POI_AUCTION_HOUSE, { "Deposit fees are eating my profits.", "Stop relisting every hour then.", nullptr }, 2 },
        { CITY_POI_AUCTION_HOUSE, { "Eternal fire is up again.", "Blacksmiths before raid night. Every week the same.", nullptr }, 2 },
        { CITY_POI_AUCTION_HOUSE, { "That trinket has been listed for a month.", "At that price it will sit another month.", nullptr }, 2 },
        { CITY_POI_AUCTION_HOUSE, { "I only ever win the auctions I regret.", "That is the auction house for you.", nullptr }, 2 },
        { CITY_POI_AUCTION_HOUSE, { "Titansteel is a seller's market.", "Everything is, if you are patient.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "Weather's turning.", "Aye. Good day to stay in the city.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "You hear about the trouble up north?", "Everyone has. Glad it's someone else's problem.", "For now." }, 3 },
        { CITY_POI_PLAZA, { "The guards changed shift early today.", "Slow morning. Even the pickpockets slept in.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "Fresh bread smell from the district again.", "Bakers are up before the sun. Bless them.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "My boots are done for.", "The cobbler by the canal does good work.", "That is where these came from." }, 3 },
        { CITY_POI_PLAZA, { "Heard the fishing is good off the docks.", "It is always good. Catching is the hard part.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "Another airship went over this morning.", "Northrend bound. Always Northrend these days.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "You look tired, friend.", "New baby at home. I sleep standing up now.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "The tavern raised its prices again.", "War tax, they call it. Thirst tax, I call it.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "Rain coming in tonight, I would wager.", "My knee agrees with you.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "Have you tried the new stew at the inn?", "Twice. Do not ask what is in it.", "Now I have to try it." }, 3 },
        { CITY_POI_PLAZA, { "My cousin joined the Argent Crusade.", "Brave lad. Cold posting, though.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "Mail coach was late again today.", "Everything from Northrend runs late.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "The city looks fine in this light.", "It does. Almost makes you forget the war.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "Lost an hour watching the duels by the gate.", "Everyone does. Do not bet on them though.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "Any word from up north?", "Same as always. We hold, they press.", nullptr }, 2 },
    };

    constexpr int16 kCrowdDialogueCount =
        static_cast<int16>(sizeof(kCrowdDialogues) / sizeof(kCrowdDialogues[0]));

    // bank <-> AH partner POI per city (ids from CityPoiRegistry)
    inline uint32 CrowdPartnerPoi(uint32 poiId)
    {
        switch (poiId)
        {
            case 26:  return 2;    // Stormwind bank -> AH
            case 2:   return 26;   // Stormwind AH -> bank
            case 136: return 102;  // Orgrimmar bank -> AH
            case 102: return 136;  // Orgrimmar AH -> bank
            default:  return 0;
        }
    }
}

#endif
