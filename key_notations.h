#ifndef KEY_NOTATIONS_H
#define KEY_NOTATIONS_H

#include <map>
#include <keyfinder/constants.h>

namespace KeyNotation
{
    typedef std::map<KeyFinder::key_t, std::string> key_map;

    /**
     * Standard Key Notation
     */
    key_map standard =
    {
        {KeyFinder::A_MAJOR,       "A" }, {KeyFinder::A_MINOR,       "Am" },
        {KeyFinder::B_FLAT_MAJOR,  "Bb"}, {KeyFinder::B_FLAT_MINOR,  "Bbm"},
        {KeyFinder::B_MAJOR,       "B" }, {KeyFinder::B_MINOR,       "Bm" },
        {KeyFinder::C_MAJOR,       "C" }, {KeyFinder::C_MINOR,       "Cm" },
        {KeyFinder::D_FLAT_MAJOR,  "Db"}, {KeyFinder::D_FLAT_MINOR,  "Dbm"},
        {KeyFinder::D_MAJOR,       "D" }, {KeyFinder::D_MINOR,       "Dm" },
        {KeyFinder::E_FLAT_MAJOR,  "Eb"}, {KeyFinder::E_FLAT_MINOR,  "Ebm"},
        {KeyFinder::E_MAJOR,       "E" }, {KeyFinder::E_MINOR,       "Em" },
        {KeyFinder::F_MAJOR,       "F" }, {KeyFinder::F_MINOR,       "Fm" },
        {KeyFinder::G_FLAT_MAJOR,  "Gb"}, {KeyFinder::G_FLAT_MINOR,  "Gbm"},
        {KeyFinder::G_MAJOR,       "G" }, {KeyFinder::G_MINOR,       "Gm" },
        {KeyFinder::A_FLAT_MAJOR,  "Ab"}, {KeyFinder::A_FLAT_MINOR,  "Abm"},
    };

    /**
     * Camelot Key Notation [http://mixedinkey.com/HowTo]
     */
    key_map camelot =
    {
        {KeyFinder::A_MAJOR,       "11B"}, {KeyFinder::A_MINOR,       "8A" },
        {KeyFinder::B_FLAT_MAJOR,  "6B" }, {KeyFinder::B_FLAT_MINOR,  "3A" },
        {KeyFinder::B_MAJOR,       "1B" }, {KeyFinder::B_MINOR,       "10A"},
        {KeyFinder::C_MAJOR,       "8B" }, {KeyFinder::C_MINOR,       "5A" },
        {KeyFinder::D_FLAT_MAJOR,  "3B" }, {KeyFinder::D_FLAT_MINOR,  "12A"},
        {KeyFinder::D_MAJOR,       "10B"}, {KeyFinder::D_MINOR,       "7A" },
        {KeyFinder::E_FLAT_MAJOR,  "5B" }, {KeyFinder::E_FLAT_MINOR,  "2A" },
        {KeyFinder::E_MAJOR,       "12B"}, {KeyFinder::E_MINOR,       "9A" },
        {KeyFinder::F_MAJOR,       "7B" }, {KeyFinder::F_MINOR,       "4A" },
        {KeyFinder::G_FLAT_MAJOR,  "2B" }, {KeyFinder::G_FLAT_MINOR,  "11A"},
        {KeyFinder::G_MAJOR,       "9B" }, {KeyFinder::G_MINOR,       "6A" },
        {KeyFinder::A_FLAT_MAJOR,  "4B" }, {KeyFinder::A_FLAT_MINOR,  "1A" },
    };

    /**
     * Open Key Notation [https://beatunes.com/en/open-key-notation.html]
     */
    key_map open_key =
    {
        {KeyFinder::A_MAJOR,       "4d" }, {KeyFinder::A_MINOR,       "1m" },
        {KeyFinder::B_FLAT_MAJOR,  "11d"}, {KeyFinder::B_FLAT_MINOR,  "8m" },
        {KeyFinder::B_MAJOR,       "6d" }, {KeyFinder::B_MINOR,       "3m" },
        {KeyFinder::C_MAJOR,       "1d" }, {KeyFinder::C_MINOR,       "10m"},
        {KeyFinder::D_FLAT_MAJOR,  "8d" }, {KeyFinder::D_FLAT_MINOR,  "5m" },
        {KeyFinder::D_MAJOR,       "3d" }, {KeyFinder::D_MINOR,       "12m"},
        {KeyFinder::E_FLAT_MAJOR,  "10d"}, {KeyFinder::E_FLAT_MINOR,  "7m" },
        {KeyFinder::E_MAJOR,       "5d" }, {KeyFinder::E_MINOR,       "2m" },
        {KeyFinder::F_MAJOR,       "12d"}, {KeyFinder::F_MINOR,       "9m" },
        {KeyFinder::G_FLAT_MAJOR,  "7d" }, {KeyFinder::G_FLAT_MINOR,  "4m" },
        {KeyFinder::G_MAJOR,       "2d" }, {KeyFinder::G_MINOR,       "11m"},
        {KeyFinder::A_FLAT_MAJOR,  "9d" }, {KeyFinder::A_FLAT_MINOR,  "6m" },
    };

    std::map<std::string, key_map> mappings =
    {
        {"standard", standard},
        {"camelot",  camelot },
        {"openkey",  open_key},
    };
}

#endif
