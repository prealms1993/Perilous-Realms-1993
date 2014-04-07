#ifndef _SPELLSH
#define _SPELLSH

/* ************************************************************************
*  file: spells.h , Implementation of magic spells.       Part of DIKUMUD *
*  Usage : Spells                                                         *
************************************************************************* */

#define MAX_BUF_LENGTH              240

#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0
#define SPELL_ARMOR                   1
#define SPELL_TELEPORT                2
#define SPELL_BLESS                   3
#define SPELL_BLINDNESS               4
#define SPELL_BURNING_HANDS           5
#define SPELL_CALL_LIGHTNING          6
#define SPELL_CHARM_PERSON            7
#define SPELL_CHILL_TOUCH             8
#define SPELL_CLONE                   9
#define SPELL_COLOUR_SPRAY           10
#define SPELL_CONTROL_WEATHER        11
#define SPELL_CREATE_FOOD            12
#define SPELL_CREATE_WATER           13
#define SPELL_CURE_BLIND             14
#define SPELL_CURE_CRITIC            15
#define SPELL_CURE_LIGHT             16
#define SPELL_CURSE                  17
#define SPELL_DETECT_EVIL            18
#define SPELL_DETECT_INVISIBLE       19
#define SPELL_DETECT_MAGIC           20
#define SPELL_DETECT_POISON          21
#define SPELL_DISPEL_EVIL            22
#define SPELL_EARTHQUAKE             23
#define SPELL_ENCHANT_WEAPON         24
#define SPELL_ENERGY_DRAIN           25
#define SPELL_FIREBALL               26
#define SPELL_HARM                   27
#define SPELL_HEAL                   28
#define SPELL_INVISIBLE              29
#define SPELL_LIGHTNING_BOLT         30
#define SPELL_LOCATE_OBJECT          31
#define SPELL_MAGIC_MISSILE          32
#define SPELL_POISON                 33
#define SPELL_PROTECT_FROM_EVIL      34
#define SPELL_REMOVE_CURSE           35
#define SPELL_SANCTUARY              36
#define SPELL_SHOCKING_GRASP         37
#define SPELL_SLEEP                  38
#define SPELL_STRENGTH               39
#define SPELL_SUMMON                 40
#define SPELL_VENTRILOQUATE          41
#define SPELL_WORD_OF_RECALL         42
#define SPELL_REMOVE_POISON          43
#define SPELL_SENSE_LIFE             44
#define SPELL_BEACON                 45
#define SPELL_DELAYED_TELEPORT	     46
#define SPELL_NOVA                   47
#define SPELL_PHASE                  48
#define SPELL_RESTORE                49
/* 47- 52 to be used */
/* NEW SPELLS are to be inserted here */
#define SPELL_IDENTIFY               53
#define SPELL_INFRAVISION            54
#define SPELL_CAUSE_LIGHT            55
#define SPELL_CAUSE_CRITICAL         56
#define SPELL_FLAMESTRIKE            57
#define SPELL_DISPEL_GOOD            58
#define SPELL_WEAKNESS               59
#define SPELL_DISPEL_MAGIC           60
#define SPELL_KNOCK                  61
#define SPELL_KNOW_ALIGNMENT         62
#define SPELL_ANIMATE_DEAD           63
#define SPELL_PARALYSIS              64
#define SPELL_REMOVE_PARALYSIS       65
#define SPELL_FEAR                   66
#define SPELL_ACID_BLAST             67
#define SPELL_WATER_BREATH           68
#define SPELL_FLY                    69
#define SPELL_CONE_OF_COLD           70
#define SPELL_METEOR_SWARM           71
#define SPELL_ICE_STORM              72
#define SPELL_SHIELD                 73
#define SPELL_MON_SUM_1              74  /* done */
#define SPELL_MON_SUM_2              75
#define SPELL_MON_SUM_3              76
#define SPELL_MON_SUM_4              77
#define SPELL_MON_SUM_5              78
#define SPELL_MON_SUM_6              79
#define SPELL_MON_SUM_7              80  /* done  */
#define SPELL_FIRESHIELD             81
#define SPELL_CHARM_MONSTER          82 
#define SPELL_CURE_SERIOUS           83
#define SPELL_CAUSE_SERIOUS          84
#define SPELL_REFRESH                85
#define SPELL_SECOND_WIND            86
#define SPELL_TURN                   87
#define SPELL_SUCCOR                 88
#define SPELL_LIGHT                  89
#define SPELL_CONT_LIGHT             90
#define SPELL_CALM                   91
#define SPELL_STONE_SKIN             92
#define SPELL_CONJURE_ELEMENTAL      93
#define SPELL_TRUE_SIGHT             94
#define SPELL_MINOR_CREATE           95
#define SPELL_FAERIE_FIRE            96
#define SPELL_FAERIE_FOG             97
#define SPELL_CACAODEMON             98
#define SPELL_POLY_SELF              99

