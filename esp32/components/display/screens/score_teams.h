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
    "ALBANIE",  "ALLEMAGNE", "ANGLETERRE", "ARABIE SAOUDITE", "ARGENTINE",
    "BELGIQUE", "BOSNIE-HERZEGOVINE", "BRESIL",    "CANADA",     "CHINE",           "COSTA RICA",
    "EGYPTE",
    "CROATIE",  "ESPAGNE",   "FRANCE",     "IRLANDE",         "ITALIE",
    "JAPON",    "PAYS-BAS",  "PORTUGAL",   "ROUMANIE",        "SUISSE",
    "TUNISIE"};

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
    "fk quarabag (AZE)",
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
    "Lyon (FRA)	",
    " (ENG)",
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
    "Azuma-Ichi", "Furano",    "Hanawa", "Hirado",      "Meiwa",
    "Minami Uwa", "Musashi",   "Naniwa", "Nankatsu SC", "Ohyama",
    "Otomo",      "Saporo FC", "Toho",
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

const char *nhl_east_teams[] = {
    // Conférence Atlantique
    "Boston Bruins", "Buffalo Sabres", "Detroit Red Wings", "Florida Panthers",
    "Montreal Canadiens", "Ottawa Senators", "Tampa Bay Lightning",
    "Toronto Maple Leafs",

    // Conférence Métropolitaine
    "Carolina Hurricanes", "Columbus Blue Jackets", "New Jersey Devils",
    "New York Islanders", "New York Rangers", "Philadelphia Flyers",
    "Pittsburgh Penguins", "Washington Capitals",

    // Nouvelles franchises 2026
    "Atlanta Thrashers"};

const tournament_t nhl_east_cup = {.title = "NHL (EST)",
                                   .teams = nhl_east_teams,
                                   .number_teams = sizeof(nhl_east_teams) /
                                                   sizeof(nhl_east_teams[0])};

const char *nhl_west_teams[] = {
    // Conférence Centrale
    "Chicago Blackhawks", "Colorado Avalanche", "Dallas Stars", "Houston Aeros",
    "Minnesota Wild", "Nashville Predators", "St. Louis Blues", "Utah Mammoth",
    "Winnipeg Jets",

    // Conférence Pacifique
    "Anaheim Ducks", "Arizona Coyotes", "Calgary Flames", "Edmonton Oilers",
    "Golden Knights de Vegas", "Los Angeles Kings", "San Jose Sharks",
    "Seattle Kraken", "Vancouver Canucks"};

const tournament_t nhl_west_cup = {.title = "NHL (OUEST)",
                                   .teams = nhl_west_teams,
                                   .number_teams = sizeof(nhl_west_teams) /
                                                   sizeof(nhl_west_teams[0])};

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
    "Bordeaux", "Lyon", "Marseille", "Nantes", "Paris", "Rodez", "Toulouse",
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

const tournament_t inazuma_eleven_cup = {
    .title = "Inazuma Eleven",
    .teams = inazuma_eleven_cup_teams,
    .number_teams =
        sizeof(inazuma_eleven_cup_teams) / sizeof(inazuma_eleven_cup_teams[0])};

// Déclaration des équipes Match sans noms
const char *sans_noms_cup_teams[] = {
    "Equipe 1",
    "Equipe 2",
};

