#include "parseData.hpp"

#include <set>

#include <ghc/filesystem.hpp>
#include <simdjson.h>

#include "formatters.hpp"

using namespace simdjson;

//Bunch of macros to make life easier, CLion has awesome macro function support!

#define PARSE_FILE(path) dom::parser parser; \
dom::element doc;                        \
auto error = parser.load(path.u8string()).get(doc); \
if (error) { errorLogger->error("Unable to parse {}! {}", path, error); return false; }

#define GET(obj, name, var) error = obj[name].get(var); \
if (error) { errorLogger->error("Unable to parse {}! {}", name, error); return false; }

#define GET_2(obj, name1, name2, var) error = obj[name1][name2].get(var); \
if (error) { errorLogger->error("Unable to parse {}.{}! {}", name1, name2, error); return false; }

#define GET_3(obj, name1, name2, name3, var) error = obj[name1][name2][name3].get(var); \
if (error) { errorLogger->error("Unable to parse {}.{}.{}! {}", name1, name2, name3, error); return false; }

#define GET_INDEX(obj, index, var) error = obj.at(index).get(var); \
if (error) { errorLogger->error("Unable to parse at index {}! {}", index, error); return false; }

#define SKIP_NULL(element) if (element.type() == dom::element_type::NULL_VALUE) continue;

#define FILE_IS_ARRAY(path) if (doc.type() != dom::element_type::ARRAY) { \
errorLogger->error("File {} is not an array!", path);                     \
return false; }

#define PARSE_DATA(name, func) if (filename == name) { \
logger->debug("Parsing {}", name);                     \
if (!func) {                                           \
errorLogger->error("Error parsing Actors.json at {}", path); \
return false; } }

bool parseData(const ghc::filesystem::path& dataFolder, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& logger, const std::shared_ptr<spdlog::logger>& errorLogger) {
    for (const auto& p : ghc::filesystem::directory_iterator(dataFolder, ghc::filesystem::directory_options::skip_permission_denied)) {
        if (!p.is_regular_file()) continue;

        auto path = p.path();
        auto filename = path.filename();

        PARSE_DATA("Actors.json", parseActors(path, parsedData, errorLogger))
        PARSE_DATA("CommonEvents.json", parseCommonEvents(path, parsedData, errorLogger))
        PARSE_DATA("Enemies.json", parseEnemies(path, parsedData, errorLogger))
        PARSE_DATA("Skills.json", parseSkills(path, parsedData, errorLogger))
        PARSE_DATA("System.json", parseSystem(path, parsedData, errorLogger))
        PARSE_DATA("Tilesets.json", parseTilesets(path, parsedData, errorLogger))

        auto sFileName = filename.u8string();
        //11 chars: MapXYZ.json
        if (sFileName.length() == 11) {
            auto res = sFileName.find("Map");
            if (res != std::string::npos) {
                logger->debug("Parsing {}", sFileName);

                if(!parseMap(path, parsedData, errorLogger)) {
                    errorLogger->error("Error parsing {}", path);
                    return false;
                }
            }
        }
    }

    //parsing Animations.json last because we need to parse everything else first so we can populate the required
    //animation ids
    auto animationsPath = ghc::filesystem::path(dataFolder).append("Animations.json");
    logger->debug("Parsing Animations.json");

    if(!parseAnimations(animationsPath, parsedData, errorLogger)) {
        errorLogger->error("Error parsing Animations.json at {}", animationsPath);
        return false;
    }

    return true;
}

