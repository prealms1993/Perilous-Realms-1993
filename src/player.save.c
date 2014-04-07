/*#include <unistd.h>*/
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include <fcns.h>
#include <externs.h>
#include <utils.h>
#include <db.h>

extern int DEBUG;
int db_boot = 0;

#define S_IRGRP 0000040/* read permission, group */
#define S_IWGRP 0000020/* write permission, group */

//Brutal hack to load Big-Endian encoded player characters.  Only needed when convering from Sparc/PowerPC to Intel architecture
//2014 - Wrote this to convert the old character files.  Not needed once files are converted
#define PLAYER_BIG_ENDIAN

#define EOF_MARK 255
#define SUB_OBJECT 254
#define OBJECT 253
#define END_OF_OBJECT 252
#define END_OF_LIST 251
#define WEAR 250
#define AFFECT 249
#define AFFECT_END 248
#define ERROR 247
#define MAIL 246

#define ID_TYPE int

#define Write(tid,v)	if (v) {\
			  t=tid;\
			  write(des,&t,sizeof(ID_TYPE));\
			  write(des,&(v),sizeof(v));\
			}


#define WriteF(tid,v)    t=tid;\
			write(des,&t,sizeof(ID_TYPE));\
			write(des,&(v),sizeof(v));

#define WriteS(tid,v)	if (v) {\
			  t=tid;\
			  write(des,&t,sizeof(ID_TYPE));\
			  t=strlen(v)+1;\
			  write(des,&t,sizeof(ID_TYPE));\
			  write(des,v,t);\
			}

#define WriteA(tid,v,nel,type)\
			if (nel) {\
			  t=tid;\
			  write(des,&t,sizeof(ID_TYPE));\
			  t=nel*sizeof(type);\
			  write(des,&t,sizeof(t));\
			  write(des,v,t);\
			}

#define WriteSt(tid,v,type) t=tid;\
			  write(des,&t,sizeof(ID_TYPE));\
			  write(des,&v,sizeof(type));

#define ID(tid) t=tid; write(des,&t,sizeof(ID_TYPE))

int SendMail(char *name, char *message)
{
  int des,t;

  des=open(name_to_path(name),O_RDWR);
  if (des<0) return(0);
  if (lseek(des,-sizeof(ID_TYPE),SEEK_END)<=0) {
    vlog(LOG_URGENT,"lseek is still at beggining of file for %s",name);
    close(des);
    return(0);
  }
  ID(MAIL);
  WriteS(1,message);
  ID(EOF_MARK);
  close(des);
  return(1);
}

void WriteAffect(int des,struct affected_type *af)
{
	int t;

	ID(AFFECT);
	Write(1,af->type);
	Write(2,af->duration);
	Write(3,af->modifier);
	Write(4,af->location);
	Write(5,af->bitvector);
	ID(AFFECT_END);
}

void WriteObject(int des,struct obj_data *i)
{
  struct obj_data *bobj;
  int t;

  /* Next: 23) */
  ID(OBJECT);
  Write(19,i->virtual);
  WriteA(2,i->obj_flags.value,MAX_VALUE,int);
  Write(3,i->obj_flags.wear_flags);
  Write(4,i->obj_flags.extra_flags);
  Write(5,i->obj_flags.weight);
  Write(6,i->obj_flags.cost);
  Write(7,i->obj_flags.cost_per_day);
  Write(8,i->obj_flags.timer);
  Write(9,i->obj_flags.bitvector);
  Write(10,i->obj_flags.xtra_bits);
  WriteA(20,i->affected,MAX_OBJ_AFFECT,struct obj_affected_type);
  bobj = (real_objp(i->virtual) ? real_objp(i->virtual)->obj : NULL);
  if((bobj) ? (str_cmp(bobj->name, i->name)) : 1){
    WriteS(12, i->name);
  }
  else{
    WriteS(12, "");
  }
  if((bobj) ? (str_cmp(bobj->description, i->description)) : 1){
    WriteS(13, i->description);
  }
  else{
    WriteS(13, "");
  }
  if((bobj) ? (str_cmp(bobj->short_description, i->short_description)) : 1){
    WriteS(14, i->short_description);
  }
  else{
    WriteS(14, "");
  }
  if((bobj) ? (str_cmp(bobj->action_description, i->action_description)) : 1){
    WriteS(15, i->action_description);
  }
  else{
    WriteS(15, "");
  }
/*	WriteS(12,i->name);
	WriteS(13,i->description);
	WriteS(14,i->short_description);
	WriteS(15,i->action_description); */
  /* for now, extra descriptions not saved */
  Write(16,i->eq_pos);
  /* Write(17,i->in_room); always -1... why save */
  Write(18,i->obj_flags.type_flag);
  Write(21, i->min_level);
  Write(22, i->time_stamp);
  Write(23, i->obj_flags.mass);
  Write(24, i->obj_flags.volume);
  Write(25, i->material);
  Write(26, i->obj_flags.length);
  ID(END_OF_OBJECT);
}

void SaveObjectList(int des,struct obj_data *list,int *count)
{
  int t;
  if (!list) return;
  
  ID(SUB_OBJECT);
  while (list) {
#if 0
    (*count)++;
    if(*count == 70){
      break;
    }
#endif
    WriteObject(des,list);
    SaveObjectList(des,list->contains,count);
    list=list->next_content;
  }
  ID(END_OF_LIST);
}

void save_char(struct char_data *ch,int load_room)
{
  int i;
  int des;
  int count = 0;
  ID_TYPE t;
  struct affected_type *temp;
  mail_item *m;

  if (IS_NPC(ch)) return;

  des = creat(name_to_path(ch->player.name),S_IREAD|S_IWRITE|S_IRGRP|S_IWGRP);

/* next: 87 */
  Write(1,ch->player.sex);
  Write(2,ch->player.class);
  WriteS(3,ch->player.name);
  WriteS(4,ch->player.short_descr);
  WriteS(5,ch->player.long_descr);
  WriteS(6,ch->player.description);
  WriteS(7,ch->player.title);
  WriteS(84,ch->player.email);
  Write(8,ch->player.level);
  Write(9,ch->player.hometown);
  WriteA(10,ch->player.talks,ch->player.ntalks,bool);
/*  WriteSt(11,ch->player.time,struct time_data); obsolete - SLB */
  Write(12,ch->player.max_level);
  Write(71,ch->player.weight);
  Write(72,ch->player.height);
  Write(14,ch->abilities.str);
  Write(16,ch->abilities.intel);
  Write(17,ch->abilities.wis);
  Write(18,ch->abilities.dex);
  Write(19,ch->abilities.con);
  Write(67,ch->abilities.chr);
  Write(68,ch->abilities.lck);
  Write(20,ch->tmpabilities.str);
  Write(22,ch->tmpabilities.intel);
  Write(23,ch->tmpabilities.wis);
  Write(24,ch->tmpabilities.dex);
  Write(25,ch->tmpabilities.con);
  Write(69,ch->tmpabilities.chr);
  Write(70,ch->tmpabilities.lck);
  Write(26,ch->points.mana);
  Write(27,ch->points.max_mana);
  Write(28,ch->points.hit);
  Write(29,ch->points.max_hit);
  Write(30,ch->points.move);
  Write(31,ch->points.max_move);
  Write(33,ch->points.gold);
  Write(34,ch->points.bankgold);
  Write(35,ch->points.exp);
  Write(75,ch->points.hitroll);
  Write(76,ch->points.damroll);
  Write(38,ch->specials.immune);
  Write(39,ch->specials.M_immune);
  Write(40,ch->specials.susc);
  Write(41,ch->specials.mult_att);
  Write(42,ch->specials.affected_by);
  Write(43,ch->specials.position);
  Write(44,ch->specials.act);
  Write(45,ch->specials.spells_to_learn);
/*  Write(46,ch->specials.carry_weight);
  Write(47,ch->specials.carry_items); obsolete - SLB */
  Write(48,ch->specials.timer);
/*  WriteSt(77,ch->specials.last_logon,time_t);
  WriteSt(78,ch->specials.last_taxed,time_t); obsolete - SLB */
  Write(49,load_room);
  WriteA(50,ch->specials.apply_saving_throw,5,sh_int);
  WriteA(51,ch->specials.conditions,3,sbyte);
  Write(52,ch->specials.invis_level);
  Write(55,ch->player.race);
  WriteS(56,ch->player.pw);
  Write(57,ch->specials.true_align);
  Write(58,ch->specials.whimpy_level);
  Write(60,ch->specials.alignment);
  Write(61,ch->build_lo);
  Write(62,ch->build_hi);
  WriteS(63,ch->prompt);
  WriteA(64,ch->points.armor,6,sh_int);
  WriteA(65,ch->points.stopping,6,sh_int);
  if (ch->kills)
    WriteA(74,ch->kills,MAX_KILL_KEEP,kill_info);
  Write(66,ch->player.guild);
  Write(81,ch->build.offset);
  WriteA(82,ch->skills,ch->nskills,struct char_skill_data);
  WriteA(83,ch->spells,ch->nspells,struct char_spell_data);
  Write(85,ch->specials.log_bits);
  if (ch->warehouse) {
    ID(86);
    SaveObjectList(des,ch->warehouse,&count);
  }
  Write(87, ch->specials.carry_mass);
  Write(88, ch->specials.carry_volume);
  Write(89, ch->points.power);
  Write(90, ch->points.max_power);
  WriteSt(91, ch->player.time.birth, time_t);
  WriteSt(92, ch->player.time.logon, time_t);
  WriteSt(93, ch->player.time.motd, time_t);
  WriteSt(94, ch->player.time.password, time_t);
  Write(95, ch->player.time.played);
  WriteS(96, ch->player.full_name);
  WriteS(97, ch->player.plan);
  WriteS(98, ch->player.last_site);

  SaveObjectList(des,ch->carrying,&count);
  for (i=0; i<MAX_WEAR; i++)
    if (ch->equipment[i]) {
      ID(WEAR);
      ID(i);
      SaveObjectList(des,ch->equipment[i],&count);
    }
  for (temp=ch->affected; temp ; temp=temp->next)
	  if (temp) WriteAffect(des,temp);
  for (m=ch->mail_list; m; m=m->next)
    if (m) {
      ID(MAIL);
      WriteS(1,m->mail);
    }
  ID(EOF_MARK);
  close(des);
}


static char *bp;

#define Skip(n)  bp += n;\
	break

#define _Read(v) memcpy((void *)&v,(void *)bp,sizeof(v));\
	bp += sizeof(v)

#define _ReadN(v,n) memcpy((void *)v,(void *)bp,n);\
	bp += n

#define Read(v) _Read(v);\
	break

#define ReadSt(v,type)  _ReadN(&v,sizeof(type));\
	break

#define _ReadSt(v, type)  _ReadN(&v, sizeof(type))

#define ReadSS(v)	_Read(temp);\
	_ReadN(v, temp); \
	break

#define ReadSD(v) 	_Read(temp);\
	v = MALLOC(temp, char); \
	_ReadN(v, temp); \
	break

#define _ReadSD(v)      _Read(temp);\
	v = MALLOC(temp, char); \
	_ReadN(v, temp);

#define ReadAD(v,n,type) _Read(temp);\
	n = temp / sizeof(type); \
	v = MALLOC(temp, type); \
	_ReadN(v, temp); \
	break

