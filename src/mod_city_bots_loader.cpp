/*
 * mod-city-bots loader
 */

void AddSC_city_bots_module();

void Addmod_city_botsScripts()
{
#ifdef MOD_PLAYERBOTS
    AddSC_city_bots_module();
#endif
}
