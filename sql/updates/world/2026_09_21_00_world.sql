-- Battle Pet Trainer support: teams of battle pets assigned to NPC trainers
CREATE TABLE IF NOT EXISTS `battle_pet_trainer` (
    `entry` MEDIUMINT(8) UNSIGNED NOT NULL COMMENT 'NPC entry of the trainer',
    `species` INT(10) UNSIGNED NOT NULL COMMENT 'Battle pet species ID (BattlePetSpecies.db2)',
    `level` TINYINT(3) UNSIGNED NOT NULL DEFAULT 1,
    `quality` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
    `breed` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`entry`, `species`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