#define ReadAS(v,n,type) _Read(temp);\
	n = temp / sizeof(type); \
	_ReadN(v, temp); \
	break

#ifdef PLAYER_BIG_ENDIAN
//Version to read big-endian files
//Careful - routines with _ in front do not include a break command
#define _ReadBE(v)  { \
if (DEBUG == 1235) nlog("_ReadBE: %d %s", sizeof(v), #v); \
if (sizeof(v) == 1) { v = *((unsigned char*)bp); bp += sizeof(v); }\
else if (sizeof(v) == 2) { _ReadBE16(v); } \
else if (sizeof(v) == 4) { _ReadBE32(v); } \
else { nlog("_ReadBE: WARNING AND ERROR %d %d", sizeof(v), __LINE__); exit(1); } \
}

#define _ReadBE16(v) { \
if (DEBUG == 1235) nlog("_ReadBE16: %d %s", sizeof(v), #v); \
	assert(sizeof(v) == 2); \
	v = __builtin_bswap16(*((unsigned short*)bp)); \
	bp += sizeof(v); }

//read list of u16's
#define _ReadBE16Num(v, n) { \
if (DEBUG == 1235) 	nlog("_ReadBE16Num: %s %d", #v, n); \
	int __i; \
for (__i = 0; __i < n; __i++) {\
	v[__i] = __builtin_bswap16(*((unsigned short*)bp)); \
	bp += sizeof(unsigned short); \
} }

//read array of u16's defined by size of array
#define _ReadBE16Array(v) { \
if (DEBUG == 1235) 	nlog("_ReadBE16Array: %d %d %s", sizeof(v), sizeof(v[0]), #v); \
	assert(sizeof(v[0]) == 2); \
	_ReadBE16Num(v, (sizeof(v) / sizeof(v[0]))); \
} 

//read 32bit 
#define _ReadBE32(v)  {\
if (DEBUG == 1235) 	nlog("_ReadBE32: %d %s", sizeof(v), #v); \
	assert(sizeof(v) == 4); \
	v = __builtin_bswap32(*((unsigned int*)bp)); \
	bp += sizeof(v); }

//read list of u32's
#define _ReadBE32Num(v, n) { \
if (DEBUG == 1235) nlog("_ReadBE32Num: %s %d", #v, n); \
int __i; \
for (__i = 0; __i < n; __i++) { \
v[__i] = __builtin_bswap32(*((unsigned int*)bp)); \
bp += sizeof(unsigned int); \
} }

	//read array of u16's defined by size of array
#define _ReadBE32Array(v) { \
if (DEBUG == 1235) 	nlog("_ReadBE32Array: %d %d %s", sizeof(v), sizeof(v[0]), #v); \
	assert(sizeof(v[0]) == 4);\
	_ReadBE32Num(v, (sizeof(v) / sizeof(v[0]))); \
	} 


