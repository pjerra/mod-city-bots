-- Remove persisted playerbot randomizer side effects from the fixed city-bot cast.
-- The module now drives these bots directly; they should not keep saved pets,
-- temporary buffs, or mount state from PlayerbotFactory::Randomize.

DELETE psc
FROM `pet_spell_cooldown` psc
JOIN `character_pet` cp ON cp.`id` = psc.`guid`
WHERE cp.`owner` BETWEEN 9000001 AND 9000400;

DELETE ps
FROM `pet_spell` ps
JOIN `character_pet` cp ON cp.`id` = ps.`guid`
WHERE cp.`owner` BETWEEN 9000001 AND 9000400;

DELETE pa
FROM `pet_aura` pa
JOIN `character_pet` cp ON cp.`id` = pa.`guid`
WHERE cp.`owner` BETWEEN 9000001 AND 9000400;

DELETE FROM `character_pet_declinedname`
WHERE `owner` BETWEEN 9000001 AND 9000400;

DELETE FROM `character_pet`
WHERE `owner` BETWEEN 9000001 AND 9000400;

DELETE FROM `character_aura`
WHERE `guid` BETWEEN 9000001 AND 9000400;
