/* ************************************************************************
*  file: utils.h, Utility module.                         Part of DIKUMUD *
*  Usage: Utility macros                                                  *
************************************************************************* */

#if __x86_64__
#error "64-Bit is not supported as pointers are cast to/from int's in a couple of places.  Compile this as a 32-bit app."
#endif

#define GetMaxLevel(ch) (ch->player.level)
#define GET_LEVEL(ch) (ch->player.level)

int CAN_SEE(struct char_data *s, struct char_data *o);

#define TRUE  1

#define FALSE 0

#define IS_DIGIT(c) (((c)>='0') && ((c)<='9'))

#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))

#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r') 

#define IF_STR(st) ((st) ? (st) : "\0")

#define CAP(st)  (*(st) = UPPER(*(st)), st)
#define IS_SET(flag,bit)  ((flag) & (bit))
#define IS_SET1(flag,bitv)  (((flag) & (bitv))==bitv)

#define SWITCH(a,b) { (a) ^= (b); \
                      (b) ^= (a); \
                      (a) ^= (b); }

#define is_set(v,n) ((v[n/32] & (1<<(0x1f&n)))!=0)
#define set_bit(v,n) (v[n/32] |= (1<<(0x1f&n)))
#define clear_bit(v,n) (v[n/32] -= (v[n/32] & (1<<(0x1f&n))))
#define init_bits(v) { int i; for (i=0; i<8; i++) v[i]=0L; }
#define init_set(v,n) { init_bits(v); set_bit(v,n); }

#define IS_AFFECTED(ch,skill) (is_set(((ch)->specials.affected_by), (skill)) )
#define IS_AWAY(ch) (ch->specials.away)

#define IS_DARK(room)  (!real_roomp(room)->light && (IS_SET(real_roomp(room)->room_flags, DARK) || real_roomp(room)->dark))

#define IS_LIGHT(room)  (real_roomp(room)->light || !(IS_SET(real_roomp(room)->room_flags, DARK) || real_roomp(room)->dark))

#define SET_BIT(var,bit)  ((var) = (var) | (bit))

#define REMOVE_BIT(var,bit)  ((var) = (var) & ~(bit) )

#define RM_FLAGS(i)  ((real_roomp(i))?real_roomp(i)->room_flags:0)

#define GET_REQ(i) (i<2  ? "Awful" :(i<4  ? "Bad"     :(i<7  ? "Poor"      :\
(i<10 ? "Average" :(i<14 ? "Fair"    :(i<20 ? "Good"    :(i<24 ? "Very good" :\
        "Superb" )))))))

#define HSHR(ch) ((ch)->player.sex ?					\
	(((ch)->player.sex == 1) ? "his" : "her") : "its")

#define HSSH(ch) ((ch)->player.sex ?					\
	(((ch)->player.sex == 1) ? "he" : "she") : "it")

#define HMHR(ch) ((ch)->player.sex ? 					\
	(((ch)->player.sex == 1) ? "him" : "her") : "it")	

#define ANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")

#define SANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

#define IS_NPC(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC))

#define IS_MOB(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC) && ((ch)->virtual>0))

#define GET_POS(ch)     ((ch)->specials.position)

#define GET_COND(ch, i) ((ch)->specials.conditions[(i)])

#define GET_NAME(ch)    ((ch)->player.name)

#define GET_TITLE(ch)   ((ch)->player.title)

#define GET_FORM(ch)    ((ch)->player.form)
#define GET_CLASS(ch)   ((ch)->player.class)

#define GET_HOME(ch)	((ch)->player.hometown)

#define GET_AGE(ch)     (age(ch).year)

#define GET_RSTR(ch)     ((ch)->abilities.str)
#define GET_RADD(ch)     ((ch)->abilities.str_add)
#define GET_RDEX(ch)     ((ch)->abilities.dex)
#define GET_RINT(ch)     ((ch)->abilities.intel)
#define GET_RWIS(ch)     ((ch)->abilities.wis)
#define GET_RCON(ch)     ((ch)->abilities.con)
#define GET_RCHR(ch)	((ch)->abilities.chr)
#define GET_RLCK(ch)	((ch)->abilities.lck)

#define GET_STR(ch)     ((ch)->tmpabilities.str)
#define GET_ADD(ch)     ((ch)->tmpabilities.str_add)
#define GET_DEX(ch)     ((ch)->tmpabilities.dex)
#define GET_INT(ch)     ((ch)->tmpabilities.intel)
#define GET_WIS(ch)     ((ch)->tmpabilities.wis)
#define GET_CON(ch)     ((ch)->tmpabilities.con)
#define GET_CHR(ch)	((ch)->tmpabilities.chr)
#define GET_LCK(ch)	((ch)->tmpabilities.lck)

#define GET_FEET_AC(ch)      ((ch)->points.armor[LOCATION_FEET])
#define GET_BODY_AC(ch)      ((ch)->points.armor[LOCATION_BODY])
#define GET_ARMS_AC(ch)      ((ch)->points.armor[LOCATION_ARMS])
#define GET_LEGS_AC(ch)      ((ch)->points.armor[LOCATION_LEGS])
#define GET_HEAD_AC(ch)      ((ch)->points.armor[LOCATION_HEAD])

#define GET_FEET_STOPPING(ch)   ((ch)->points.stopping[LOCATION_FEET])
#define GET_BODY_STOPPING(ch)   ((ch)->points.stopping[LOCATION_BODY])
#define GET_ARMS_STOPPING(ch)   ((ch)->points.stopping[LOCATION_ARMS])
#define GET_LEGS_STOPPING(ch)   ((ch)->points.stopping[LOCATION_LEGS])
#define GET_HEAD_STOPPING(ch)   ((ch)->points.stopping[LOCATION_HEAD])

#define GET_HIT(ch)     ((ch)->points.hit)

#define GET_MAX_HIT(ch) (hit_limit(ch))

#define GET_MOVE(ch)    ((ch)->points.move)

#define GET_MAX_MOVE(ch) (move_limit(ch))

#define GET_MANA(ch)      ((ch)->points.mana)
#define GET_MAX_MANA(ch)  (mana_limit(ch))
#define GET_POWER(ch)     ((ch)->points.power)
#define GET_MAX_POWER(ch) (power_limit(ch))

#define GET_GOLD(ch)    ((ch)->points.gold)

#define GET_BANK(ch)    ((ch)->points.bankgold)

#define GET_EXP(ch)     ((ch)->points.exp)

#define GET_HEIGHT(ch)  ((ch)->player.height)

#define GET_WEIGHT(ch)  ((ch)->player.weight)

#define GET_SEX(ch)     ((ch)->player.sex)

#define GET_RACE(ch)     ((ch)->player.race)

#define GET_HITROLL(ch) ((ch)->points.hitroll)

#define GET_DAMROLL(ch) ((ch)->points.damroll)

#define AWAKE(ch) (GET_POS(ch) > POSITION_SLEEPING && GET_POS(ch) <= POSITION_STANDING)
#define CAN_FIGHT(ch,v) (AWAKE((ch)) && (ch)->in_room==(v)->in_room && \
		(!IS_AFFECTED((ch), AFF_PARALYSIS)))

#define WAIT_STATE(ch, cycle)  ((ch)->specials.wait = (cycle))

/* Object And Carry related macros */

#define CAN_SEE_OBJ(sub, obj)                                           \
	(   ( (!IS_NPC(sub)) && (GetMaxLevel(sub)>LOW_IMMORTAL))       ||   \
        ( (( !IS_SET((obj)->obj_flags.extra_flags, ITEM_INVISIBLE) ||   \
	     IS_AFFECTED((sub),AFF_DETECT_INVISIBLE) ) &&               \
	     !IS_AFFECTED((sub),AFF_BLIND)) &&                          \
             (IS_LIGHT(sub->in_room))))

#define GET_OBJ_MATERIAL(obj) ((obj)->material)

#define GET_ITEM_TYPE(obj) ((obj)->obj_flags.type_flag)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags,part))

#define GET_OBJ_WEIGHT(obj) ((obj)->obj_flags.mass / 454)
#define GET_OBJ_MASS(obj)   ((obj)->obj_flags.mass)
#define GET_OBJ_VOLUME(obj) ((obj)->obj_flags.volume)
#define GET_OBJ_LENGTH(obj) ((obj)->obj_flags.length)

/* redefinition of maximum weight carried */
#define CAN_CARRY_M(ch) (MAX(10000, GET_STR(ch) * 10000 - 50000))
/* #define CAN_CARRY_W(ch) (str_app[GET_STR(ch)].carry_w) */

/* redefinition of maximum volume carried */
#define CAN_CARRY_V(ch) (GET_DEX(ch) * 4000 + GET_WEIGHT(ch) * 50)

#define IS_CARRYING_M(ch) ((ch)->specials.carry_mass)
/* #define IS_CARRYING_W(ch) ((ch)->specials.carry_weight) */

#define IS_CARRYING_V(ch) ((ch)->specials.carry_volume)

#define CAN_CARRY_OBJ(ch, obj) \
  (((IS_CARRYING_M(ch) + GET_OBJ_MASS(obj)) <= CAN_CARRY_M(ch)) && \
  ((IS_CARRYING_V(ch) + GET_OBJ_VOLUME(obj)) <= CAN_CARRY_V(ch)))

/*
#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))
*/

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&          \
    CAN_SEE_OBJ((ch),(obj)))

#define IS_OBJ_STAT(obj,stat) (IS_SET((obj)->obj_flags.extra_flags,stat))

/* char name/short_desc(for mobs) or someone?  */

#define PERS(ch, vict)   (                                          \
	CAN_SEE(vict, ch) ?						                                    \
	  (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr) :	\
	  "someone")

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	(obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")

#define OUTSIDE(ch) (!IS_SET(real_roomp((ch)->in_room)->room_flags,INDOORS) && \
				!(real_roomp((ch)->in_room)->sector_type==0))

#define IS_IMMORTAL(ch) ((GetMaxLevel(ch)>=LOW_IMMORTAL)&&(!IS_NPC(ch)))

#define IS_POLICE(ch) ((ch->virtual == 3060) || \
                       (ch->virtual == 3069) || \
                       (ch->virtual == 3067))

#define IS_CORPSE(obj) (GET_ITEM_TYPE((obj))==ITEM_CONTAINER && \
			(obj)->obj_flags.value[3] && \
			isname("corpse", (obj)->name))

#define EXIT(ch, door)  (real_roomp((ch)->in_room)->dir_option[door])

int exit_ok(struct room_direction_data *, struct room_data **);

#define CAN_GO(ch, door) (EXIT(ch,door)&&real_roomp(EXIT(ch,door)->to_room) \
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

#define CAN_GO_HUMAN(ch, door) (EXIT(ch,door) && \
			  real_roomp(EXIT(ch,door)->to_room) \
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))

#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)
#define GET_TRUE_ALIGN(ch) ((ch)->specials.true_align)

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_TRUE_GOOD(ch) (GET_TRUE_ALIGN(ch) == 1000)
#define IS_TRUE_EVIL(ch) (GET_TRUE_ALIGN(ch) == -1000)
#define IS_TRUE_NEUTRAL(ch) (GET_TRUE_ALIGN(ch) == 0)

#define IS_ALTAR(ch) (IS_TRUE_GOOD(ch) ? ROOM_FLAG(ch->in_room, \
  GOOD_ALTAR) : (IS_TRUE_EVIL(ch) ? ROOM_FLAG(ch->in_room, EVIL_ALTAR) : \
  ROOM_FLAG(ch->in_room, NEUTRAL_ALTAR)))

#define ck_null(s) ((s)?(s):"(null)")

#define EXP_NEEDED(k,c) ((unsigned long)(1000.0*classes[c].multi*(double)((k)<=100?((k-1)*(k-1)):((k-100)*2+100)*((k-100)*2+100))))

#define HAS_EXP(ch,j) (GET_EXP(ch)>=(EXP_NEEDED(j,GET_CLASS(ch))))
#define CAN_ADVANCE(ch) HAS_EXP(ch,1+GET_LEVEL(ch))

#define CHK_PLAYER_ZONE(ch,vnum)\
	if (GetMaxLevel(ch)<SILLYLORD) {\
	  if (!ch->build_lo || !ch->build_hi) {\
		send_to_char("You don't have a zone yet.\n\r",ch);\
		return;\
	  }\
	  if (vnum<ch->build_lo || vnum>ch->build_hi) {\
		char buf[100];\
		sprintf(buf,"Your zone is from %ld to %ld only.\n\r",\
			ch->build_lo,ch->build_hi);\
		send_to_char(buf,ch);\
		return;\
	  }\
	}

#define CLEAR_BIT(var,bitv) if (IS_SET(var,bitv)) REMOVE_BIT(var,bitv)

#define CONDITION(percent)  (\
		(percent >= 98)?"excellent":\
		(percent >= 90)?"a few scratches":\
		(percent >= 75)?"small wounds":\
		(percent >= 50)?"quite a few wounds":\
		(percent >= 30)? "big nasty wounds":\
		(percent >= 15)?"pretty hurt":\
		(percent >= 0)?"awful":\
			"dying")

#define IS_MAGIC(obj) (IS_SET(obj->obj_flags.extra_flags,ITEM_MAGIC))

#define CHAR_IN_PRIV_ZONE(CH) \
  IS_SET(real_roomp((CH)->in_room)->room_flags, PRIVATE)

#define CHAR_IN_SAFE_ZONE(CH)   \
  IS_SET(real_roomp((CH)->in_room)->room_flags, PEACEFUL)
#define DETECT_MAGIC(ch) (IS_AFFECTED(ch,AFF_DETECT_MAGIC)||IsClass(ch,BARBARIAN))

#define ROOM_FLAG(ro_no,flag) (IS_SET(real_roomp((ro_no))->room_flags,(flag)))

