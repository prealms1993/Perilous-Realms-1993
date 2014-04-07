/* ************************************************************************
*  File: fight.c , Combat module.                         Part of DIKUMUD *
*  Usage: Combat system and messages.                                     *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <fcns.h>
#include <limits.h>
#include <utils.h>
#include <comm.h>
#include <handler.h>
#include <interpreter.h>
#include <db.h>
#include <spells.h>
#include <race.h>
#include <ticks.h>
#include <externs.h>

/* Structures */

struct char_data *combat_list = 0;   /* head of l-list of fighting chars    */
struct char_data *combat_next_dude = 0; /* Next dude global trick           */


/* External structures */

extern struct hash_header room_db;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data  *object_list;
extern struct char_data *character_list;
extern char *ItemDamType[];
extern int ItemSaveThrows[22][5];
/* External procedures */

s32 modify_exp(struct char_data *ch,s32 xp,ulong vnum)
{
#define NUM_DIVISIONS 5
#define NUM_BREAK 20

  if(num_kills(ch,vnum) > (NUM_DIVISIONS * NUM_BREAK))
  return(xp * .05);

  xp *= (NUM_DIVISIONS-MIN(num_kills(ch,vnum)/NUM_BREAK,NUM_DIVISIONS));
  xp /= NUM_DIVISIONS;
  if (IS_SET(ch->specials.act,PLR_DEBUG))
  sendf(ch,"Kills: %d of %d - %d\n",num_kills(ch,vnum),vnum,xp);
  return(xp);
}


/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit",    "hits"},            /* TYPE_HIT      */
  {"pound",  "pounds"},          /* TYPE_BLUDGEON */
  {"pierce", "pierces"},         /* TYPE_PIERCE   */
  {"slash",  "slashes"},         /* TYPE_SLASH    */
  {"whip",   "whips"},           /* TYPE_WHIP     */
  {"claw",   "claws"},           /* TYPE_CLAW     */
  {"bite",   "bites"},           /* TYPE_BITE     */
  {"sting",  "stings"},          /* TYPE_STING    */
  {"crush",  "crushes"},         /* TYPE_CRUSH    */
  {"cleave", "cleaves"},
  {"stab",   "stabs"},
  {"smash",  "smashes"},
  {"smite",  "smites"}
};




/* The Fight related routines */


void appear(struct char_data *ch)
{
  act("$n slowly fade into existence.", FALSE, ch,0,0,TO_ROOM);

  if (affected_by_spell(ch, SPELL_INVISIBLE))
    affect_from_char(ch, SPELL_INVISIBLE);

  clear_bit(ch->specials.affected_by, AFF_INVISIBLE);
}



void load_messages(void)
{
  FILE *f1;
  int i,type;
  struct message_type *messages;
  char chk[100];

  if (!(f1 = fopen(MESS_FILE, "r"))){
    perror(MESS_FILE);
    exit(0);
  }

  /*
    find the memset way of doing this...
    */

  for (i = 0; i < MAX_MESSAGES; i++)	{
    fight_messages[i].a_type = 0;
    fight_messages[i].number_of_attacks=0;
    fight_messages[i].msg = 0;
  }

  fscanf(f1, " %s \n", chk);

  i = 0;

  while(*chk == 'M')	{
    fscanf(f1," %d\n", &type);

    if(i>=MAX_MESSAGES){
      log("Too many combat messages.");
      exit(0);
    }

    CREATE(messages,struct message_type,1);
    fight_messages[i].number_of_attacks++;
    fight_messages[i].a_type=type;
    messages->next=fight_messages[i].msg;
    fight_messages[i].msg=messages;

    messages->die_msg.attacker_msg      = fread_string(f1);
    messages->die_msg.victim_msg        = fread_string(f1);
    messages->die_msg.room_msg          = fread_string(f1);
    messages->miss_msg.attacker_msg     = fread_string(f1);
    messages->miss_msg.victim_msg       = fread_string(f1);
    messages->miss_msg.room_msg         = fread_string(f1);
    messages->hit_msg.attacker_msg      = fread_string(f1);
    messages->hit_msg.victim_msg        = fread_string(f1);
    messages->hit_msg.room_msg          = fread_string(f1);
    messages->god_msg.attacker_msg      = fread_string(f1);
    messages->god_msg.victim_msg        = fread_string(f1);
    messages->god_msg.room_msg          = fread_string(f1);
    fscanf(f1, " %s \n", chk);
    i++;
  }

  fclose(f1);
}


void update_pos( struct char_data *victim )
{

  if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POSITION_STUNNED)) {
    return;
  } else if (GET_HIT(victim) > 0 ) {
    if (!IS_AFFECTED(victim, AFF_PARALYSIS)) {
      GET_POS(victim) = POSITION_STANDING;
    } else {
      GET_POS(victim) = POSITION_STUNNED;
    }
  } else if (GET_HIT(victim) <= -11) {
    GET_POS(victim) = POSITION_DEAD;
  } else if (GET_HIT(victim) <= -6) {
    GET_POS(victim) = POSITION_MORTALLYW;
  } else if (GET_HIT(victim) <= -3) {
    GET_POS(victim) = POSITION_INCAP;
  } else {
    GET_POS(victim) = POSITION_STUNNED;
  }
}


int check_peaceful(struct char_data *ch, char *msg)
{
  struct room_data *rp;

  rp = real_roomp(ch->in_room);
  if (rp && rp->room_flags&PEACEFUL) {
    send_to_char(msg, ch);
    return 1;
  }
  return 0;
}

/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
  if (ch == vict) return;
  if (!CORRUPT(ch,vict)) return;
  if (ch->specials.fighting) {
    vlog(LOG_URGENT,"Fighting character set to fighting another.");
    return;
  }

  if (vict->attackers <= 5) {
    vict->attackers+=1;
  } else {
    log("more than 6 people attacking one target");
  }

  if(IS_AFFECTED(ch,AFF_SLEEP))
    affect_from_char(ch,SPELL_SLEEP);

  ch->specials.fighting = vict;
  GET_POS(ch) = POSITION_FIGHTING;

  /* set up the next blow */
  add_event(calc_speed(ch, 0), EVT_STRIKE, ch, NULL, NULL, -1, 0, NULL, event_strike);
}

static void start_fighting(struct char_data *ch, struct char_data *victim)
{
  if (!CORRUPT(ch,victim)) return;
  if (victim != ch) {
    if (GET_POS(victim) > POSITION_STUNNED) {
      if (!(victim->specials.fighting))
	if ((!IS_NPC(ch))||(!IS_SET(ch->specials.act, ACT_IMMORTAL))) {
	  if (ch->attackers < 6) {
	    set_fighting(victim, ch);
	  }
	} else {
	  return;
	}
    }
  }

  if (victim != ch) {
    if (GET_POS(ch) > POSITION_STUNNED) {	
      if (!(ch->specials.fighting))
	if ((!IS_NPC(ch))||(!IS_SET(ch->specials.act, ACT_IMMORTAL))) {
	  set_fighting(ch, victim);
	} else {
	  return;
	}
    }
  }
}

/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch)
{
  struct char_data *fighting;

/*if(!ch->specials.fighting){
    vlog(LOG_URGENT,"Character not fighting at invocation of stop_fighting");
    return;
  } */

  for(fighting = real_roomp(ch->in_room)->people; fighting;
      fighting = fighting->next_in_room){
    if(fighting->specials.fighting == ch){
      ch->attackers--;
      if(ch->attackers < 0){
        vlog(LOG_DEBUG, "stop_fighting(): too few attackers of ch");
        ch->attackers = 0;
      }
      fighting->specials.fighting = NULL;
      GET_POS(fighting) = POSITION_STANDING;
      update_pos(fighting);
    }
  }

  if(ch->specials.fighting){
    ch->specials.fighting->attackers--;
    if (ch->specials.fighting->attackers < 0) {
      vlog(LOG_DEBUG, "stop_fighting(): too few attackers of fighting");
      ch->specials.fighting->attackers = 0;
    }

    ch->specials.fighting = NULL;
    GET_POS(ch) = POSITION_STANDING;
    update_pos(ch);
  }
}

#define MAX_NPC_CORPSE_TIME 5
#define MAX_PC_CORPSE_TIME 20

void make_corpse(struct char_data *ch)
{
  struct obj_data *corpse, *o;
  struct obj_data *money;	
  char buf[MAX_INPUT_LENGTH];
  int i, ADeadBody=FALSE;

  CREATE(corpse, struct obj_data, 1);
  clear_object(corpse);

  corpse->in_room = NOWHERE;

  if (!IS_NPC(ch) || (!IsUndead(ch))) {
    sprintf(buf, "corpse %s",ch->player.name);
    corpse->name = mystrdup(buf);

    sprintf(buf, "The corpse of %s is lying here.",
	    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->description = mystrdup(buf);

    sprintf(buf, "the corpse of %s",
	    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->short_description = mystrdup(buf);

    ADeadBody = TRUE;

  } else if (IsUndead(ch)) {
    corpse->name = mystrdup("dust pile");
    corpse->description = mystrdup("A pile of dust is here.");
    corpse->short_description = mystrdup("a pile of dust");	
  }

  corpse->contains = ch->carrying;
  if(GET_GOLD(ch)>0) {
    money = create_money(GET_GOLD(ch));
    GET_GOLD(ch)=0;
    obj_to_obj(money,corpse);
  }

  corpse->obj_flags.type_flag = ITEM_CONTAINER;
  corpse->obj_flags.wear_flags = ITEM_TAKE;
  corpse->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
  corpse->obj_flags.value[2] = (int)GET_WEIGHT(ch)*0.60;
  corpse->obj_flags.value[3] = 1; /* corpse identifyer */
  if (ADeadBody) {
    corpse->obj_flags.mass = (GET_WEIGHT(ch) * 454) + IS_CARRYING_M(ch);
  } else {
    corpse->obj_flags.mass = IS_CARRYING_M(ch);
  }
  corpse->obj_flags.cost_per_day = 10000000;
  if (IS_NPC(ch))
    corpse->obj_flags.timer = MAX_NPC_CORPSE_TIME;
  else
    corpse->obj_flags.timer = MAX_PC_CORPSE_TIME;

  for (i=0; i<MAX_WEAR; i++)
    if (ch->equipment[i])
      obj_to_obj(unequip_char(ch, i), corpse);

  ch->carrying = 0;
  IS_CARRYING_M(ch) = 0;
  IS_CARRYING_V(ch) = 0;

  if (IS_NPC(ch)) {
    corpse->held_for = 0;
    corpse->obj_flags.value[4] = ch->virtual;
  } else {
    corpse->obj_flags.value[4] = 0;
    if (!IS_SET(ch->specials.act,PLR_CORRUPT)) /* don't save stuff for corrupt people */
      corpse->held_for = mystrdup(GET_NAME(ch));
  }
  corpse->carried_by = 0;
  corpse->equipped_by = 0;

  corpse->next = object_list;
  object_list = corpse;

  for(o = corpse->contains; o; o = o->next_content)
    o->in_obj = corpse;

  object_list_new_owner(corpse, 0);

  obj_to_room(corpse, ch->in_room);
}

void change_alignment(struct char_data *ch, struct char_data *victim)
{

  int i;
  struct obj_data *obj;

  if (IS_NPC(ch)) return;

  if (IS_GOOD(ch) && (IS_GOOD(victim))) {
    GET_ALIGNMENT(ch) -= ((GET_ALIGNMENT(victim)  / 300) * (MAX(1,GetMaxLevel(victim) - GetMaxLevel(ch))));
  } else if (IS_EVIL(ch) && (IS_GOOD(victim))) {
    GET_ALIGNMENT(ch) -= ((GET_ALIGNMENT(victim) / 300) * (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch))));
  } else if (IS_EVIL(victim) && (IS_GOOD(ch))) {
    GET_ALIGNMENT(ch) -= ((GET_ALIGNMENT(victim) / 300) * (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch))));
  } else if (IS_EVIL(ch) && (IS_EVIL(victim))) {
    GET_ALIGNMENT(ch) -= ((GET_ALIGNMENT(victim) / 300) * (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch))));
  } else { /* neutral aspects... move towards char's pref end */
    GET_ALIGNMENT(ch) -= ((GET_ALIGNMENT(victim) / 300) * (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch))));
  }
  GET_ALIGNMENT(ch) = MAX(GET_ALIGNMENT(ch), -1000);
  GET_ALIGNMENT(ch) = MIN(GET_ALIGNMENT(ch), 1000);

