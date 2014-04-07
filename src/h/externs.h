#ifndef _DIKU_EXTERNS_
#define _DIKU_EXTERNS_

#include "spells.h"
#include "interpreter.h"

extern int errno;
extern struct str_app_type str_app[];
extern struct zone_data *zone_table;
extern int top_of_zone_table;
extern struct hash_header room_db;
extern struct hash_header mob_db;
extern struct hash_header obj_db;
extern struct connection_data *connection_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct event_t *event_queue[];

extern char credits[];
extern char news[];
extern char oldnews[];
extern char donations[];
extern char motd[];
extern time_t motd_time;
extern char info[];
extern char wizlist[];
extern char welcome[];
extern char toomany[];
extern char *dirs[];
extern char *where[];
extern char *class_names[];
extern char *color_liquid[];
extern char *fullness[];

extern int RacialMax[][4];
extern int boottime;
extern char *room_bits[];
extern struct time_info_data time_info;
extern char *weekdays[];
extern char *month_name[];

extern struct weather_data weather_info;
extern char *spells[];
extern char *skills[];
extern int top_of_helpt;
extern struct help_index_element *help_index;
extern FILE *help_fl;
extern char help[];
extern struct command_info cmd_info[];
extern char *player_pflags[];
extern char *race_header_types[];
extern char *header_types[];

extern int thaco[4][51];
extern int spell_status[];
extern struct spell_info_type spell_info[];
extern int mobs_in_game,mobs_with_special;
extern int total_objects;
extern long Uptime;
extern char *init_sbrk,*db_sbrk;
extern char *connected_types[];

extern struct material_t materials[];
extern struct weapon_t weapons[];

extern char *item_types[];
extern char *wear_bits[];
extern char *extra_bits[];
extern char *drinks[];
extern char *weapon_types[];
extern char *damage_types[];
extern char *material_types[];

extern char *dirs[];
extern char *room_bits[];
extern char *exit_bits[];
extern char *sector_types[];

extern char *equipment_types[];
extern char *affected_bits[];
extern char *affected1_bits[];
extern char *immunity_names[];
extern char *apply_types[];
extern char *pc_class_types[];
extern char *npc_class_types[];
extern char *race_list[];
extern char *location_list[];
extern char *location_text[];
extern char *action_bits[];
extern char *player_bits[];
extern char *position_types[];
extern char *connected_types[];
extern int DEBUG;
extern int crash_save;
extern long reboot_time;
extern int WizLock;
extern int AcceptAllPasswords;	//2014
extern int baddoms;
extern char wizlocked[BADDOMS][32];
extern int n_no_new;
extern char no_new[BADDOMS][32];

extern class_entry *classes;
extern form_t *forms;
extern shop_data *shop_index;
extern int rev_dir[];
extern int number_of_shops;

extern int MAX_DEF_CLASS;
extern int MAX_DEF_FORM;
extern int movement_loss[];
extern int drink_aff[][3];
extern struct dex_app_type dex_app[];
extern struct dex_skill_type dex_app_skill[];
extern termcap_entry termlist[];
#endif