bool parseEvents(simdjson::dom::array& eventList, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger) {
    for (dom::element listItem : eventList) {
        uint64_t code;
        auto error = listItem["code"].get(code);

        dom::array parameters;
        GET(listItem, "parameters", parameters)

        /*
         * RPG Maker MV code list (only important ones):
         * - 101: show text with actor face, [0] is face name
         *
         * - 132: change battle bgm, [0].name is bgm name
         * - 133: change victory me, [0].name is me name
         * - 139: change defeat me, [0].name is me name
         * - 140: change vehicle bgm, [1].name is bgm name
         *
         * - 205: set movement route (see 505)
         *
         * - 212: show animation, [2] is index of animation (maybe not needed)
         * - 231: show picture, [1] is picture name
         *
         * - 241: play bgm, [0].name is bgm name
         * - 245: play bgs, [0].name is bgs name
         * - 249: play me, [0].name is me name
         * - 250: play se, [0].name is se name
         * - 261: play movie, [0] is movie name
         *
         * - 282: change tileset, [0] is index of tileset (maybe not needed)
         * - 283: change battle back, [0] is battlebacks1 and [1] is battlebacks2
         * - 284: change parallax, [0] is image name
         *
         * - 322: change actor images, [1] is face name, [3] is character name, [5] is battler name
         * - 323: change vehicle image, [1] is face name
         *
         * - 337: show battle animation, [1] is index of animation (maybe not needed)
         *
         * - 505: argument of 205 where each item in parameters also has code+parameters fields:
         *      - 41: change character image, [0] is character name
         *      - 44: play se, [0].name is se name
         */

        if (code == 101) {
            std::string_view faceName;
            GET_INDEX(parameters, 0, faceName)

            parsedData->faceNames.emplace(faceName);
        } else if (code == 132 || code == 133 || code == 139 || code == 241
                   || code == 245 || code == 249 || code == 250) {
            dom::object audioObj;
            GET_INDEX(parameters, 0, audioObj)

            std::string_view audioName;
            GET(audioObj, "name", audioName)

            if (code == 132 || code == 241) {
                parsedData->bgmNames.emplace(audioName);
            } else if (code == 133 || code == 139 || code == 249) {
                parsedData->meNames.emplace(audioName);
            } else if (code == 245) {
                parsedData->bgsNames.emplace(audioName);
            } else if (code == 250) {
                parsedData->seNames.emplace(audioName);
            }
        } else if (code == 140) {
            dom::object vehicleObj;
            GET_INDEX(parameters, 1, vehicleObj)

            std::string_view bgmName;
            GET(vehicleObj, "name", bgmName)

            parsedData->bgmNames.emplace(bgmName);
        } else if (code == 212 || code == 337) {
            uint64_t id;
            GET_INDEX(parameters, 1, id)

            parsedData->animationIds.insert(id);
        } else if (code == 231) {
            std::string_view pictureName;
            GET_INDEX(parameters, 1, pictureName)

            parsedData->pictureNames.emplace(pictureName);
        } else if (code == 261) {
            std::string_view movieName;
            GET_INDEX(parameters, 0, movieName)

            parsedData->movieNames.emplace(movieName);
        } else if (code == 283) {
            std::string_view battlebacks1Name;
            std::string_view battlebacks2Name;

            GET_INDEX(parameters, 0, battlebacks1Name)
            GET_INDEX(parameters, 1, battlebacks2Name)

            parsedData->battleback1Names.emplace(battlebacks1Name);
            parsedData->battleback2Names.emplace(battlebacks2Name);
        } else if (code == 284) {
            std::string_view parallaxName;
            GET_INDEX(parameters, 0, parallaxName)

            parsedData->parallaxNames.emplace(parallaxName);
        } else if (code == 322) {
            std::string_view faceName;
            std::string_view characterName;
            std::string_view actorBattlerName;

            GET_INDEX(parameters, 1, faceName)
            GET_INDEX(parameters, 3, characterName)
            GET_INDEX(parameters, 5, actorBattlerName)

            parsedData->faceNames.emplace(faceName);
            parsedData->characterNames.emplace(characterName);
            parsedData->actorBattlerNames.emplace(actorBattlerName);
        } else if (code == 323) {
            std::string_view faceName;
            GET_INDEX(parameters, 1, faceName)

            parsedData->faceNames.emplace(faceName);
        } else if (code == 505) {
            dom::object extraObj;
            GET_INDEX(parameters, 0, extraObj)

            uint64_t extraCode;
            GET(extraObj, "code", extraCode)

            dom::array extraParameters;
            GET(extraObj, "parameters", extraParameters)

            if (extraCode == 41) {
                std::string_view characterName;
                GET_INDEX(extraParameters, 0, characterName)

                parsedData->characterNames.emplace(characterName);
            } else if (extraCode == 44) {
                dom::object seObj;
                GET_INDEX(extraParameters, 0, seObj)

                std::string_view seName;
                GET(seObj, "name", seName)

                parsedData->seNames.emplace(seName);
            }
        }
    }

    return true;
}

bool parseActors(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger) {
    PARSE_FILE(path)
    FILE_IS_ARRAY(path)

    for (dom::element element : doc) {
        SKIP_NULL(element)
        /*
         * Actors.json:
         * battlerName => img/sv_actors/{battlerName}.png
         * characterName (w/ index) => img/characters/{characterName}.png
         * faceName (w/ index) => img/faces/{faceName}.png
         */

        std::string_view battlerName;
        std::string_view characterName;
        std::string_view faceName;

        GET(element, "battlerName", battlerName)
        GET(element, "characterName", characterName)
        GET(element, "faceName", faceName)

        parsedData->actorBattlerNames.emplace(battlerName);
        parsedData->characterNames.emplace(characterName);
        parsedData->faceNames.emplace(faceName);
    }

    return true;
}