/* short and sweet */
  for (i=0; i<MAX_WEAR; i++)
    if (ch->equipment[i]) {
	obj=unequip_char(ch,i);
	equip_char(ch,obj,i);
    }
}

void death_cry(struct char_data *ch)
{
  int door, was_in;

  if (!real_roomp(ch->in_room))
    return;

  act("Your blood freezes as you hear $n's death cry.", FALSE, ch,0,0,TO_ROOM);
  was_in = ch->in_room;

  for (door = 0; door <= 5; door++) {
    if (CAN_GO(ch, door))	{
      ch->in_room = (real_roomp(was_in))->dir_option[door]->to_room;
      act("Your blood freezes as you hear someones death cry.",FALSE,ch,0,0,TO_ROOM);
      ch->in_room = was_in;
    }
  }
}

void raw_kill(struct char_data *ch)
{

/* hack to prevent some problems */
/*
  struct char_data *i;
  for (i=character_list; i; i=i->next)
	if (i==ch) break;
  if (!i) {
    vlog(LOG_URGENT,"Attempt to kill non-existant player");
    return;
  } */

  stop_fighting(ch);

  death_cry(ch);
  /* remove the problem with poison, and other spells */
  spell_dispel_magic(IMPLEMENTOR,ch,ch,0,IMPLEMENTOR);

  /* give them some food and water so they don't whine.  */
  GET_COND(ch,THIRST)=20;
  GET_COND(ch,FULL)=20;

  /* return them from polymorph */

  make_corpse(ch);
  if (!IS_NPC(ch))
    save_char(ch,NOWHERE);
  FREE(ch->kills);
/* extract_char(ch); */
  /* fix the problem of vanishing character structs by keeping them around */
  char_from_room(ch);
  char_to_room(ch, 0);
  add_event(1, EVT_DEATH, ch, NULL, NULL, -1, 0, NULL, event_death);
}

void event_death(event_t *event)
{
  /* anything to do with this character should be over */
  extract_char(event->ch);
  return;
}

void die(struct char_data *ch)
{
  /* Must not allow players to take advantage of dead mobiles in arena! */
  if (IS_NPC(ch) && ROOM_FLAG(ch->in_room,ARENA)) {
     act("Your blood turns to ice as you hear the agonizing shrieks of $n.",
	 FALSE,ch,0,0,TO_ROOM);
     act("$n's body glows brightly for an instant and then slowly fades away.\n", FALSE,ch,0,0,TO_ROOM);
     extract_char(ch);
     return;
  }

  if (!IS_NPC(ch) && ROOM_FLAG(ch->in_room,ARENA)) {
    rem_char_events(ch);
    act("$n is magically whisked away.\n",FALSE,ch,0,0,TO_ROOM);
    do_restore(ch,"",-1);
    char_from_room(ch);
    char_to_room(ch,3000);
    while (ch->affected) affect_remove(ch,ch->affected);

    act("$n appears from nowhere... musta had a bad day.\n",FALSE,ch,0,0,TO_ROOM);
    sendf(ch,"You have been defeated.\n");
    return;
  }

  gain_exp(ch, MAX(-500000*(1+GetMaxLevel(ch)/30),GET_EXP(ch)/-4L));

  DeleteHatreds(ch);
  DeleteFears(ch);
  raw_kill(ch);
}



void group_gain(struct char_data *ch, struct char_data *victim)
{
  char buf[256];
  int no_members, share;
  ulong vnum;
  struct char_data *k;
  struct follow_type *f;

  vnum=victim->virtual;
  if (!(k=ch->master))
    k = ch;

  /* can't get exp for killing players */
  if (!IS_NPC(victim)) {
    return;
  }

  if (IS_AFFECTED(k, AFF_GROUP) &&
      (k->in_room == ch->in_room))
    no_members = GetMaxLevel(k);
  else
    no_members = 0;

  for (f=k->followers; f; f=f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	(f->follower->in_room == ch->in_room))
      no_members+=GetMaxLevel(f->follower);

  if (no_members >= 1)
    share = (GET_EXP(victim)/no_members);
  else
    share = 0;

  if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)) {
    sprintf(buf,"You recieve %d experience of a total %d.",
		NEXP(k,modify_exp(k,share*GetMaxLevel(k),vnum)),
		share*no_members);
    act(buf, FALSE, k, 0, 0, TO_CHAR);
    gain_exp(k,MIN(MAX_XP_GAIN(k),modify_exp(k,share*GetMaxLevel(k),vnum)));
    change_alignment(k, victim);
  }

  for (f=k->followers; f; f=f->next) {
    if (f && IS_AFFECTED(f->follower, AFF_GROUP) &&
				(f->follower->in_room == ch->in_room)) {
      sprintf(buf,"You recieve %d experience of a total %d.",
	NEXP(f->follower,
		modify_exp(f->follower,share*GetMaxLevel(f->follower),vnum)),
	share*no_members);
      act(buf, FALSE, f->follower,0,0,TO_CHAR);
      gain_exp(f->follower,
	MIN(MAX_XP_GAIN(f->follower),
		modify_exp(f->follower,share*GetMaxLevel(f->follower),vnum)));
      change_alignment(f->follower, victim);
    }
  }
}

char *replace_string(char *str, char *weapon, char *weapon_s,
      char *location, struct char_data *victim)
{
  static char buf[256];
  char *cp;

  cp = buf;

  for (; *str; str++) {
    if (*str == '#') {
      switch(*(++str)) {
      case 'W' :
	for (; *weapon; *(cp++) = *(weapon++));
	break;
      case 'w' :
	for (; *weapon_s; *(cp++) = *(weapon_s++));
	break;
      case 'L' :
        for (; *location; *(cp++) = *(location++));
        break;
      default :
	  *(cp++) = '#';
	break;
      }
    } else {
      *(cp++) = *str;
    }

    *cp = 0;
  } /* For */

  return(buf);
}


char *location_name(struct char_data *vict, int location)
{
  char **table;

  extern char *humanoid_limbs[];
  extern char *dragon_limbs[];
  extern char *avian_limbs[];
  extern char *insectoid_limbs[];	

  switch(GET_RACE(vict)) {
    case RACE_INSECT:
    case RACE_ARACHNID:
      table = insectoid_limbs;
    break;
    case RACE_DRAGON:
    case RACE_DEMON:
    case RACE_DEVIL:
       table = dragon_limbs;
    break;
    case RACE_BIRD:
      table = avian_limbs;
    break;
    default:
      table = humanoid_limbs;
    break;
  }

  return(table[location]);
}

