#ifndef _FCNS_H_
#define _FCNS_H_

#include <sys/types.h>
#include <structs.h>
#include <hash.h>
#include <misc_proto.h>


/* File: interpreter.c */

void build_command_tree();
int norm(int min,int avg,int max);
s16 generate_weight(int race);
s16 generate_height(int race);
int search_block(char *arg, char **list, bool exact);
int find_command(char *cmd);
int old_search_block(char *argument,int begin,int length,char **list,int mode);
int command_interpreter(struct char_data *ch, char **argument);
void command_process(struct char_data *ch, int cmd, char *argument);
void argument_interpreter(char *argument,char *first_arg,char *second_arg );
int is_number(char *str);
char *one_argument(char *argument, char *first_arg );
char *last_argument(char *argument, char *last_arg);
void only_argument(char *argument, char *dest);
int fill_word(char *argument);
int is_abbrev(char *arg1, char *arg2);
void half_chop(char *string, char *arg1, char *arg2);
int special(struct char_data *ch, int cmd, char *arg);
int find_name(char *name);
int new_name_ok(char *name);
int valid_passwd(char *passwd, char *name);
int _parse_name(char *arg, char *name);
void pre_CONFIRM_GET(struct connection_data *d);
void state_CONFIRM_GET(struct connection_data *d, char *arg);
void pre_QRACE(struct connection_data *d);
void state_QRACE(struct connection_data *d, char *arg);
void pre_NME(struct connection_data *d);
void state_NME(struct connection_data *d, char *arg);
int player_count();
void pre_PWDNRM(struct connection_data *d);
void state_PWDNRM(struct connection_data *d, char *arg);
void pre_PWDEXP(struct connection_data *d);
void state_PWDEXP(struct connection_data *d, char *arg);
void state_PWDECNF(struct connection_data *d, char *arg);
void pre_RMOTD(struct connection_data *d);
void state_RMOTD(struct connection_data *d,char *arg);
void state_NMECNF(struct connection_data *d,char *arg);
void pre_PWDGET(struct connection_data *d);
void state_PWDGET(struct connection_data *d, char *arg);
void pre_PWDCNF(struct connection_data *d);
void state_PWDCNF(struct connection_data *d, char *arg);
void pre_QSEX(struct connection_data *d);
void state_QSEX(struct connection_data *d, char *arg);
void pre_QCLASS(struct connection_data *d);
void state_QCLASS(struct connection_data *d, char *arg);
void pre_FINAL(struct connection_data *d);
void state_FINAL(struct connection_data *d,char *arg);
void state_DELETE_ME(struct connection_data *d, char *arg);
void menu_delete(struct connection_data *d, char *arg);
void menu_regen(struct connection_data *d,char *arg);
void state_REGEN(struct connection_data *d, char *arg);
void menu_enter(struct connection_data *d, char *arg);
void menu_newdesc(struct connection_data *d, char *arg);
void menu_readstory(struct connection_data *d, char *arg);
void menu_newpasswd(struct connection_data *d, char *arg);
void menu_finger(struct connection_data *d);
void state_FULLNAME(struct connection_data *d, char *arg);
void state_EMAIL(struct connection_data *d, char *arg);
void state_SLCT(struct connection_data *d, char *arg);
void state_PWDNEW(struct connection_data *d, char *arg);
void state_PWDNCNF(struct connection_data *d, char *arg);
void pre_QALIGN(struct connection_data *d);
void state_QALIGN(struct connection_data *d,char *arg);
void pre_QSTATS(struct connection_data *d);
void state_QSTATS(struct connection_data *d,char *arg);
void do_terminal(struct char_data *ch, char *arg);
void pre_TERM(struct connection_data *d);
void state_TERM(struct connection_data *d, char *arg);
void nanny(struct connection_data *d, char *arg);


/* File: comm.c */

int unfriendly_domain(char *h,char baddomain[BADDOMS][32],int baddoms);
int run_the_game(int port);
void handle_remote_mud(int inp, int outp, int excp);
int game_loop(int s);
void rem_char_events(struct char_data *ch);
void rem_obj_events(struct obj_data *obj);
void add_event(int delay, int type, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int room, int virtual, char *args, void (*function)(event_t *));
void zapper();
int get_from_q(struct txt_q *queue, char *dest,int fold);
int get_from_pq(struct txt_q *queue, char *dest,int fold, int *command);
void write_to_pq(char *txt, struct txt_q *queue, int priority, int command);
void write_to_q(const char *txt, struct txt_q *queue,int fold);
struct timeval timediff(struct timeval *a, struct timeval *b);
void flush_queues(struct connection_data *d);
int init_socket(int port);
void mud_connect();
int new_id(int s,char *from);
int same_addr(int ip1[4], int ip2[4]);
int allow_desc(int cnt, struct connection_data *desc);
int new_connection(int s);
int process_output(struct connection_data *t);
int write_to_id(int desc, char *txt,int term);
int process_input(struct connection_data *t);
void close_sockets(int s);
void close_socket(struct connection_data *d);
void nonblock(int s);
void send_to_char(const char *messg, struct char_data *ch);
void save_all();
void send_to_all(char *messg);
void send_to_outdoor(char *messg);
void send_to_desert(char *messg);
void send_to_out_other(char *messg);
void send_to_arctic(char *messg);
void send_to_except(char *messg, struct char_data *ch);
void send_to_room(char *messg, int room);
void send_to_room_except(char *messg, int room, struct char_data *ch);
void send_to_room_except_two (char *messg, int room, struct char_data *ch1, struct char_data *ch2);
void act(char *str, int hide_invisible, struct char_data *ch,struct obj_data *obj, void *vict_obj, int type);
void make_prompt(struct char_data *ch,char *b);
void do_open_remote(struct char_data *ch, char *arg, int cmd);


/* File: act.comm.c */

void do_say(struct char_data *ch, char *arg, int cmd);
void do_shout(struct char_data *ch, char *argument, int cmd);
void do_chat(struct char_data *ch, char *argument, int cmd);
void do_commune(struct char_data *ch, char *argument, int cmd);
void do_tell(struct char_data *ch, char *argument, int cmd);
void do_whisper(struct char_data *ch, char *argument, int cmd);
void do_ask(struct char_data *ch, char *argument, int cmd);
void do_write(struct char_data *ch, char *argument, int cmd);
void do_mail(struct char_data *ch, char *arg, int cmd);
void do_beep(struct char_data *ch, char *argument, int cmd);
void do_cls(struct char_data *ch, char *argument, int cmd);
void do_reset(struct char_data *ch, char *argument, int cmd);
void do_channel(struct char_data *ch, char *argument, int cmd);
void do_auction(struct char_data *ch, char *argument, int cmd);
void do_gossip(struct char_data *ch, char *argument, int cmd);
void do_prompt(struct char_data *ch, char *arg, int cmd);
int load_prompt(int number, char *buf);
void do_emote(struct char_data *ch, char *argument, int cmd);


/* File: act.inf1.c */