bool parseAnimations(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger) {
    PARSE_FILE(path)
    FILE_IS_ARRAY(path)

    for (dom::element element : doc) {
        SKIP_NULL(element)
        dom::object obj = element;
        /*
         * Animations.json:
         * animation1Name => img/animations/{animation1Name}.png
         * animation2Name => img/animations/{animation2Name}.png
         * timings[i].se.name => audio/se/{name}.m4a/ogg
         */

        uint64_t id;
        GET(obj, "id", id)

        auto it = parsedData->animationIds.find(id);
        if (it == parsedData->animationIds.end())
            continue;

        std::string_view animation1Name;
        std::string_view animation2Name;

        GET(obj, "animation1Name", animation1Name)
        GET(obj, "animation2Name", animation2Name)

        parsedData->animationNames.emplace(animation1Name);
        parsedData->animationNames.emplace(animation2Name);

        dom::element timings;
        GET(obj, "timings", timings)

        if (timings.type() != dom::element_type::ARRAY) {
            errorLogger->error("Animation timings is not an array!");
            return false;
        }

        for (dom::element timing : timings) {
            dom::element se;
            GET(timing, "se", se)
            SKIP_NULL(se)

            std::string_view seName;
            GET(se, "name", seName)

            parsedData->seNames.emplace(seName);
        }
    }

    return true;
}

bool parseCommonEvents(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger) {
    PARSE_FILE(path)
    FILE_IS_ARRAY(path)

    for (dom::element element : doc) {
        SKIP_NULL(element)

        dom::array list;
        GET(element, "list", list)

        if(!parseEvents(list, parsedData, errorLogger))
            return false;
    }

    return true;
}

bool parseEnemies(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger) {
    PARSE_FILE(path)
    FILE_IS_ARRAY(path)

    for (dom::element element : doc) {
        SKIP_NULL(element)
        /*
         * Enemies.json:
         * battlerName => img/enemies/{battlerName}.png
         */

        std::string_view battlerName;
        GET(element, "battlerName", battlerName)

        parsedData->enemyBattlerNames.emplace(battlerName);
    }

    return true;
}

bool parseMap(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger) {
    PARSE_FILE(path)
    /*
     * Map.json:
     *
     * battleback1Name => img/battlebacks1/{}.png
     * battleback2Name =>img/battlebacks2/{}.png
     *
     * bgm.name => audio/bgm/{}.m4a/ogg
     * bgs.name => audio/bgs/{}.m4a/ogg
     *
     * parallaxName => img/parallaxes/{}.png
     *
     * events[n].pages[n]:
     *  - image.characterName => img/characters/{}.png
     *  - list[n] => same as CommonEvents.json
     */

    std::string_view battleback1Name;
    std::string_view battleback2Name;

    GET(doc, "battleback1Name", battleback1Name)
    GET(doc, "battleback2Name", battleback2Name)

    std::string_view bgmName;
    std::string_view bgsName;

    GET_2(doc, "bgm", "name", bgmName)
    GET_2(doc, "bgs", "name", bgsName)

    std::string_view parallaxName;
    GET(doc, "parallaxName", parallaxName)

    parsedData->battleback1Names.emplace(battleback1Name);
    parsedData->battleback2Names.emplace(battleback2Name);

    parsedData->bgmNames.emplace(bgmName);
    parsedData->bgsNames.emplace(bgsName);

    parsedData->parallaxNames.emplace(parallaxName);

    dom::array events;
    GET(doc, "events", events)

    for (dom::element event : events) {
        SKIP_NULL(event)

        dom::array pages;
        GET(event, "pages", pages)

        for (dom::element page : pages) {
            SKIP_NULL(page)

            std::string_view image;
            GET_2(page, "image", "characterName", image)

            parsedData->characterNames.emplace(image);

            dom::array list = page["list"];

            if (!parseEvents(list, parsedData, errorLogger))
                return false;
        }
    }

    return true;
}

bool parseSkills(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger) {
    PARSE_FILE(path)
    FILE_IS_ARRAY(path)

    for (dom::element element : doc) {
        SKIP_NULL(element)
        /*
         * Skills.json:
         * animationId
         */

        int64_t animationId;
        GET(element, "animationId", animationId)

        if (animationId == -1 || animationId == 0) continue;

        parsedData->animationIds.insert(static_cast<uint64_t>(animationId));
    }

    return true;
}