void dam_message(int dam, struct char_data *ch, struct char_data *victim,
                 int w_type)
{
  struct obj_data *wield;
  char *buf, *location;
  int snum;

  struct dam_weapon_type {
    char *to_room;
    char *to_char;
    char *to_victim;
  };

  static struct dam_weapon_type armor_msg[] = {
    {"$n's #w bounces harmlessly off $N's armor.",   /* Armor msg 1 */
     "Your #w bounces off $N's armored #L.",
     "$n #W fiercly, but can't penetrate your armored #L." },
    {"$n's #w fails to penetrate $N's armor.",       /* Armor msg 2 */
     "Your #w deflects harmlessly off $N's #L",
     "Your armor protects you from $n's #W" },
    {"$n #W at $N's #L, but fails to penetrate $S armor", /* Armor msg 3*/
     "You #w at $N, but can't penetrate $S armor.",
     "$n skillfully #W your #L, but your armor protects you."}

  };

  static struct dam_weapon_type miss_msg[] = {
    {"$n misses $N.",                           /* Miss msg 1 */
     "You miss $N.",
     "$n misses you." },
    {"$n tries to #w $N, but misses",           /* Miss msg 2 */
     "$N slips past your attack.",
     "$n misses your #L with $s #w." },
    {"$n's #w whispers past $N's #L",          /* Miss msg 3 */
     "Your #w misses $N",
     "$n's #w barely misses your #L" }
  };

  static struct dam_weapon_type hit_msg_1[] = {
    {"$n bruises $N in the #L with $s #w.",            /*  1.. 3  */
     "You bruise $N in the #L as you #w $M.",
     "$n bruises you in the #L as $e #W you." },
    {"$n barely scratches $N in the #L.",
     "You barely scratch $N in the #L.",
     "$n barely scratches you in the #L." },
    {"$n barely scratches $N in the #L.",
     "You barely scratch $N in the #L.",
     "$n barely scratches you in the #L." }
  };

  static struct dam_weapon_type hit_msg_2[] = {
    {"$n #W $N in the #L.",                                /*  4.. 6  */
     "You #w $N in the #L.",
     "$n #W you in the #L." },
    {"$n #W $N in the #L.",                                /*  4.. 6  */
     "You #w $N in the #L.",
     "$n #W you in the #L." },
    {"$n #W $N in the #L.",                                /*  4.. 6  */
     "You #w $N in the #L.",
     "$n #W you in the #L." }
  };

  static struct dam_weapon_type hit_msg_3[] = {
    {"$n #W $N hard in the #L.",                           /*  7..10  */
     "You #w $N hard in the #L.",
     "$n #W you hard in the #L." },
    {"$n scores a blow to $N's #L with $s #W",
     "You score to $N's #L with your #W",
     "$n #W you hard in the #L." },
    {"$n wounds $N with $s #W",
     "Your #w wounds $N in the #L.",
     "$n wounds your #L with $s #W" }
  };

  static struct dam_weapon_type hit_msg_4[] = {
    {"$n #W $N very hard in the #L.",                      /* 11..14  */
     "You #w $N very hard in the #L.",
     "$n #W you very hard in the #L."},
    {"$n sends $N reeling with a blow to the #L.",
     "You send $N reeling with your #w.",
     "$n sends you reeling with a blow to your #L."},
    {"$n #W $N very hard in the #L.",                      /* 11..14  */
     "You #w $N very hard in the #L.",
     "$n #W you very hard in the #L."}
  };

  static struct dam_weapon_type hit_msg_5[] = {
    {"$n #W $N extremely well in the #L.",                /* 15..20  */
     "You #w $N extremely well in the #L.",
     "$n #W you extremely well in the #L."},
    {"$n #W $N extremely well in the #L.",                /* 15..20  */
     "You #w $N extremely well in the #L.",
     "$n #W you extremely well in the #L."},
    {"$n #W $N extremely well in the #L.",                /* 15..20  */
     "You #w $N extremely well in the #L.",
     "$n #W you extremely well in the #L."}
  };

  static struct dam_weapon_type hit_msg_6[] = {
    {"$n massacres $N with $s #w in the #L.",     /* > 20    */
     "You massacre $N with your #w in the #L.",
     "$n massacres you with $s #w in the #L."},
    {"$n massacres $N with $s #w in the #L.",     /* > 20    */
     "You massacre $N with your #w in the #L.",
     "$n massacres you with $s #w in the #L."},
    {"$n massacres $N with $s #w in the #L.",     /* > 20    */
     "You massacre $N with your #w in the #L.",
     "$n massacres you with $s #w in the #L."}
  };

static struct dam_weapon_type hit_msg_7[] = {
	{ "$n obliterates $N with $s #w in the #L.",     /* > 30    */
	  "You obliterate $N with your #w in the #L.",
	  "$n obliterates you with $s #w in the #L."},
	{ "$n obliterates $N with $s #w in the #L.",     /* > 30    */
	  "You obliterate $N with your #w in the #L.",
	  "$n obliterates you with $s #w in the #L."},
	{ "$n obliterates $N with $s #w in the #L.",     /* > 30    */
	  "You obliterate $N with your #w in the #L.",
	  "$n obliterates you with $s #w in the #L."}
};

static struct dam_weapon_type hit_msg_8[] = {
	{ "$n annihilates $N with $s #w in the #L.",     /* > 50    */
	  "You annihilate $N with your #w in the #L.",
	  "$n annihilates you with $s #w in the #L."},
	{ "$n annihilates $N with $s #w in the #L.",     /* > 50    */
	  "You annihilate $N with your #w in the #L.",
	  "$n annihilates you with $s #w in the #L."},
	{ "$n annihilates $N with $s #w in the #L.",     /* > 50    */
	  "You annihilate $N with your #w in the #L.",
	  "$n annihilates you with $s #w in the #L."}
};

  static struct dam_weapon_type death_msg[] = {
    {"$n smashes $N's head to a bloody pulp with $s #w.",
     "You reduce $N's head to a bloody pulp with your #w.",
     "$n's #W cleaves through your head in a spray of blood."},
     {"Your #W rents a hole in the $N, spewing innerds all over the floor.",
      "$n's #w scatters your innerds across the room.",
      "$n's #w opens #N's stomach, spewing innerds all over the floor."},
     {"You #w $N's head clean off, sending it flying off in the distance.",
      "Your head is ripped from your shoulders by $n's #w.",
      "$n's smacks $N's head clear off into the distance."},

     {"Your blow makes a loud CRACK as you chatter $N's spine!",
      "$n's blow shatters your spine, killing you instantly.",
      "$n's blow makes a loud CRACK as $s breaks $N's spine!"},

     {"You flay the last bits of $N's flesh, clean down to the bone!",
      "$n flays the last bits of flesh from your skin.",
      "$n flays the flesh from $N, clean down to the bone!"},
  };

  struct dam_weapon_type dam_weapons;

  w_type -= TYPE_HIT;   /* Change to base of table with text */

  wield = ch->equipment[WIELD];

  if((dam != -1) && (GET_HIT(victim)-dam <= -12)) {
      snum = number(0,0);
      dam_weapons = death_msg[snum];
  } else {
      snum = number(0,2);
      	if (dam == -1) {
        	dam_weapons = armor_msg[snum];
	} else if (dam == 0) {
        	dam_weapons = miss_msg[snum];
      	} else if (dam <= 3) {
        	dam_weapons = hit_msg_1[snum];
      	} else if (dam <= 7) {
        	dam_weapons = hit_msg_2[snum];
      	} else if (dam <= 10) {
        	dam_weapons = hit_msg_3[snum];
      	} else if (dam <= 15) {
        	dam_weapons = hit_msg_4[snum];
      	} else if (dam <= 20) {
        	dam_weapons = hit_msg_5[snum];
      	} else if (dam <= 30) {
        	dam_weapons = hit_msg_6[snum];
	} else if (dam <= 50) {
		dam_weapons = hit_msg_7[snum];
	} else {
		dam_weapons = hit_msg_8[snum];
      }
  }

location = location_name(victim, ch->points.hitloc);
  buf = replace_string(dam_weapons.to_room, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular,location,victim);
  act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
  buf = replace_string(dam_weapons.to_char, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular,location,victim);
  act(buf, FALSE, ch, wield, victim, TO_CHAR);
  buf = replace_string(dam_weapons.to_victim, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular,location,victim);
  act(buf, FALSE, ch, wield, victim, TO_VICT);

}

/*
int new_damage(struct char_data *ch, struct char_data *victim, int damage,
  int type)
{
  int8 susres;

  if(GET_POS(victim) <= POSITION_DEAD) return(TRUE);

  susres = ch->susres[type].value;
  if(susres < 0){
    susres = -susres;
    damage += (damage * susres) / 100;
  }
  else{
    damage -= (damage * susres) / 100;
  }

  return(FALSE);
}
*/

