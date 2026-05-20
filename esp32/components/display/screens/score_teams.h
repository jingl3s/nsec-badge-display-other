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
    "ALBANIE",
    "ALLEMAGNE", "ANGLETERRE", "ARABIE SAOUDITE", "ARGENTINE", "BELGIQUE",
    "BRESIL",    "CANADA",     "CHINE",           "COSTA RICA","CROATIE", "ESPAGNE",
    "FRANCE",    "IRLANDE",    "ITALIE",          "JAPON",     "PAYS-BAS",
    "PORTUGAL",  "ROUMANIE",   "SUISSE",          "TUNISIE"
};

const tournament_t world_cup = {.title = "Monde",
                                .teams = world_cup_teams,
                                .number_teams = sizeof(world_cup_teams) /
                                                sizeof(world_cup_teams[0])};

const char *europe_cup_teams[] = {
    "AC Milan (ITA)",
    "Ajax Amsterdam (NED)",
    "Anderlecht (BEL)",
    "Antwerp (BEL)",
    "Arsenal (ENG)",
    "As Monaco (MON)",
    "AS Roma (ITA)",
    "Aston Villa (ENG)",
    "Atlanta Bergame (ITA)",
    "Atletic Bilbao(ESP)",
    "Atletico Madrid (ESP)",
    "Az Alkamaar(NED)",
    "Bayer Leverkusen (GER)",
    "Bayern Munich (GER)",
    "Benfica (POR)",
    "Bodø/Glimt (NOR)",
    "Borussia Dortmund (GER)",
    "Braga (POR)",
    "Brest (FRA)",
    "Celge(slo)",
    "Celtic (SCO)",
    "Chelsea (ENG)",
    "Club Brugge (BEL)",
    "Copenhague (DEN)",
    "Dinamo Zagreb (CRO)",
    "Eintracht Francfort (GER)",
    "FC Barcelone (ESP)",
    "FC Porto (POR)",
    "FC Zurich (SUI)",
    "FCSB (ROU)",
    "Fenerbahçe (TUR)",
    "Ferencvacos (HON)",
    "Feyenoord Rotterdam (NED)",
    "Galatasaray (TUR)",
    "HJK Helsinki (FIN)",
    "Hoffenheim (GER)",
    "Inter Milan (ITA)",
    "Juventus (ITA)",
    "Lazio Rome (ITA)",
    "Lille (FRA)",
    "Liverpool (ENG)",
    "Lokomotiv Moskow(RUS)",
    "Ludogorets Razgrad (BUL)",
    "Lugano(SUI)",
    "Lyon (FRA)",
    "Maccabi Haifa (ISR)",
    "Manchester City (ENG)",
    "Manchester United (ENG)",
    "Midtjylland (DEN)",
    "Olympiacos (GRE)",
    "Omonia Nicosie (CYP)",
    "Paok(GRE)",
    "PSG (FRA)",
    "PSV Eindhoven (NED)",
    "Rangers(SCO)",
    "RasenBallsport leipzig(GER)",
    "RB Leipzig (GER)",
    "RB Salzburg (AUT)",
    "Real Betis (ESP)",
    "Real Madrid (ESP)",
    "Real Sociedad (ESP)",
    "Red Bull Salzburg (AUT)",
    "Sevilla FC (ESP)",
    "Shakhtar Donetsk (UKR)",
    "Sheriff Tiraspol (MDA)",
    "Slovan Bratislava (SVK)",
    "Sporting Club Portugal(POR)",
    "Sturm Graz (AUT)",
    "Tottenham Hotspur (ENG)",
    "Twente (NED)",
    "Union Berlin (GER)",
    "Union Saint-Gilloise (FRA)",
    "Viktoria Plzen (CZE)",
    "West Ham United (ENG)",
    "Wolsburg(GER)",
};

const tournament_t europe_cup = {.title = "Ligue Europe UEFA",
                                 .teams = europe_cup_teams,
                                 .number_teams = sizeof(europe_cup_teams) /
                                                 sizeof(europe_cup_teams[0])};

const char *tsubasa_cup_teams[] = {
    "Azuma-Ichi",
    "Furano",
    "Hanawa",
    "Hirado",
    "Meiwa",
    "Minami Uwa",
    "Musashi",
    "Naniwa",
    "Nankatsu SC",
    "Ohyama",
    "Otomo",
    "Saporo FC",
    "Toho",
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

// Déclaration des équipes france 2024
const char *france_cup_teams[] = {
    "Bordeaux",
    "Lyon",
    "Marseille",
    "Nantes",
    "Paris",
    "Rodez",
    "Toulouse",
    };

const tournament_t france_cup = {.title = "FRANCE",
                              .teams = france_cup_teams,
                              .number_teams = sizeof(france_cup_teams) /
                                              sizeof(france_cup_teams[0])};

const char *inazuma_eleven_cup_teams[] = {
    "Alpine",
    "Black flower",
    "Chaos",
    "Cloitre Sacre",
    "College zeus",
    "Diamond Dust",
    "Epsilon Plus",
    "Epsilon",
    "Genesis",
    "Kirkwood",
    "L'Academie Alius",
    "L'equipe de Fauxshore",
    "L'institue oculte",
    "La Nouvelle Royale Academie",
    "Le college wife",
    "Les cyberthec",
    "Les Empereurs Noirs",
    "Les Lions du Desert",
    "Mary Times",
    "Otaku",
    "Prominence",
    "Raimon",
    "Royale academie",
    "Shuriken",
    "Tempete des Gemeaux",
    "Terria",
    "Wild"
};


const tournament_t inazuma_eleven_cup = {.title = "Inazuma Eleven",
                              .teams = inazuma_eleven_cup_teams,
                              .number_teams = sizeof(inazuma_eleven_cup_teams) /
                                              sizeof(inazuma_eleven_cup_teams[0])};


// Déclaration des équipes Match sans noms
const char *sans_noms_cup_teams[] = {
    "Equipe 1",
    "Equipe 2",
    };

const tournament_t sans_noms_cup = {.title = "Sans noms",
                              .teams = sans_noms_cup_teams,
                              .number_teams = sizeof(sans_noms_cup_teams) /
                                              sizeof(sans_noms_cup_teams[0])};

// Déclaration des équipes Match sans noms
const char *personal_cup_teams[] = {
    "Team Dran 1",
    "Team Dran 2",
    "Team Dreams",
    "Team Icendio 1",
    "Team Icendio 2",
    "Team Pendragon",
    "Team Persona",
    "Team Zooganic",
    "Stockolm",
    "Ultra Madrid",
    "Goohre (Leg)",
    "Ninjago Smh",
    "Qazw Sertyuo Fsyq (Pay)",
    "Der Iona",
    };

const tournament_t personal_cup = {.title = "Personnelle",
                              .teams = personal_cup_teams,
                              .number_teams = sizeof(personal_cup_teams) /
                                              sizeof(personal_cup_teams[0])};


const tournament_t *tournaments[] = {
    &world_cup,   &europe_cup,
    &tsubasa_cup, &france_cup, &mls_cup,
    &nba_cup,     &nhl_cup, &inazuma_eleven_cup, &personal_cup,
    &sans_noms_cup};

extern const tournament_t *tournaments[];

extern const lv_color_t teams_color[]; // Declaration

extern const uint8_t teams_color_size;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
