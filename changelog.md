# Changelog
All notable changes to this project will be documented in this file.

## [Unreleased]
### Added
- All Kanto Pokémon
- Pokemon:castMove('MoveName') to Script Interface
- Teleport Pokémon to the player if he is away.
- Item Attribute Price
- Item Attribute CorpseGender
- Item Attribute CorpseType
- Genders tag to Pokemon XML file
- Add dittochance tag to Pokemon XML
- Create Ditto corpse if the pokemon is a Ditto when it dies
- Limit of 65535 effects
- Some Pokeballs goback effects
- Configurable Pokeballs in 'data/XML/Pokeballs.xml'
- Set corpse type when Pokémon die
- Catchable Pokémon flag
- PokemonType:isCatchable() to Script Interface
- PokemonType_t to enums.h
- ItemType:isPokeball() to Script Interface
- Level limitation for Pokeball usage in Pokeballs.xml
- Pokeball required level description to item
- /reload pokeballs
- Configurable Player gainHP in config.lua
- Catch System
- Pokémon of the Player can send emotes in catch and shop system
- Pokémon Nature
- Configure Pokémon genders from number 51 to 151
- Item Attribute PokemonId
- Save Pokémon in database

### Changed
- Monster(s) to Pokémon(s)
- Spell(s) to Move(s)
- SLOT_FEET to SLOT_POKEBALL
- SLOT_AMMO to SLOT_SUPPORT
- SLOT_LEGS to SLOT_PORTRAIT

### Removed
- All Tibia monsters
- All Tibia spells
- Blood splash when Pokémon receive an attack
- Player's Pokémon experience gain
- All Player attack damage
- Player cast moves

### Fixed
- Graveler name
- NidoranF and NidoranM filename and names
- Mr.Mme name