int damage(struct char_data *ch, struct char_data *victim, int dam, int attacktype)
{
  char buf[MAX_INPUT_LENGTH];
  struct message_type *messages;
  int i,j,nr,max_hit,exp;
  struct room_data	*rp;


  if  (GET_POS(victim) <= POSITION_DEAD) return(TRUE);
  assert(GET_POS(victim) > POSITION_DEAD);

  if (!CORRUPT(ch,victim)) return(FALSE);
  rp = real_roomp(ch->in_room);
  if (rp && rp->room_flags&PEACEFUL &&
      attacktype!=SPELL_POISON /* poison is allowed */
      ) {
    vlog(LOG_DEBUG,"damage(,,,%d) called in PEACEFUL room", attacktype);
    return(0);
  }

  dam = SkipImmortals(victim, dam);
  if (dam <= -1)	/* NO NEGATIVE DAMAGE */
    return(FALSE);

/* commented out for damage outside of room - SLB */
/*  if (ch->in_room != victim->in_room)
    return(FALSE); */

  if ((ch->specials.fighting != victim) && (victim != ch) &&
      ROOM_FLAG(ch->in_room, ARENA))
    sports_cast("%s and %s engage in mortal combat.",
		GET_NAME(ch),GET_NAME(victim));

  /* added if to prevent fighting people not in room - SLB */
  if (ch->in_room == victim->in_room)
    start_fighting(ch,victim);

  if (victim->master == ch)
    stop_follower(victim);

  if (IS_AFFECTED(ch, AFF_INVISIBLE))
    appear(ch);

  if (dam > 0) {
    if (attacktype >= TYPE_HIT || attacktype==TYPE_UNDEFINED) {
      if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
        dam >>= 1;
      }
    }

    if ((attacktype > SPELL_RESERVED_DBC) && (attacktype < TYPE_HIT)) {
      if (IS_AFFECTED(victim, AFF_INVULNERABLE)) {
        dam >>= 1;
        act("$N's globe shield flares crimson as it absorbs your spell!",
        FALSE, ch, 0, victim, TO_CHAR);
        act("$N's globe shield flares crimson as it absorbs $n's spell!",
        FALSE, ch, 0, victim, TO_ROOM);
        act("Your globe shield flares crimson as it absorbs $n's spell!",
        FALSE, ch, 0, victim, TO_VICT);
      }
    }
    dam = MAX(dam, 1);
  }

  dam = PreProcDam(victim, attacktype,dam);

  dam = WeaponCheck(ch, victim, attacktype, dam);

  if (!ROOM_FLAG(victim->in_room,ARENA))
    DamageStuff(victim, attacktype, dam);

  dam=MAX(dam,0);

  if (ch && (IsClass(ch,ASSASSIN)||IsClass(ch,CLASS_IMMORTAL)) &&
				attacktype==SKILL_BACKSTAB) {
	int need;
        int roll,i;

    need=0;
    if (get_skill(ch,SKILL_ASSASINATE)<number(1,101)) need=1;
    for (i=SKILL_ASSASINATE2; i<=SKILL_ASSASINATE15; i++)
	if (get_skill(ch,i)<number(1,101)) need++;
    need+=((GetMaxLevel(ch)-GetMaxLevel(victim))/10);
    roll=number(1,100);
    if (roll<=need) {
	dam=GET_HIT(victim)+11;
	send_to_char("BINGO! You assasinated it!\n",ch);
    }
  }

  GET_HIT(victim)-=dam;

  if (IS_AFFECTED(victim, AFF_FIRESHIELD)&&!IS_AFFECTED(ch, AFF_FIRESHIELD)) {
    if (victim != ch && damage(victim, ch, dam, SPELL_FIREBALL)) {
      update_pos(victim);
      if (GET_POS(victim) != POSITION_DEAD)
        return(FALSE);
      else
        return(TRUE);
    }
  }
  update_pos(victim);

  if ((attacktype >= TYPE_HIT) && (attacktype <= TYPE_SMITE)) {
    if (!ch->equipment[WIELD]) {
      dam_message(dam, ch, victim, TYPE_HIT);
    } else {
      dam_message(dam, ch, victim, attacktype);
    }
  } else {
    int mess_found=0;
    for(i = 0; i < MAX_MESSAGES; i++) {
      if (fight_messages[i].a_type == attacktype) {
	mess_found=1;
	nr=dice(1,fight_messages[i].number_of_attacks);
	for(j=1,messages=fight_messages[i].msg;(j<nr)&&(messages);j++)
	  messages=messages->next;
	
	if (!IS_NPC(victim) && (GetMaxLevel(victim) > MAX_MORT)){
	  act(messages->god_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD],
		victim, TO_CHAR);
	  act(messages->god_msg.victim_msg, FALSE, ch, ch->equipment[WIELD],
		victim, TO_VICT);
	  act(messages->god_msg.room_msg, FALSE, ch, ch->equipment[WIELD],
		victim, TO_NOTVICT);
	} else if (dam != 0) {
	  if (GET_POS(victim) == POSITION_DEAD) {
	    act(messages->die_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD],
		victim, TO_CHAR);
	    act(messages->die_msg.victim_msg, FALSE, ch, ch->equipment[WIELD],
		victim, TO_VICT);
	    act(messages->die_msg.room_msg, FALSE, ch, ch->equipment[WIELD],
		victim, TO_NOTVICT);
	  } else {
	    act(messages->hit_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD],
		victim, TO_CHAR);
	    act(messages->hit_msg.victim_msg, FALSE, ch, ch->equipment[WIELD],
		victim, TO_VICT);
	    act(messages->hit_msg.room_msg, FALSE, ch, ch->equipment[WIELD],
		victim, TO_NOTVICT);
	  }
	} else { /* Dam == 0 */
	  act(messages->miss_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD],
		victim, TO_CHAR);
	  act(messages->miss_msg.victim_msg, FALSE, ch, ch->equipment[WIELD],
		victim, TO_VICT);
	  act(messages->miss_msg.room_msg, FALSE, ch, ch->equipment[WIELD],
		victim, TO_NOTVICT);
	}
      }
    }
    if (!mess_found) vlog(LOG_DEBUG,"FIGHT:No messages (%d)",attacktype);
  }
  switch (GET_POS(victim)) {
    case POSITION_MORTALLYW:
	act("$n is mortally wounded, and will die soon, if not aided.",
		TRUE, victim, 0, 0, TO_ROOM);
	act("You are mortally wounded, and will die soon, if not aided.",
		FALSE, victim, 0, 0, TO_CHAR);
    break;
    case POSITION_INCAP:
	act("$n is incapacitated and will slowly die, if not aided.",
		TRUE, victim, 0, 0, TO_ROOM);
	act("You are incapacitated and you will slowly die, if not aided.",
		FALSE, victim, 0, 0, TO_CHAR);
    break;
    case POSITION_STUNNED:
	act("$n is stunned, but will probably regain consciousness again.",
		TRUE, victim, 0, 0, TO_ROOM);
	act("You're stunned, but you will probably regain consciousness again.",
		FALSE, victim, 0, 0, TO_CHAR);
    break;
    case POSITION_DEAD:
      act("$n is dead! R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
      act("You are dead!  Sorry...", FALSE, victim, 0, 0, TO_CHAR);
    break;
    default:  /* >= POSITION SLEEPING */
      max_hit=hit_limit(victim);

      if (dam > (max_hit/5))
	act("That Really HURT!",FALSE, victim, 0, 0, TO_CHAR);

      if (GET_HIT(victim) < MIN(max_hit / 5, victim->specials.whimpy_level)) {
       if (HasObject(victim, 25))
	do_save_player(victim);
       else {
	send_to_char("Whimping out!\n",ch);
	do_flee(victim,"",0);
	}
      }

      if (GET_HIT(victim) < (max_hit/5)) {
	act("You wish that your wounds would stop BLEEDING so much!",
		FALSE,victim,0,0,TO_CHAR);
	if (IS_NPC(victim)) {
	  if (IS_SET(victim->specials.act, ACT_WIMPY))
	    do_flee(victim, "", 0);
	} else {
	  if (IS_SET(victim->specials.act, PLR_WIMPY)) {
	    if (HasObject(victim, 25)) /* if player has obj 25 save them */
	     do_save_player(victim);   /* they bought insurance so send a guard */
            else 
	    {
	     do_flee(victim, "", 0);
	     }
	    }
	}
    }
    break;
  }

  if (!IS_MOB(victim) && !(victim->desc) && GET_HIT(victim)>0) {
    int room;

    room=victim->in_room;
    do_flee(victim, "", 0);
    if (room!=victim->in_room) {
      act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
      victim->specials.was_in_room = victim->in_room;
      if (victim->in_room != NOWHERE)
	  char_from_room(victim);
      char_to_room(victim, 1);
      save_char(victim,victim->specials.was_in_room);
    }
  }

/*if (GET_POS(victim) == POSITION_DEAD) {
    if (ch->specials.fighting == victim)
      stop_fighting(ch);
  }

  if (!AWAKE(victim))
    if (victim->specials.fighting)
      stop_fighting(victim); */

  if (GET_POS(victim) == POSITION_DEAD) {
    stop_fighting(victim);
    if (IS_MOB(victim)) {
      if (IS_AFFECTED(ch, AFF_GROUP) && (ch->master || ch->followers)) {
	group_gain(ch, victim);
      } else {
	/* Calculate level-difference bonus */
	exp = MIN(MAX(1,GET_EXP(victim)),MAX_XP_GAIN(ch));
	if (IS_NPC(victim)) {
	  sendf(ch,"You gain %d experience points.\n",
		NEXP(ch,modify_exp(ch,exp,victim->virtual)));
	  gain_exp(ch, modify_exp(ch,exp,victim->virtual));
	}
	change_alignment(ch, victim);
      }
    } else {
      if (victim->in_room != NOWHERE) {
        if (ROOM_FLAG(victim->in_room,ARENA))
	  sports_cast("%s was defeated by %s.",GET_NAME(victim),GET_NAME(ch));
	vlog(LOG_DEATH,"%s killed by %s at %s [%d]",
		GET_NAME(victim),
		(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
		(real_roomp(victim->in_room))->name,ch->in_room);
	if (GetMaxLevel(ch) > 10 && IS_NPC(ch)) {
	  sprintf(buf, "Ha!  %s died too easily! I want a tougher challenge!",
		GET_NAME(victim));
	  do_gossip(ch, buf, 0); }
      } else {
	vlog(LOG_URGENT|LOG_DEATH,"%s killed by %s at Nowhere.",
		GET_NAME(victim),
		(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
      }
    }
    if (IS_MOB(victim))
      add_kill(ch,victim->virtual);
    die(victim);
    /*  if the victim is dead, return TRUE. */
    victim = 0;
    return(TRUE);
  }
 /* EXP for hitting */ 

  if (IS_MOB(victim)) {
    GET_EXP(ch)+=dam/6;
    GET_EXP(victim)-=dam/6;
  }
  return(FALSE); 
}


int HeightClass(struct char_data *ch)
{

  if(GET_HEIGHT(ch) <= 60) /* 2'0" */
    return(0);
  else if(GET_HEIGHT(ch) <= 135) /* 4'6" */
    return(1);
  else if(GET_HEIGHT(ch) <= 185) /* 6'2" */
    return(2);
  else if(GET_HEIGHT(ch) <= 240) /* 8'0" */
    return(3);
  else if(GET_HEIGHT(ch) <= 360) /* 12' 0" */
    return(4);
  else if(GET_HEIGHT(ch) <= 480) /* 16' 0" */
    return(5);
  else
    return(6);
}

void hit(struct char_data *ch, struct char_data *victim, int type)
{
  struct obj_data *wielded = NULL;
  struct obj_data *held = NULL;
  struct obj_data *obj = 0;
  int w_type, hw_type=0;
  int victim_ac, calc_thaco, calc_hthaco;
  int absorb, dam, dead=0;
  byte diceroll, hdiceroll=0;
  struct room_data	*rp;
  int xtra_attack = 0;	/* is this extra attack skill or normal attack */
  extern int locations[11][10];
  extern struct str_app_type str_app[];
  extern struct dex_app_type dex_app[];

  if (!CORRUPT(ch,victim)) return;
  rp = real_roomp(ch->in_room);
  if (rp && rp->room_flags&PEACEFUL) {
    vlog(LOG_DEBUG,"hit() called in PEACEFUL room");
    return;
  }

  if (ch->in_room != victim->in_room) {
    vlog(LOG_DEBUG,"Not in same room when fighting: %s, %s",
	ch->player.name, victim->player.name);
    return;
  }

  if (victim->attackers >= 6 && ch->specials.fighting != victim) {
    send_to_char("You can't attack them, no room!\n", ch);
    return;
  }

  if ((ch->attackers >= 6) && (victim->specials.fighting != ch)) {
    send_to_char("There are too many other people in the way.\n", ch);
    return;
  }

  GET_MOVE(ch) -= 1;

  if (victim == ch) {
    if (Hates(ch,victim)) {
	log("FIX ME (fight.c)");
      /*RemHatred(ch, victim->hateful); ??? FIX ??? */
    }
    return;
  }

  if (type >= SKILL_XATTACK1 && type <= SKILL_XATTACK9) {
    xtra_attack = type - 10000; /* it's true */
    type = TYPE_UNDEFINED;
  }

  if (IS_WEAPON(ch->equipment[HOLD])) {
    held = ch->equipment[HOLD];
    switch (held->obj_flags.value[3]) {
    case 0  : hw_type = TYPE_SMITE; break;
    case 1  : hw_type = TYPE_STAB;  break;
    case 2  : hw_type = TYPE_WHIP; break;
    case 3  : hw_type = TYPE_SLASH; break;
    case 4  : hw_type = TYPE_SMASH; break;
    case 5  : hw_type = TYPE_CLEAVE; break;
    case 6  : hw_type = TYPE_CRUSH; break;
    case 7  : hw_type = TYPE_BLUDGEON; break;
    case 8  : hw_type = TYPE_CLAW; break;
    case 9  : hw_type = TYPE_BITE; break;
    case 10 : hw_type = TYPE_STING; break;
    case 11 : hw_type = TYPE_PIERCE; break;

      default : w_type = TYPE_HIT; break;
    }
  }

  if (IS_WEAPON(ch->equipment[WIELD])) {
    wielded = ch->equipment[WIELD];
    switch (wielded->obj_flags.value[3]) {
    case 0  : w_type = TYPE_SMITE; break;
    case 1  : w_type = TYPE_STAB;  break;
    case 2  : w_type = TYPE_WHIP; break;
    case 3  : w_type = TYPE_SLASH; break;
    case 4  : w_type = TYPE_SMASH; break;
    case 5  : w_type = TYPE_CLEAVE; break;
    case 6  : w_type = TYPE_CRUSH; break;
    case 7  : w_type = TYPE_BLUDGEON; break;
    case 8  : w_type = TYPE_CLAW; break;
    case 9  : w_type = TYPE_BITE; break;
    case 10 : w_type = TYPE_STING; break;
    case 11 : w_type = TYPE_PIERCE; break;

      default : w_type = TYPE_HIT; break;
    }
  }	else {
    if (IS_NPC(ch) && (ch->specials.attack_type >= TYPE_HIT))
      w_type = ch->specials.attack_type;
    else
      w_type = TYPE_HIT;
  }

  /* Calculate the raw armor including magic armor */

  if (!IS_NPC(ch))
    calc_thaco = THACO(ch);
  else
    /* THAC0 for monsters is set in the HitRoll */
    calc_thaco = 20;

  /* held thaco is worse than normal */
  calc_hthaco = (20 - get_skill(ch, SKILL_DUAL_WIELD) / 5) + calc_thaco + 2;
  calc_thaco -= GET_HITROLL(ch);

  diceroll = number(1,20);
  if(wielded){
    if(wielded->obj_flags.value[0] && !IS_NPC(ch)){
      diceroll -= ((100 - get_skill(ch, wielded->obj_flags.value[0] +
	PROF_BASE)) / 10);
    }
  }
  diceroll = MAX(1, diceroll);

  if(held){
    hdiceroll = number(1, 20);
    if(held->obj_flags.value[0] && !IS_NPC(ch)){
      hdiceroll -= ((100 - get_skill(ch, held->obj_flags.value[0] +
	PROF_BASE)) / 10);
    }
    hdiceroll = MAX(1, hdiceroll);
  }

  if(!ch->points.aimloc)
     ch->points.aimloc = LOCATION_BODY;

  if(type <= 0) {
     ch->points.hitloc = (number(0,5) ?
           ch->points.aimloc :
           locations[HeightClass(ch)-HeightClass(victim)+6][number(0,9)]);
  } else {
     ch->points.hitloc = LOCATION_BODY; /* backstabs, kick, bashes, etc. */
  }

  if (ch->points.aimloc>=LOCATION_MAX || ch->points.aimloc<0)
	ch->points.aimloc=LOCATION_BODY;
  if (ch->points.hitloc>=LOCATION_MAX || ch->points.hitloc<0)
	ch->points.hitloc=LOCATION_BODY;

  victim_ac  = victim->points.armor[ch->points.hitloc]/10;

  if (!AWAKE(victim))
    victim_ac -= dex_app[GET_DEX(victim)].defensive;

  victim_ac = MAX(-10, victim_ac);  /* -10 is lowest */

  if (IS_SET(ch->specials.act,PLR_DEBUG)) {
	sendf(ch,"Dice: %d, Calc_thaco: %d, Victim_AC: %d\n",
			diceroll,
			calc_thaco,
			victim_ac);
  }
  if ((diceroll < 20) && AWAKE(victim) &&
      ((diceroll==1) || ((calc_thaco-diceroll) > victim_ac))) {
      /* oops, we missed */
    if (type == SKILL_BACKSTAB)
      damage(ch, victim, 0, SKILL_BACKSTAB);
    else {
      damage(ch, victim, 0, w_type);
    }
    /* learn from our mistake */
    if (xtra_attack) {
    	NewFailSkill (ch, xtra_attack);
    }
  } else {

    dam = str_app[GET_STR(ch)].todam;
    dam += GET_DAMROLL(ch);

    if (!wielded) {
      if (IS_NPC(ch))
	dam += dice(ch->specials.damnodice, ch->specials.damsizedice);
      else
	dam += number(0,2);  /* Max. 2 dam with bare hands */
    } else {
      if (wielded->obj_flags.value[2]) {
        dam += dice(wielded->obj_flags.value[1], wielded->obj_flags.value[2]);
	if (wielded->obj_flags.value[0] && !IS_NPC(ch)) {
	  dam = ((dam * get_skill(ch, wielded->obj_flags.value[0] +
	    PROF_BASE)) / 100);
        }
      } else {
        act("$p snaps into pieces!", TRUE, ch, wielded, 0, TO_CHAR);
	act("$p snaps into pieces!", TRUE, ch, wielded, 0, TO_ROOM);
	if ((obj = unequip_char(ch, WIELD))!=NULL) {
	  MakeScrap(ch, obj);
	  dam += 1;
	}
      }
    }

    if (GET_POS(victim) < POSITION_FIGHTING)
      dam *= 1+(POSITION_FIGHTING-GET_POS(victim))/3;
    /* Position  sitting  x 1.33 */
    /* Position  resting  x 1.66 */
    /* Position  sleeping x 2.00 */
    /* Position  stunned  x 2.33 */
    /* Position  incap    x 2.66 */
    /* Position  mortally x 3.00 */

    if (GET_POS(victim) <= POSITION_DEAD)
      return;

    if(!type) /* Hitloc is only valid for melee combat  */
    switch(ch->points.hitloc) {
      case LOCATION_FEET: dam *= 0.6;
      break;
      case LOCATION_LEGS: dam *= 0.8;
      break;
      case LOCATION_BODY: dam *= 1.0;
      break;
      case LOCATION_ARMS: dam *= 0.8;
      break;
      case LOCATION_HEAD: dam *= 1.8;
      break;
      default:
        nlog("Error: hitloc out of range (fight.c : hit()) hitloc = %d",ch->points.hitloc);
      break;
    }

    dam = MAX(1, dam);  /* Not less than 0 damage */

    /* Calculate how much damage the PC can absorb */

    absorb = number(0 ,victim->points.stopping[ch->points.hitloc]);

    if(victim_ac  - (calc_thaco -  diceroll) > 0)
       absorb -= 2 * (victim_ac - (calc_thaco - diceroll));

    switch(w_type) {
      case TYPE_HIT: absorb *= 1.0; break;
      case TYPE_SMITE: absorb *= 1.2; break;
      case TYPE_STAB: absorb *= 0.8; break;
      case TYPE_WHIP: absorb *= 1.4; break;
      case TYPE_SLASH: absorb *= 0.8; break;
      case TYPE_SMASH: absorb *= 1.2; break;
      case TYPE_CLEAVE: absorb *= 0.7; break;
      case TYPE_CRUSH: absorb *= 1.2; break;
      case TYPE_BLUDGEON: absorb *= 1.3; break;
      case TYPE_CLAW: absorb *= 0.8; break;
      case TYPE_BITE: absorb *= 0.8; break;
      case TYPE_STING: absorb *= 0.8; break;
      case TYPE_PIERCE: absorb *= 0.8; break;
      default:
        log("Error: w_type out of range (fight.c : hit())");
      break;
    }

    absorb = MAX(absorb,0);

    dam -= absorb; /* Armor reduces some damage */

    if(dam <= 0) {
     start_fighting(ch,victim);
     dam_message(-1,ch,victim,w_type); /* Display armor msgs */
     return;
    }

    if (type == SKILL_BACKSTAB) {
      int mult,i;
      mult = 2;
      for (i=SKILL_BACKSTAB1; i<=SKILL_BACKSTAB10; i++)
	if (number(1,101)<get_skill(ch,i)) mult++;
      dam *= mult;
      dead = damage(ch, victim, dam, SKILL_BACKSTAB);
    } else {
      dead = damage(ch, victim, dam, w_type);

      /*
       *  if the victim survives, lets hit him with a
       *  weapon spell
       */

      if (!dead) {
	WeaponSpell(ch,victim,w_type);
      }
    }
  }
  if (!dead && held && (diceroll > 18) &&
      ((calc_thaco - diceroll) > victim_ac) && (type != SKILL_BACKSTAB)) {
    if ((hdiceroll < 20) && AWAKE(victim) &&
        (((calc_hthaco-hdiceroll) > victim_ac))) {
      /* oops, we missed */
      damage(ch, victim, 0, w_type);
      /* learn from our mistake */
      if (xtra_attack) {
      	NewFailSkill (ch, xtra_attack);
      }
    } else {
      dam = str_app[GET_STR(ch)].todam;
  
      if (held->obj_flags.value[2]) {
        dam += dice(held->obj_flags.value[1], held->obj_flags.value[2]);
        if (held->obj_flags.value[0] && !IS_NPC(ch)) {
          dam = ((dam * get_skill(ch, held->obj_flags.value[0] + PROF_BASE)) /
	    100);
        }
      }
      else {
        act("$p snaps into pieces!", TRUE, ch, held, 0, TO_CHAR);
  	act("$p snaps into pieces!", TRUE, ch, held, 0, TO_ROOM);
  	if ((obj = unequip_char(ch, HOLD)) != NULL) {
  	  MakeScrap(ch, obj);
  	  dam += 1;
        }
      }
  
      if (GET_POS(victim) < POSITION_FIGHTING)
        dam *= 1+(POSITION_FIGHTING-GET_POS(victim))/3;
      /* Position  sitting  x 1.33 */
      /* Position  resting  x 1.66 */
      /* Position  sleeping x 2.00 */
      /* Position  stunned  x 2.33 */
      /* Position  incap    x 2.66 */
      /* Position  mortally x 3.00 */
  
      if (GET_POS(victim) <= POSITION_DEAD)
        return;
  
      if(!type) /* Hitloc is only valid for melee combat  */
      switch(ch->points.hitloc) {
        case LOCATION_FEET: dam *= 0.6;
        break;
        case LOCATION_LEGS: dam *= 0.8;
        break;
        case LOCATION_BODY: dam *= 1.0;
        break;
        case LOCATION_ARMS: dam *= 0.8;
        break;
        case LOCATION_HEAD: dam *= 1.8;
        break;
        default:
          nlog("Error: hitloc out of range (fight.c : hit()) hitloc = %d",ch->points.hitloc);
        break;
      }
  
      dam = MAX(1, dam);  /* Not less than 0 damage */
  
      /* Calculate how much damage the PC can absorb */
  
      absorb = number(0 ,victim->points.stopping[ch->points.hitloc]);
  
      if(victim_ac  - (calc_hthaco -  hdiceroll) > 0)
         absorb -= 2 * (victim_ac - (calc_hthaco - hdiceroll));
  
      switch(hw_type) {
        case TYPE_HIT: absorb *= 1.0; break;
        case TYPE_SMITE: absorb *= 1.2; break;
        case TYPE_STAB: absorb *= 0.8; break;
        case TYPE_WHIP: absorb *= 1.4; break;
        case TYPE_SLASH: absorb *= 0.8; break;
        case TYPE_SMASH: absorb *= 1.2; break;
        case TYPE_CLEAVE: absorb *= 0.7; break;
        case TYPE_CRUSH: absorb *= 1.2; break;
        case TYPE_BLUDGEON: absorb *= 1.3; break;
        case TYPE_CLAW: absorb *= 0.8; break;
        case TYPE_BITE: absorb *= 0.8; break;
        case TYPE_STING: absorb *= 0.8; break;
        case TYPE_PIERCE: absorb *= 0.8; break;
        default:
          log("Error: hw_type out of range (fight.c : hit())");
        break;
      }
  
      absorb = MAX(absorb,0);
  
      dam -= absorb; /* Armor reduces some damage */
  
      if(dam <= 0) {
       start_fighting(ch,victim);
       dam_message(-1,ch,victim,hw_type); /* Display armor msgs */
       return;
      }
  
      dead = damage(ch, victim, dam, hw_type);
  
      /*
       *  if the victim survives, lets hit him with a
       *  weapon spell
       */
  
      if (!dead) {
  	WeaponSpell(ch,victim,hw_type);
      }
    }
  }
}

void mob_attack(struct char_data *ch,struct char_data *vict)
{
  int which;
  if (ch->nskills>0 && number(1,101)<ch->specials.magic_use) {
    which=number(1,ch->nskills)-1;
    switch (ch->skills[which].skill_number) {
      case SKILL_BASH: do_bash(ch,"",-1); break; /* bash person fighting */
      case SKILL_KICK: do_kick(ch,"",-1); break; /* kick person fighting */
      case SPELL_FIREBALL: /* base level=50 */
/* the mobs that have this spell should be at least 50st level (or you
   can change the below) -- but you get the gist of it */
	act("$n utters the words, 'Burn in hell!'",FALSE,ch,0,0,TO_ROOM);
	spell_fireball(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-49);
	break;					 /* ^ effective level */
      case SPELL_BLINDNESS: /* base level = 0 */
	act("$n utters the words, 'Lights out'",FALSE,ch,0,0,TO_ROOM);
	spell_blindness(GetMaxLevel(ch),ch,vict,NULL,0);
	break;
      case SPELL_BURNING_HANDS: /* base level = 1 */
	act("$n utters the words, 'Is it hot in here or what?'",FALSE,ch,0,0,TO_ROOM);
	spell_burning_hands(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch));
	break;
      case SPELL_MAGIC_MISSILE: /* base level = 3 */
	act("$n utters the words, 'Look ma, no hands!'",FALSE,ch,0,0,TO_ROOM);
	spell_magic_missile(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-2);
	break;
      case SPELL_CHILL_TOUCH: /* base level = 0 */
	act("$n utters the words, 'Brrrrr!'",FALSE,ch,0,0,TO_ROOM);
	spell_chill_touch(GetMaxLevel(ch),ch,vict,NULL,0);
	break;
      case SPELL_COLOUR_SPRAY: /* base level = 55 */
	act("$n utters the words, 'Look at the pretty colors!'",FALSE,ch,0,0,TO_ROOM);
	spell_colour_spray(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-54);
	break;
      case SPELL_CURSE: /* base level = 0 */
	act("$n utters the words, 'Uh oh... who broke the mirror!'",FALSE,ch,0,0,TO_ROOM);
	spell_curse(GetMaxLevel(ch),ch,vict,NULL,0);
	break;
      case SPELL_EARTHQUAKE: /* damaged based on level, not effective level */
	act("$n utters the words, 'Rock and roll!'",FALSE,ch,0,0,TO_ROOM);
	spell_earthquake(GetMaxLevel(ch),ch,vict,NULL,0);
	break;
      case SPELL_HARM: /* damge based on level */
	act("$n utters the words, 'This won't hurt!'",FALSE,ch,0,0,TO_ROOM);
	spell_harm(GetMaxLevel(ch),ch,vict,NULL,0);
	break;
      case SPELL_LIGHTNING_BOLT: /* base level=12 */
	act("$n utters the words, 'Zaaaaappp!'",FALSE,ch,0,0,TO_ROOM);
	spell_lightning_bolt(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-11);
	break;
      case SPELL_POISON: /* base level=0 */
	act("$n utters the words, 'Look out for the snake!'",FALSE,ch,0,0,TO_ROOM);
	spell_poison(GetMaxLevel(ch),ch,vict,NULL,0);
	break;
      case SPELL_SHOCKING_GRASP: /* base level=5 */
	act("$n utters the words, 'Pzzt!'",FALSE,ch,0,0,TO_ROOM);
	spell_shocking_grasp(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-4);
	break;
      case SPELL_SLEEP: /* base level = 0*/
	act("$n utters the words, 'Are you tired?'",FALSE,ch,0,0,TO_ROOM);
	spell_sleep(GetMaxLevel(ch),ch,vict,NULL,0);
	break;
      case SPELL_ENERGY_DRAIN: /* base level=5 */
        act("$n utters the words, 'you have to much energy!'",FALSE,ch,0,0,TO_ROOM);
        spell_energy_drain(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-4);
        break;
      case SPELL_CHAIN_LIGHTNING:
        act("$n utters the words, 'pretty sparks'",FALSE,ch,0,0,TO_ROOM);
        spell_chain_lightn(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-49);
        break;
      case SPELL_WORD_OF_RECALL:
        act("$n utters the words, 'leave me alone and begone!'",FALSE,ch,0,0,TO_ROOM);
        spell_word_of_recall(GetMaxLevel(ch),ch,vict,NULL,0);
        break; 
      case SPELL_FLAMESTRIKE:
        act("$n utters the words, 'ooh look at the lines of fire'",FALSE,ch,0,0,TO_ROOM);
        spell_flamestrike(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-49);
        break;
      case SPELL_DISPEL_GOOD:
        act("$n utters the words, 'goody goody begone!'",FALSE,ch,0,0,TO_ROOM);
        spell_dispel_good(GetMaxLevel(ch),ch,vict,NULL,0);
        break;
      case SPELL_WEAKNESS:
        act("$n utters the words, 'oh my you look tired you should relax'",FALSE,ch,0,0,TO_ROOM);
        spell_weakness(GetMaxLevel(ch),ch,vict,NULL,0);
        break;
      case SPELL_DISPEL_MAGIC:
        act("$n utters the words, 'oh nasty magic'",FALSE,ch,0,0,TO_ROOM);
        spell_dispel_magic(GetMaxLevel(ch),ch,vict,NULL,0);
        break;
      case SPELL_FEAR:
        act("$n utters the words, 'you look scared..'",FALSE,ch,0,0,TO_ROOM);
        spell_fear(GetMaxLevel(ch),ch,vict,NULL,0);
        break;
      case SPELL_ACID_BLAST:
        act("$n utters the words, 'wow watch that armor melt!'",FALSE,ch,0,0,TO_ROOM);
        spell_acid_blast(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-19);
        break;
      case SPELL_CONE_OF_COLD:
        act("$n utters the words, 'watch out for flying ice!'",FALSE,ch,0,0,TO_ROOM);
        spell_cone_of_cold(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-19);
	break;
      case SPELL_METEOR_SWARM:
	act("$n utters the words, 'watch out for flying meteors!'",FALSE,ch,0,0,TO_ROOM);
        spell_meteor_swarm(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-39);
	break;
      case SPELL_ICE_STORM:
        act("$n utters the words, 'a blizzard!'",FALSE,ch,0,0,TO_ROOM);
	spell_ice_storm(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-39);
	break;
      case SPELL_WARP_WEAPON:
	act("$n utters the words, 'do the time warp!'",FALSE,ch,0,0,TO_ROOM);
	spell_warp_weapon(GetMaxLevel(ch),ch,vict,NULL,0);
	break;
      case SPELL_SHRIEK:
	act("$n screams loudly",FALSE,ch,0,0,TO_ROOM);
	spell_shriek(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-39);
	break;
      case SPELL_MAGE_FIRE:
	act("$n utters the words, 'burn burn burn'",FALSE,ch,0,0,TO_ROOM);
	spell_mage_fire(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-39);
	break;
      case SPELL_FIRESTORM:
	act("$n utters the words, 'nothing like a clean sweep'",FALSE,ch,0,0,TO_ROOM);
	spell_firestorm(GetMaxLevel(ch),ch,vict,NULL,GetMaxLevel(ch)-39);
	break;
      default:
	nlog("Mob attacking %d no case for %d",
		ch->virtual,
		ch->skills[which].skill_number);
    }
    return;
  }
  hit(ch,vict,TYPE_UNDEFINED);
}

void event_strike(event_t *event)
{
  struct char_data *ch;

  ch = event->ch;
  if(!ch->specials.fighting)
    return;

  if(AWAKE(ch) && (ch->specials.fighting->in_room == ch->in_room)){
    if(check_peaceful(ch, "A strange force prevents you from striking.\n")){
      vlog(LOG_DEBUG, "event_strike found %s fighting in a peaceful room.",
	GET_NAME(ch));
      return;
    }
    if(IS_NPC(ch))
      DevelopHatred(ch, ch->specials.fighting);
    if(CAN_FIGHT(ch, ch->specials.fighting)){
/*    if(IS_NPC(ch)) mob_attack(ch, ch->specials.fighting);
      else           hit(ch, ch->specials.fighting, TYPE_UNDEFINED); */
      hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
    }
    if(AWAKE(ch) && ch->specials.fighting){
      if(ch->specials.fighting->in_room == ch->in_room){
        /* set up the next blow */
        add_event(calc_speed(ch, 0), EVT_STRIKE, ch, NULL, NULL, -1, 0, NULL, event_strike);
      }
      else{
        stop_fighting(ch);
      }
    }
  }
  else if(AWAKE(ch)){
    stop_fighting(ch);
  }
}

struct char_data *FindVictim( struct char_data *ch)
{
  struct char_data *tmp_ch;
  unsigned char found=FALSE;
  unsigned short ftot=0,ttot=0,ctot=0,ntot=0,mtot=0;
  unsigned short total;
  unsigned short fjump=0,njump=0,cjump=0,mjump=0,tjump=0;

  if (ch->in_room < 0) return(0);

  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))&&
	(!IS_AFFECTED(tmp_ch, AFF_SNEAK)) && (ch!=tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {
	if (!IS_NPC(tmp_ch)||(IS_SET(tmp_ch->specials.act, ACT_ANNOYING))) {
	  if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	    found = TRUE;  /* a potential victim has been found */
	    if (!IS_NPC(tmp_ch)) {
	      if(is_fighter(tmp_ch))
		ftot++;
	      else if (is_cleric(tmp_ch))
		ctot++;
	      else if (is_magicu(tmp_ch))
		mtot++;
	      else if (is_thief(tmp_ch))
		ttot++;
	    } else {
	      ntot++;
	    }
	  }
	}
      }
    }
  }

  /* if no legal enemies have been found, return 0 */

  if (!found) {
    return(0);
  }

  /*
    give higher priority to fighters, clerics, thieves, magic users if int <= 12
    give higher priority to fighters, clerics, magic users thieves is inv > 12
    give higher priority to magic users, fighters, clerics, thieves if int > 15
    */

  /*
    choose a target
    */

  if (ch->abilities.intel <= 3) {
    fjump=2; cjump=2; tjump=2; njump=2; mjump=2;
  } else if (ch->abilities.intel <= 9) {
    fjump=4; cjump=3;tjump=2;njump=2;mjump=1;
  } else if (ch->abilities.intel <= 12) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=2;
  } else if (ch->abilities.intel <= 15) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=3;
  } else {
    fjump=3;cjump=3;tjump=2;njump=1;mjump=3;
  }

  total = (fjump*ftot)+(cjump*ctot)+(tjump*ttot)+(njump*ntot)+(mjump*mtot);

  total = number(1,total);

  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))&&
	(!IS_AFFECTED(tmp_ch, AFF_SNEAK)) && (ch != tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {
	if (!IS_NPC(tmp_ch) || (IS_SET(tmp_ch->specials.act, ACT_ANNOYING))) {
	  if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	    if (IS_NPC(tmp_ch)) {
	      total -= njump;
	    } else if (is_fighter(tmp_ch)) {
	      total -= fjump;
	    } else if (is_cleric(tmp_ch)) {
	      total -= cjump;
	    } else if (is_magicu(tmp_ch)) {
	      total -= mjump;
	    } else {
	      total -= tjump;
	    }
	    if (total <= 0)
	      return(tmp_ch);
	  }
	}
      }
    }
  }

  if (ch->specials.fighting)
    return(ch->specials.fighting);

  return(0);
}

