#pragma once
#include "lvgl/lvgl.h"
#ifndef TEAMS_COLORS_H
#define TEAMS_COLORS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _tournament {
    const char *title;
    const char **teams;
    const uint8_t number_teams;
} tournament_t;

const char *world_cup_teams[] = {
    "ALLEMAGNE", "ANGLETERRE", "ARABIE SAOUDITE", "ARGENTINE", "CHINE",
    "BELGIQUE",  "BRESIL",     "CANADA",          "ESPAGNE",   "FRANCE",
    "IRLANDE",   "JAPON",      "PAYS-BAS",        "PORTUGAL",  "ROUMANIE",
    "SUISSE",    "TUNISIE",
};

const tournament_t world_cup = {.title = "Monde",
                                .teams = world_cup_teams,
                                .number_teams = sizeof(world_cup_teams) /
                                                sizeof(world_cup_teams[0])};

const char *europe_cup_teams[] = {
    "Arsenal (ENG)",
    "West Ham United (ENG)",
    "Anderlecht (BEL)",
    "FCSB (ROU)",
    "PSV Eindhoven (NED)",
    "Bodø/Glimt (NOR)",
    "FC Zurich (SUI)",
    "AS Roma (ITA)",
    "Ludogorets Razgrad (BUL)",
    "Real Betis (ESP)",
    "HJK Helsinki (FIN)",
    "Braga (POR)",
    "Hoffenheim (GER)",
    "Lyon (FRA)",
    "Slovan Bratislava (SVK)",
    "Lazio Rome (ITA)",
    "Feyenoord Rotterdam (NED)",
    "Sturm Graz (AUT)",
    "Midtjylland (DEN)",
    "Manchester United (ENG)",
    "Real Sociedad (ESP)",
    "Sheriff Tiraspol (MDA)",
    "Omonia Nicosie (CYP)",
    "Olympiacos (GRE)",
    "Eintracht Francfort (GER)",
    "Antwerp (BEL)",
    "Fenerbahçe (TUR)",
    "Celtic (SCO)",
    "Union Berlin (GER)",
};

const tournament_t europe_cup = {.title = "Ligue Europe UEFA",
                                 .teams = europe_cup_teams,
                                 .number_teams = sizeof(europe_cup_teams) /
                                                 sizeof(europe_cup_teams[0])};

const char *uefa_champions_cup_teams[] = {
    "Bayern Munich (GER)",     "Manchester City (ENG)",
    "Real Madrid (ESP)",       "Liverpool (ENG)",
    "Chelsea (ENG)",           "PSG (FRA)",
    "AC Milan (ITA)",          "FC Barcelone (ESP)",
    "Atlético Madrid (ESP)",   "Juventus (ITA)",
    "Borussia Dortmund (GER)", "RB Leipzig (GER)",
    "Tottenham Hotspur (ENG)", "Ajax Amsterdam (NED)",
    "Inter Milan (ITA)",       "Benfica (POR)",
    "Manchester United (ENG)", "FC Porto (POR)",
    "Shakhtar Donetsk (UKR)",  "Sevilla FC (ESP)",
    "Bayer Leverkusen (GER)",  "RB Salzburg (AUT)",
    "Club Brugge (BEL)",       "Celtic (SCO)",
    "Galatasaray (TUR)",       "Viktoria Plzen (CZE)",
    "Dinamo Zagreb (CRO)",     "Red Bull Salzburg (AUT)",
    "Sheriff Tiraspol (MDA)",  "Maccabi Haifa (ISR)",
    "Copenhague (DEN)"};

const tournament_t uefa_champions_cup = {
    .title = "Ligue Champions UEFA",
    .teams = uefa_champions_cup_teams,
    .number_teams =
        sizeof(uefa_champions_cup_teams) / sizeof(uefa_champions_cup_teams[0])};

const char *tsubasa_cup_teams[] = {
    "Nankatsu SC", "Meiwa",  "Furano",     "Musashi", "Hanawa",     "Naniwa",
    "Toho",        "Ohyama", "Azuma-Ichi", "Hirado",  "Minami Uwa",
};

const tournament_t tsubasa_cup = {.title = "Captain Tsubasa",
                                  .teams = tsubasa_cup_teams,
                                  .number_teams = sizeof(tsubasa_cup_teams) /
                                                  sizeof(tsubasa_cup_teams[0])};

const char *nba_cup_teams[] = {
    // Conférence Est
    "Boston Celtics", "Brooklyn Nets", "New York Knicks", "Philadelphia 76ers",
    "Toronto Raptors", "Chicago Bulls", "Cleveland Cavaliers",
    "Detroit Pistons", "Indiana Pacers", "Milwaukee Bucks", "Charlotte Hornets",
    "Miami Heat", "Orlando Magic", "Washington Wizards", "Atlanta Hawks",
    // Conférence Ouest
    "Golden State Warriors", "Phoenix Suns", "Los Angeles Clippers",
    "Los Angeles Lakers", "Denver Nuggets", "Dallas Mavericks",
    "Memphis Grizzlies", "Minnesota Timberwolves", "New Orleans Pelicans",
    "Oklahoma City Thunder", "Portland Trail Blazers", "Sacramento Kings",
    "San Antonio Spurs", "Utah Jazz", "Houston Rockets"};

