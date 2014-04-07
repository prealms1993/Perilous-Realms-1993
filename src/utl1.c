#include <structs.h>
#include <utils.h>

extern int DEBUG;
extern racial_adjustment race_adj[];

void log(char *str)
{
printf("%s\n",str);
}

/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
	int chk, i;


	if ((!arg2) || (!arg1))
		return(1);

	for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
	if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
	if (chk < 0)
		return (-1);
	else
		return (1);
	return(0);
}
/* Create a duplicate of a string */
char *mystrdup(char *source) {
	char *new;

	if (source) {
		CREATE(new, char, strlen(source) + 1);
		return(strcpy(new, source));
	}
	CREATE(new, char, 1);
	*new = '\0';
	return(new);
}


char *name_to_path(char *name)
{
  static char buffer[200];
  char *p,*q;

  sprintf(buffer,"lib/stash/%c/%s",name[0],name);
  p=buffer;
  for (p=buffer; *p; *p++)
    if (isupper(*p))
        *p=tolower(*p);
  return(buffer);
}

int number(int from, int to)
{
   if (to - from + 1 )
        return((random() % (to - from + 1)) + from);
   else
       return(from);
}

#include <stdarg.h>

#include <time.h>


void vlog(int level, char* fmt, ...) {
	va_list args;
	char s[MAX_STRING_LENGTH];

	va_start(args, fmt);
	vsprintf(s, fmt, args);
	va_end(args);
	log(s);
}

void nlog(char* fmt, ...) {
	va_list args;
	char s[MAX_STRING_LENGTH];

	va_start(args, fmt);
	vsprintf(s, fmt, args);
	va_end(args);
	log(s);
}
int norm(int min,int avg,int max)
{
  int i;
  int percent;
  double tick,amt;
  int range;

/* attemp to get a more "normal" random number */
  percent=0;
  for (i=0; i<10; i++) percent+=number(1,100);
  percent/=10;

  if (percent>50)
    range=max-avg;
  else
    range=avg-min;
  percent=percent-50;
  tick=range/50.0;
  amt=tick*percent;
  return(amt+avg);
}

s16 generate_weight(int race)
{
  int i,min,max,avg,percent;
  i=0;
  while (race_adj[i].race!=race && race_adj[i].race) i++;
  if (race_adj[i].race==race) {
    min=race_adj[i].weight[0];
    avg=race_adj[i].weight[1];
    max=race_adj[i].weight[2];
    return(norm(min,avg,max));
  } else return(175);
}

s16 generate_height(int race)
{
  int i,min,max,avg,percent;
  i=0;
  while (race_adj[i].race!=race && race_adj[i].race) i++;
  if (race_adj[i].race==race) {
    min=race_adj[i].height[0];
    avg=race_adj[i].height[1];
    max=race_adj[i].height[2];
    return(norm(min,avg,max));
  }
  else {
    nlog("Race %d has no ratial norm.",race);
    return(175);
  }
}

void free_obj(struct obj_data *obj)
{
        struct extra_descr_data *this, *next_one;

        if (DEBUG>1) nlog("Free Obj: %s",obj->name);
        FREE(obj->name);
        FREE(obj->description);
        FREE(obj->short_description);
        FREE(obj->action_description);

        for( this = obj->ex_description ;
                (this != 0);this = next_one )
        {
                next_one = this->next;
                FREE(this->keyword);
                FREE(this->description);
                FREE(this);
        }

        FREE(obj);
}

/* release memory allocated for a char struct */
void free_char(struct char_data *ch)
{
  struct obj_data *o;
  extern struct obj_data *object_list;

  for (;object_list;object_list=o) {
    o=object_list->next;
    free_obj(object_list);
  }
  FREE(ch->player.name);
  FREE(ch->player.title);
  FREE(ch->player.short_descr);
  FREE(ch->player.long_descr);
  FREE(ch->player.description);
  FREE(ch->player.sounds);
  FREE(ch->player.distant_snds);
  FREE(ch->player.name);
  FREE(ch->player.title);
  FREE(ch->player.short_descr);
  FREE(ch->player.long_descr);
  FREE(ch->player.description);
  FREE(ch->player.sounds);
  FREE(ch->player.distant_snds);
  FREE(ch);
}

index_mem *real_objp(int num)
{
  return(0);
}



int find_spell(struct char_data *ch,int spell)
{
  int i;

  if (!ch->spells) return(-1);
  for (i=0; i<ch->nspells; i++)
    if (ch->spells[i].spell_number==spell) return(i);
  return(-1);
}

int get_spell(struct char_data *ch,int spell)
{
  int i;
  i=find_spell(ch,spell);
  return(ch->spells[i].learned);
}

void set_spell(struct char_data *ch,int spell,int mod)
{
  int i;
  if (!ch->spells) {
        CREATE(ch->spells,struct char_spell_data,1);
        ch->nspells=1;
        ch->spells[0].spell_number=spell;
        ch->spells[0].learned=mod;
  } else {
        if ((i=find_spell(ch,spell))<0) {
          ch->spells=(struct char_spell_data *)
            REALLOC(ch->spells,(++ch->nspells)*sizeof(struct char_spell_data),
		struct char_spell_data);
          if (!ch->spells) {
                log("REALLOC: spell_set");
                exit(0);
          }
          ch->spells[ch->nspells-1].spell_number=spell;
          ch->spells[ch->nspells-1].learned=mod;
        } else {
          if (mod>=100)
            ch->spells[i].learned=mod;
          else
            ch->spells[i].learned+=mod;
        }
  }
}

int find_skill(struct char_data *ch,int skill)
{
  int i;

  if (skill>9999) skill-=10000;
  if (!ch->skills) return(-1);
  for (i=0; i<ch->nskills; i++)
    if (ch->skills[i].skill_number==skill) return(i);
  return(-1);
}

int get_skill(struct char_data *ch,int skill)
{
  int i;

  if (skill>9999) skill-=10000;
  i=find_skill(ch,skill);
  return(ch->skills[i].learned);
}

void set_skill(struct char_data *ch,int skill,int mod)
{
  int i;
  if (skill>9999) skill-=10000;
  if (!ch->skills) {
        CREATE(ch->skills,struct char_skill_data,1);
        ch->nskills=1;
        ch->skills[0].skill_number=skill;
        ch->skills[0].learned=mod;
  } else {
        if ((i=find_skill(ch,skill))<0) {
          ch->skills=
            REALLOC(ch->skills,(++ch->nskills)*sizeof(struct char_skill_data),
		struct char_skill_data);
          if (!ch->skills) {
                log("REALLOC: skill_set");
                exit(0);
          }
          ch->skills[ch->nskills-1].skill_number=skill;
          ch->skills[ch->nskills-1].learned=mod;
        } else {
          if (mod>=100)
            ch->skills[i].learned=mod;
          else
            ch->skills[i].learned+=mod;
        }
  }
}

void clear_object(struct obj_data *obj)
{
        memset(obj, '\0', sizeof(struct obj_data));

        obj->in_room      = NOWHERE;
}


void finish_obj(struct obj_data *o)
{
  extern material_t materials[];

  if(!o->material){
    switch(o->obj_flags.type_flag){
      case ITEM_LIGHT: o->material = MAT_WOOD; break;
      case ITEM_SCROLL: o->material = MAT_PAPER; break;
      case ITEM_WAND: o->material = MAT_WOOD; break;
      case ITEM_STAFF: o->material = MAT_WOOD; break;
      case ITEM_WEAPON: o->material = MAT_STEEL; break;
      case ITEM_FIREWEAPON: o->material = MAT_WOOD; break;
      case ITEM_MISSILE: o->material = MAT_WOOD; break;
      case ITEM_TREASURE: o->material = MAT_GEMSTONE; break;
      case ITEM_ARMOR: o->material = MAT_LEATHER; break;
      case ITEM_POTION: o->material = MAT_GLASS; break;
      case ITEM_WORN: o->material = MAT_CLOTH; break;
      case ITEM_CONTAINER: o->material = MAT_CLOTH; break;
      case ITEM_NOTE: o->material = MAT_PAPER; break;
      case ITEM_DRINKCON: o->material = MAT_WOOD; break;
      case ITEM_KEY: o->material = MAT_IRON; break;
      case ITEM_FOOD: o->material = MAT_ORGANIC; break;
      case ITEM_MONEY: o->material = MAT_GOLD; break;
      case ITEM_BOAT: o->material = MAT_WOOD; break;
      case ITEM_BOARD: o->material = MAT_WOOD; break;
      case ITEM_BOOK: o->material = MAT_PAPER; break;
      case ITEM_SPELL_POUCH: o->material = MAT_LEATHER; break;
    }
  }
  if(o->obj_flags.mass == -1){
    o->obj_flags.mass = 454 * o->obj_flags.weight;
  }
  if(o->obj_flags.volume == -1){
    /* fetch volume info from density and mass */
    o->obj_flags.volume = (100 * o->obj_flags.mass) / materials[o->material].density;
  }
  if(IS_WEAPON(o) && !o->obj_flags.value[0]){
    if(real_objp(o->virtual)){
      o->obj_flags.value[0] = real_objp(o->virtual)->obj->obj_flags.value[0];
    }
  }
#if 0
  if(!o->obj_flags.length && IS_WEAPON(o)){
    /* fetch the length from the default list according to type */
    o->obj_flags.length = weapons[o->obj_flags.value[0]].length;
  }
  if(IS_WEAPON(o) && !o->obj_flags.value[4]){
    o->obj_flags.value[4] = weapons[o->obj_flags.value[0]].speed;
  }
#endif
}