struct char_data *FindAnyVictim( struct char_data *ch)
{
  struct char_data *tmp_ch;
  unsigned char found=FALSE;
  unsigned short ftot=0,ttot=0,ctot=0,ntot=0,mtot=0;
  unsigned short total;
  unsigned short fjump=0,njump=0,cjump=0,mjump=0,tjump=0;

  if (!real_roomp(ch->in_room)) return(0);

  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))) {
      if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	if (!SameRace(ch, tmp_ch) || (!IS_NPC(tmp_ch))) {
	  found = TRUE;  /* a potential victim has been found */
	  if (!IS_NPC(tmp_ch)) {
	    if(is_fighter(tmp_ch))
	      ftot++;
	    else if (is_cleric(tmp_ch))
	      ctot++;
	    else if (is_magicu(tmp_ch))
	      mtot++;
	    else if (is_thief(tmp_ch))
	      ttot++;
	  } else {
	    ntot++;
	  }
	}
      }
    }
  }

  /* if no legal enemies have been found, return 0 */

  if (!found) {
    return(0);
  }

  /*
    give higher priority to fighters, clerics, thieves, magic users if int <= 12
    give higher priority to fighters, clerics, magic users thieves is inv > 12
    give higher priority to magic users, fighters, clerics, thieves if int > 15
    */

  /*
    choose a target
    */

  if (ch->abilities.intel <= 3) {
    fjump=2; cjump=2; tjump=2; njump=2; mjump=2;
  } else if (ch->abilities.intel <= 9) {
    fjump=4; cjump=3;tjump=2;njump=2;mjump=1;
  } else if (ch->abilities.intel <= 12) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=2;
  } else if (ch->abilities.intel <= 15) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=3;
  } else {
    fjump=3;cjump=3;tjump=2;njump=1;mjump=3;
  }

  total = (fjump*ftot)+(cjump*ctot)+(tjump*ttot)+(njump*ntot)+(mjump*mtot);

  total = number(1,total);

  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))) {
      if (!SameRace(tmp_ch, ch) || (!IS_NPC(tmp_ch))) {
	if (IS_NPC(tmp_ch)) {
	  total -= njump;
	} else if (is_fighter(tmp_ch)) {
	  total -= fjump;
	} else if (is_cleric(tmp_ch)) {
	  total -= cjump;
	} else if (is_magicu(tmp_ch)) {
	  total -= mjump;
	} else {
	  total -= tjump;
	}
	if (total <= 0)
	  return(tmp_ch);
      }
    }
  }

  if (ch->specials.fighting)
    return(ch->specials.fighting);

  return(0);

}