#define _ReadNBE(v,n) \
if (DEBUG == 1235) 	nlog("_ReadNBE: %d %s", n, #v); \
	memcpy((void *)v,(void *)bp,n);\
	bp += n

#define ReadBE(v) \
	_ReadBE(v);\
	break

#define ReadStBE(v,type) \
if (DEBUG == 1235) 	nlog("ReadStBE: %d %s %s", sizeof(type), #type, #v); \
	_ReadNBE(&v, sizeof(type)); \
	break

#define _ReadStBE(v, type)  \
if (DEBUG == 1235) 	nlog("_ReadStBE: %d %s %s", sizeof(type), #type, #v); \
	_ReadNBE(&v, sizeof(type))

#define ReadSSBE(v)	\
if (DEBUG == 1235) 	nlog("ReadSSBE: %s", #v); \
	_ReadBE(temp); \
	_ReadNBE(v, temp); \
	break

#define ReadSDBE(v) 	\
if (DEBUG == 1235) nlog("ReadSDBE: %s", #v); \
	_ReadBE(temp); \
	v = MALLOC(temp, char); \
	_ReadNBE(v, temp); \
	break

#define _ReadSDBE(v)      \
if (DEBUG == 1235) nlog("_ReadSDBE: %s", #v); \
	_ReadBE(temp); \
	v = MALLOC(temp, char); \
	_ReadNBE(v, temp);

#define ReadADBE(v,n,type) \
if (DEBUG == 1235) 	nlog("ReadADBE: %d %s %s", n, #v, #type); \
	_ReadBE(temp); \
	n = temp / sizeof(type); \
	v = MALLOC(temp, type); \
	_ReadNBE(v, temp); \
	break

#define ReadASBE(v,n,type) { \
if (DEBUG == 1235) 	nlog("ReadASBE: %d %s %s", n, #v, #type); \
	_ReadBE(temp); \
	n = temp / sizeof(type); \
	if (sizeof(type) == 1) { _ReadNBE(v, temp); }\
	else if (sizeof(type) == 2) { _ReadBE16Num(v, n); } \
	else if (sizeof(type) == 4) { _ReadBE32Num(v, n); } \
	else { nlog("ReadASBE: WARNING AND ERROR %d %d", sizeof(type), __LINE__); exit(1); } \
}\
break
//use this to do a manual conversion in the load routine
#define ReadASBE_Raw(v,n,type) \
if (DEBUG == 1235) 	nlog("ReadASBE_Raw: %d %s %s", n, #v, #type); \
	_ReadBE(temp); \
	n = temp / sizeof(type); \
	_ReadN(v, temp); \
	break
#endif 

static char player_name[100];

void ReadAffect(struct affected_type *af)
{
  int id;

  do {
    _Read(id);
    if (DEBUG==1234) nlog("A:%d",id);
    switch(id) {
	case 1:	Read(af->type);
	case 2: Read(af->duration);
	case 3: Read(af->modifier);
	case 4: Read(af->location);
	case 5: Read(af->bitvector);
	case AFFECT_END: break;
	default: {
			char s[100];
			sprintf(s,"Unknown type %d in ReadAffect",id);
			log(s);
			exit(1);
		}
    }
  } while (id!=AFFECT_END);
}

void ReadObject(struct obj_data *i)
{
  int temp,id,t;
  struct obj_data *bobj = NULL;

  i->next = object_list;
  object_list = i;

  /* initialize mass and volume in case the fields aren't present */
  i->obj_flags.mass = i->obj_flags.volume = -1;

  do {
    _Read(id);
    if (DEBUG==1234) nlog("O:%d",id);
    switch(id) {
	case 1:	abort(); /* no longer used */
	case 2: bzero(i->obj_flags.value,sizeof(i->obj_flags.value));
		ReadAS(i->obj_flags.value,t,int);
	case 3: Read(i->obj_flags.wear_flags);
	case 4: Read(i->obj_flags.extra_flags);
	case 5: Read(i->obj_flags.weight);
	case 6: Read(i->obj_flags.cost);
	case 7: Read(i->obj_flags.cost_per_day);
	case 8: Read(i->obj_flags.timer);
	case 9: Read(i->obj_flags.bitvector);
	case 10: Read(i->obj_flags.xtra_bits);
        case 12: _ReadSD(i->name);
                 if(bobj && !*(i->name)){
                   FREE(i->name);
                   i->name = mystrdup(bobj->name);
                 }
                 break;
        case 13: _ReadSD(i->description);
                 if(bobj && !*(i->description)){
                   FREE(i->description);
                   i->description = mystrdup(bobj->description);
                 }
                 break;
        case 14: _ReadSD(i->short_description);
                 if(bobj && !*(i->short_description)){
                   FREE(i->short_description);
                   i->short_description = mystrdup(bobj->short_description);
                 }
                 break;
        case 15: _ReadSD(i->action_description);
                 if(bobj && !*(i->action_description)){
                   FREE(i->action_description);
                   i->action_description = mystrdup(bobj->action_description);
                 }
                 break;
/*	case 13: ReadSD(i->description);
	case 14: ReadSD(i->short_description);
	case 15: ReadSD(i->action_description); */
	case 16: Read(i->eq_pos);
	case 17: Read(i->in_room);
	case 18: Read(i->obj_flags.type_flag);
	case 19: _Read(i->virtual);
  bobj = (real_objp(i->virtual) ? real_objp(i->virtual)->obj : NULL);
                 break;
#if 0
		 if (DEBUG==1234) nlog("VNUM:%d",i->virtual);
		 i->item_number=real_object(i->virtual);
		 break;
#endif
	case 20: ReadAS(i->affected,t, struct obj_affected_type);
	case 21: Read(i->min_level);
	case 22: Read(i->time_stamp);
        case 23: Read(i->obj_flags.mass);
        case 24: Read(i->obj_flags.volume);
        case 25: Read(i->material);
	case 26: Read(i->obj_flags.length);
	case END_OF_OBJECT: finish_obj(i); break;
	default: {
			char s[100];
			sprintf(s,"Unknown type %d in ReadObj",id);
			log(s);
			exit(1);
		}
    }
  } while (id!=END_OF_OBJECT);
  i->in_room = -1;
  if (real_objp(i->virtual)) {
    if (db_boot) {
	if (real_objp(i->virtual)->max_at_boot<32000)
          printf("%7d %s\n",i->virtual,player_name);
	real_objp(i->virtual)->boot_count++;
    } else
	real_objp(i->virtual)->count++;
  }
}

void ReadObjList(struct obj_data *list, struct obj_data *parent,struct char_data *ch)
{
  struct obj_data *p, *last;
  int id;

  bzero(list,sizeof(struct obj_data));
  clear_object(list);

  p=list;
  last=NULL;
  do {
    _Read(id);
    if (DEBUG==1234) nlog("L:%d",id);
    switch(id) {
	case SUB_OBJECT: 
			CREATE(last->contains,struct obj_data,1);
			ReadObjList(last->contains,last,NULL);
			break;
	case END_OF_LIST:
			FREE(p);
			if (last)
			  last->next_content=NULL;
			break;
	case OBJECT:	ReadObject(p);
			p->in_obj=parent;
			CREATE(p->next_content,struct obj_data,1);
			bzero(p->next_content,sizeof(struct obj_data));
			clear_object(p->next_content);
			last = p;
			p= p->next_content;
			break;
	default: {
			char s[100];
			sprintf(s,"Unknown type %d in ReadObjList",id);
			log(s);
			exit(1);
		}
    }
  } while (id!=END_OF_LIST);
}

#ifdef PLAYER_BIG_ENDIAN
void ReadAffectBE(struct affected_type *af)
{
	int id;

	do {
		_ReadBE(id);
		if (DEBUG == 1234) nlog("A:%d", id);
		switch (id) {
		case 1:	ReadBE(af->type);
		case 2: ReadBE(af->duration);
		case 3: ReadBE(af->modifier);
		case 4: ReadBE(af->location);
		case 5: _ReadBE32Array(af->bitvector); break;
		case AFFECT_END: break;
		default: {
					 char s[100];
					 sprintf(s, "Unknown type %d in ReadAffect", id);
					 log(s);
					 exit(1);
		}
		}
	} while (id != AFFECT_END);
}
void ReadObjectBE(struct obj_data *i)
{
	int temp, id, t;
	struct obj_data *bobj = NULL;

	i->next = object_list;
	object_list = i;

	/* initialize mass and volume in case the fields aren't present */
	i->obj_flags.mass = i->obj_flags.volume = -1;

	do {
		_ReadBE(id);
		if (DEBUG == 1234) nlog("O:%d", id);
		switch (id) {
		case 1:	abort(); /* no longer used */
		case 2: bzero(i->obj_flags.value, sizeof(i->obj_flags.value));
				ReadASBE(i->obj_flags.value, t, int);
		case 3: ReadBE(i->obj_flags.wear_flags);
		case 4: ReadBE(i->obj_flags.extra_flags);
		case 5: ReadBE(i->obj_flags.weight);
		case 6: ReadBE(i->obj_flags.cost);
		case 7: ReadBE(i->obj_flags.cost_per_day);
		case 8: ReadBE(i->obj_flags.timer);
		case 9: _ReadBE32Array(i->obj_flags.bitvector); break;
		case 10: ReadBE(i->obj_flags.xtra_bits);
		case 12: _ReadSDBE(i->name);
			if (bobj && !*(i->name)){
				FREE(i->name);
				i->name = mystrdup(bobj->name);
			}
			break;
		case 13: _ReadSDBE(i->description);
			if (bobj && !*(i->description)){
				FREE(i->description);
				i->description = mystrdup(bobj->description);
			}
			break;
		case 14: _ReadSDBE(i->short_description);
			if (bobj && !*(i->short_description)){
				FREE(i->short_description);
				i->short_description = mystrdup(bobj->short_description);
			}
			break;
		case 15: _ReadSDBE(i->action_description);
			if (bobj && !*(i->action_description)){
				FREE(i->action_description);
				i->action_description = mystrdup(bobj->action_description);
			}
			break;
			/*	case 13: ReadSD(i->description);
			case 14: ReadSD(i->short_description);
			case 15: ReadSD(i->action_description); */
		case 16: ReadBE(i->eq_pos);
		case 17: ReadBE(i->in_room);
		case 18: ReadBE(i->obj_flags.type_flag);
		case 19: _ReadBE(i->virtual);
			bobj = (real_objp(i->virtual) ? real_objp(i->virtual)->obj : NULL);
			break;
#if 0
			if (DEBUG == 1234) nlog("VNUM:%d", i->virtual);
			i->item_number = real_object(i->virtual);
			break;
#endif
		case 20: 
			//ReadAS(i->affected, t, struct obj_affected_type);
			ReadASBE_Raw(i->affected, t, struct obj_affected_type);
			//byteswap post-load
			{
				int j;
				for (j = 0; j < t; j++) {
					i->affected[j].modifier = __builtin_bswap32(i->affected[j].modifier);
				}
			}

		case 21: ReadBE(i->min_level);
		case 22: ReadBE(i->time_stamp);
		case 23: ReadBE(i->obj_flags.mass);
		case 24: ReadBE(i->obj_flags.volume);
		case 25: ReadBE(i->material);
		case 26: ReadBE(i->obj_flags.length);
		case END_OF_OBJECT: finish_obj(i); break;
		default: {
					 char s[100];
					 sprintf(s, "Unknown type %d in ReadObj", id);
					 log(s);
					 exit(1);
		}
		}
	} while (id != END_OF_OBJECT);
	i->in_room = -1;
	if (real_objp(i->virtual)) {
		if (db_boot) {
			if (real_objp(i->virtual)->max_at_boot<32000)
				printf("%7d %s\n", i->virtual, player_name);
			real_objp(i->virtual)->boot_count++;
		}
		else
			real_objp(i->virtual)->count++;
	}
}
void ReadObjListBE(struct obj_data *list, struct obj_data *parent, struct char_data *ch)
{
	struct obj_data *p, *last;
	int id;

	bzero(list, sizeof(struct obj_data));
	clear_object(list);

	p = list;
	last = NULL;
	do {
		_ReadBE(id);
		if (DEBUG == 1234) nlog("L:%d", id);
		switch (id) {
		case SUB_OBJECT:
			CREATE(last->contains, struct obj_data, 1);
			ReadObjListBE(last->contains, last, NULL);
			break;
		case END_OF_LIST:
			FREE(p);
			if (last)
				last->next_content = NULL;
			break;
		case OBJECT:	
			ReadObjectBE(p);
			p->in_obj = parent;
			CREATE(p->next_content, struct obj_data, 1);
			bzero(p->next_content, sizeof(struct obj_data));
			clear_object(p->next_content);
			last = p;
			p = p->next_content;
			break;
		default: {
					 char s[100];
					 sprintf(s, "Unknown type %d in ReadObjList", id);
					 log(s);
					 exit(1);
		}
		}
	} while (id != END_OF_LIST);
}



int new_load_char_big_endian(struct char_data *ch, char *name, unsigned char * buf)
{
	struct obj_data *obj;
	int des;
	ID_TYPE id;
	int i, temp, t;
	int pos;
	int br;
	struct char_skill_data skill;
	struct char_spell_data spell;

	bp = buf;
	do {
		_ReadBE(id);
		if (DEBUG == 1234) nlog("(be)M:%d", id);
		switch (id) {
		case 1: ReadBE(ch->player.sex);
		case 2: ReadBE(ch->player.class);
		case 3: ReadSDBE(ch->player.name);
		case 4: ReadSDBE(ch->player.short_descr);
		case 5: ReadSDBE(ch->player.long_descr);
		case 6: ReadSDBE(ch->player.description);
		case 7: ReadSDBE(ch->player.title);
		case 8: ReadBE(ch->player.level);
		case 9: ReadBE(ch->player.hometown);
		case 10: ReadADBE(ch->player.talks, ch->player.ntalks, bool);
		case 11: /* obsolete */
			_ReadStBE(ch->player.time.birth, time_t);
			_ReadStBE(ch->player.time.logon, time_t);
			ReadBE(ch->player.time.played);
		case 12: ReadBE(ch->player.max_level);
		case 14: ReadBE(ch->abilities.str);
		case 16: ReadBE(ch->abilities.intel);
		case 17: ReadBE(ch->abilities.wis);
		case 18: ReadBE(ch->abilities.dex);
		case 19: ReadBE(ch->abilities.con);
		case 20: ReadBE(ch->tmpabilities.str);
		case 22: ReadBE(ch->tmpabilities.intel);
		case 23: ReadBE(ch->tmpabilities.wis);
		case 24: ReadBE(ch->tmpabilities.dex);
		case 25: ReadBE(ch->tmpabilities.con);
		case 26: ReadBE(ch->points.mana);
		case 27: ReadBE(ch->points.max_mana);
		case 28: ReadBE(ch->points.hit);
		case 29: ReadBE(ch->points.max_hit);
		case 30: ReadBE(ch->points.move);
		case 31: ReadBE(ch->points.max_move);
		case 32: ReadBE(ch->points.armor[4]);
		case 33: ReadBE(ch->points.gold);
		case 34: ReadBE(ch->points.bankgold);
		case 35: ReadBE(ch->points.exp);
		case 38: ReadBE(ch->specials.immune);
		case 39: ReadBE(ch->specials.M_immune);
		case 40: ReadBE(ch->specials.susc);
		case 41: ReadBE(ch->specials.mult_att);
		case 42: _ReadBE32Array(ch->specials.affected_by); break;
		case 43: ReadBE(ch->specials.position);
		case 44: ReadBE(ch->specials.act);
		case 45: ReadBE(ch->specials.spells_to_learn);
		case 46: /* obsolete */ ReadBE(ch->specials.carry_weight);
		case 47: /* obsolete */ ReadBE(ch->specials.carry_items);
		case 48: ReadBE(ch->specials.timer);
		case 49: ReadBE(ch->specials.was_in_room);
		case 50: ReadASBE(ch->specials.apply_saving_throw, t, sh_int);
		case 51: ReadASBE(ch->specials.conditions, t, sbyte);
		case 52: ReadBE(ch->specials.invis_level);
		case 54: ReadADBE(ch->affected, ch->naffected, struct affected_type);
		case 55: ReadBE(ch->player.race);
		case 56: ReadSDBE(ch->player.pw);
		case 57: ReadBE(ch->specials.true_align);
		case 58: ReadBE(ch->specials.whimpy_level);
		case 60: ReadBE(ch->specials.alignment);
		case 61: ReadBE(ch->build_lo);
		case 62: ReadBE(ch->build_hi);
		case 63: ReadSDBE(ch->prompt);
		case 64: ReadASBE(ch->points.armor, t, sh_int);
		case 65: ReadASBE(ch->points.stopping, t, sh_int);
		case 66: ReadBE(ch->player.guild);
		case 67: ReadBE(ch->abilities.chr);
		case 68: ReadBE(ch->abilities.lck);
		case 69: ReadBE(ch->tmpabilities.chr);
		case 70: ReadBE(ch->tmpabilities.lck);
		case 71: ReadBE(ch->player.weight);
		case 72: ReadBE(ch->player.height);
		case 73: /* obsolete */ nlog("Error 73"); break;
		case 74: CREATE(ch->kills, kill_info, MAX_KILL_KEEP);
			bzero(ch->kills, sizeof(kill_info)* MAX_KILL_KEEP);
			ReadASBE_Raw(ch->kills, t, kill_info);
			//byteswap post-load
			{
				for (i = 0; i < t; i++) {
					ch->kills[i].nkills = __builtin_bswap16(ch->kills[i].nkills);
					ch->kills[i].vnum = __builtin_bswap32(ch->kills[i].vnum);
				}
			}
			break;
		case 75: ReadBE(ch->points.hitroll);
		case 76: ReadBE(ch->points.damroll);
		case 77: /* obsolete */
			Skip(sizeof(time_t));
		case 78: /* obsolete */
			Skip(sizeof(time_t));
		case 79:
		case 80: break;
		case 81: ReadBE(ch->build.offset);
		case 82: ch->nskills = 0;
			_ReadBE(temp);
			temp /= sizeof(struct char_skill_data);
			for (i = 0; i < temp; i++){
				_ReadSt(skill, struct char_skill_data);
				if (find_skill(ch, skill.skill_number) < 0)
					set_skill(ch, skill.skill_number, skill.learned);
			}
			break;
			/*                ReadAD(ch->skills,ch->nskills,struct char_skill_data); */
		case 83: ch->nspells = 0;
			_ReadBE(temp);
			temp /= sizeof(struct char_spell_data);
			for (i = 0; i < temp; i++){
				_ReadSt(spell, struct char_spell_data);
				if (find_spell(ch, spell.spell_number) < 0)
					set_spell(ch, spell.spell_number, spell.learned);
			}
			break;
			/*                ReadAD(ch->spells,ch->nspells,struct char_spell_data); */
		case 84: ReadSDBE(ch->player.email);
		case 85: ReadBE(ch->specials.log_bits);
		case 86: _ReadBE(id); /* get rid of sub_object */
			CREATE(ch->warehouse, struct obj_data, 1);
			ReadObjListBE(ch->warehouse, NULL, ch);
			for (obj = ch->warehouse; obj; obj = obj->next_content)
				obj->in_warehouse = ch;
			break;
		case 87: ReadBE(ch->specials.carry_mass);
		case 88: ReadBE(ch->specials.carry_volume);
		case 89: ReadBE(ch->points.power);
		case 90: ReadBE(ch->points.max_power);
		case 91: ReadBE(ch->player.time.birth);
		case 92: ReadBE(ch->player.time.logon);
		case 93: ReadBE(ch->player.time.motd);
		case 94: ReadBE(ch->player.time.password);
		case 95: ReadBE(ch->player.time.played);
		case 96: ReadSDBE(ch->player.full_name);
		case 97: ReadSDBE(ch->player.plan);
		case 98: ReadSDBE(ch->player.last_site);
		case MAIL: {
					   mail_item *m;
					   _ReadBE(t);
					   CREATE(m, mail_item, 1);
					   m->next = ch->mail_list;
					   ch->mail_list = m;
					   ReadSDBE(m->mail);
		}
		case AFFECT: {
						 struct affected_type *tmp;

						 CREATE(tmp, struct affected_type, 1);
						 bzero(tmp, sizeof(struct affected_type));
						 tmp->next = ch->affected;
						 ch->affected = tmp;
						 ReadAffectBE(ch->affected);
						 break;
		}
		case WEAR:
			_ReadBE(pos);
			_ReadBE(id); /* Get rid of SUB_OBJECT */
			CREATE(ch->equipment[pos], struct obj_data, 1);
			ReadObjListBE(ch->equipment[pos], NULL, ch);
			ch->equipment[pos]->equipped_by = ch;
			break;
		case SUB_OBJECT: 
			CREATE(ch->carrying, struct obj_data, 1);
			ReadObjListBE(ch->carrying, NULL, ch);
			IS_CARRYING_M(ch) = 0;
			IS_CARRYING_V(ch) = 0;
			for (obj = ch->carrying; obj; obj = obj->next_content){
				IS_CARRYING_M(ch) += GET_OBJ_MASS(obj);
				IS_CARRYING_V(ch) += GET_OBJ_VOLUME(obj);
				obj->carried_by = ch;
			}
			break;
		case EOF_MARK: break;
		case ERROR:
		default: {
					 nlog("%s: unknown load type %d", name, id);
					 exit(0);
					 return(0);
		}
		}
	} while (id != EOF_MARK);

#if 0
	if (GET_CLASS(ch) >= MAX_DEF_CLASS) GET_CLASS(ch) = 0;
#endif
	return(1);
}
#endif


#define MAX_PLAYER_FILE_SIZE (65536*2)

int new_load_char(struct char_data *ch, char *name)
{
	struct obj_data *obj;
	int des;
	ID_TYPE id;
	int i, temp, t;
	int pos;
	int br;
	u32 bytes_read;
	unsigned char buf[MAX_PLAYER_FILE_SIZE];
	struct char_skill_data skill;
	struct char_spell_data spell;

	if (DEBUG == 2) nlog("name: %s", name);
	ch->affected = NULL;
	strcpy(player_name, name);

	des = open(name_to_path(name), O_RDONLY);
	if (des <= 0) return(-1);
	bytes_read = 0;
	do {
		br = read(des, buf + bytes_read, sizeof(buf)-bytes_read);
		bytes_read += br;
	} while (br != 0 && bytes_read < sizeof(buf));
	close(des);
	if (bytes_read <= sizeof(id)) {
		vlog(LOG_URGENT, "Couldn't read from %s", name);
		return(-1);
	}
	if (br == 0 && bytes_read >= sizeof(buf)) {
		vlog(LOG_URGENT, "Player file exceeds max size:%s",
			name);
		return (-1);
	}
	bp = buf + bytes_read - sizeof(id);
	_Read(id);
#ifdef PLAYER_BIG_ENDIAN
	//2014 - Support for loading 1993 Big-Endian player characters
	if (__builtin_bswap32(id) == EOF_MARK) {
		vlog(LOG_URGENT, "Converting Big-endian player file %s.  ", name);
		return(new_load_char_big_endian(ch, name, buf));
	}
	else
#endif
	if (id != EOF_MARK) {
		vlog(LOG_URGENT, "Corrupted player file %s", name);
		return(-1);
	}
	bp = buf;
	do {
		_Read(id);
		if (DEBUG == 1234) nlog("M:%d", id);
		switch (id) {
		case 1: Read(ch->player.sex);
		case 2: Read(ch->player.class);
		case 3: ReadSD(ch->player.name);
		case 4: ReadSD(ch->player.short_descr);
		case 5: ReadSD(ch->player.long_descr);
		case 6: ReadSD(ch->player.description);
		case 7: ReadSD(ch->player.title);
		case 8: Read(ch->player.level);
		case 9: Read(ch->player.hometown);
		case 10: ReadAD(ch->player.talks, ch->player.ntalks, bool);
		case 11: /* obsolete */
			_ReadSt(ch->player.time.birth, time_t);
			_ReadSt(ch->player.time.logon, time_t);
			Read(ch->player.time.played);
		case 12: Read(ch->player.max_level);
		case 14: Read(ch->abilities.str);
		case 16: Read(ch->abilities.intel);
		case 17: Read(ch->abilities.wis);
		case 18: Read(ch->abilities.dex);
		case 19: Read(ch->abilities.con);
		case 20: Read(ch->tmpabilities.str);
		case 22: Read(ch->tmpabilities.intel);
		case 23: Read(ch->tmpabilities.wis);
		case 24: Read(ch->tmpabilities.dex);
		case 25: Read(ch->tmpabilities.con);
		case 26: Read(ch->points.mana);
		case 27: Read(ch->points.max_mana);
		case 28: Read(ch->points.hit);
		case 29: Read(ch->points.max_hit);
		case 30: Read(ch->points.move);
		case 31: Read(ch->points.max_move);
		case 32: Read(ch->points.armor[4]);
		case 33: Read(ch->points.gold);
		case 34: Read(ch->points.bankgold);
		case 35: Read(ch->points.exp);
		case 38: Read(ch->specials.immune);
		case 39: Read(ch->specials.M_immune);
		case 40: Read(ch->specials.susc);
		case 41: Read(ch->specials.mult_att);
		case 42: Read(ch->specials.affected_by);
		case 43: Read(ch->specials.position);
		case 44: Read(ch->specials.act);
		case 45: Read(ch->specials.spells_to_learn);
		case 46: /* obsolete */ Read(ch->specials.carry_weight);
		case 47: /* obsolete */ Read(ch->specials.carry_items);
		case 48: Read(ch->specials.timer);
		case 49: Read(ch->specials.was_in_room);
		case 50: ReadAS(ch->specials.apply_saving_throw, t, sh_int);
		case 51: ReadAS(ch->specials.conditions, t, sbyte);
		case 52: Read(ch->specials.invis_level);
		case 54: ReadAD(ch->affected, ch->naffected, struct affected_type);
		case 55: Read(ch->player.race);
		case 56: ReadSD(ch->player.pw);
		case 57: Read(ch->specials.true_align);
		case 58: Read(ch->specials.whimpy_level);
		case 60: Read(ch->specials.alignment);
		case 61: Read(ch->build_lo);
		case 62: Read(ch->build_hi);
		case 63: ReadSD(ch->prompt);
		case 64: ReadAS(ch->points.armor, t, sh_int);
		case 65: ReadAS(ch->points.stopping, t, sh_int);
		case 66: Read(ch->player.guild);
		case 67: Read(ch->abilities.chr);
		case 68: Read(ch->abilities.lck);
		case 69: Read(ch->tmpabilities.chr);
		case 70: Read(ch->tmpabilities.lck);
		case 71: Read(ch->player.weight);
		case 72: Read(ch->player.height);
		case 73: /* obsolete */ nlog("Error 73"); break;
		case 74: CREATE(ch->kills, kill_info, MAX_KILL_KEEP);
			bzero(ch->kills, sizeof(kill_info)* MAX_KILL_KEEP);
			ReadAS(ch->kills, t, kill_info);
		case 75: Read(ch->points.hitroll);
		case 76: Read(ch->points.damroll);
		case 77: /* obsolete */
			Skip(sizeof(time_t));
		case 78: /* obsolete */
			Skip(sizeof(time_t));
		case 79:
		case 80: break;
		case 81: Read(ch->build.offset);
		case 82: ch->nskills = 0;
			_Read(temp);
			temp /= sizeof(struct char_skill_data);
			for (i = 0; i < temp; i++){
				_ReadSt(skill, struct char_skill_data);
				if (find_skill(ch, skill.skill_number) < 0)
					set_skill(ch, skill.skill_number, skill.learned);
			}
			break;
			/*                ReadAD(ch->skills,ch->nskills,struct char_skill_data); */
		case 83: ch->nspells = 0;
			_Read(temp);
			temp /= sizeof(struct char_spell_data);
			for (i = 0; i < temp; i++){
				_ReadSt(spell, struct char_spell_data);
				if (find_spell(ch, spell.spell_number) < 0)
					set_spell(ch, spell.spell_number, spell.learned);
			}
			break;
			/*                ReadAD(ch->spells,ch->nspells,struct char_spell_data); */
		case 84: ReadSD(ch->player.email);
		case 85: Read(ch->specials.log_bits);
		case 86: _Read(id); /* get rid of sub_object */
			CREATE(ch->warehouse, struct obj_data, 1);
			ReadObjList(ch->warehouse, NULL, ch);
			for (obj = ch->warehouse; obj; obj = obj->next_content)
				obj->in_warehouse = ch;
			break;
		case 87: Read(ch->specials.carry_mass);
		case 88: Read(ch->specials.carry_volume);
		case 89: Read(ch->points.power);
		case 90: Read(ch->points.max_power);
		case 91: ReadSt(ch->player.time.birth, time_t);
		case 92: ReadSt(ch->player.time.logon, time_t);
		case 93: ReadSt(ch->player.time.motd, time_t);
		case 94: ReadSt(ch->player.time.password, time_t);
		case 95: Read(ch->player.time.played);
		case 96: ReadSD(ch->player.full_name);
		case 97: ReadSD(ch->player.plan);
		case 98: ReadSD(ch->player.last_site);
		case MAIL: {
					   mail_item *m;
					   _Read(t);
					   CREATE(m, mail_item, 1);
					   m->next = ch->mail_list;
					   ch->mail_list = m;
					   ReadSD(m->mail);
		}
		case AFFECT: {
						 struct affected_type *tmp;

						 CREATE(tmp, struct affected_type, 1);
						 bzero(tmp, sizeof(struct affected_type));
						 tmp->next = ch->affected;
						 ch->affected = tmp;
						 ReadAffect(ch->affected);
						 break;
		}
		case WEAR:
			_Read(pos);
			_Read(id); /* Get rid of SUB_OBJECT */
			CREATE(ch->equipment[pos], struct obj_data, 1);
			ReadObjList(ch->equipment[pos], NULL, ch);
			ch->equipment[pos]->equipped_by = ch;
			break;
		case SUB_OBJECT: CREATE(ch->carrying, struct obj_data, 1);
			ReadObjList(ch->carrying, NULL, ch);
			IS_CARRYING_M(ch) = 0;
			IS_CARRYING_V(ch) = 0;
			for (obj = ch->carrying; obj; obj = obj->next_content){
				IS_CARRYING_M(ch) += GET_OBJ_MASS(obj);
				IS_CARRYING_V(ch) += GET_OBJ_VOLUME(obj);
				obj->carried_by = ch;
			}
			break;
		case EOF_MARK: break;
		case ERROR:
		default: {
					 nlog("%s: unknown load type %d", name, id);
					 return(0);
		}
		}
	} while (id != EOF_MARK);

#if 0
	if (GET_CLASS(ch)>=MAX_DEF_CLASS) GET_CLASS(ch)=0;
#endif

	return(1);
}