#define IS_CONN(d) ((d)->character && ((d)->connected==CON_PLYNG || (d)->connected==CON_OBJECT))

#define WAS_IN_ROOM(d) ((d)->character->specials.was_in_room)

#ifdef MALLOC_DEBUG
#define FREE(p) { if ((void*)(p)) freex((void*)(p)); (p)=0; }
#define MALLOC(x,t) (t *)mallocx(x)
#define REALLOC(x,s,t) x?(t *)reallocx(x,s):(t *)mallocx(s)
#define CREATE(result, type, number)  do {\
	if (!((result) = MALLOC((number)*(4*(1+sizeof(type)/4)),type)))\
		{ perror("malloc failure"); abort(); } } while(0)

#else
#define FREE(p) { if ((void*)(p)) free((void*)(p)); ((p))=0;}
//#define FREE(p) { if ((void*)(p)) free((void*)(p)); ((char*)(p))=0;}
#define REALLOC(x,s,t) (x?(t *)realloc(x,s):(t *)malloc(s))
#define CREATE(result, type, number)  do {\
	if (!((result) = calloc(sizeof(type),(number))))\
		{ perror("calloc failure"); printf("sizeof=%d number=%d (%s, %d)\n",sizeof(type),number,__FILE__, __LINE__);abort(); } } while(0)
#define MALLOC(x,t) (t *)malloc(x)

#endif

#define IsBuilder(ch) (GetMaxLevel(ch)>=REAL_IMMORT && GetMaxLevel(ch)<DEMIGOD)

#define IS_CITIZEN(ch) ((IS_NPC(ch) && IS_SET(ch->specials.act,ACT_CITIZEN)))

#define IN_ARENA(ch) (ROOM_FLAG(ch->in_room,ARENA))

#define IN_CONTEST(ch) (ch->in_room>=900 && ch->in_room<=963 && GetMaxLevel(ch)<=MAX_MORT)

#define IS_AGGR(ch) (IS_NPC(ch) && IS_SET((ch)->specials.act,ACT_AGGRESSIVE))

#define MOB_FLAG(ch,flag) (IS_NPC(ch) && IS_SET((ch)->specials.act,flag))


#define RECREATE(result,type,number) do {\
  if (!((result) = REALLOC((result), sizeof(type) * (number),type)))\
		{ perror("realloc failure"); abort(); } } while(0)


#define MAX_XP_GAIN(ch) (GetMaxLevel((ch))*3000)

#define NEXP(ch,xp) (MIN(MAX_XP_GAIN(ch),xp))

#define CLASS(ch) classes[GET_CLASS(ch)]
#define FORM(ch) (ch)->player.formdef

#define IsClass(ch,class) (GET_CLASS(ch)==class)
#define SET_LEVEL(ch,x) (GET_LEVEL(ch)=(x))
#define THACO(ch) MAX(20-(GET_LEVEL(ch)/CLASS(ch).thaco_level),CLASS(ch).thaco_min)
#define is_fighter(ch) ((CLASS(ch).flags&FLAG_FIGHTER)!=0)
#define is_cleric(ch) ((CLASS(ch).flags&FLAG_CLERIC)!=0)
#define is_magicu(ch) ((CLASS(ch).flags&FLAG_MAGE)!=0)
#define is_thief(ch) ((CLASS(ch).flags&FLAG_THIEF)!=0)
#define OFFSET(ch,val) ((val)<=0?(val):(val)<1000?(val)+(ch)->build.offset:(val))
#define LOG_BITS(ch) (ch)->specials.log_bits
#define LOG(ch,type)  ((LOG_BITS(ch) & (type))!=0)
#define ITEM_TYPE(obj,type) (GET_ITEM_TYPE(obj)==(type))
#define VALUE0(obj) (obj)->obj_flags.value[0]
#define VALUE1(obj) (obj)->obj_flags.value[1]
#define VALUE2(obj) (obj)->obj_flags.value[2]
#define VALUE3(obj) (obj)->obj_flags.value[3]
#define VALUE4(obj) (obj)->obj_flags.value[4]
#define VALUE(obj,i) (obj)->obj_flags.value[(i)]
#define IS_WEAPON(obj)  ((obj) ? \
  (((obj)->obj_flags.type_flag == ITEM_WEAPON) || \
  ((obj)->obj_flags.type_flag == ITEM_FIREWEAPON) || \
  ((obj)->obj_flags.type_flag == ITEM_MISSILE)) : 0)
#define IS_TWO_HANDED(obj)  (IS_WEAPON(obj) ? \
  (weapons[(obj)->obj_flags.value[0]].flags & WEAPF_TWO) : 0)