void do_kills(struct char_data *ch,char *arg, int cmd);
int compare_name(struct connection_data **c,struct connection_data **d);
int compare_host(struct connection_data **c, struct connection_data **d);
int compare_level(struct connection_data **c, struct connection_data **d);
int compare_fd(struct connection_data **c,struct connection_data **d);
void do_users(struct char_data *ch, char *arg, int cmd);
void do_exits(struct char_data *ch, char *argument, int cmd);
void do_score(struct char_data *ch, char *argument, int cmd);
void do_time(struct char_data *ch, char *argument, int cmd);
void do_weather(struct char_data *ch, char *argument, int cmd);
void do_help(struct char_data *ch, char *argument, int cmd);
void do_listhelp(struct char_data *ch, char *argument, int cmd);
void do_listcommands(struct char_data *ch, char *argument, int cmd);
void do_wizhelp(struct char_data *ch, char *argument, int cmd);
char *select_flags(struct char_data *ch, struct char_data *victim);
void do_finger(struct char_data *ch, char *argument, int cmd);
void do_who(struct char_data *ch, char *argument, int cmd);
void do_linkdead(struct char_data *ch, char *arg, int cmd);
void do_credits(struct char_data *ch, char *argument, int cmd);
void do_oldnews(struct char_data *ch, char *argument, int cmd);
void do_motd(struct char_data *ch, char *argument, int cmd);
void do_news(struct char_data *ch, char *argument, int cmd);
void do_donations(struct char_data *ch, char *argument, int cmd);
void do_info(struct char_data *ch, char *argument, int cmd);
void do_wizlist(struct char_data *ch, char *argument, int cmd);
void do_where(struct char_data *ch,char *arg,int cmd);
void do_levels(struct char_data *ch, char *argument, int cmd);
void set_mock(struct char_data *ch, struct mock_char_data *mock_char);
void do_consider(struct char_data *ch, char *argument, int cmd);
void do_spells(struct char_data *ch, char *argument, int cmd);
void do_world(struct char_data *ch, char *argument, int cmd);
char *value_stat(int value);
void do_attribute(struct char_data *ch, char *argument, int cmd);
void do_users(struct char_data *ch, char *argument, int cmd);
void do_notell(struct char_data *ch, char *argument, int cmd);


/* File: act.move.c */

void NotLegalMove(struct char_data *ch);
int ValidMove( struct char_data *ch, int cmd);
int RawMove(struct char_data *ch, int dir);
void fall_down (struct char_data *ch);
void strip_char(struct char_data *ch);
int death_trap (struct char_data *ch);
int MoveOne(struct char_data *ch, int dir);
int MoveGroup( struct char_data *ch, int dir);
void DisplayOneMove(struct char_data *ch, int dir, int was_in);
void DisplayGroupMove(struct char_data *ch, int dir, int was_in, int total);
void do_move(struct char_data *ch, char *argument, int cmd);
void DisplayMove(struct char_data *ch, int dir, int was_in, int total);
int AddToCharHeap( struct char_data *heap[50], int *top, int total[50], struct char_data *k);
int find_door(struct char_data *ch, char *type, char *dir);
void open_door(struct char_data *ch, int dir);
void do_open(struct char_data *ch, char *argument, int cmd);
void do_close(struct char_data *ch, char *argument, int cmd);
struct obj_data *has_readied(struct char_data *ch, int vnum);
int has_key(struct char_data *ch, int key);
void do_lock(struct char_data *ch, char *argument, int cmd);
int phys_unlock_door(struct char_data *ch, struct obj_data *key, int dir);
void do_unlock(struct char_data *ch, char *argument, int cmd);
void do_pick(struct char_data *ch, char *argument, int cmd);
void do_enter(struct char_data *ch, char *argument, int cmd);
void do_leave(struct char_data *ch, char *argument, int cmd);
void do_pray(struct char_data *ch, char *argument, int cmd);
void do_stand(struct char_data *ch, char *argument, int cmd);
void do_sit(struct char_data *ch, char *argument, int cmd);
void do_rest(struct char_data *ch, char *argument, int cmd);
void do_sleep(struct char_data *ch, char *argument, int cmd);
void do_wake(struct char_data *ch, char *argument, int cmd);
void do_follow(struct char_data *ch, char *argument, int cmd);
void raw_unlock_door( struct char_data *ch, struct room_direction_data *exitp, int door);
void raw_open_door(struct char_data *ch, int dir);
int check_falling( struct char_data *ch);


/* File: act.obj1.c */

int can_carry(struct char_data *from,struct char_data *ch, struct obj_data *obj);
int phys_get(struct char_data *ch, struct obj_data *obj_object, struct obj_data *sub_object);
int corpse_consent(struct char_data *ch,struct obj_data *o);
void do_get(struct char_data *ch, char *argument, int cmd);
int CAN_DROP(struct char_data *ch, struct obj_data *obj);
int phys_drop(struct char_data *ch, struct obj_data *obj);
void do_drop(struct char_data *ch, char *argument, int cmd);
void do_put(struct char_data *ch, char *argument, int cmd);
void do_give(struct char_data *ch, char *argument, int cmd);


/* File: act.obj2.c */

void weight_change_object(struct obj_data *obj, int weight);
void name_from_drinkcon(struct obj_data *obj);
void name_to_drinkcon(struct obj_data *obj,int type);
void do_fill(struct char_data *ch, char *arg, int cmd);
void do_drink(struct char_data *ch, char *argument, int cmd);
void do_eat(struct char_data *ch, char *argument, int cmd);
void do_pour(struct char_data *ch, char *argument, int cmd);
void do_sip(struct char_data *ch, char *argument, int cmd);
void do_taste(struct char_data *ch, char *argument, int cmd);
void perform_wear(struct char_data *ch, struct obj_data *obj_object, int keyword);
int wear(struct char_data *ch, struct obj_data *obj, int keyword);
int phys_wear(struct char_data *ch, struct obj_data *obj);
void do_wear(struct char_data *ch, char *argument, int cmd);
int phys_wield(struct char_data *ch, struct obj_data *obj);
void do_wield(struct char_data *ch, char *argument, int cmd);
void do_grab(struct char_data *ch, char *argument, int cmd);
int phys_remove(struct char_data *ch, int eq_pos);
void do_remove(struct char_data *ch, char *argument, int cmd);


/* File: guild.c */

char *how_good(int percent);
int GainLevel(struct char_data *ch);
struct char_data *FindMobInRoomWithFunction(int room, int (*func)());
int GuildMaster(struct char_data *ch, int cmd, char *arg);


/* File: events.c */

int validate_char(struct char_data *ch);
int validate_obj(struct obj_data *obj);
void event_tick();
void do_events(struct char_data *ch, char *arg,int cmd);
void do_cancel(struct char_data *ch, char *arg, int cmd);


/* File: act.offensive.c */

int CORRUPT(struct char_data *ch,struct char_data *v);
int find_kill_index(struct char_data *ch, ulong vnum);
int num_kills(struct char_data *ch, ulong vnum);
void add_kill(struct char_data *ch, ulong vnum);
void do_hit(struct char_data *ch, char *argument, int cmd);
void do_kill(struct char_data *ch, char *argument, int cmd);
void do_backstab(struct char_data *ch, char *argument, int cmd);
void do_order(struct char_data *ch, char *argument, int cmd);
void do_flee(struct char_data *ch, char *argument, int cmd);
void do_batter(struct char_data *ch, char *argument, int cmd);
int phys_bash(struct char_data *ch, struct char_data *victim);
void do_bash(struct char_data *ch, char *argument, int cmd);
void do_rescue(struct char_data *ch, char *argument, int cmd);
void do_assist(struct char_data *ch, char *argument, int cmd);
void do_kick(struct char_data *ch, char *argument, int cmd);
void do_wimp(struct char_data *ch, char *argument, int cmd);
void do_breath(struct char_data *ch, char *argument, int cmd);
void do_shoot(struct char_data *ch, char *argument, int cmd);


/* File: act.other.c */

void do_gain(struct char_data *ch, char *argument, int cmd);
void do_guard(struct char_data *ch, char *argument, int cmd);
void do_sacrifice(struct char_data *ch, char *argument, int cmd);
void do_donate(struct char_data *ch, char *argument, int cmd);
int phys_junk(struct char_data *ch, struct obj_data *obj);
void do_junk(struct char_data *ch, char *argument, int cmd);
void do_qui(struct char_data *ch, char *argument, int cmd);
void do_title(struct char_data *ch, char *argument, int cmd);
void all_to_room(struct char_data *ch);
void do_quit(struct char_data *ch, char *argument, int cmd);
void do_save(struct char_data *ch, char *argument, int cmd);
void do_not_here(struct char_data *ch, char *argument, int cmd);
void do_sneak(struct char_data *ch, char *argument, int cmd);
void do_hide(struct char_data *ch, char *argument, int cmd);
void do_steal(struct char_data *ch, char *argument, int cmd);
void do_practice(struct char_data *ch, char *arg, int cmd);
void do_idea(struct char_data *ch, char *argument, int cmd);
void do_typo(struct char_data *ch, char *argument, int cmd);
void do_bug(struct char_data *ch, char *argument, int cmd);
void do_noexits(struct char_data *ch, char *argument, int cmd);
void do_brief(struct char_data *ch, char *argument, int cmd);
void do_compact(struct char_data *ch, char *argument, int cmd);
void do_group(struct char_data *ch, char *argument, int cmd);
void do_quaff(struct char_data *ch, char *argument, int cmd);
void do_recite(struct char_data *ch, char *argument, int cmd);
void do_use(struct char_data *ch, char *argument, int cmd);
void do_plr_noshout(struct char_data *ch, char *argument, int cmd);
void do_choose(struct char_data *ch, char *arg, int cmd);


/* File: act.social.c */

char *fread_action(FILE *fl);
void boot_social_messages(void);
int find_action(int cmd);
void do_action(struct char_data *ch, char *argument, int cmd);
void do_insult(struct char_data *ch, char *argument, int cmd);
void boot_pose_messages(void);
void do_pose(struct char_data *ch, char *argument, int cmd);


/* File: act.wizard.c */

void boot_no_rent();
int is_no_rent(unsigned long vnum);
void do_norent(struct char_data *ch, char *arg, int cmd);
void do_skill(struct char_data *ch, char *arg,int cmd);
void do_data(struct char_data *ch, char *arg, int cmd);
void do_fine(struct char_data *ch, char *arg, int cmd);
void arena_monitor(void);
void do_chaos_open(struct char_data *ch, char *arg, int cmd);
void do_statistics(struct char_data *ch, char *arg, int cmd);
void do_polymorph(struct char_data *ch, char *argument, int cmdnum);
void do_toggle(struct char_data *ch, int field, char *on, char *off);
void do_set(struct char_data *ch, char *arg, int cmd);
void do_running_log(struct char_data *ch, char *argument, int cmdnum);
void do_char_log(struct char_data *ch,char *argument, int cmdnum);
void do_log(struct char_data *ch,char *argument, int cmdnum);
void do_highfive(struct char_data *ch, char *argument, int cmd);
void do_nonew(struct char_data *ch, char *argument, int cmd);
void do_wizlock(struct char_data *ch, char *argument, int cmd);
void do_echo(struct char_data *ch, char *argument, int cmd);
void do_reboot (struct char_data *ch, char *arg, int cmd);
void handle_reboot (void);
void do_system(struct char_data *ch, char *argument, int cmd);
void do_trans(struct char_data *ch, char *argument, int cmd);
void do_list_spells(struct char_data *ch, char *arg, int *cmd);
void do_list_skills(struct char_data *ch, char *arg, int *cmd);
void do_shutdow(struct char_data *ch, char *argument, int cmd);
void do_shutdown(struct char_data *ch, char *argument, int cmd);
void do_snoop(struct char_data *ch, char *argument, int cmd);
void do_switch(struct char_data *ch, char *argument, int cmd);
void do_return(struct char_data *ch, char *argument, int cmd);
void do_force(struct char_data *ch, char *argument, int cmd);
void do_load(struct char_data *ch, char *argument, int cmd);
void roll_abilities(struct char_data *ch);
void do_start(struct char_data *ch);
void do_advance(struct char_data *ch, char *argument, int cmd);
void do_reroll(struct char_data *ch, char *argument, int cmd);
void do_restore(struct char_data *ch, char *argument, int cmd);
void do_noshout(struct char_data *ch, char *argument, int cmd);
void do_nohassle(struct char_data *ch, char *argument, int cmd);
void do_stealth(struct char_data *ch, char *argument, int cmd);
void do_debug(struct char_data *ch, char *argument, int cmd);
void do_invis(struct char_data *ch, char *argument, int cmd);
void do_noaffect(struct char_data *ch, char *argument, int cmd);
void do_file(struct char_data *ch, char *arg, int cmd);
void do_police(struct char_data *ch, char *argument, int cmd);
void do_passwd(struct char_data *ch, char *arg, int cmd);
void do_clean(struct char_data *ch, char *arg, int cmd);
void do_linkload(struct char_data *ch, char *arg, int cmd);
void do_crash(struct char_data *ch, char *arg, int cmd);
void do_trashcan(struct char_data *ch, char *arg, int cmd);
void do_maxplayers(struct char_data *ch, char *arg, int cmd);
void do_setall(struct char_data *ch, char *arg,int cmd);


/* File: handler.c */

void stopAllFromConsenting(struct char_data *ch);
char *fname(char *namelist);
int split_string(char *str, char *sep, char **argv);
int isname(char *str, char *namelist);
void init_string_block(struct string_block *sb);
int append_to_string_block(struct string_block *sb, char *str);
void page_string_block(struct string_block *sb, struct char_data *ch);
void destroy_string_block(struct string_block *sb);
int good_affect(struct obj_affected_type *affect);
void affect_modify(struct char_data *ch,byte loc,u32 mod, u32 *bitv, bool add);
void affect_total(struct char_data *ch);
void affect_to_char( struct char_data *ch, struct affected_type *af );
void affect_remove( struct char_data *ch, struct affected_type *af );
void affect_from_char( struct char_data *ch, short skill);
bool affected_by_spell( struct char_data *ch, short skill );
void affect_join( struct char_data *ch, struct affected_type *af, bool avg_dur, bool avg_mod );
void char_from_room(struct char_data *ch);
void char_to_room(struct char_data *ch, int room);
void obj_to_warehouse(struct obj_data *obj, struct char_data *ch);
void obj_from_warehouse(struct obj_data *obj);
void obj_to_char(struct obj_data *object, struct char_data *ch);
void obj_from_char(struct obj_data *object);
int apply_ac(struct char_data *ch, int eq_pos, int state);
void equip_char(struct char_data *ch, struct obj_data *obj, int pos);
struct obj_data *unequip_char(struct char_data *ch, int pos);
int get_number(char **name);
struct obj_data *get_obj_in_list(char *name, struct obj_data *list);
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list);
struct obj_data *get_obj_num(int nr);
struct char_data *get_char_room(char *name, int room);
struct char_data *get_char(char *name);
struct char_data *get_char_num(int nr);
void obj_to_room(struct obj_data *object, int room);
void obj_from_room(struct obj_data *object);
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to);
void obj_from_obj(struct obj_data *obj);
void object_list_new_owner(struct obj_data *list, struct char_data *ch);
void extract_obj(struct obj_data *obj);
void update_char_objects( struct char_data *ch );
void extract_char(struct char_data *ch);
struct char_data *get_char_room_vis(struct char_data *ch, char *name);
struct char_data *get_char_vis_world(struct char_data *ch, char *name, slong *count);
struct char_data *get_char_vis(struct char_data *ch, char *name);
struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, struct obj_data *list);
struct obj_data *get_obj_vis_world(struct char_data *ch, char *name, slong *count);
struct obj_data *get_obj_vis(struct char_data *ch, char *name);
struct obj_data *get_obj_vis_accessible(struct char_data *ch, char *name);
struct obj_data *create_money( int amount );
struct obj_data *get_obj_vis_equ(struct char_data *ch, char *name);
int generic_find(char *arg, int bitvector, struct char_data *ch, struct char_data **tar_ch, struct obj_data **tar_obj);


/* File: db.c */

void boot_db(void);
void reset_time(void);
void boot_players(void);
void cleanout_room(struct room_data *rp);
void completely_cleanout_room(struct room_data *rp);
void load_one_room(FILE *fl, struct room_data *rp);
void boot_world(void);
void old_boot_world(void);
void allocate_room(int room_number);
void setup_dir(FILE *fl, int room, int dir);
void renum_zone_table(void);
void boot_zones(void);
int read_one_room(FILE *fp);
void boot_shp();
void finish_obj(struct obj_data *o);
void init_obj(struct obj_data *o);
void boot_obj();
void boot_mob();
struct obj_data *get_obj(slong vnum);
struct char_data *get_mob(slong vnum);
struct obj_data *read_object(slong nr, int type);
void zone_update(void);
void reset_zone(int zone);
int is_empty(int zone_nr);
void delete_entry(struct char_data *ch, char *arg, int cmd);
void create_entry(char *name);
char *fread_string(FILE *fl);
void free_char(struct char_data *ch);
void free_obj(struct obj_data *obj);
int file_to_q(char *name, struct connection_data *d, int more);
int file_to_string(char *name, char *buf);
void reset_char(struct char_data *ch);
void clear_char(struct char_data *ch);
void clear_object(struct obj_data *obj);
void init_char(struct char_data *ch);
struct room_data *real_roomp(slong virtual);
index_mem *real_mobp(slong virtual);
index_mem *real_objp(slong virtual);
void wipe_obj(struct obj_data *obj);
void special_boot_check(struct char_data *ch);


/* File: utility.c */

void cftime(char *buf,char *fmt,time_t *time);
int MIN(int a, int b);
int MAX(int a, int b);
void ForAllObjects(struct char_data *ch, int val,int (*func)(struct obj_data *,struct char_data *,int));
int GetItemClassRestrictions(struct obj_data *obj);
int CAN_SEE(struct char_data *s, struct char_data *o);
int exit_ok(struct room_direction_data	*exit, struct room_data **rpp);
int MobVnum( struct char_data *c);
int ObjVnum( struct obj_data *o);
void Zwrite (FILE *fp, char cmd, int tf, int arg1, int arg2, int arg3, char *desc);
void RecZwriteObj(FILE *fp, struct obj_data *o);
FILE *MakeZoneFile( struct char_data *c);
int IsImmune(struct char_data *ch, int bit);
int IsResist(struct char_data *ch, int bit);
int IsSusc(struct char_data *ch, int bit);
int number(int from, int to);
int dice(int number, int size);
int scan_number(char *text, int *rval);
int str_cmp(char *arg1, char *arg2);
int strn_cmp(char *arg1, char *arg2, int n);
void log2(char *str);
void log(char *str);
void slog(char *str);
void sprintbit(unsigned long vektor, char *names[], char *result);
void sprinttype(int type, char *names[], char *result);
struct time_info_data real_time_passed(time_t t2, time_t t1);
struct time_info_data mud_time_passed(time_t t2, time_t t1);
struct time_info_data age(struct char_data *ch);
char in_group ( struct char_data *ch1, struct char_data *ch2);
char getall(char *name, char *newname);
int getabunch(char *name, char  *newname);
int DetermineExp( struct char_data *mob, int exp_flags);
void event_river(struct event_t *event);
void RoomSave(struct char_data *ch, long start, long end);
void RoomLoad(struct char_data *ch,int start, int end,char *name);
void fake_setup_dir(FILE *fl, int room, int dir);
int IsHumanoid( struct char_data *ch);
int IsAnimal( struct char_data *ch);
int IsUndead( struct char_data *ch);
int IsLycanthrope( struct char_data *ch);
int IsDiabolic( struct char_data *ch);
int IsReptile( struct char_data *ch);
int HasHands( struct char_data *ch);
int IsPerson( struct char_data *ch);
int IsExtraPlanar( struct char_data *ch);
void SetHunting( struct char_data *ch, struct char_data *tch);
void CallForGuard( struct char_data *ch, struct char_data *vict, int lev);
void StandUp (struct char_data *ch);
void FighterMove( struct char_data *ch);
void DevelopHatred( struct char_data *ch, struct char_data *v);
void event_teleport(event_t *event);
int HasObject( struct char_data *ch, int ob_num);
int room_of_object(struct obj_data *obj);
struct char_data *char_holding(struct obj_data *obj);
int RecCompObjNum( struct obj_data *o, int obj_num);
void RestoreChar(struct char_data *ch);
void RemAllAffects( struct char_data *ch);
int which_class(struct char_data *ch,char *s,int list_on_error);
int can_have(struct char_data *ch, struct obj_data *o);
char *str_str(char *p,char *q);


/* File: spec_assign.c */

void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);


/* File: shop.c */

int shop_keeper(struct char_data *ch, int cmd, char *arg);
void assign_the_shopkeepers();
void shop_give_inventory(struct char_data *ch);


/* File: limits.c */

int calc_speed(struct char_data *ch, int hand);
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6);
int mana_limit(struct char_data *ch);
int power_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);
int power_gain(struct char_data *ch);
int mana_gain(struct char_data *ch);
int hit_gain(struct char_data *ch);
int move_gain(struct char_data *ch);
void advance_level(struct char_data *ch);
void drop_level(struct char_data *ch);
void set_title(struct char_data *ch);
void gain_exp(struct char_data *ch, slong gain);
void gain_exp_regardless(struct char_data *ch, slong gain);
void gain_condition(struct char_data *ch,int condition,int value);
void check_idling(struct char_data *ch);
void point_update( void );
int ObjFromCorpse( struct obj_data *c);


/* File: mobact.c */

int act_state(struct char_data *ch);
int act_fight(struct char_data *ch);
int act_caster(struct char_data *ch);
int eval_spells(struct char_data *ch);
int maintain_pos(struct char_data *ch);
int aid_friends(struct char_data *ch);
int spread_hatred(struct char_data *ch);
char *uniq_equip(struct char_data *ch, int eq_pos);
int discard_items(struct char_data *ch);
int find_items(struct char_data *ch);
int rate_list(struct obj_data *list, struct char_data *ch);
int rate_effects(struct obj_data *obj, struct char_data *ch);
int rate_weapon(struct obj_data *obj, struct char_data *ch);
int rate_armor(struct obj_data *obj, struct char_data *ch);
void mobile_guardian(struct char_data *ch);
void mobile_wander(struct char_data *ch);
void MobHunt(struct char_data *ch);
void MobScavenge(struct char_data *ch);
void mobile_healer(struct char_data *ch);
int SameRace( struct char_data *ch1, struct char_data *ch2);
int AssistFriend( struct char_data *ch);
int mobile_alive(struct char_data *mob);
void event_mobile(event_t *event);
void for_each_char(void (*to_do)(struct char_data *));
void mobile_activity(void);


/* File: fight.c */