bool parseSystem(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger) {
    PARSE_FILE(path)
    /*
     * System.json:
     *
     * airship.bgm.name => audio/bgm/{}.m4a/ogg
     * airship.characterName => img/characters/{}.png
     *
     * boat.bgm.name => audio/bgm/{}.m4a/ogg
     * boat.characterName => img/characters/{}.png
     *
     * ship.bgm.name => audio/bgm/{}.m4a/ogg
     * ship.characterName => img/character/{}.png
     *
     * battleback1Name => img/battlebacks1/{}.png
     * battleback2Name => img/battlebacks2/{}.png
     *
     * battlerName => img/enemies/{}.png
     *
     * title1Name => img/titles1/{}.png
     * title2Name => img/titles2/{}.png
     *
     * sounds[n].name => audio/se/{}.m4a/ogg
     *
     * battleBgm.name => audio/bgm/{}.m4a/ogg
     * titleBgm.name => audio/bgm/{}.m4a/ogg
     *
     * defeatMe.name => audio/me/{}.m4a/ogg
     * gameoverMe.name => audio/me/{}.m4a/ogg
     * victoryMe.name => audio/me/{}.m4a/ogg
     *
     */

    std::string_view airshipBgm;
    std::string_view boatBgm;
    std::string_view shipBgm;

    GET_3(doc, "airship", "bgm", "name", airshipBgm)
    GET_3(doc, "boat", "bgm", "name", boatBgm)
    GET_3(doc, "ship", "bgm", "name", shipBgm)

    parsedData->bgmNames.emplace(airshipBgm);
    parsedData->bgmNames.emplace(boatBgm);
    parsedData->bgmNames.emplace(shipBgm);

    std::string_view airshipCharacter;
    std::string_view boatCharacter;
    std::string_view shipCharacter;

    GET_2(doc, "airship", "characterName", airshipCharacter)
    GET_2(doc, "boat", "characterName", boatCharacter)
    GET_2(doc, "ship", "characterName", shipCharacter)

    parsedData->characterNames.emplace(airshipCharacter);
    parsedData->characterNames.emplace(boatCharacter);
    parsedData->characterNames.emplace(shipCharacter);

    std::string_view battleback1Name;
    std::string_view battleback2Name;

    GET(doc, "battleback1Name", battleback1Name)
    GET(doc, "battleback2Name", battleback2Name)

    parsedData->battleback1Names.emplace(battleback1Name);
    parsedData->battleback2Names.emplace(battleback2Name);

    std::string_view battlerName;
    GET(doc, "battlerName", battlerName)

    parsedData->enemyBattlerNames.emplace(battlerName);

    std::string_view title1Name;
    std::string_view title2Name;

    GET(doc, "title1Name", title1Name)
    GET(doc, "title2Name", title2Name)

    parsedData->title1Names.emplace(title1Name);
    parsedData->title2Names.emplace(title2Name);

    std::string_view battleBgm;
    std::string_view titleBgm;

    GET_2(doc, "battleBgm", "name", battleBgm)
    GET_2(doc, "titleBgm", "name", titleBgm)

    parsedData->bgmNames.emplace(battleBgm);
    parsedData->bgmNames.emplace(titleBgm);

    std::string_view defeatMe;
    std::string_view gameoverMe;
    std::string_view victoryMe;

    GET_2(doc, "defeatMe", "name", defeatMe)
    GET_2(doc, "gameoverMe", "name", gameoverMe)
    GET_2(doc, "victoryMe", "name", victoryMe)

    parsedData->meNames.emplace(defeatMe);
    parsedData->meNames.emplace(gameoverMe);
    parsedData->meNames.emplace(victoryMe);

    dom::element sounds;
    GET(doc, "sounds", sounds)

    if (sounds.type() != dom::element_type::ARRAY) {
        errorLogger->error("Sounds is not an array!");
        return false;
    }

    for (dom::object sound : sounds) {
        std::string_view soundName;
        GET(sound, "name", soundName)
        parsedData->seNames.emplace(soundName);
    }

    return true;
}

bool parseTilesets(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger) {
    PARSE_FILE(path)
    FILE_IS_ARRAY(path)

    for (dom::element element : doc) {
        SKIP_NULL(element)
        /*
         * Tilesets.json:
         * tilesetNames[n] => img/tilesets/{}.png+{}.txt
         */

        dom::array names;
        GET(element, "tilesetNames", names)
        for (std::string_view tilesetName : names) {
            if (tilesetName.empty()) continue;
            parsedData->tilesetNames.emplace(tilesetName);
        }
    }

    return true;
}