int PreProcDam(struct char_data *ch, int type, int dam)
{

  unsigned Our_Bit=0;

  /*
    long, intricate list, with the various bits and the various spells and
    such determined
    */

  switch (type) {
  case SPELL_FIREBALL:
  case SPELL_BURNING_HANDS:
  case SPELL_FLAMESTRIKE:
  case SPELL_FIRE_BREATH:
  case ROOM_FIRE:
    Our_Bit = IMM_FIRE;
    break;

  case SPELL_SHOCKING_GRASP:
  case SPELL_LIGHTNING_BOLT:
  case SPELL_CALL_LIGHTNING:
  case SPELL_LIGHTNING_BREATH:
    Our_Bit = IMM_ELEC;
    break;
  case SPELL_CHILL_TOUCH:		
  case SPELL_CONE_OF_COLD:		
  case SPELL_ICE_STORM:			
  case SPELL_FROST_BREATH:
    case ROOM_COLD:
    Our_Bit = IMM_COLD;
    break;

  case SPELL_MAGIC_MISSILE:
  case SPELL_COLOUR_SPRAY:
  case SPELL_GAS_BREATH:
  case SPELL_METEOR_SWARM:				
    Our_Bit = IMM_ENERGY;
    break;

  case SPELL_ENERGY_DRAIN:
    Our_Bit = IMM_DRAIN;
    break;

  case SPELL_ACID_BREATH:
  case SPELL_ACID_BLAST:
    Our_Bit = IMM_ACID;
    break;
  case SKILL_BACKSTAB:				
    Our_Bit = IMM_BACKSTAB;
    break;
  case TYPE_PIERCE:
  case TYPE_STING:
  case TYPE_STAB:
    Our_Bit = IMM_PIERCE;
    break;
  case TYPE_SLASH:
  case TYPE_WHIP:
  case TYPE_CLEAVE:
  case TYPE_CLAW:
    Our_Bit = IMM_SLASH;
    break;
  case TYPE_BLUDGEON:
  case TYPE_HIT:
  case SKILL_KICK:
  case TYPE_CRUSH:
  case TYPE_BITE:
  case TYPE_SMASH:
  case TYPE_SMITE:
    Our_Bit = IMM_BLUNT;
    break;
  case SPELL_POISON:
    Our_Bit = IMM_POISON;
    break;
  default:
    return(dam);
    break;
  }

  if (IS_SET(ch->specials.susc, Our_Bit))
    dam *= 2;

  if (IS_SET(ch->specials.immune, Our_Bit))
    dam /= 2;

  if (IS_SET(ch->specials.M_immune, Our_Bit) ||
      ((GET_RACE(ch) == RACE_UNDEAD) && (type == SKILL_BACKSTAB)))
    dam = 0;

  return(dam);
}