const tournament_t nba_cup = {.title = "NBA",
                              .teams = nba_cup_teams,
                              .number_teams = sizeof(nba_cup_teams) /
                                              sizeof(nba_cup_teams[0])};

const char *nhl_cup_teams[] = {
    // Conférence Atlantique

    "Boston Bruins", "Buffalo Sabres", "Detroit Red Wings", "Florida Panthers",
    "Montreal Canadiens", "Ottawa Senators", "Tampa Bay Lightning",
    "Toronto Maple Leafs",

    // Conférence Métropolitaine

    "Carolina Hurricanes", "Columbus Blue Jackets", "New Jersey Devils",
    "New York Islanders", "New York Rangers", "Philadelphia Flyers",
    "Pittsburgh Penguins", "Washington Capitals",

    // Conférence Centrale

    "Chicago Blackhawks", "Colorado Avalanche", "Dallas Stars",
    "Minnesota Wild", "Nashville Predators", "St. Louis Blues", "Winnipeg Jets",

    // Conférence Pacifique

    "Anaheim Ducks", "Arizona Coyotes", "Calgary Flames", "Edmonton Oilers",
    "Los Angeles Kings", "San Jose Sharks", "Seattle Kraken",
    "Vancouver Canucks"};

const tournament_t nhl_cup = {.title = "NHL",
                              .teams = nhl_cup_teams,
                              .number_teams = sizeof(nhl_cup_teams) /
                                              sizeof(nhl_cup_teams[0])};

// Déclaration des équipes MLS 2024
const char *mls_cup_teams[] = {
    // Conference Est
    "Atlanta United FC",
    "Chicago Fire FC",
    "FC Cincinnati",
    "Columbus Crew SC",
    "D.C. United",
    "Inter Miami CF",
    "CF Montréal",
    "New England Revolution",
    "New York City FC",
    "New York Red Bulls",
    "Orlando City SC",
    "Philadelphia Union",
    "Toronto FC"
    // Conference Ouest
    "Austin FC",
    "Colorado Rapids",
    "FC Dallas",
    "Houston Dynamo FC",
    "Los Angeles FC",
    "Los Angeles Galaxy",
    "Minnesota United FC",
    "Nashville SC",
    "Real Salt Lake",
    "Sporting Kansas City",
    "Seattle Sounders FC",
    "San Jose Earthquakes",
    "St. Louis City SC",
    "Vancouver Whitecaps FC"};

const tournament_t mls_cup = {.title = "MLS",
                              .teams = mls_cup_teams,
                              .number_teams = sizeof(mls_cup_teams) /
                                              sizeof(mls_cup_teams[0])};

const char *mixed_all_soccer_teams[] = {
    // world_cup_teams
    "FRANCE",
    "CANADA",
    "PORTUGAL",
    "JAPON",
    "PAYS-BAS",
    "ROUMANIE",
    "IRLANDE",
    "SUISSE",
    "TUNISIE",
    "ALLEMAGNE",
    "BELGIQUE",
    "ARABIE SAOUDITE",
    "ESPAGNE",
    "BRESIL",
    "ARGENTINE",

    // uefa_champions_cup_teams
    "Bayern Munich (GER)",
    "Manchester City (ENG)",
    "Real Madrid (ESP)",
    "Liverpool (ENG)",
    "Chelsea (ENG)",
    "PSG (FRA)",
    "AC Milan (ITA)",
    "FC Barcelone (ESP)",
    "Atlético Madrid (ESP)",
    "Juventus (ITA)",
    "Borussia Dortmund (GER)",
    "RB Leipzig (GER)",
    "Tottenham Hotspur (ENG)",
    "Ajax Amsterdam (NED)",
    "Inter Milan (ITA)",
    "Benfica (POR)",
    "Manchester United (ENG)",
    "FC Porto (POR)",
    "Shakhtar Donetsk (UKR)",
    "Sevilla FC (ESP)",
    "Bayer Leverkusen (GER)",
    "RB Salzburg (AUT)",
    "Club Brugge (BEL)",
    "Celtic (SCO)",
    "Galatasaray (TUR)",
    "Viktoria Plzen (CZE)",
    "Dinamo Zagreb (CRO)",
    "Red Bull Salzburg (AUT)",
    "Sheriff Tiraspol (MDA)",
    "Maccabi Haifa (ISR)",
    "Copenhague (DEN)"
    // Tsubasa
    "Nankatsu SC",
    "Meiwa",
    "Furano",
    "Musashi",
    "Hanawa",
    "Naniwa",
    "Toho",
    "Ohyama",
    "Azuma-Ichi",
    "Hirado",
    "Minami Uwa",
};

const tournament_t mixed_all_soccer_cup = {
    .title = "Melange Soccer",
    .teams = mixed_all_soccer_teams,
    .number_teams =
        sizeof(mixed_all_soccer_teams) / sizeof(mixed_all_soccer_teams[0])};

const tournament_t *tournaments[] = {
    &world_cup,   &europe_cup, &uefa_champions_cup,
    &tsubasa_cup, &mls_cup,    &mixed_all_soccer_cup,
    &nba_cup,     &nhl_cup};

extern const tournament_t *tournaments[];

extern const lv_color_t teams_color[]; // Declaration

extern const uint8_t teams_color_size;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif