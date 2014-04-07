
char *songs[]=
{
   "haven",
   "\n",
};

char *spells[]=
{
   "none",
   "armor",               /* 1 */
   "teleport",
   "bless",
   "blindness",
   "burning hands",
   "call lightning",
   "charm person",
   "chill touch",
   "clone",
   "colour spray",
   "control weather",     /* 11 */
   "create food",
   "create water",
   "cure blind",
   "cure critic",
   "cure light",
   "curse",
   "detect evil",
   "detect invisibility",
   "detect magic",
   "detect poison",       /* 21 */
   "dispel evil",
   "earthquake",
   "enchant weapon",
   "energy drain",
   "fireball",
   "harm",
   "heal",
   "invisibility",
   "lightning bolt",
   "locate object",      /* 31 */
   "magic missile",
   "poison",
   "protection from evil",
   "remove curse",
   "sanctuary",
   "shocking grasp",
   "sleep",
   "strength",
   "summon",
   "ventriloquate",      /* 41 */
   "word of recall",
   "remove poison",
   "sense life",         /* 44 */
   "beacon",
   "delayed teleport",
   "nova",
   "phase",
   "restore",
   "*50",
   "*51",
   "*52", 
/* NON-CASTABLE SPELLS (Scrolls/potions/wands/staffs) */
   "identify",           /* 53 */
   "infravision",        
   "cause light",        
   "cause critical",
   "flamestrike",
   "dispel good",      
   "weakness",
   "dispel magic",
   "knock",
   "know alignment",
   "animate dead",
   "paralyze",
   "remove paralysis",
   "fear",
   "acid blast",  /* 67 */
   "water breath",
   "fly",
   "cone of cold",   /* 70 */
   "meteor swarm",
   "ice storm",
   "shield",
   "monsum one",
   "monsum two",
   "monsum three",
   "monsum four",
   "monsum five",
   "monsum six",
   "monsum seven",  /* 80 */
   "fireshield",
   "charm monster",
   "cure serious",
   "cause serious",
   "refresh",
   "second wind",
   "turn",
   "succor",
   "create light",
   "continual light",	/* 90 */
   "calm",
   "stone skin",
   "conjure elemental",
   "true sight",
   "minor creation",
   "faerie fire",
   "faerie fog",
   "cacaodemon",
   "polymorph self",
   "mana",	/* 100 */
   "astral walk",
   "pword blind",
   "summon object",   
   "find familiar",
   "*105",  /* dfned as dragon breath ALSO spells.h farther dwn */
   "scare",
   "sending",
   "chain lightning",
   "slow poison",
   "aid",	/* 110 */
   "golem",
   "command",
   "invulnerability",   /* First try at this... hope it works */
   "gust of wind",
   "warp weapon",
   "pword kill",
   "mystic heal",
   "protect elemental fire",
   "portal",
   "shriek",	/* 120 */
   "drain vigor light",
   "drain vigor serious",
   "drain vigor critic",
   "wither",
   "summon demon",
   "fury",
   "summon elemental ii",
   "mage fire",
   "firestorm",
   "full harm",	/* 130 */
   "holy word",
   "unholy word",
   "translook",
   "telelook",
   "teleimage",
   "transmove",
   "telemove",
   "vitilize mana",
   "rejuvenate",
   "age",	/* 140 */
   "ageing",
   "haste",
   "group invisibility",
   "heroes feast",
   "full heal",
   "scry",
   "nosleep",
   "nosummon",
   "dexterity",
   "nocharm",	/* 150 */
   "*151",
   "recharger",
   "chilly",
   "sunray",
   "metalskin",
   "vampiric touch",
   "vigorize light",
   "vigorize serious",
   "vigorize critic",
   "vitality",	/* 160 */
   "farsee",
   "heroism",
   "ice lance",
   "freeze",
   "fire bolt",
   "repulsor",
   "mind thrust",
   "disruptor",
   "dispel invisibility",
   "nofear",	/* 170 */
   "*scan",
   "*batter",
   "*doorbash",
   "**174",
   "**175",
   "**176",
   "**177",
   "**178",
   "**179",
   "*track",
   "*find trap",
   "*set trap",
   "*disarm",
   "*read magic",
   "*berserk",
   "*subterfuge",
   "*hitall",
   "*assasinate",
   "*appraise",
   "*extra attack one",	
   "*extra attack two",
   "*extra attack three",
   "*extra attack four",
   "*extra attack five",
   "*extra attack six",
   "*extra attack seven",
   "*extra attack eight",
   "*extra attack nine",  
   "green slime",
   "geyser",	/* 200 */ /* supposed first breath weapon (NOT!) */
   "fire breath",
   "gas breath",
   "frost breath",
   "acid breath",
   "lightning breath", /* 205 */  /* supposed last breath weapon (MAYBE) */
   "haven",
   "*207",    /* the dweebs put weapon hit types starting here...ack */
   "*208",
   "*209",
   "*210",
   "*211",
   "*212",
   "\n"
};

char *skills[]=
{  "none",                /* 0 */
   "sneak",               /* 1 */
   "hide",                /* 2 */
   "steal",               /* 3 */
   "backstab",            /* 4 */
   "pick",                /* 5 */
   "kick",                /* 6 */
   "bash",                /* 7 */
   "rescue",              /* 8 */
   "scan",                /* 9 */
   "batter",              /* 10 */
   "doorbash",            /* 11 */
   "track",               /* 12 */
   "find trap",           /* 13 */
   "set trap",            /* 14 */
   "disarm",              /* 15 */
   "read magic",          /* 16 */
   "berserk",             /* 17 */
   "subterfuge",          /* 18 */
   "hitall",              /* 19 */
   "assasinate one",      /* 20 */
   "appraise",            /* 21 */
   "extra attack one",    /* 22 */
   "extra attack two",    /* 23 */
   "extra attack three",  /* 24 */
   "extra attack four",   /* 25 */
   "extra attack five",   /* 26 */
   "extra attack six",    /* 27 */
   "extra attack seven",  /* 28 */
   "extra attack eight",  /* 29 */
   "extra attack nine",   /* 30 */
   "backstab one",        /* 31 */
   "backstab two",        /* 32 */
   "backstab three",      /* 33 */
   "backstab four",       /* 34 */
   "backstab five",       /* 35 */
   "backstab six",        /* 36 */
   "backstab seven",      /* 37 */
   "backstab eight",      /* 38 */
   "backstab nine",       /* 39 */
   "backstab ten",        /* 40 */
   "assasinate two",      /* 41 */
   "assasinate three",    /* 42 */
   "assasinate four",     /* 43 */
   "assasinate five",     /* 44 */
   "assasinate six",      /* 45 */
   "assasinate seven",    /* 46 */
   "assasinate eight",    /* 47 */
   "assasinate nine",     /* 48 */
   "assasinate ten",      /* 49 */
   "assasinate eleven",   /* 50 */
   "assasinate twelve",   /* 51 */
   "assasinate thirteen", /* 52 */
   "assasinate fourteen", /* 53 */
   "assasinate fifteen",  /* 54 */
   "retreat",		  /* 55 */
   "dual wield",          /* 56 */
   "examine wards",       /* 57 */
   "aid",                 /* 58 */
   "\n"
};