s32 modify_exp(struct char_data *ch,s32 xp,ulong vnum);
void appear(struct char_data *ch);
void load_messages(void);
void update_pos( struct char_data *victim );
int check_peaceful(struct char_data *ch, char *msg);
void set_fighting(struct char_data *ch, struct char_data *vict);
void stop_fighting(struct char_data *ch);
void make_corpse(struct char_data *ch);
void change_alignment(struct char_data *ch, struct char_data *victim);
void death_cry(struct char_data *ch);
void raw_kill(struct char_data *ch);
void event_death(event_t *event);
void die(struct char_data *ch);
void group_gain(struct char_data *ch, struct char_data *victim);
char *location_name(struct char_data *vict, int location);
int damage(struct char_data *ch, struct char_data *victim, int dam, int attacktype);
int HeightClass(struct char_data *ch);
void hit(struct char_data *ch, struct char_data *victim, int type);
void mob_attack(struct char_data *ch,struct char_data *vict);
void event_strike(event_t *event);
struct char_data *FindVictim( struct char_data *ch);
struct char_data *FindAnyVictim( struct char_data *ch);
int PreProcDam(struct char_data *ch, int type, int dam);
int DamageOneItem( struct char_data *ch, int dam_type, struct obj_data *obj);
void MakeScrap( struct char_data *ch, struct obj_data *obj);
void DamageAllStuff( struct char_data *ch, int dam_type);
int DamageItem(struct char_data *ch, struct obj_data *o, int num);
int ItemSave( struct obj_data *i, int dam_type);
int DamagedByAttack( struct obj_data *i, int dam_type);
int WeaponCheck(struct char_data *ch, struct char_data *v, int type, int dam);
void DamageStuff(struct char_data *v, int type, int dam);
int GetItemDamageType( int type);
int SkipImmortals(struct char_data *v, int amnt);
void WeaponSpell( struct char_data *c, struct char_data *v, int type);
struct char_data *FindAnAttacker(struct char_data *ch);
void shoot( struct char_data *ch, struct char_data *victim);
int SwitchTargets( struct char_data *ch, struct char_data *vict);
void perform_special_rooms (int pulse);


/* File: modify.c */

void string_add(struct connection_data *d, char *str);
void quad_arg(char *arg, int *type, char *name, int *field, char *string);
void do_string(struct char_data *ch, char *arg, int cmd);
void bisect_arg(char *arg, int *field, char *string);
char *one_word(char *argument, char *first_arg );
struct help_index_element *build_help_index(FILE *fl, int *num);
void page_string(struct connection_data *d, char *str, int keep_internal);
void show_string(struct connection_data *d, char *input);


/* File: weather.c */

void weather_and_time(int mode);
void another_hour(int mode);
void weather_change(void);
void ChangeWeather( int change);
void GetMonth( int month);


/* File: spells1.c */

void cast_nova(int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_burning_hands (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_chain_lightn(int level, struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_call_lightning (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_chill_touch(int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_shocking_grasp(int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_colour_spray (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_earthquake (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_energy_drain (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_fireball (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_harm (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_lightning_bolt (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_acid_blast (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_cone_of_cold (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_firestorm (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_ice_storm (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_meteor_swarm (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_flamestrike (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_magic_missile (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_cause_light (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_cause_serious (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_cause_critic (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_geyser (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_green_slime (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level);
void cast_invulnerability(int level, struct char_data *ch, char *arg,int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_haste(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_translook( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_telelook( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_teleimage( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_age(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_ageing(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_fury(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_calm( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_full_heal( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_mystic_heal( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_scry( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_protection_from_evil( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_nosleep(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_heroism(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_dexterity(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_nocharm(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_charm_person( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_telemove( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_transmove( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_vigorize_critic(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_vitalize_mana(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_cure_critic( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_vigorize_light( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_vigorize_serious( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_summon_demon( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_summon_elemental_ii( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_nofear( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_nosummon( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_dispel_invisible(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_dispel_good( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_chilly( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_metalskin(int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_wither(int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_drain_vigor_light( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_drain_vigor_serious( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_drain_vigor_critic( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_farsee( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_vitality( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_rejuvenate(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_recharger( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_shriek(  int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_mage_fire( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_ice_lance( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_freeze( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_fire_bolt( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_repulsor( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_mind_thrust( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_disruptor( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_full_harm( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_holy_word ( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_unholy_word ( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level);
void cast_sunray( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);
void cast_vampiric_touch(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level);


/* File: spells2.c */

void cast_resurrection(int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_mana (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_haven (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_armor (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_stone_skin (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_astral_walk (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_teleport (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_bless (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_infravision (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_true_seeing (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_blindness (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_light (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_cont_light (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_clone (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_control_weather (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_create_food (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_create_water (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_water_breath (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_protect_elem_fire (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_protect_elem_cold (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_flying (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_cure_blind (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_cure_light (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_cure_serious (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_refresh (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_second_wind (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_shield (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_curse (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_detect_evil (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_detect_invisibility (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_detect_magic (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_detect_poison (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_dispel_evil (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_faerie_fire (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_enchant_weapon (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_enchant_armor (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_heal (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_invisibility (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_locate_object (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_poison (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_remove_curse (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_remove_poison (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_remove_paralysis (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_sanctuary (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_fireshield (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_sleep (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_strength (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_ventriloquate (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_word_of_recall (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_summon (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_charm_person (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_charm_monster (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_sense_life (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_identify (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_dragon_breath(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *potion, int eff_level);
void cast_fire_breath (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_frost_breath (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_acid_breath (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_gas_breath (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_lightning_breath (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_knock (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_know_alignment (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_weakness (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_dispel_magic (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_animate_dead (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_succor (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_paralyze (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_fear (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_turn (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_faerie_fog (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_poly_self (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_minor_creation (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_conjure_elemental (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_cacaodemon (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_mon_sum1 (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_mon_sum2 (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_mon_sum3 (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_mon_sum4 (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_mon_sum5 (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_mon_sum6 (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_mon_sum7 (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level);
void cast_portal( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level );
void cast_pword_kill( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level );
void cast_pword_blind( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level );
void cast_scare( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level );
void cast_familiar( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level );
void cast_aid( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level );
void cast_slow_poison( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level );
void cast_gust_of_wind( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level );
void cast_warp_weapon( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level );
void cast_beacon( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level );
void cast_delayed_teleport( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level );


/* File: spell_parser.c */

struct obj_data *SpellComponent(struct char_data *ch, int vnum);
int is_spell_component(struct char_data *ch, int vnum);
int has_components(struct char_data *ch, int spell);
void use_components(struct char_data *ch, int spell);
void affect_update ();
void clone_char (struct char_data *ch);
void clone_obj( struct obj_data *obj );
int count_followers (struct char_data *ch);
bool circle_follow (struct char_data *ch, struct char_data *victim);
void stop_follower ( struct char_data *ch);
void die_follower ( struct char_data *ch);
void add_follower (struct char_data *ch,struct char_data *leader);
void say_spell ( struct char_data *ch, int si);
int saving_throw_val(struct char_data *ch,int save_type);
bool saves_spell ( struct char_data *ch, sh_int save_type);
bool ImpSaveSpell ( struct char_data *ch, sh_int save_type, int mod);
char *skip_spaces ( char *string);
void do_cast( struct char_data *ch, char *argument, int cmd);
int phys_cast(struct char_data *ch, int spl, struct char_data *vict, char * argument);
void event_spell(event_t *event);


/* File: reception.c */

char *name_to_path(char *name);
void update_obj_file(void);
int receptionist(struct char_data *ch, int cmd, char *arg);
void take_obj_list(struct char_data *ch,struct obj_data *o,int *amount,struct char_data *give_to);
void fine(struct char_data *ch,int amount,struct char_data *to_whom);
int rent_cost(struct char_data *ch, struct obj_data *o,int threshold);
int rent_cost2(const struct obj_data *o, int threshold);
int real_cost(struct char_data *ch,struct obj_data *o,int threshold);
int value_of_char_equip(struct char_data *ch,int threshold);
int amount_of_tax(struct char_data *ch);
int count_items(struct char_data *ch);
int count_rentables(struct obj_data *list);
struct obj_data *next_junk(struct obj_data *obj);
void remove_junk(struct char_data *ch, int do_msg);
void rent_char(struct char_data *ch);


/* File: constants.c */



/* File: spec_mob.c */

int deductable(struct char_data *ch,int threshold);
struct char_data *InsuranceGuy();
int contains_item(struct obj_data *obj, ulong vnum);
int has_object(struct char_data *ch, ulong vnum);
int insurance_salesman(struct char_data *ch,int cmd, char *arg);
int is_target_room_p(int room, void *tgt_room);
int named_object_on_ground(int room, void *c_data);
void npc_steal(struct char_data *ch,struct char_data *victim);
int snake(struct char_data *ch, int cmd, char *arg);
int PaladinGuildGuard( struct char_data *ch, int cmd, char *arg);
int AbyssGateKeeper( struct char_data *ch, int cmd, char *arg);
int blink( struct char_data *ch, int cmd, char *arg);
int WeapRepairGuy( struct char_data *ch, int cmd, char *arg);
int RepairGuy( struct char_data *ch, int cmd, char *arg);
int citizen(struct char_data *ch, int cmd, char *arg);
int ghoul(struct char_data *ch, int cmd, char *arg);
int WizardGuard(struct char_data *ch, int cmd, char *arg);
int vampire(struct char_data *ch, int cmd, char *arg);
int wraith(struct char_data *ch, int cmd, char *arg);
int shadow(struct char_data *ch, int cmd, char *arg);
int geyser(struct char_data *ch, int cmd, char *arg);
int green_slime(struct char_data *ch, int cmd, char *arg);
int breath_weapon(struct char_data *ch, struct char_data *target, int mana_cost, void (*func)());
int use_breath_weapon(struct char_data *ch, struct char_data *target, int cost, void (*func)());
int BreathWeapon(struct char_data *ch, int cmd, char *arg);
int DracoLich(struct char_data *ch, int cmd, char *arg);
int Drow(struct char_data *ch, int cmd, char *arg);
int Leader(struct char_data *ch, int cmd, char *arg);
int thief(struct char_data *ch, int cmd, char *arg);
int magic_user(struct char_data *ch, int cmd, char *arg);
int cleric(struct char_data *ch, int cmd, char *arg);
int guild_guard(struct char_data *ch, int cmd, char *arg);
int Inquisitor(struct char_data *ch, int cmd, char *arg);
int puff(struct char_data *ch, int cmd, char *arg);
int regenerator( struct char_data *ch, int cmd, char *arg);
int replicant( struct char_data *ch, int cmd, char *arg);
int Tytan(struct char_data *ch, int cmd, char *arg);
int AbbarachDragon(struct char_data *ch, int cmd, char *arg);
int fido(struct char_data *ch, int cmd, char *arg);
int janitor(struct char_data *ch, int cmd, char *arg);
int tormentor(struct char_data *ch, int cmd, char *arg);
int Fighter(struct char_data *ch, int cmd, char *arg);
int RustMonster(struct char_data *ch, int cmd, char *arg);
int temple_labrynth_liar(struct char_data *ch, int cmd, char *arg);
int temple_labrynth_sentry(struct char_data *ch, int cmd, char *arg);
int Whirlwind (struct char_data *ch, int cmd, char *arg);
int NudgeNudge(struct char_data *ch, int cmd, char *arg);
int AGGRESSIVE(struct char_data *ch, int cmd, char *arg);
int cityguard(struct char_data *ch, int cmd, char *arg);
int Ringwraith( struct char_data *ch, int cmd, char *arg);
int WarrenGuard(struct char_data *ch, int cmd, char *arg);
int find_path(int in_room, int (*predicate)(), void *c_data, int depth);
int choose_exit(int in_room, int tgt_room, int depth);
int go_direction(struct char_data *ch, int dir);
int sisyphus(struct char_data *ch, int cmd, char *arg);
int abyss_guard(struct char_data *ch, int cmd, char *arg);
int jabberwocky(struct char_data *ch, int cmd, char *arg);
int flame(struct char_data *ch, int cmd, char *arg);
int banana(struct char_data *ch, int cmd, char *arg);
int paramedics(struct char_data *ch, int cmd, char *arg);
int jugglernaut(struct char_data *ch, int cmd, char *arg);
int delivery_elf(struct char_data *ch, int cmd, char *arg);
int delivery_beast(struct char_data *ch, int cmd, char *arg);
int Keftab(struct char_data *ch, int cmd, char *arg);
int StormGiant(struct char_data *ch, int cmd, char *arg);
int Manticore(struct char_data *ch, int cmd, char *arg);
int Kraken(struct char_data *ch, int cmd, char *arg);
int tarin_good(struct char_data *ch, int cmd, char *arg);
int tarin(struct char_data *ch, int cmd, char *arg);
int god(struct char_data *ch, int cmd, char *arg);
int andy_wilcox(struct char_data *ch, int cmd, char *arg);
int eric_johnson(struct char_data *ch, int cmd, char *arg);
int tax_collector(struct char_data *ch,int cmd, char *arg);
int mayor(struct char_data *ch, int cmd, char *arg);
int zm_tired(struct char_data *zmaster);
int zm_stunned_followers(struct char_data *zmaster);
void zm_init_combat(struct char_data *zmaster, struct char_data *target);
int zm_kill_fidos(struct char_data *zmaster);
int zm_kill_aggressor(struct char_data *zmaster);
int zombie_master(struct char_data *ch, int cmd, char *arg);
int House(struct char_data *ch, int cmd, char *arg);
int ninja_master(struct char_data *ch, int cmd, char *arg);


/* File: signals.c */

void signal_fatal(int sig);
void wakeup_request();
void hupsig();
void logsig(int sig);
void signal_setup();


/* File: board.c */

void InitBoards();
void InitABoard(struct obj_data *obj);
struct Board *FindBoardInRoom(int room);
int board(struct char_data *ch, int cmd, char *arg);
void post_message(struct char_data *ch);
void write_msg(struct char_data *ch, char *arg, struct Board *b);
int board_remove_msg(struct char_data *ch, char *arg, struct Board *b);
int board_display_msg(struct char_data *ch, char *arg, struct Board *b);


/* File: magic.c */

void spell_nova(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_magic_missile(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_chill_touch(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_burning_hands(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_shocking_grasp(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_lightning_bolt(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_colour_spray(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_energy_drain(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_fireball(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_earthquake(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_dispel_evil(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_call_lightning(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_harm(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_haven(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_armor(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
struct room_data *spell_char_to_mob (int location, int success, struct char_data *ch);
void spell_astral_walk(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_teleport(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_bless(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_blindness(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_clone(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_control_weather(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_create_food(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_create_water(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_cure_blind(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_cure_critic(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_cure_light(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_curse(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_detect_evil(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_detect_invisibility(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_detect_magic(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_detect_poison(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_enchant_weapon(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_heal(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_invisibility(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_locate_object(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_poison(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_protection_from_evil(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_remove_curse(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_remove_poison(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_fireshield(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_sanctuary(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_strength(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_ventriloquate(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_word_of_recall(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_charm_monster(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_identify(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_fire_breath(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_frost_breath(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_acid_breath(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_gas_breath(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_lightning_breath(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_invulnerability(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_shriek(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_drain_vigor_light(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_drain_vigor_serious(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_drain_vigor_critic(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_wither(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_summon_demon(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_fury(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_calm(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_summon_elemental_ii(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_mage_fire(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_firestorm(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_full_harm(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_holy_word(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_unholy_word(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_translook(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_telelook(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_teleimage(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_transmove(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_telemove(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_vitalize_mana(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_rejuvenate(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_age(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_ageing(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_cure_serious(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_invis_group(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_heroes_feast(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_full_heal(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_mystic_heal(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_scry(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_nosleep(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_sleep(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_nosummon(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_dexterity(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_summon(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_charm_person(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_nocharm(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_sense_life(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_haste(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_recharger(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_chilly(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_sunray(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_metalskin(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_vampiric_touch(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_vigorize_light(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_vigorize_serious(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_vigorize_critic(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_vitality(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_farsee(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_fear(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_heroism(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_ice_lance(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_freeze(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_fire_bolt(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_repulsor(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_mind_thrust(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_disruptor(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level);
void spell_dispel_invisible(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);


/* File: magic2.c */

int max_follow(struct char_data *ch);
void spell_resurrection(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_cause_light(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_cause_critical(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_cause_serious(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_mana(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_second_wind(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_flamestrike(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_dispel_good(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_turn(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_remove_paralysis(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_succor(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_detect_charm(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_true_seeing(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_poly_self(int level, struct char_data *ch, struct char_data *mob, struct obj_data *obj, int eff_level);
void spell_minor_create(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_stone_skin(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_infravision(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_shield(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_weakness(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_acid_blast(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_cone_of_cold(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_ice_storm(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_poison_cloud(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_chain_lightn(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_major_create(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_summon_obj(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_sending(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_meteor_swarm(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_Create_Monster(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_light(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_fly(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_refresh(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_water_breath(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_cont_light(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_animate_dead(int level, struct char_data *ch, struct char_data *victim, struct obj_data *corpse,int eff_level);
void spell_know_alignment(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_dispel_magic(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_paralyze(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_fear(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_prot_align_group(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_conjure_elemental(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_faerie_fire (int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_faerie_fog (int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_cacaodemon(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_improved_identify(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_geyser(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_green_slime(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
struct char_data *find_random_mob(struct char_data *ch);
void spell_portal(int level, struct char_data *ch, struct char_data *tmp_ch, struct obj_data *obj, int eff_level);
void spell_pword_kill(int  level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_pword_blind(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_scare(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_familiar(int level, struct char_data *ch, struct char_data **victim, struct obj_data *obj, int eff_level);
void spell_aid(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_slow_poison(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_gust_of_wind(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_warp_weapon(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);


/* File: skills.c */

int iron_mass(struct obj_data *obj);
int metal_mass(struct obj_data *obj);
int stealth_penalty(struct char_data *ch);
int power_penalty(struct char_data *ch);
int max_learn_skill(struct char_data *ch,int skill);
int max_learn_spell(struct char_data *ch, int spell);
int find_spell(struct char_data *ch,int spell);
int get_spell(struct char_data *ch,int spell);
void set_spell(struct char_data *ch,int spell,int mod);
int find_skill(struct char_data *ch,int skill);
int get_skill(struct char_data *ch,int skill);
void set_skill(struct char_data *ch,int skill,int mod);
int exact_search(char *arg,char **list);
void do_spells(struct char_data *ch, char *arg, int cmd);
void do_skills(struct char_data *ch, char *arg, int cmd);
void do_proficiencies(struct char_data *ch, char *arg, int cmd);
spell_entry *find_spell_entry(struct char_data *ch,int num);
skill_entry *find_skill_entry(struct char_data *ch,int num);
void NewFailSkill(struct char_data *ch,int skill);
void FailSkill(struct char_data *ch,int skill);
int boot_form(int form, FILE *f);
void boot_forms();
int boot_class(int class,FILE *f);
void boot_classes();


/* File: opinion.c */

void FreeHates( struct char_data *ch);
void FreeFears( struct char_data *ch);
int RemHated( struct char_data *ch, struct char_data *pud);
int AddHated( struct char_data *ch, struct char_data *pud);
void AddHatred( struct char_data *ch, int parm_type, int parm);
void RemHatred( struct char_data *ch, unsigned short bitv);
int Hates( struct char_data *ch, struct char_data *v);
int Fears( struct char_data *ch, struct char_data *v);
int RemFeared( struct char_data *ch, struct char_data *pud);
int AddFeared( struct char_data *ch, struct char_data *pud);
void AddFears( struct char_data *ch, int parm_type, int parm);
struct char_data *FindAHatee( struct char_data *ch);
struct char_data *FindAFearee( struct char_data *ch);
void ZeroHatred(struct char_data *ch, struct char_data *v);
void ZeroFeared(struct char_data *ch, struct char_data *v);
void DeleteHatreds(struct char_data *ch);
void DeleteFears(struct char_data *ch);


/* File: trap.c */

void do_settrap( struct char_data *ch, char *arg, int cmd);
int CheckForMoveTrap(struct char_data *ch, int dir);
int CheckForInsideTrap(struct char_data *ch, struct obj_data *i);
int CheckForAnyTrap(struct char_data *ch, struct obj_data *i);
int CheckForGetTrap(struct char_data *ch, struct obj_data *i);
int TriggerTrap( struct char_data *ch, struct obj_data *i);
void FindTrapDamage( struct char_data *v, struct obj_data *i);
void TrapDamage(struct char_data *v, int damtype, int amnt, struct obj_data *t);
void TrapDam(struct char_data *v, int damtype, int amnt, struct obj_data *t);
void TrapTeleport(struct char_data *v);
void TrapSleep(struct char_data *v);
void InformMess( struct char_data *v);


/* File: magicutils.c */

void SwitchStuff( struct char_data *giver, struct char_data *taker);
void FailCharm(struct char_data *victim, struct char_data *ch);
void FailSleep(struct char_data *victim, struct char_data *ch);
void FailPara(struct char_data *victim, struct char_data *ch);
void FailCalm(struct char_data *victim, struct char_data *ch);


/* File: multiclass.c */

int BitCount(unsigned long n);


/* File: hash.c */

void init_hash_table(struct hash_header	*ht, int rec_size, int table_size);
void destroy_hash_table(struct hash_header *ht, void (*gman)());
void *hash_find(struct hash_header *ht, int key);
int hash_enter(struct hash_header *ht, int key, void *data);
void *hash_find_or_create(struct hash_header *ht, int key);
void *hash_remove(struct hash_header *ht, int key);
void hash_iterate(struct hash_header *ht, void (*func)(), void *cdata);


/* File: sound.c */

int RecGetObjRoom(struct obj_data *obj);
void MakeNoise(int room, char *local_snd, char *distant_snd);
void MakeSound(int pulse);


/* File: spec_rooms.c */

int dump(struct char_data *ch, int cmd, char *arg);
int pet_shops(struct char_data *ch, int cmd, char *arg);
int compare_cost(const void *p1, const void *p2);
void update_warehouse(struct char_data *ch);
int bank (struct char_data *ch, int cmd, char *arg);
int pray_for_items(struct char_data *ch, int cmd, char *arg);
int hospital(struct char_data *ch, int cmd, char *arg);
int bounce(struct char_data *ch,int cmd, char *arg);


/* File: player.save.c */

int SendMail(char *name, char *message);
void WriteAffect(int des,struct affected_type *af);
void WriteObject(int des,struct obj_data *i);
void SaveObjectList(int des,struct obj_data *list,int *count);
void save_char(struct char_data *ch,int load_room);
void ReadAffect(struct affected_type *af);
void ReadObject(struct obj_data *i);
void ReadObjList(struct obj_data *list, struct obj_data *parent,struct char_data *ch);
void ReadAffectBE(struct affected_type *af);
void ReadObjectBE(struct obj_data *i);
void ReadObjListBE(struct obj_data *list, struct obj_data *parent, struct char_data *ch);
int new_load_char_big_endian(struct char_data *ch, char *name, unsigned char * buf);
int new_load_char(struct char_data *ch, char *name);


/* File: online.c */

void confirm_exit(struct char_data *ch);
void do_edit(struct char_data *ch, char *arg, int cmd);
void do_oedit(struct char_data *ch, char *arg, int cmd);
void do_osave(struct char_data *ch, char *arg, int cmd);
char *one_number(char *arg,slong *tmp);
void read_one_object(FILE *obj_f,struct obj_data *obj);
void do_oload(struct char_data *ch, char *arg, int cmd);
void do_ocreate(struct char_data *ch, char *arg, int cmd);
void do_form(struct char_data *ch,char *arg, int cmd);
void do_olook(struct char_data *ch, char *argument, int cmd);
void do_medit(struct char_data *ch, char *arg, int cmd);
void do_mcreate(struct char_data *ch, char *arg, int cmd);
void do_msave(struct char_data *ch, char *arg, int cmd);
void do_mat(struct char_data *ch,char *arg, int cmd);
void set_oedesc(struct char_data *ch, struct obj_data *obj, char *keywds);
void set_otype(struct char_data *ch, struct obj_data *obj, char *arg);
void set_oaffect(struct char_data *ch, struct obj_data *obj, char *arg, int a);
void do_oset(struct char_data *ch, char *argument, int cmd);


/* File: act.new.c */

void queue_point(int x,int y);
void grid_expand(int x, int y,int max_x,int max_y);
void do_map(struct char_data *ch, char *arg, int cmd);
void do_priority(struct char_data *ch, char *arg, int cmd);
void do_away(struct char_data *ch, char *arg, int cmd);
void do_enable(struct char_data *ch, char *arg, int cmd);
void do_disable(struct char_data *ch, char *arg, int cmd);
void do_arena(struct char_data *ch, char *arg, int cmd);
void do_puke(struct char_data *ch, char *argument, int cmd);
void do_sing(struct char_data *ch, char *argument, int cmd);
void do_slice(struct char_data *ch, char *argument, int cmd);
char *Nice_Name(struct char_data *seer, struct char_data *ch);
int in_group_and_room(struct char_data *ch1,struct char_data *ch2);
int num_in_group_and_room(struct char_data *ch);
void do_split(struct char_data *ch, char *arg, int cmd);
void do_scan(struct char_data *ch, char *argument, int cmd);
void do_throw(struct char_data *ch, char *argument, int cmd);
void do_turn(struct char_data *ch, char *argument, int cmd);
void do_aggr(struct char_data *ch, char *arg, int cmd);
void do_gsay(struct char_data *ch, char *arg, int cmd);
void do_consent(struct char_data *ch, char *argument, int cmd);
void do_report(struct char_data *ch, char *argument, int cmd);
void do_gamble(struct char_data *ch, char *argument, int cmd);
void do_polish(struct char_data *ch, char *argument, int cmd);
void do_dodge(struct char_data *ch, char *argument, int cmd);
void do_hitall(struct char_data *ch, char *arg, int cmd);
void do_berserk(struct char_data *ch, char *arg, int cmd);
void do_nokill(struct char_data *ch, char *arg, int cmd);
void do_aid(struct char_data *ch, char *arg, int cmd);
void do_subterfuge(struct char_data *ch, char *arg, int cmd);
void berserk_setRecover(struct char_data *ch, int how_long);
void berserk_setBerserk(struct char_data *ch, int type, int how_long);
void do_save_player(struct char_data *ch);
void do_appraise(struct char_data *ch, char *argument, int cmd);
void identify_new(struct char_data *ch, struct obj_data *object, int percent);


/* File: oedit.c */

int my_search(char *arg,char **list);
int range_check_list(struct char_data *ch,int n, char **list);
void bit_vector(int *bitv,struct char_data *ch,char *arg, char **list);
void bit_vectorl(unsigned long int *bitv, struct char_data *ch, char *arg, char **list);
void print_flags(struct char_data *ch,int bitv,char **list);
void list_list(struct char_data *ch,char **list);
int generic_affect(struct char_data *ch,char *arg,int which);
void oedit_prompt(struct char_data *ch,char *arg);


/* File: spec_obj.c */

int chalice(struct char_data *ch, int cmd, char *arg);
int kings_hall(struct char_data *ch, int cmd, char *arg);
int portal(struct char_data *ch, int cmd, char *arg);
int scraps(struct char_data *ch, int cmd, char *arg, struct obj_data *obj, int type);


/* File: nlog.c */

void logl(int level,char *dl,char *str);
void open_player_log();
void log_action(struct char_data *ch, char *arg);
void close_player_log();


/* File: medit.c */

void medit_prompt(struct char_data *ch,char *arg);


/* File: ticks.c */

void do_ticks(struct char_data *ch, char *arg, int cmd);
void tick_usage(struct char_data *ch, int flag);
void tick_reset(void);
void tick_show(struct char_data *ch);
void do_timer(struct char_data *ch, char *arg, int cmd);
void timer_usage(struct char_data *ch);
void timer_reset(void);
void timer_show(struct char_data *ch);


/* File: act.builder.c */

void add_area(char *name, int offset);
void boot_areas();
void print_bitv(FILE *f,char **bits,ulong bitv);
char *room_number(int base,int room);
void write_string(FILE *f,char *s);
void write_string1(FILE *f, char *s);
void write_room(FILE *f,int base,int room);
void do_viewlog(struct char_data *ch, char *arg, int cmd);
void do_instazone(struct char_data *ch, char *argument, int cmdnum);
void do_rload(struct char_data *ch, char *arg, int cmd);
void do_rsave(struct char_data *ch, char *arg, int cmd);
void save_world(struct char_data *ch,char *arg,int cmd);
void do_at(struct char_data *ch, char *argument, int cmd);
void do_goto(struct char_data *ch, char *argument, int cmd);
void do_purge(struct char_data *ch, char *argument, int cmd);
void do_zone(struct char_data *ch, char *argument, int cmd);
void do_show(struct char_data *ch, char *argument, int cmd);
void do_offset(struct char_data *ch, char *arg, int cmd);
void do_foreach(struct char_data *ch,char *arg, int cmd);
void bin_write_room(FILE *fp,int virtual);
void do_oldload(struct char_data *ch, char *arg, int cmd);


/* File: memory.c */

char *mallocx(int size,char *file);
void freex(void *p,char *file);
char *reallocx(void *old,int new_size,char *file);
void malloc_dump(int fd);


/* File: debug.c */

int is_alpha(char *s);
int valid_player(struct char_data *ch);
int valid_object(struct obj_data *obj);
void integrity_check();


/* File: act.look.c */

void do_look(struct char_data *ch, char *argument, int cmd);
void do_read(struct char_data *ch, char *argument, int cmd);
int hash_string(char *s);
void do_examine(struct char_data *ch, char *argument, int cmd);
void do_inventory(struct char_data *ch, char *argument, int cmd);
void do_equipment(struct char_data *ch, char *argument, int cmd);


/* File: stat.c */

void stat_room(struct char_data *ch,int room);
void stat_char(struct char_data *ch,struct char_data *k);
void stat_obj(struct char_data*ch,struct obj_data *j);
void do_stat(struct char_data *ch, char *argument, int cmd);


/* File: skills1.c */

void do_disarm(struct char_data *ch, char *argument, int cmd);
void do_track(struct char_data *ch, char *argument, int cmd);
int track( struct char_data *ch, struct char_data *vict);
int dir_track( struct char_data *ch, struct char_data *vict);
void slam_into_wall( struct char_data *ch, struct room_direction_data *exitp);
void do_doorbash( struct char_data *ch, char *arg, int cmd);
void do_retreat(struct char_data *ch, char *arg, int cmd);


/* File: magic3.c */

void spell_protect_elem_fire(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_protect_elem_cold(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void spell_beacon(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);
void delayed_teleport(struct char_data *ch, int data);
void spell_delayed_teleport(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level);


/* File: maze.c */

void create_maze(int room,int h,int v);


/* File: termcap.c */

void reload_terms();
void init_terms();

#endif