#define SPELL_MANA                  100
#define SPELL_ASTRAL_WALK           101
#define SPELL_PWORD_BLIND           102
#define SPELL_SUMMON_OBJ            103
#define SPELL_FAMILIAR              104
/* #define SPELL_CHAIN_LIGHTNING       105 */
#define SPELL_SCARE                 106
#define SPELL_SENDING               107
#define SPELL_CHAIN_LIGHTNING       108
#define SPELL_SLOW_POISON           109


#define SPELL_AID                   110
#define SPELL_GOLEM                 111
#define SPELL_COMMAND               112
#define SPELL_INVULNERABLE          113
#define SPELL_GUST_OF_WIND          114
#define SPELL_WARP_WEAPON           115
#define SPELL_PWORD_KILL            116
#define SPELL_MYSTIC_HEAL           117
#define SPELL_PROTECT_ELEM_FIRE     118


#define SPELL_PORTAL            119
#define SPELL_SHRIEK		120
#define SPELL_DRAIN_V_LIGHT	121
#define SPELL_DRAIN_V_SERIOUS	122
#define SPELL_DRAIN_V_CRITIC	123
#define SPELL_WITHER		124
#define SPELL_SUM_DEMON		125
#define SPELL_FURY		126
#define SPELL_SUM_ELEM_II	127
#define SPELL_MAGE_FIRE		128
#define SPELL_FIRESTORM		129
#define SPELL_FULL_HARM		130
#define SPELL_HOLY_WORD		131
#define SPELL_UNHOLY_WORD	132
#define SPELL_TRANSLOOK		133
#define SPELL_TELELOOK		134
#define SPELL_TELEIMAGE		135
#define SPELL_TRANSMOVE		136
#define SPELL_TELEMOVE		137
#define SPELL_VIT_MANA		138
#define SPELL_REJUVENATE	139
#define SPELL_AGE		140
#define SPELL_AGEING		141
#define SPELL_HASTE		142
#define SPELL_G_INVIS		143
#define SPELL_HERO_FEAST	144
#define SPELL_F_HEAL		145
#define SPELL_SCRY		146
#define SPELL_NOSLEEP		147
#define SPELL_NOSUMMON		148
#define SPELL_DEXTERITY		149
#define SPELL_NOCHARM		150
#define SPELL_SPELL_HASTE	151
#define SPELL_RECHARGER		152
#define SPELL_CHILLY		153
#define SPELL_SUNRAY		154
#define SPELL_METALSKIN		155
#define SPELL_VAMPIRIC_TOUCH	156
#define SPELL_VIG_LIGHT		157
#define SPELL_VIG_SERIOUS	158
#define SPELL_VIG_CRIT		159
#define SPELL_VITALITY		160
#define SPELL_FARSEE		161
#define SPELL_HEROISM		162
#define SPELL_ICE_LANCE		163
#define SPELL_FREEZE		164
#define SPELL_FIRE_BOLT		165
#define SPELL_REPULSOR		166
#define SPELL_MIND_THRUST	167
#define SPELL_DISRUPTOR		168
#define SPELL_DISPEL_INVIS	169
#define SPELL_NOFEAR		170
#define SPELL_PROTECT_ELEM_COLD 174

/*#define SKILL_SCAN		171
#define SKILL_BATTER            172
#define SKILL_DOORBASH          173

#define SKILL_HUNT              180
#define SKILL_FIND_TRAP         181
#define SKILL_SET_TRAP          182
#define SKILL_DISARM            183
#define SKILL_READ_MAGIC        184
#define SKILL_BERSERK		185
#define SKILL_SUBTERFUGE	186
#define SKILL_HITALL		187
#define SKILL_ASSASINATE	188
#define SKILL_APPRAISE		189
#define SKILL_XATTACK1		190
#define SKILL_XATTACK2		191
#define SKILL_XATTACK3		192
#define SKILL_XATTACK4		193
#define SKILL_XATTACK5		194
#define SKILL_XATTACK6		195
#define SKILL_XATTACK7		196
#define SKILL_XATTACK8		197
#define SKILL_XATTACK9		198
*/
#define SPELL_DRAGON_BREATH          105
#define SPELL_GREEN_SLIME            199

