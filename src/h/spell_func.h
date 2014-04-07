#ifndef _SPELLFUNCH
#define _SPELLFUNCH

#define CR TAR_CHAR_ROOM
#define OI TAR_OBJ_INV
#define OE TAR_OBJ_EQUIP
#define FV TAR_FIGHT_VICT
#define VI TAR_VIOLENT
#define IG TAR_IGNORE
#define SN TAR_SELF_NONO
#define NA TAR_NAME
#define CW TAR_CHAR_WORLD
#define SO TAR_SELF_ONLY
#define OR TAR_OBJ_ROOM
#define OW TAR_OBJ_WORLD

#define DEA POSITION_DEAD
#define MOR POSITION_MORTALLYW
#define INC POSITION_INCAP
#define STU POSITION_STUNNED
#define SLE POSITION_SLEEPING
#define RES POSITION_RESTING
#define FIG POSITION_FIGHTING
#define STA POSITION_STANDING
#define SIT POSITION_SITTING

#define V SPLF_VERBAL
#define G SPLF_GESTURES

/* 1 beat = 1/4 combat round */

struct spell_info_type spell_info[] = {
/*      beats pos target            func */
/* 0 */ {  0, STA, V | G, IG,                NULL },
/* 1 */ {  4, STA, V | G, CR,                cast_armor },
/* 2 */ {  4, FIG, V | G, CR | FV,           cast_teleport},
/* 3 */ {  4, STA, V | G, OI | OE | CR,      cast_bless},
/* 4 */ {  8, FIG, V | G, CR | FV | VI,      cast_blindness},
/* 5 */ {  8, FIG, V | G, IG | VI,           cast_burning_hands},
/* 6 */ { 12, FIG, V | G, CR | FV | VI,      cast_call_lightning},
/* 7 */ {  4, STA, V | G, CR | SN | FV | VI, cast_charm_person},
/* 8 */ {  4, FIG, V,  CR | FV | VI,      cast_chill_touch},
/* 9 */ {  4, STA, V | G, CR,                cast_clone},
/* 10*/ {  8, FIG, G,  CR | FV | VI,      cast_colour_spray},
/* 11*/ { 12, STA, V | G, IG,                cast_control_weather},
/* 12*/ {  4, STA, V | G, IG,                cast_create_food},
/* 13*/ {  4, STA, V | G, OI | OE,           cast_create_water},
/* 14*/ {  4, STA, V | G, CR,                cast_cure_blind},
/* 15*/ {  8, FIG, V | G, CR,                cast_cure_critic},
/* 16*/ {  4, FIG, V | G, CR,                cast_cure_light},
/* 17*/ {  8, STA, V | G, CR | OR | OI | OE | FV | VI, cast_curse},
/* 18*/ {  4, STA, V | G, CR, 		  cast_detect_evil},
/* 19*/ {  4, STA, V | G, CR,                cast_detect_invisibility},
/* 20*/ {  4, STA, V | G, CR, 	          cast_detect_magic},
/* 21*/ {  4, STA, V | G, CR | OI | OE,      cast_detect_poison},
/* 22*/ {  8, FIG, V | G, CR | FV, 	  cast_dispel_evil},
/* 23*/ {  8, FIG, V | G, IG | VI,           cast_earthquake},
/* 24*/ { 16, STA, V | G, OI | OE,           cast_enchant_weapon},
/* 25*/ { 12, FIG, V | G, CR | FV | VI,      cast_energy_drain},
/* 26*/ { 12, FIG, V | G, IG | VI,           cast_fireball},
/* 27*/ { 12, FIG, V | G, CR | FV | VI,      cast_harm},
/* 28*/ {  4, FIG, V | G, CR,                cast_heal},
/* 29*/ {  4, STA, G,  CR | OI | OR | OE, cast_invisibility},
/* 30*/ {  8, FIG, V | G, CR | FV | VI,      cast_lightning_bolt},
/* 31*/ {  4, STA, V | G, OW,           NULL /*cast_locate_object*/},
/* 32*/ {  4, FIG, V | G, CR | FV | VI,      cast_magic_missile},
/* 33*/ {  8, FIG, V | G, CR | SN | OI | OE | FV | VI, cast_poison},
/* 34*/ {  4, STA, V | G, CR,                cast_protection_from_evil},
/* 35*/ {  4, STA, V | G, CR | OI | OE | OR, cast_remove_curse},
/* 36*/ { 12, STA, V | G, CR,                cast_sanctuary},
/* 37*/ {  4, FIG, V | G, CR | FV | VI,      cast_shocking_grasp},
/* 38*/ {  8, STA, V | G, CR | FV,           cast_sleep},
/* 39*/ {  4, STA, V | G, CR,                cast_strength},
/* 40*/ { 12, STA, V | G, CW,                cast_summon},
/* 41*/ {  4, STA, G,  CR | OR | SN,      cast_ventriloquate},
/* 42*/ {  4, FIG, V,  CR | SO,           cast_word_of_recall},
/* 43*/ {  4, STA, V | G, CR | OI | OR,      cast_remove_poison},
/* 44*/ {  4, STA, V | G, CR,                cast_sense_life},
/* 45*/ {  4, STA, V | G, IG,                cast_beacon},
/* 46*/ {  4, STA, V | G, CR | FV,           cast_delayed_teleport},
/* 47*/ {  8, STA, V | G, IG | VI,           cast_nova},
/* 48*/ {  8, STA, V | G, IG | VI,           NULL},
/* 49*/ {  8, STA, V | G, IG | VI,           NULL},
/* 50*/ {  0, STA, V | G, IG,                NULL},
/* 51*/ {  0, STA, V | G, IG,                NULL},
/* 52*/ {  0, STA, V | G, IG,                NULL},
/* 53*/ {  1, STA, V | G, IG,                cast_identify},
/* 54*/ {  4, STA, V | G, CR,                cast_infravision},
/* 55*/ {  4, FIG, V | G, CR | FV | VI,      cast_cause_light},
/* 56*/ {  8, FIG, V | G, CR | FV | VI,      cast_cause_critic},
/* 57*/ { 12, FIG, V | G, CR | FV | VI,      cast_flamestrike},
/* 58*/ { 12, FIG, V | G, CR | FV,           cast_dispel_good},
/* 59*/ {  4, FIG, V | G, CR | FV | VI,      cast_weakness},
/* 60*/ {  4, FIG, V | G, CR | FV,           cast_dispel_magic},
/* 61*/ {  4, STA, V | G, IG,                cast_knock},
/* 62*/ {  4, FIG, G,  CR | FV,           cast_know_alignment},
/* 63*/ {  8, STA, V | G, OR,                cast_animate_dead},
/* 64*/ { 12, FIG, V | G, CR | FV | VI,      cast_paralyze},
/* 65*/ {  4, FIG, V | G, CR | FV,           cast_remove_paralysis},
/* 66*/ {  4, FIG, V | G, CR | FV | VI,      cast_fear},
/* 67*/ {  8, FIG, G,  CR | FV | VI,      cast_acid_blast},
/* 68*/ {  4, FIG, G,  CR,                cast_water_breath},
/* 69*/ {  4, FIG, V | G, CR,                cast_flying},
/* 70*/ {  8, FIG, G,  IG | VI,           cast_cone_of_cold},
/* 71*/ {  8, FIG, V | G, CR | FV | VI,      cast_meteor_swarm},
/* 72*/ { 12, FIG, V | G, IG | VI,           cast_ice_storm},
/* 73*/ {  8, FIG, V | G, CR,                cast_shield},
/* 74*/ {  8, FIG, V | G, IG,                cast_mon_sum1},
/* 75*/ {  8, FIG, V | G, IG,                cast_mon_sum2},
/* 76*/ {  8, FIG, V | G, IG,                cast_mon_sum3},
/* 77*/ {  8, FIG, V | G, IG,                cast_mon_sum4},
/* 78*/ {  8, FIG, V | G, IG,                cast_mon_sum5},
/* 79*/ {  8, FIG, V | G, IG,                cast_mon_sum6},
/* 80*/ {  8, STA, V | G, IG,                cast_mon_sum7},
/* 81*/ {  8, STA, V | G, CR | SO,           cast_fireshield},
/* 82*/ { 12, STA, V | G, CR | VI,           cast_charm_monster},
/* 83*/ {  4, FIG, V | G, CR,                cast_cure_serious},
/* 84*/ {  4, FIG, V | G, CR | VI,           cast_cause_serious},
/* 85*/ {  4, STA, V | G, CR,                cast_refresh},
/* 86*/ {  4, STA, V | G, CR,                cast_second_wind},
/* 87*/ {  4, STA, V | G, CR,                cast_turn},
/* 88*/ {  8, STA, V | G, IG,                cast_succor},
/* 89*/ {  4, STA, G,  IG,                cast_light},
/* 90*/ {  8, STA, G,  IG,                cast_cont_light},
/* 91*/ {  8, STA, V | G, CR,                cast_calm},
/* 92*/ {  8, STA, V | G, SO,                cast_stone_skin},
/* 93*/ {  8, STA, V | G, IG,                cast_conjure_elemental},
/* 94*/ {  8, STA, V | G, CR,                cast_true_seeing},
/* 95*/ {  8, STA, V | G, IG,                cast_minor_creation},
/* 96*/ {  4, STA, G,  CR | SN | VI,      cast_faerie_fire},
/* 97*/ {  8, STA, G,  IG,                cast_faerie_fog},
/* 98*/ {  8, STA, V | G, IG,                cast_cacaodemon},
/* 99*/ {  4, FIG, G,  IG,                cast_poly_self},
/*100*/ {  4, FIG, V | G, IG,                cast_mana},
/*101*/ {  4, STA, V | G, CW,                NULL /*cast_astral_walk*/},
/*102*/ { 12, FIG, V,  CR | FV | VI,      cast_pword_blind},
/*103*/ {  0, STA, V | G, IG,                NULL},
/*104*/ {  8, STA, V | G, IG,                cast_familiar},
/*105*/ {  0, STA, V | G, IG,                NULL},
/*106*/ {  8, FIG, V | G, CR | FV | VI,      cast_scare},
/*107*/ {  0, STA, V | G, IG,                NULL},
/*108*/ { 12, FIG, V | G, CR | FV | VI,      cast_chain_lightn },
/*109*/ {  4, STA, V | G, CR,                cast_slow_poison},
/*110*/ {  8, FIG, V | G, CR,                cast_aid},
/*111*/ {  0, STA, V | G, IG,                NULL},
/*112*/ {  0, STA, V | G, IG,                NULL},
/*113*/ {  4, STA, V | G, CR | SO,           cast_invulnerability},
/*114*/ {  8, SIT, G,  IG,                cast_gust_of_wind},
/*115*/ {  4, STA, V | G, CR | FV | VI | OR | OI, cast_warp_weapon},
/*116*/ { 12, STA, V,  CR | FV | VI,      cast_pword_kill},
/*117*/ { 12, STA, V | G, CR,                cast_mystic_heal},
/*118*/ { 12, FIG, V | G, CR,                cast_protect_elem_fire},
/*119*/ { 12, STA, V | G, CW,                cast_portal},
/*120*/ { 12, FIG, V | G, CR | FV,           cast_shriek},
/*121*/ { 12, STA, V | G, CR | FV,           cast_drain_vigor_light},
/*122*/ { 12, STA, V | G, CR | FV,           cast_drain_vigor_serious},
/*123*/ { 12, STA, V | G, CR | FV,           cast_drain_vigor_critic},
/*124*/ { 12, FIG, V | G, CR | FV | VI,      cast_wither},
/*125*/ { 12, STA, V | G, IG,                cast_summon_demon},
/*126*/ { 12, STA, V | G, CR,                cast_fury},
/*127*/ { 12, STA, V | G, IG,                cast_summon_elemental_ii},
/*128*/ { 12, FIG, V | G, CR | FV,           cast_mage_fire},
/*129*/ { 12, FIG, V | G, IG,                cast_firestorm},
/*130*/ { 12, FIG, V | G, CR | FV,           cast_full_harm},
/*131*/ { 12, FIG, V,  CR | FV,           cast_holy_word},
/*132*/ { 12, FIG, V,  CR | FV,           cast_unholy_word},
/*133*/ { 12, STA, V | G, CW,                cast_translook},
/*134*/ { 12, STA, V | G, CW,                cast_telelook},
/*135*/ { 12, STA, V | G, CW,                cast_teleimage},
/*136*/ { 12, STA, V | G, CW,                cast_transmove},
/*137*/ { 12, STA, V | G, CW,                cast_telemove},
/*138*/ { 12, STA, V | G, CR | SO,           cast_vitalize_mana},
/*139*/ { 12, STA, V | G, CR,                cast_rejuvenate},
/*140*/ { 12, FIG, V | G, CR,                cast_age},
/*141*/ {  4, STA, V | G, CR,                cast_ageing}, /* may not work */
/*142*/ {  4, FIG, V | G, CR,                cast_haste},
	{  0, STA, V | G, IG,                NULL},
	{  0, STA, V | G, IG,                NULL},
#if 0
/*143*/ {  4, STA, V | G, CR,                cast_invis_group}, /* prob wont work */
/*144*/ {  4, STA, V | G, CR,                cast_heroes_feast}, /* may not work */
#endif
/*145*/ { 12, STA, V | G, CR,                cast_full_heal},
/*146*/ { 12, STA, V | G, CR,                cast_scry},
/*147*/ {  4, STA, V | G, CR,                cast_nosleep},
/*148*/ {  4, STA, V | G, CR,                cast_nosummon},
/*149*/ {  4, STA, V | G, CR,                cast_dexterity},
/*150*/ {  4, STA, G,  CR,                cast_nocharm},
/*151*/ {  0, STA, V | G, IG,                NULL},
/*152*/ {  4, STA, V | G, CR,                cast_recharger},
/*153*/ { 12, STA, V | G, CR | FV,           cast_chilly},
/*154*/ { 12, STA, V | G, CR | FV,           cast_sunray},
/*155*/ { 12, STA, V | G, CR | SO,           cast_metalskin},
/*156*/ { 12, STA, V | G, CR,                cast_vampiric_touch},
/*157*/ { 12, FIG, V | G, CR,                cast_vigorize_light},
/*158*/ { 12, FIG, V | G, CR,                cast_vigorize_serious},
/*159*/ { 12, FIG, V | G, CR,                cast_vigorize_critic},
/*160*/ { 12, FIG, V | G, CR | SO,           cast_vitality},
/*161*/ { 12, STA, V | G, CR | SO,           cast_farsee},
/*162*/ { 12, STA, V | G, CR,                cast_heroism},
/*163*/ { 12, FIG, V | G, CR | FV,           cast_ice_lance},
/*164*/ { 12, FIG, V | G, CR | FV,           cast_freeze},
/*165*/ { 12, FIG, V | G, CR | FV,           cast_fire_bolt},
/*166*/ { 12, FIG, V | G, CR | FV,           cast_repulsor},
/*167*/ { 12, FIG, V | G, CR | FV,           cast_mind_thrust},
/*168*/ { 12, FIG, V | G, CR | FV,           cast_disruptor},
/*169*/ {  4, STA, V | G, CR | OI | OR | OE, cast_dispel_invisible},
#if 0
/*170*/ {  4, STA, V | G, CR,                cast_nofear}, /* may not work */
#endif
        {  0, STA, V | G, IG,                NULL},
/*171*/ {  0, STA, V | G, IG,                NULL},
/*172*/ {  0, STA, V | G, IG,                NULL},
/*173*/ {  0, STA, V | G, IG,                NULL},
/*174*/ {  4, FIG, V | G, CR,                cast_protect_elem_cold},
/*175*/ {  0, STA, V | G, IG,                NULL},
/*176*/ {  0, STA, V | G, IG,                NULL},
/*177*/ {  0, STA, V | G, IG,                NULL},
/*178*/ {  0, STA, V | G, IG,                NULL},
/*179*/ {  0, STA, V | G, IG,                NULL},
/*180*/ {  0, STA, V | G, IG,                NULL}, /* skill hunt */
/*181*/ {  0, STA, V | G, IG,           NULL}, /* skill find trap */
/*182*/ {  0, STA, V | G, IG,           NULL}, /* skill set trap */
/*183*/ {  0, STA, V | G, IG,           NULL}, /* skill disarm */
/*184*/ {  0, STA, V | G, IG,           NULL}, /* skill read magic */
/*185*/ {  0, STA, V | G, IG,           NULL}, /* skill berserk */
/*186*/ {  0, STA, V | G, IG,           NULL}, /* skill subterfuge */
/*187*/ {  0, STA, V | G, IG,           NULL}, /* skill hitall */
/*188*/ {  0, STA, V | G, IG,           NULL}, /* skill assasinate */
/*189*/ {  0, STA, V | G, IG,           NULL}, /* skill appraise */
/*190*/ {  0, STA, V | G, IG,                NULL},
/*191*/ {  0, STA, V | G, IG,                NULL},
/*192*/ {  0, STA, V | G, IG,                NULL},
/*193*/ {  0, STA, V | G, IG,                NULL},
/*194*/ {  0, STA, V | G, IG,                NULL},
/*195*/ {  0, STA, V | G, IG,                NULL},
/*196*/ {  0, STA, V | G, IG,                NULL},
/*197*/ {  0, STA, V | G, IG,                NULL},
/*198*/ {  0, STA, V | G, IG,                NULL},
/*199*/ {  0, STA, V | G, IG,                NULL},
/*200*/ {  0, STA, V | G, IG,                NULL},
/*201*/ {  0, STA, V | G, IG,                NULL},
/*202*/ {  0, STA, V | G, IG,                NULL},
/*203*/ {  0, STA, V | G, IG,                NULL},
/*204*/ {  0, STA, V | G, IG,                NULL},
/*205*/ {  0, STA, V | G, IG,                NULL},
/*206*/ {  0, STA, V | G, SO,                cast_haven}, /* song Haven */
};

#endif
