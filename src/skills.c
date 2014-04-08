#include <ctype.h>

#include <fcns.h>
//#include <spells.h>
#include <utils.h>
#include <externs.h>

class_entry *classes=NULL;
form_t *forms = NULL;
int MAX_DEF_CLASS=0;
int MAX_DEF_FORM=0;

int iron_mass(struct obj_data *obj)
{
  int mass = 0;
  struct obj_data *subobj;

  for(subobj = obj->contains; subobj; subobj = subobj->next_content){
    mass += iron_mass(subobj);
  }

  if(materials[obj->material].flags & MATF_METAL){
    mass += (GET_OBJ_MASS(obj) * materials[obj->material].iron) / 100;
  }

  return(mass);
}

int metal_mass(struct obj_data *obj)
{
  int mass = 0;
  struct obj_data *subobj;

  for(subobj = obj->contains; subobj; subobj = subobj->next_content){
    mass += metal_mass(subobj);
  }

  if(materials[obj->material].flags & MATF_METAL){
    mass += GET_OBJ_MASS(obj);
  }

  return(mass);
}

int stealth_penalty(struct char_data *ch)
{
  int i, eq_mass = 0, ar_mass = 0;

  for(i = 0; i < MAX_WEAR; i++){
    if(ch->equipment[i]){
      switch(i){
        case WEAR_LIGHT:
	case WEAR_FINGER_L:
	case WEAR_FINGER_R:
	case WEAR_WRIST_L:
	case WEAR_WRIST_R:
        case WIELD:
        case HOLD:
        case POUCH:
	  break;
        default:
          ar_mass += metal_mass(ch->equipment[i]);
          break;
      }
    }
  }

  return(MAX(-95, -((ar_mass / 150) + (eq_mass / 300))));
}

int power_penalty(struct char_data *ch)
{
  int i, eq_mass = 0, cr_mass = 0;
  struct obj_data *obj;

  for(i = 0; i < MAX_WEAR; i++){
    if(ch->equipment[i]){
      eq_mass += iron_mass(ch->equipment[i]);
    }
  }

  for(obj = ch->carrying; obj; obj = obj->next_content){
    cr_mass += iron_mass(obj);
  }

  return(MAX(-95, -((eq_mass / 150) + (cr_mass / 300))));
}

int max_learn_skill(struct char_data *ch,int skill)
{
  skill_entry *sk;
  if (IS_MOB(ch)) return(100);
  sk=find_skill_entry(ch,skill);
  if (!sk) return(0); else return(sk->max_learn);
}