#define FIRST_BREATH_WEAPON	     200
#define SPELL_GEYSER                 200
#define SPELL_FIRE_BREATH            201
#define SPELL_GAS_BREATH             202
#define SPELL_FROST_BREATH           203
#define SPELL_ACID_BREATH            204
#define SPELL_LIGHTNING_BREATH       205
#define LAST_BREATH_WEAPON	     205
#define SPELL_HAVEN                  206
#define MAX_EXIST_SPELL             207  /* move this and change it */


#define TYPE_HIT                     306
#define TYPE_BLUDGEON                307
#define TYPE_PIERCE                  308
#define TYPE_SLASH                   309
#define TYPE_WHIP                    310 /* EXAMPLE */
#define TYPE_CLAW                    311  /* NO MESSAGES WRITTEN YET! */
#define TYPE_BITE                    312  /* NO MESSAGES WRITTEN YET! */
#define TYPE_STING                   313  /* NO MESSAGES WRITTEN YET! */
#define TYPE_CRUSH                   314  /* NO MESSAGES WRITTEN YET! */
#define TYPE_CLEAVE                  315
#define TYPE_STAB                    316
#define TYPE_SMASH                   317
#define TYPE_SMITE                   318

#define TYPE_SUFFERING               320

# define ROOM_FIRE		321
# define ROOM_COLD		322
# define ROOM_WATER		323
# define ROOM_AIRLESS		324
# define ROOM_GAS		325
# define ROOM_UNUSED1		326
# define ROOM_UNUSED2		327
# define ROOM_UNUSED3		328
# define ROOM_UNUSED4		329
# define ROOM_UNUSED5		330

/* More anything but spells and weapontypes can be insterted here! */





#define MAX_TYPES 70

#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4


#define MAX_SPL_LIST	215


#define TAR_IGNORE	 (1<< 0)
#define TAR_CHAR_ROOM	 (1<< 1)
#define TAR_CHAR_WORLD	 (1<< 2)
#define TAR_FIGHT_SELF	 (1<< 3)
#define TAR_FIGHT_VICT	 (1<< 4)
#define TAR_SELF_ONLY	 (1<< 5) /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_NONO	 (1<< 6) /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_OBJ_INV	 (1<< 7)
#define TAR_OBJ_ROOM	 (1<< 8)
#define TAR_OBJ_WORLD	 (1<< 9)
#define TAR_OBJ_EQUIP	 (1<<10)
#define TAR_NAME	 (1<<11)
#define TAR_VIOLENT	 (1<<12)
#define TAR_ROOM         (1<<13)  /* spells which target the room  */

#define SPLF_VERBAL   1
#define SPLF_GESTURES 2

struct spell_info_type
{
	byte beats;             /* Heartbeats until ready for next */
	byte minimum_position;  /* Position for caster 			*/
        int  flags;             /* flags */
	sh_int targets;         /* See below for use with TAR_XXX  */
	void (*spell_pointer) \
	      (int level, struct char_data *ch, char *arg, int type,
	      struct char_data *tar_ch, struct obj_data *tar_obj,int eff_level);
};

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4


/* Attacktypes with grammar */

struct attack_hit_type {
  char *singular;
  char *plural;
};

#define SKILL_SNEAK             10001
#define SKILL_HIDE              10002
#define SKILL_STEAL             10003
#define SKILL_BACKSTAB          10004
#define SKILL_PICK_LOCK         10005
#define SKILL_KICK              10006
#define SKILL_BASH              10007
#define SKILL_RESCUE            10008
#define SKILL_SCAN              10009
#define SKILL_BATTER            10010
#define SKILL_DOORBASH          10011
#define SKILL_HUNT              10012
#define SKILL_FIND_TRAP         10013
#define SKILL_SET_TRAP          10014
#define SKILL_DISARM            10015
#define SKILL_READ_MAGIC        10016
#define SKILL_BERSERK           10017
#define SKILL_SUBTERFUGE        10018
#define SKILL_HITALL            10019
#define SKILL_ASSASINATE        10020
#define SKILL_APPRAISE          10021
#define SKILL_XATTACK1          10022
#define SKILL_XATTACK2          10023
#define SKILL_XATTACK3          10024
#define SKILL_XATTACK4          10025
#define SKILL_XATTACK5          10026
#define SKILL_XATTACK6          10027
#define SKILL_XATTACK7          10028
#define SKILL_XATTACK8          10029
#define SKILL_XATTACK9          10030
#define SKILL_BACKSTAB1		10031
#define SKILL_BACKSTAB10	10040
#define SKILL_ASSASINATE2	10041
#define SKILL_ASSASINATE15	10054
#define SKILL_RETREAT		10055
#define SKILL_DUAL_WIELD        10056
#define SKILL_EXAMINE_WARD      10057
#define SKILL_AID               10058

#define PROF_BASE               5000

#endif