const tournament_t sans_noms_cup = {.title = "Sans noms",
                                    .teams = sans_noms_cup_teams,
                                    .number_teams =
                                        sizeof(sans_noms_cup_teams) /
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

const tournament_t personal_cup = {
    .title = "Personnelle",
    .teams = personal_cup_teams,
    .number_teams = sizeof(personal_cup_teams) / sizeof(personal_cup_teams[0])};

const char *ao_ashi_cup_teams[] = {
    "Aomori Seiran",
    "Esperion B",
    "Funabashi Gakuin",
    "Kanto Integrated",
    "Seikyo",
    "Tokyo City Esperion",
    "Tokyo Musashino",
    "Tokyo Shurin",
    "Tokyo Vans",
};

const tournament_t ao_ashi_cup = {.title = "Ao Ashi",
                                  .teams = ao_ashi_cup_teams,
                                  .number_teams = sizeof(ao_ashi_cup_teams) /
                                                  sizeof(ao_ashi_cup_teams[0])};

// J1 League 2026
const char *j_league_cup_teams[] = {
    "Avispa Fukuoka",    "Cerezo Osaka",         "Fagiano Okayama",
    "FC Tokyo",          "Gamba Osaka",          "JEF United Chiba",
    "Kashima Antlers",   "Kashiwa Reysol",       "Kawasaki Frontale",
    "Kyoto Sanga",       "Machida Zelvia",       "Mito HollyHock",
    "Nagoya Grampus",    "Sanfrecce Hiroshima",  "Shimizu S-Pulse",
    "Tokyo Verdy",       "Urawa Red Diamonds",   "V-Varen Nagasaki",
    "Vissel Kobe",       "Yokohama F. Marinos",
};

const tournament_t j_league_cup = {.title = "J-League",
                                   .teams = j_league_cup_teams,
                                   .number_teams =
                                       sizeof(j_league_cup_teams) /
                                       sizeof(j_league_cup_teams[0])};

// Ligue 1 2026-2027
const char *ligue_1_cup_teams[] = {
    "Angers SCO",       "AJ Auxerre",        "Stade Brestois",
    "Le Havre AC",      "Le Mans FC",        "RC Lens",
    "LOSC Lille",       "FC Lorient",        "Olympique Lyonnais",
    "Olympique Marseille", "AS Monaco",      "OGC Nice",
    "Paris FC",         "Paris Saint-Germain", "Stade Rennais",
    "RC Strasbourg",    "Toulouse FC",       "ESTAC Troyes",
};

const tournament_t ligue_1_cup = {.title = "Ligue 1",
                                  .teams = ligue_1_cup_teams,
                                  .number_teams = sizeof(ligue_1_cup_teams) /
                                                  sizeof(ligue_1_cup_teams[0])};

// UCI WorldTeams 2026 (road cycling). ASCII hyphens only: the built-in LVGL
// fonts have no glyph for the en dash used in the official names.
const char *cycling_road_cup_teams[] = {
    "Alpecin-Premier Tech",
    "Decathlon CMA CGM",
    "EF Education-EasyPost",
    "Groupama-FDJ United",
    "Lidl-Trek",
    "Lotto-Intermarche",
    "Movistar Team",
    "Netcompany INEOS",
    "NSN Cycling Team",
    "Red Bull-Bora-Hansgrohe",
    "Soudal-Quick-Step",
    "Team Bahrain Victorious",
    "Team Jayco-AlUla",
    "Team Picnic-PostNL",
    "UAE Team Emirates XRG",
    "Uno-X Mobility",
    "Visma-Lease a Bike",
    "XDS Astana Team",
};

const tournament_t cycling_road_cup = {
    .title = "Velo route",
    .teams = cycling_road_cup_teams,
    .number_teams =
        sizeof(cycling_road_cup_teams) / sizeof(cycling_road_cup_teams[0])};

// Pro road cyclists, written without diacritics for the built-in LVGL fonts.
const char *cyclists_cup_teams[] = {
    "Adam Yates",         "Arnaud De Lie",       "Ben Healy",
    "Ben O'Connor",       "Biniam Girmay",       "Carlos Rodriguez",
    "Christophe Laporte", "David Gaudu",         "Dylan Groenewegen",
    "Egan Bernal",        "Enric Mas",           "Felix Gall",
    "Filippo Ganna",      "Florian Lipowitz",    "Giulio Ciccone",
    "Isaac del Toro",     "Jai Hindley",         "Jasper Philipsen",
    "Joao Almeida",       "Jonas Vingegaard",    "Jonathan Milan",
    "Juan Ayuso",         "Julian Alaphilippe",  "Kaden Groves",
    "Kevin Vauquelin",    "Lenny Martinez",      "Mads Pedersen",
    "Mathieu van der Poel", "Matteo Jorgenson",  "Mattias Skjelmose",
    "Michael Matthews",   "Mikel Landa",         "Neilson Powless",
    "Olav Kooij",         "Oscar Onley",         "Paul Seixas",
    "Primoz Roglic",      "Remco Evenepoel",     "Richard Carapaz",
    "Romain Gregoire",    "Santiago Buitrago",   "Sepp Kuss",
    "Tadej Pogacar",      "Thibau Nys",          "Tim Merlier",
    "Tim Wellens",        "Tobias Johannessen",  "Tom Pidcock",
    "Valentin Madouas",   "Wout van Aert",
};

const tournament_t cyclists_cup = {.title = "Cyclistes",
                                   .teams = cyclists_cup_teams,
                                   .number_teams =
                                       sizeof(cyclists_cup_teams) /
                                       sizeof(cyclists_cup_teams[0])};

// New cups are appended last so the cup index persisted in NVS stays valid
// for the existing entries.
const tournament_t *tournaments[] = {
    &world_cup,    &europe_cup,    &tsubasa_cup,
    &france_cup,   &mls_cup,       &nba_cup,
    &nhl_east_cup, &nhl_west_cup,  &inazuma_eleven_cup,
    &personal_cup, &sans_noms_cup, &ao_ashi_cup,
    &j_league_cup, &ligue_1_cup,   &cycling_road_cup,
    &cyclists_cup};

extern const tournament_t *tournaments[];

extern const lv_color_t teams_color[]; // Declaration

extern const uint8_t teams_color_size;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