int DamageOneItem( struct char_data *ch, int dam_type, struct obj_data *obj)
{
  int num;
  char buf[256];

  if (GET_ITEM_TYPE(obj)==ITEM_SPELL_POUCH) return(FALSE);
  num = DamagedByAttack(obj, dam_type);
  if (num != 0) {
    sprintf(buf, "%s is %s.\n",obj->short_description,
	    ItemDamType[dam_type-1]);
    send_to_char(buf,ch);
    if (num == -1) {  /* destroy object if fail one last save */
      if (!ItemSave(obj, dam_type)) {
	return(TRUE);
      }
    } else {   /* "damage item"  (armor), (weapon) */
      if (DamageItem(ch, obj, num)) {
	return(TRUE);
      }
    }
  }
  return(FALSE);

}


void MakeScrap( struct char_data *ch, struct obj_data *obj)
{
  char buf[200];
  struct obj_data *t;

  act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_CHAR);
  act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_ROOM);

  t = get_obj(30);

  sprintf(buf, "Scraps from %s lie in a pile here.",
	  obj->short_description);

  t->obj_flags.timer2=10;
  t->description = mystrdup(buf);
  obj_to_room(t, ch->in_room);
  obj_from_char(obj);
  extract_obj(obj);
}

void DamageAllStuff( struct char_data *ch, int dam_type)
{
  int j;
  struct obj_data *obj, *next;

  /* this procedure takes all of the items in equipment and inventory
     and damages the ones that should be damaged */

  /* equipment */

  for (j = 0; j < MAX_WEAR; j++) {
    if (ch->equipment[j]) {
      obj = ch->equipment[j];
      if (DamageOneItem(ch, dam_type, obj)) { /* TRUE == destroyed */
	if ((obj = unequip_char(ch,j))!=NULL) {
	  MakeScrap(ch, obj);
	} else {
	  log("hmm, really wierd!");
	}
      }
    }
  }

  /* inventory */

  obj = ch->carrying;
  while (obj) {
    next = obj->next_content;
    if (DamageOneItem(ch, dam_type, obj)) {
      MakeScrap(ch, obj);
    }
    obj = next;
  }

}


int DamageItem(struct char_data *ch, struct obj_data *o, int num)
{
  int p;
  struct obj_data *t;

  /*  damage weaons or armor */

  if (ITEM_TYPE(o,ITEM_ARMOR)) {
    p=o->eq_pos;
    if (o->equipped_by)
	t=unequip_char(ch,o->eq_pos);
    else t=NULL;
    o->obj_flags.value[0] -= num;
    if (t)
      equip_char(ch,t,p);
    if (o->obj_flags.value[0] < 0)
      return(TRUE);
  } else if (ITEM_TYPE(o,ITEM_WEAPON)) {
    o->obj_flags.value[2] -= num;
    if (o->obj_flags.value[2] <= 0) {
      return(TRUE);
    }
  }
  return(FALSE);
}

int ItemSave( struct obj_data *i, int dam_type)
{
  int num, j;

  num = number(1,20);
  if (num <= 1) return(FALSE);
  if (num >= 20) return(TRUE);

  for(j=0; j<MAX_OBJ_AFFECT; j++)
    if ((i->affected[j].location == APPLY_SAVING_SPELL) ||
	(i->affected[j].location == APPLY_SAVE_ALL)) {
      num -= i->affected[j].modifier;
    }
  if (i->affected[j].location != APPLY_NONE) {
    num += 1;
  }
  if (i->affected[j].location == APPLY_HITROLL) {
    num += i->affected[j].modifier;
  }

  if (!ITEM_TYPE(i,ITEM_ARMOR))
    num += 1;

  if (num <= 1) return(FALSE);
  if (num >= 20) return(TRUE);

  if (num >= ItemSaveThrows[(int)GET_ITEM_TYPE(i)-1][dam_type-1]) {
    return(TRUE);
  } else {
    return(FALSE);
  }
}



int DamagedByAttack( struct obj_data *i, int dam_type)
{
  int num = 0;

  if (ITEM_TYPE(i,ITEM_ARMOR) || ITEM_TYPE(i,ITEM_WEAPON)){
    while (!ItemSave(i,dam_type)) {
      num+=1;
    }
    return(num / 2);
  } else {
    if (ItemSave(i, dam_type) || ItemSave(i,dam_type) || ItemSave(i,dam_type)) {
      return(0);
    } else {
      return(-1);
    }
  }
}