int max_learn_spell(struct char_data *ch, int spell)
{
  spell_entry *sp;
  if (IS_MOB(ch)) return(100);
  sp=find_spell_entry(ch,spell);
  if (!sp) return(0); else return(sp->max_learn);
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
  if (i>=0) return(MIN(ch->spells[i].learned,max_learn_spell(ch,spell)));
  else return(0);
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
          ch->spells=
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
  if (i>=0) return(MIN(max_learn_skill(ch,skill),ch->skills[i].learned));
  else return(0);
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


int exact_search(char *arg,char **list)
{
  int i;

  if (!*arg) return(-1);
  i=0;
  while (*list[i]!='\n' && str_cmp(list[i],arg)!=0) i++;
  if (*list[i]=='\n')
    return(-1);
  else return(i);
}


void do_spells(struct char_data *ch, char *arg, int cmd)
{
  spell_entry *s;
  class_entry *c;
  struct string_block sb;
  int count=1;
  int i;

  for (;isspace(*arg); arg++);
  if (!*arg) c=classes+GET_CLASS(ch);
  else {
    i =which_class(ch,arg,1);
    if (i<0) return;
    c=classes+i;
  }

  init_string_block(&sb);
  sb_printf(&sb,"Spells for %s\n",c->class_name);
  for (s=c->spells; s; s=s->next) {
    sb_printf(&sb,"[%2d] %-20s Lv:%4d mana:%4d Max: %3d Pre: %s",
		count++,
		spells[s->num],
		s->min_level,
		s->mana,
		s->max_learn,
		s->n_pre_reqs?"":"None\n");
    if (s->n_pre_reqs) {
	for (i=0; i<s->n_pre_reqs; i++)
	  sb_printf(&sb,"%s%s",i!=0?", ":"",spells[s->pre_reqs[i]]);
	sb_printf(&sb,"\n");
    }
  }
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
}

void do_skills(struct char_data *ch, char *arg, int cmd)
{
  struct string_block sb;
  skill_entry *s;
  class_entry *c;
  int count=1;
  int i;

  for (;isspace(*arg); arg++);
  if (!*arg) c=classes+GET_CLASS(ch);
  else {
    i =which_class(ch,arg,1);
    if (i<0) return;
    c=classes+i;
  }

  init_string_block(&sb);
  sb_printf(&sb,"Skills for %s\n",c->class_name);
  for (s=c->skills; s; s=s->next) {
    if(s->num < PROF_BASE){
      sb_printf(&sb,"[%2d] %-20s Lv:%4d Max: %3d Pre: %s",
		  count++,
		  skills[s->num],
		  s->min_level,
		  s->max_learn,
		  s->n_pre_reqs?"":"None\n");
      if (s->n_pre_reqs) {
	  for (i=0; i<s->n_pre_reqs; i++)
	    sb_printf(&sb,"%s%s",i!=0?", ":"",skills[s->pre_reqs[i]]);
	  sb_printf(&sb,"\n");
      }
    }
  }
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
}

void do_proficiencies(struct char_data *ch, char *arg, int cmd)
{
  struct string_block sb;
  skill_entry *s;
  class_entry *c;
  int count=1;
  int i;

  for (;isspace(*arg); arg++);
  if (!*arg) c=classes+GET_CLASS(ch);
  else {
    i =which_class(ch,arg,1);
    if (i<0) return;
    c=classes+i;
  }

  init_string_block(&sb);
  sb_printf(&sb,"Proficiencies for %s\n",c->class_name);
  for (s=c->skills; s; s=s->next) {
    if(s->num >= PROF_BASE){
      sb_printf(&sb,"[%2d] %s %-20s Lv:%4d Max: %3d Pre: %s",
		  count++,
		  (weapons[s->num - PROF_BASE].flags & WEAPF_TWO) ? "<2h>" :
		  "    ",
		  weapon_types[s->num - PROF_BASE],
		  s->min_level,
		  s->max_learn,
		  s->n_pre_reqs?"":"None\n");
      if (s->n_pre_reqs) {
	  for (i=0; i<s->n_pre_reqs; i++)
	    sb_printf(&sb,"%s%s",i!=0?", ":"",
	      weapon_types[s->pre_reqs[i] - PROF_BASE]);
	  sb_printf(&sb,"\n");
      }
    }
  }
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
}

spell_entry *find_spell_entry(struct char_data *ch,int num)
{
  spell_entry *s;
  for (s=CLASS(ch).spells; s; s=s->next)
	if (s->num==num) return(s);
  return(0);
}

skill_entry *find_skill_entry(struct char_data *ch,int num)
{
  skill_entry *s;
  for (s=CLASS(ch).skills; s; s=s->next)
    if (s->num==num) return(s);
  return(0);
}

void NewFailSkill(struct char_data *ch,int skill)
{
  struct skill_entry *sk;

  sk=find_skill_entry(ch,skill);
  if (!sk) return;
  if (number (1, 100) < 26) {	/* you lose a pt */
    if (get_skill(ch,sk->num)>sk->max_at_guild && get_skill(ch,sk->num)>0) {
      if (number (1, 101) > get_skill(ch,sk->num)) {
	set_skill(ch,sk->num,-number (1, 5));
	sendf(ch,"Your confidence is shaken in your ability to use this skill.\n");
	}
    }
  } else {	/* you gain a pt if at max from guild */
    if (get_skill(ch,sk->num)>=sk->max_at_guild && 
	  get_skill(ch,sk->num)<sk->max_learn) {
      if (number(1,100)>get_skill(ch,sk->num)) {
	sendf(ch,"In failure, you better learn the nuances of this skill.\n");
	set_skill(ch,sk->num, number (1, 5));
      }
    }
  }
}

void FailSkill(struct char_data *ch,int skill)
{
  struct skill_entry *sk;

  sk=find_skill_entry(ch,skill);
  if (!sk) return;
  if (get_skill(ch,sk->num)>=sk->max_at_guild && sk->max_at_guild &&
      get_skill(ch,sk->num)<sk->max_learn) {
    if (number(1,100)>get_skill(ch,sk->num)) {
      sendf(ch,"In failure, you better learn the nuances of this skill.\n");
      set_skill(ch,sk->num,1);
    }
  }
}

int boot_form(int form, FILE *f)
{
	char input[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char token[MAX_STRING_LENGTH];
	locdef_t *nl, *ld;
	char *p, *q, *r;
	int index, hit_factor, hp_factor;

	/*FREE(forms[form].locdefs);
	FREE(forms[form].name);*/

	forms[form].virtual = form;

	ld = NULL;
	while (fgets(input, sizeof(input), f)) {
		input[strlen(input) - 1] = '\0';
		if ((p = strchr(input, ':')) != NULL) {
			q = input; r = token;
			while (q != p) {
				*(r++) = LOWER(*q);
				q++;
			}
			*r = '\0';
			for (p++; isspace(*p); p++);
			if (!strcmp(token, "index")) {
				return(atoi(p));
			}
			else if (!strcmp(token, "name")) {
				forms[form].name = mystrdup(p);
				vlog(LOG_DEBUG, "Booting %s (%d)", forms[form].name, form);
			}
		}
		else if (strncmp(input, "LOC", 3) == 0);
		else {
			if (!strchr(input, '|')) continue;
			p = buf;
			q = input;
			while (*q != '|')
				*(p++) = *(q++);
			p--;
			while (isspace(*p)) p--;
			*(++p) = '\0';
			index = exact_search(buf, location_list);
			if (index < 0) {
				vlog(LOG_DEBUG, "Not found: '%s'", buf);
				continue;
			}
			hit_factor = atoi(++q);
			while (*q != '|') q++;
			hp_factor = atoi(++q);
			CREATE(nl, locdef_t, 1);
			bzero(nl, sizeof(nl));
			nl->type = index;
			nl->hit_factor = hit_factor;
			nl->hp_factor = hp_factor;
			if (ld){
				ld->next = nl;
				ld = nl;
			}
			else{
				ld = forms[form].locdefs = nl;
			}
			forms[form].nlocs++;
		}
	}
	return(-1);
}

void boot_forms()
{
	int i, form;
	FILE *f;
	char s[80];

	if (forms) {
		locdef_t *ld, *nl;
		for (form = 0; form < MAX_DEF_FORM; form++)
		if (forms[form].locdefs) {
			for (ld = forms[form].locdefs; ld; ld = nl) {
				nl = ld->next;
				FREE(ld);
			}
		}
		FREE(forms);
	}

	f = fopen("forms", "r");
	if (!f) {
		perror("forms");
		exit(1);
	}
	fgets(s, sizeof(s), f);
	MAX_DEF_FORM = atoi(s);
	vlog(LOG_DEBUG, "There are %d forms", MAX_DEF_FORM);
	CREATE(forms, form_t, MAX_DEF_FORM);
	for (i = 0; i < MAX_DEF_FORM; i++)
		memset(&forms[i], sizeof(form_t), 0);
	i = boot_form(0, f);
	while (i >= 0) {
		i = boot_form(i, f);
		if (i >= MAX_DEF_FORM) {
			vlog(LOG_URGENT, "Number of forms is too low in 'forms' file.");
			exit(1);
		}
	}
	fclose(f);
}

/* will return index of next class it finds... -1 = EOF */
int boot_class(int class,FILE *f)
{
  char input[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char token[MAX_STRING_LENGTH];
  u16 pre_req[50];
  u32 components[50];
  skill_entry *sk,*nk;
  spell_entry *sp,*np;
  char *p,*q,*r;
  int state = 0;
  int i,index,min_mana,min_level,max_learn,max_guild,n_pre_req,n_components;
  int cost,difficulty,source;

  min_mana=source=0;
  for (i=0; i<6; i++) {
    classes[class].saves[i]=100;
    classes[class].decrease[i]=5;
    classes[class].minsave[i]=2;
  }
  classes[class].nitems=0;
  classes[class].nraces=0;
  classes[class].multi=1.0;
  classes[class].alignment=0;
  FREE(classes[class].items);
  FREE(classes[class].races);
  FREE(classes[class].class_name);

  classes[class].index=class;

  np=NULL;
  nk=NULL;
  state=0;
  while (fgets(input,sizeof(input),f)) {
    input[strlen(input)-1]='\0';
    if ((p=strchr(input,':'))!=NULL) {
      q=input; r=token;
      while (q!=p) {
	*(r++) = LOWER(*q);
	q++;
      }
      *r='\0';
      for (p++; isspace(*p); p++);
      if (!strcmp(token,"index")) return(atoi(p));
      else if (!strcmp(token,"classname")) {
	classes[class].class_name=mystrdup(p);
	vlog(LOG_DEBUG,"Booting %s (%d)",classes[class].class_name,class);
      } else if (!strcmp(token,"abbrv")) strcpy(classes[class].abbrev,p);
      else if (!strcmp(token,"hp")) {
	p=one_argument(p,buf); classes[class].hp[0]=atoi(buf);
	p=one_argument(p,buf); classes[class].hp[1]=atoi(buf);
	p=one_argument(p,buf); classes[class].hp[2]=atoi(buf);
      } else if (!strcmp(token,"flags")) {
    #define FLAG(char,value) case char: classes[class].flags |= value; break;
	for (; *p; p++) 
	  switch (UPPER(*p)) {
		FLAG('F',FLAG_FIGHTER);
		FLAG('C',FLAG_CLERIC);
		FLAG('M',FLAG_MAGE);
		FLAG('T',FLAG_THIEF);
	      default: vlog(LOG_DEBUG,"Flag %c unknown",*p);
	  }
      } else if (!strcmp(token, "prof")) {
        p = one_argument(p, buf);
        if((i = exact_search(buf, weapon_types)) >= 0)
          classes[class].proficiency = i;
        else vlog(LOG_DEBUG,"Proficiency \"%s\" not found",buf);
      } else if (!strcmp(token,"thac0")) {
	p=one_argument(p,buf); classes[class].thaco_level=atoi(buf);
	p=one_argument(p,buf); classes[class].thaco_min=atoi(buf);
      } else if (!strcmp(token, "speed")) {
	p = one_argument(p, buf); classes[class].speed_level = atoi(buf);
	p = one_argument(p, buf); classes[class].speed_max = atoi(buf);
      } else if (!strcmp(token,"mult")) {
	classes[class].multi=atof(p);
      } else if (!strcmp(token,"min")) {
	for (i=0; i<7; i++) {
	  p=one_argument(p,buf);
	  classes[class].min[i]=atoi(buf);
	}
      } else if (!strcmp(token,"saves")) {
	for (i=0; i<6; i++) {
	  p=one_argument(p,buf);
	  classes[class].saves[i]=atoi(buf);
	}
      } else if (!strcmp(token,"decrease")) {
	for (i=0; i<6; i++) {
	  p=one_argument(p,buf);
	  classes[class].decrease[i]=atoi(buf);
	}
      } else if (!strcmp(token,"minsave")) {
	for (i=0; i<6; i++) {
	  p=one_argument(p,buf);
	  classes[class].minsave[i]=atoi(buf);
	}
      } else if (!strcmp(token,"max")) {
	for (i=0; i<7; i++) {
	  p=one_argument(p,buf);
	  classes[class].max[i]=atoi(buf);
	}
      } else if (!strcmp(token,"items")) {
	int items[100];
	classes[class].nitems=0;
	while (p && *p) {
	  p=one_argument(p,buf);
	  if (*buf)
	    items[classes[class].nitems++]=atoi(buf);
	}
	CREATE(classes[class].items,u32,classes[class].nitems);
	for (i=0; i<classes[class].nitems; i++)
	  classes[class].items[i]=items[i];
      } else if (!strcmp(token,"align")) {
	char ch;
	classes[class].alignment=0;
	for (i=0; i<3 && *p; i++,p++) {
	  ch=LOWER(*p);
	  if (ch=='n') classes[class].alignment |= ALIGN_N;
	  else if (ch=='g') classes[class].alignment |= ALIGN_G;
	  else if (ch=='e') classes[class].alignment |= ALIGN_E;
	}
      } else if (!strcmp(token,"races")) {
	u16 races[100];
	extern char *race_header_types[];
	buf[2]='\0';
        classes[class].nraces=0;
	while (p && *p) {
	  buf[0]= *(p++);
	  buf[1]= *(p++);
	  buf[0] = UPPER(buf[0]);
	  buf[1] = LOWER(buf[1]);
	  if ((i=exact_search(buf,race_header_types))>=0)
	    races[classes[class].nraces++]=i;
	  else vlog(LOG_DEBUG,"Race %s not found",buf);
	  while (isspace(*p)) p++;
	}
	CREATE(classes[class].races,u16,classes[class].nraces);
	for (i=0; i<classes[class].nraces; i++)
	  classes[class].races[i]=races[i];
      }
    } else if (strncmp(input,"SKILL",5)==0) state=0; 
    else if (strncmp(input,"SPELL",5)==0) state=1;
    else if (strncmp(input,"PROF",4)==0) state=2;
    else {
      if (!strchr(input,'|')) continue;
      p=buf;
      q=input;
      while (*q!='|')
	*(p++)=*(q++);
      p--;
      while (isspace(*p)) p--;
      *(++p)='\0';
      index = exact_search(buf,((state == 1)?spells:((state == 0)?skills:weapon_types)));
      if (index<0) {
	vlog(LOG_DEBUG,"Not found: '%s'",buf);
	continue;
      }
      min_level = atoi(++q);
      while (*q!='|') q++;
      difficulty=atoi(++q);
      while (*q!='|') q++;
      cost=atoi(++q);
      while (*q!='|') q++;
      max_guild = atoi(++q);
      while (*q!='|') q++;
      max_learn = atoi(++q);
      if (state == 1) { /* spells need two more things */
        while (*q!='|') q++;
	min_mana = atoi(++q);
	while (*q!='|') q++;
	while (!isalpha(*q)) q++;
	switch(toupper(*q)){
          case 'M':
	    source = SOURCE_MANA;
	    break;
	  case 'P':
	    source = SOURCE_POWER;
	    break;
	}
      }
      n_pre_req=0;
      n_components=0;
      while (*q)
	if (*q=='|') break;
	else q++;
      if (*q) {
	q++;
	do {
	  while (*q==',' || isspace(*q)) q++;
	  if ((*q=='|') || (!*q)) break;
	  p=buf;
	  memset(buf,sizeof(buf),'\0');
	  while (*q && *q!='|' && *q!=',')
	    *(p++) = *(q++);
	  p--;
	  if (*buf) {
	    while (isspace(*p)) p--;
	    *(++p)='\0';
	  }
	  if (!*buf) continue;
          i = exact_search(buf,((state == 1)?spells:((state == 0)?skills:weapon_types)));
	  if (i>0)
	    pre_req[n_pre_req++] = i;
	  else
	    vlog(LOG_DEBUG,"Pre req: '%s' not found(%d)",buf,index);
	} while (*q==',');
	if (*q && state == 1) {
	  q++;
	  while (q && *q) {
	    q=one_argument(q,buf);
	    if (*buf)
	      components[n_components++]=atoi(buf);
	  }
	}
      }
      if (state == 1) {
	CREATE(sp,spell_entry,1);
	bzero(sp,sizeof(sp));
	sp->num=index;
	sp->min_level=min_level;
	sp->max_at_guild=max_guild;
	sp->max_learn=max_learn;
	sp->cost=cost;
	sp->difficulty=difficulty;
	sp->n_pre_reqs=n_pre_req;
	sp->n_spell_components=n_components;
        sp->mana=min_mana;
	sp->source=source;
	if (n_pre_req>0) {
	  CREATE(sp->pre_reqs,u16,n_pre_req);
	  for (i=0; i<n_pre_req; i++)
	    sp->pre_reqs[i]=pre_req[i];
	}
	if (n_components>0) {
	  //2014 - Why did I have this code in here?  Old code was just the CREATE().
#if 0
      printf("Callocing %d of size %d\n", n_components, sizeof(u32));
	  /*sp->components = calloc(sizeof(u32), n_components);*/
	  sp->components = malloc(sizeof(u32) * n_components);
	  if(sp->components == NULL) exit(5);
	  memset(sp->components, 0, sizeof(u32)*n_components);
#else
	  CREATE(sp->components,u32,n_components);
#endif
	  for (i=0; i<n_components; i++)
	    sp->components[i]=components[i];
	}
	if (np) {
	  np->next=sp;
	  np=sp;
	} else
	  np=classes[class].spells=sp;
      } else {
        CREATE(sk,skill_entry,1);
	bzero(sk,sizeof(sk));
        if(state == 2) sk->num=index+PROF_BASE;
	else sk->num=index;
	sk->min_level=min_level;
	sk->max_at_guild=max_guild;
	sk->cost=cost;
	sk->difficulty=difficulty;
	sk->max_learn=max_learn;
	sk->n_pre_reqs=n_pre_req;
	if (n_pre_req>0) {
	  CREATE(sk->pre_reqs,u16,n_pre_req);
	  for (i=0; i<n_pre_req; i++)
	    if(state == 2) sk->pre_reqs[i]=pre_req[i]+PROF_BASE;
	    else sk->pre_reqs[i]=pre_req[i];
	}
	if (nk) {
	  nk->next=sk;
	  nk=sk;
	} else nk=classes[class].skills=sk;
      }
    }
  }
  return(-1);
}

void boot_classes()
{
  int i,class;
  FILE *f;
  char s[80];

  
  printf("Boot classes...\n");
  if (classes) {
    spell_entry *sp,*np;
    skill_entry *sk,*nk;
    for (class=0; class<MAX_DEF_CLASS; class++)
    if (classes[class].skills || classes[class].spells) {
      for (sk=classes[class].skills; sk; sk=nk) {
	nk=sk->next;
	FREE(sk->pre_reqs);
	FREE(sk);
      }
      for (sp=classes[class].spells; sp; sp=np) {
	np=sp->next;
	FREE(sp->pre_reqs);
	FREE(sp->components);
	FREE(sp);
      }
    }
    FREE(classes);
  }

  f=fopen("classes","r");
  if (!f) {
    perror("classes");
    exit(1);
  }
  fgets(s,sizeof(s),f);
  MAX_DEF_CLASS=atoi(s);
  vlog(LOG_DEBUG,"There are %d classes",MAX_DEF_CLASS);
  CREATE(classes,class_entry,MAX_DEF_CLASS);
  for (i=0; i<MAX_DEF_CLASS; i++)
    memset(classes+i,sizeof(class_entry),0);
  i=boot_class(0,f);
  while (i>=0)
    i=boot_class(i,f);
  fclose(f);
  printf("Boot classes Done!\n");
}
