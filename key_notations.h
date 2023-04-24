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
        {KeyFinder::A_MAJOR,       "A Major" }, {KeyFinder::A_MINOR,       "Am Minor" },
        {KeyFinder::B_FLAT_MAJOR,  "Bb  Major"}, {KeyFinder::B_FLAT_MINOR,  "Bbm Minor"},
        {KeyFinder::B_MAJOR,       "B Major" }, {KeyFinder::B_MINOR,       "Bm Minor" },
        {KeyFinder::C_MAJOR,       "C Major" }, {KeyFinder::C_MINOR,       "Cm Minor" },
        {KeyFinder::D_FLAT_MAJOR,  "Db Major"}, {KeyFinder::D_FLAT_MINOR,  "Dbm Minor"},
        {KeyFinder::D_MAJOR,       "D Major" }, {KeyFinder::D_MINOR,       "Dm Minor" },
        {KeyFinder::E_FLAT_MAJOR,  "Eb Major"}, {KeyFinder::E_FLAT_MINOR,  "Ebm Minor"},
        {KeyFinder::E_MAJOR,       "E Major" }, {KeyFinder::E_MINOR,       "Em Minor" },
        {KeyFinder::F_MAJOR,       "F Major" }, {KeyFinder::F_MINOR,       "Fm Minor" },
        {KeyFinder::G_FLAT_MAJOR,  "Gb Major"}, {KeyFinder::G_FLAT_MINOR,  "Gbm Minor"},
        {KeyFinder::G_MAJOR,       "G Major" }, {KeyFinder::G_MINOR,       "Gm Minor" },
        {KeyFinder::A_FLAT_MAJOR,  "Ab Major"}, {KeyFinder::A_FLAT_MINOR,  "Abm Minor"},
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