int WeaponCheck(struct char_data *ch, struct char_data *v, int type, int dam)
{
  int Immunity, total, j;

  Immunity = -1;
  if (IS_SET(ch->specials.M_immune, IMM_NONMAG)) {
    Immunity = 0;
  }
  if (IS_SET(ch->specials.M_immune, IMM_PLUS1)) {
    Immunity = 1;
  }
  if (IS_SET(ch->specials.M_immune, IMM_PLUS2)) {
    Immunity = 2;
  }
  if (IS_SET(ch->specials.M_immune, IMM_PLUS3)) {
    Immunity = 3;
  }
  if (IS_SET(ch->specials.M_immune, IMM_PLUS4)) {
    Immunity = 4;
  }

  if (Immunity < 0)
    return(dam);

  if ((type < TYPE_HIT) || (type > TYPE_SMITE))  {
    return(dam);
  } else {
    if (type == TYPE_HIT) {
      if (IS_NPC(ch) && (GetMaxLevel(ch) > (3*Immunity)+1)) {
	return(dam);
      } else {
	return(0);
      }
    } else {
      total = 0;
      if (!ch->equipment[WIELD])
	return(0);
      for(j=0; j<MAX_OBJ_AFFECT; j++)
	if ((ch->equipment[WIELD]->affected[j].location == APPLY_HITROLL) ||
	    (ch->equipment[WIELD]->affected[j].location == APPLY_HITNDAM)) {
	  total += ch->equipment[WIELD]->affected[j].modifier;
	}
      if (total > Immunity) {
	return(dam);
      } else {
	return(0);
      }
    }
  }
}


void DamageStuff(struct char_data *v, int type, int dam)
{
  int num, dam_type;
  struct obj_data *obj;

  if (type >= TYPE_HIT && type <= TYPE_SMITE) {
    num = number(3,17);  /* wear_neck through hold */
    if (v->equipment[num]) {
      if ((type == TYPE_BLUDGEON && dam > 10) ||
	  (type == TYPE_CRUSH && dam > 5) ||
	  (type == TYPE_SMASH && dam > 10) ||
	  (type == TYPE_BITE && dam > 15) ||
	  (type == TYPE_CLAW && dam > 20) ||
	  (type == TYPE_SLASH && dam > 30) ||
	  (type == TYPE_SMITE && dam > 10) ||
	  (type == TYPE_HIT && dam > 20)) {
	if (DamageOneItem(v, BLOW_DAMAGE, v->equipment[num])) {
	  if ((obj = unequip_char(v,num))!=NULL) {
	    MakeScrap(v, obj);
	  }
	}
      }
    }
  } else {
    dam_type = GetItemDamageType(type);
    if (dam_type) {
      DamageAllStuff(v, dam_type);
    }
  }

}


int GetItemDamageType( int type)
{

  switch(type) {
  case SPELL_FIREBALL:
  case SPELL_FLAMESTRIKE:
  case SPELL_FIRE_BREATH:
    return(FIRE_DAMAGE);
    break;

  case SPELL_LIGHTNING_BOLT:
  case SPELL_CALL_LIGHTNING:
  case SPELL_LIGHTNING_BREATH:
    return(ELEC_DAMAGE);
    break;

  case SPELL_CONE_OF_COLD:
  case SPELL_ICE_STORM:
  case SPELL_FROST_BREATH:
    return(COLD_DAMAGE);
    break;

  case SPELL_COLOUR_SPRAY:
  case SPELL_METEOR_SWARM:
  case SPELL_GAS_BREATH:
    return(BLOW_DAMAGE);
    break;

  case SPELL_ACID_BREATH:
  case SPELL_ACID_BLAST:
    return(ACID_DAMAGE);
  default:
    return(0);
    break;
  }

}

int SkipImmortals(struct char_data *v, int amnt)
{
  /* You can't damage an immortal! */

  if ((GetMaxLevel(v)>MAX_MORT) && !IS_NPC(v))
    amnt = 0;

  /* special type of monster */		
  if (IS_NPC(v) && (IS_SET(v->specials.act, ACT_IMMORTAL))) {
    amnt = -1;
  /*  send_to_char("Woops you missed..\n",ch);  */
  }
  return(amnt);

}


void WeaponSpell( struct char_data *c, struct char_data *v, int type)
{
  int j, num;

  if ((c->in_room == v->in_room) && (GET_POS(v) != POSITION_DEAD)) {
    if ((c->equipment[WIELD]) && ((type == TYPE_PIERCE) ||
				  (type == TYPE_SLASH)  ||
				  (type == TYPE_BLUDGEON))) {
      for(j=0; j<MAX_OBJ_AFFECT; j++)
	if (c->equipment[WIELD]->affected[j].location ==
	    APPLY_WEAPON_SPELL) {
	  num = c->equipment[WIELD]->affected[j].modifier;
	  ((*spell_info[num].spell_pointer)
	   (6, c, "", SPELL_TYPE_WAND, v, 0,6));
	}
    }
  }
}

struct char_data *FindAnAttacker(struct char_data *ch)
{
  struct char_data *tmp_ch;
  unsigned char found=FALSE;
  unsigned short ftot=0,ttot=0,ctot=0,ntot=0,mtot=0;
  unsigned short total;
  unsigned short fjump=0,njump=0,cjump=0,mjump=0,tjump=0;

  if (ch->in_room < 0) return(0);

  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;
       tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))&&
	(!IS_AFFECTED(tmp_ch, AFF_SNEAK)) && (ch!=tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {
	if (!IS_NPC(tmp_ch)||(IS_SET(tmp_ch->specials.act, ACT_ANNOYING))) {
	  if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	    if (tmp_ch->specials.fighting == ch) {
	      found = TRUE;  /* a potential victim has been found */
	      if (!IS_NPC(tmp_ch)) {
		if(is_fighter(tmp_ch))
		  ftot++;
		else if (is_cleric(tmp_ch))
		  ctot++;
		else if (is_magicu(tmp_ch))
		  mtot++;
		else if (is_thief(tmp_ch))
		  ttot++;
	      } else {
		ntot++;
	      }
	    }
	  }
	}
      }
    }
  }

  /* if no legal enemies have been found, return 0 */

  if (!found) {
    return(0);
  }

  /*
    give higher priority to fighters, clerics, thieves, magic users if int <= 12
    give higher priority to fighters, clerics, magic users thieves is inv > 12
    give higher priority to magic users, fighters, clerics, thieves if int > 15
    */

  /*
    choose a target
    */

  if (ch->abilities.intel <= 3) {
    fjump=2; cjump=2; tjump=2; njump=2; mjump=2;
  } else if (ch->abilities.intel <= 9) {
    fjump=4; cjump=3;tjump=2;njump=2;mjump=1;
  } else if (ch->abilities.intel <= 12) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=2;
  } else if (ch->abilities.intel <= 15) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=3;
  } else {
    fjump=3;cjump=3;tjump=2;njump=1;mjump=3;
  }

  total = (fjump*ftot)+(cjump*ctot)+(tjump*ttot)+(njump*ntot)+(mjump*mtot);

  total = number(1,total);

  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;
       tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))&&
	(!IS_AFFECTED(tmp_ch, AFF_SNEAK)) && (ch != tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {
	if (!IS_NPC(tmp_ch) || (IS_SET(tmp_ch->specials.act, ACT_ANNOYING))) {
	  if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	    if (tmp_ch->specials.fighting == ch) {
	      if (IS_NPC(tmp_ch)) {
		total -= njump;
	      } else if (is_fighter(tmp_ch)) {
		total -= fjump;
	      } else if (is_cleric(tmp_ch)) {
		total -= cjump;
	      } else if (is_magicu(tmp_ch)) {
		total -= mjump;
	      } else {
		total -= tjump;
	      }
	      if (total <= 0)
		return(tmp_ch);
	    }
	  }
	}
      }
    }
  }

  if (ch->specials.fighting)
    return(ch->specials.fighting);

  return(0);
}

#if 0
void shoot( struct char_data *ch, struct char_data *victim)
{

  /*
  **  check for bow and arrow.
  */

  bow = ch->equipment[HOLD];
  arrow = ch->equipment[WIELD];

  if (!bow) {
    send_to_char("You need a bow-like weapon\n", ch);
    return;
  } else if (!arrow) {
    send_to_char("You need a projectile to shoot!\n", ch);
  } else if (!bow && !arrow) {
    send_to_char("You need a bow-like item, and a projectile to shoot!\n");
  } else {
    arrowVnum = ObjVnum(arrow);
    found = FALSE;
    for (i = 0; i< 4 && !founde; i++) {
      if (bow->obj_flags.value[i] == arrowVnum) {
	found = TRUE;
      }
    }
    if (!found) {
      send_to_char("That projectile does not fit in that projector.\n", ch);
      return;
    }
    /*
    **  check for bonuses on the bow.
    */
    for(j=0; j<MAX_OBJ_AFFECT; j++)
	if (bow->affected[j].location ==
	    APPLY_ARROW_HIT_PLUS) {
	  hitbon += bow->affected[j].modifier;
	} else if (bow->affected[j].location ==
	    APPLY_ARROW_DAM_PLUS) {
	  dambon += bow->affected[j].modifier;
	}
    /*
    **   temporarily add those bonuses.
    */
    /*
    **   fire the weapon.
    */
  }
}
#endif

int SwitchTargets( struct char_data *ch, struct char_data *vict)
{

  if (!ch->specials.fighting) {
    hit(ch, vict, TYPE_UNDEFINED);
    return(TRUE);
  }

  if (ch->specials.fighting != vict) {
    if (ch->specials.fighting->specials.fighting == ch) {
      send_to_char("You can't shoot weapons at close range!\n", ch);
      return(FALSE);
    } else {
      stop_fighting(ch);
      hit(ch, vict, TYPE_UNDEFINED);
    }
  } else {
    hit(ch, vict, TYPE_UNDEFINED);
    return(TRUE);
  }
  return (FALSE);
}

void perform_special_rooms (int pulse)
{
  struct char_data *ch;
  struct room_data *rp;
  int dam;
  
  	if (pulse < 0) 
    		return;
  
  	for (ch = character_list; ch; ch = ch->next) {
    		if (IS_NPC(ch))
      			continue;
		rp = real_roomp (ch->in_room);
		if (rp == NULL)
			return;

		if ((rp->sector_type == SECT_FIRE) &&
	           (!affected_by_spell(ch, SPELL_PROTECT_ELEM_FIRE))){
			sendf (ch, "You burn.\n");
			dam = dice (5, 10);
			damage (ch, ch, dam, ROOM_FIRE);
		}
		if ((rp->sector_type == SECT_COLD) &&
                   (!affected_by_spell(ch, SPELL_PROTECT_ELEM_COLD))){
			sendf (ch, "You freeze.\n");
			dam = dice (5, 10);
			damage (ch, ch, dam, ROOM_COLD);
		}
                if ((rp->sector_type == SECT_UNDERWATER) &&
                   (!affected_by_spell(ch, SPELL_WATER_BREATH))){
                        sendf (ch, "You are drowning\n");
                        dam = dice (5, 10);                                     
                        damage (ch, ch, dam, ROOM_COLD);                        
                }
	}
}
