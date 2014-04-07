PF = $&
#CFLAGS = -g -O2 -DBSD -Ih -DUSE_MALLOP -fno-builtin
#Windows
CFLAGS = -g -O2 -Ih -pipe -fno-builtin 
LIBS = -lcrypt
LD=i686-pc-cygwin-gcc.exe
CC=i686-pc-cygwin-gcc.exe
EXT=.exe
#Unix
#CFLAGS = -g -O2 -DBSD -Ih -pipe -fno-builtin
#EXE = dmserver
#LIBS = -lcrypt
#LD=gcc
#CC=gcc
#EXT=

EXE = dmserver$(EXT)
O=
##
OBJS =  $Odb.o $Oskills.o $Ooedit.o $Omedit.o $Oonline.o\
	$Ointerpreter.o $Ocomm.o $Ospec_mob.o\
	$Oplayer.save.o $Oact.new.o $Oact.builder.o\
	$Oguild.o $Ospec_obj.o $Oevents.o\
	$Oact.inf1.o $Oact.move.o $Oact.obj1.o\
 	$Oact.obj2.o $Oact.offensive.o $Oact.other.o\
	$Oact.social.o	$Oact.wizard.o	$Ohandler.o\
	$Otermcap.o $Outility.o $Ospec_assign.o\
	$Oshop.o $Olimits.o $Omobact.o\
	$Ofight.o $Omodify.o	$Oweather.o\
	$Ospells1.o $Ospells2.o\
	$Oreception.o $Oconstants.o $Oact.comm.o\
	$Osignals.o $Oboard.o	$Omagic.o\
	$Omagic2.o $Oopinion.o $Omagic3.o\
	$Otrap.o $Omagicutils.o $Omulticlass.o\
	$Ohash.o $Osound.o $Ospec_rooms.o\
	$Oticks.o $Onlog.o $Odebug.o\
	$Oact.look.o	$Ostat.o $Oskills1.o $Omaze.o $Ospell_parser.o
 
#memory.o

SRCS =  interpreter.c comm.c act.comm.c act.inf1.c act.inf1.new.c\
	act.move.c act.obj1.c act.obj2.c guild.c events.c\
	act.offensive.c act.other.c act.social.c act.wizard.c handler.c \
	db.c utility.c spec_assign.c shop.c limits.c mobact.c \
	fight.c modify.c weather.c spells1.c spells2.c spell_parser.c \
	reception.c constants.c spec_mob.c signals.c board.c magic.c \
	magic2.c skills.c opinion.c trap.c magicutils.c multiclass.c hash.c \
	sound.c spec_rooms.c player.save.c online.c act.new.c\
	oedit.c spec_obj.c nlog.c medit.c ticks.c act.builder.c\
	memory.c debug.c act.look.c stat.c skills1.c magic3.c utility.new.c\
	maze.c termcap.c
TRAN_SRCS = tran.c tran_util.c

#$O/%.o: %.c
#	$(CC) $(CFLAGS) -o $@ -c $<
.c.o:
	$(CC) $(CFLAGS) -c $<

all : $(EXE)

$(EXE) : $(PF)  ${OBJS}
	$(LD) $(CFLAGS) -o $(EXE) ${OBJS} $(LIBS)

$(MEXE) : $(PF)  ${MOBJS}
	$(LD) $(CFLAGS) -o $(MEXE) ${MOBJS} $(MLIBS)

UTL_OBJS = player.save.o utl1.o constants.o
GEN_OBJS = player.save.o utl1.o genlist.o constants.o
PUR_OBJS = player.save.o utl1.o purge.o constants.o
CON_OBJS = player.save.o utl1.o convert.o constants.o db1.o hash.o
TRAN_OBJS = tran.o tran_util.o

tran: $(PF) $(TRAN_OBJS) constants.o
	$(LD) $(CFLAGS) -o tran $(TRAN_OBJS) constants.o

list: $(PF) $(UTL_OBJS) list.o
	$(LD) $(CFLAGS) -o list $(UTL_OBJS) list.o

purge: $(PF) $(PUR_OBJS)
	$(LD) $(CFLAGS) -o purge $(PUR_OBJS)

convert: $(PF) $(CON_OBJS)
	$(LD) $(CFLAGS) -o convert $(CON_OBJS)

genlist: $(PF) $(GEN_OBJS)
	$(LD) $(CFLAGS) -o genlist $(GEN_OBJS) -lmalloc

depend :
	makedepend -f makefile -Yh ${SRCS} ${TRAN_SRCS} 2> /dev/null
	

fcns: proto
	./proto$(EXT) $(SRCS)

proto: strip.c
	$(CC) -O -o proto$(EXT) strip.c

clean:
	-rm -f $(OBJS)
	-rm -f $(TRAN_OBJS)
	-rm $(EXE)
	-rm proto$(EXT)
	-rm *.exe
	-rm *.o



# DO NOT DELETE THIS LINE -- make depend depends on it.

interpreter.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h
interpreter.o: h/comm.h h/interpreter.h h/db.h h/utils.h h/limits.h h/inter.h
interpreter.o: h/race.h h/rooms.h h/ticks.h h/externs.h h/spells.h
comm.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
comm.o: h/comm.h h/handler.h h/db.h h/ticks.h h/externs.h h/spells.h
comm.o: h/interpreter.h
act.comm.o: h/structs.h h/compat.h h/utils.h h/comm.h h/interpreter.h
act.comm.o: h/handler.h h/db.h h/spells.h h/fcns.h h/hash.h h/misc_proto.h
act.comm.o: h/externs.h
act.inf1.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
act.inf1.o: h/comm.h h/interpreter.h h/handler.h h/db.h h/spells.h h/limits.h
act.inf1.o: h/race.h h/trap.h h/externs.h
act.move.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
act.move.o: h/comm.h h/interpreter.h h/handler.h h/db.h h/spells.h h/trap.h
act.move.o: h/ticks.h h/externs.h
act.obj1.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
act.obj1.o: h/comm.h h/interpreter.h h/handler.h h/db.h h/spells.h h/trap.h
act.obj1.o: h/externs.h
act.obj2.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
act.obj2.o: h/comm.h h/interpreter.h h/handler.h h/db.h h/spells.h
act.obj2.o: h/externs.h
guild.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
guild.o: h/comm.h h/interpreter.h h/handler.h h/db.h h/spells.h h/limits.h
guild.o: h/externs.h
events.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
events.o: h/externs.h h/spells.h h/interpreter.h
act.offensive.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h
act.offensive.o: h/externs.h h/spells.h h/interpreter.h h/utils.h h/comm.h
act.offensive.o: h/handler.h h/db.h h/limits.h h/race.h h/ticks.h
act.other.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h
act.other.o: h/limits.h h/utils.h h/comm.h h/interpreter.h h/handler.h h/db.h
act.other.o: h/spells.h h/externs.h h/ticks.h
act.social.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h
act.social.o: h/utils.h h/comm.h h/interpreter.h h/handler.h h/db.h
act.social.o: h/spells.h h/externs.h
act.wizard.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h
act.wizard.o: h/utils.h h/comm.h h/interpreter.h h/handler.h h/db.h
act.wizard.o: h/spells.h h/limits.h h/externs.h
handler.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
handler.o: h/comm.h h/db.h h/handler.h h/interpreter.h h/spells.h h/externs.h
db.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/ticks.h
db.o: h/externs.h h/spells.h h/interpreter.h h/utils.h h/db.h h/comm.h
db.o: h/handler.h h/limits.h h/race.h h/rooms.h
utility.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
utility.o: h/spells.h h/race.h h/db.h h/comm.h h/ticks.h h/externs.h
utility.o: h/interpreter.h
spec_assign.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h
spec_assign.o: h/externs.h h/spells.h h/interpreter.h h/db.h h/rooms.h
spec_assign.o: h/utils.h
shop.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/comm.h
shop.o: h/handler.h h/db.h h/interpreter.h h/utils.h h/externs.h h/spells.h
limits.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/limits.h
limits.o: h/utils.h h/spells.h h/comm.h h/race.h h/ticks.h h/externs.h
limits.o: h/interpreter.h
mobact.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/externs.h
mobact.o: h/spells.h h/interpreter.h h/utils.h h/handler.h h/db.h h/comm.h
mobact.o: h/trap.h h/ticks.h
fight.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/limits.h
fight.o: h/utils.h h/comm.h h/handler.h h/interpreter.h h/db.h h/spells.h
fight.o: h/race.h h/ticks.h h/externs.h
modify.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
modify.o: h/interpreter.h h/handler.h h/db.h h/comm.h h/externs.h h/spells.h
weather.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
weather.o: h/comm.h h/handler.h h/interpreter.h h/db.h h/externs.h h/spells.h
spells1.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
spells1.o: h/comm.h h/db.h h/interpreter.h h/spells.h h/handler.h h/externs.h
spells2.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
spells2.o: h/comm.h h/db.h h/interpreter.h h/spells.h h/handler.h h/poly.h
spells2.o: h/externs.h
spell_parser.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h
spell_parser.o: h/utils.h h/comm.h h/db.h h/interpreter.h h/spells.h
spell_parser.o: h/handler.h h/ticks.h h/externs.h h/spell_func.h
reception.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/comm.h
reception.o: h/handler.h h/db.h h/interpreter.h h/utils.h h/spells.h
reception.o: h/externs.h
constants.o: h/structs.h h/compat.h h/limits.h h/trap.h h/spell_list.h
constants.o: h/race.h
spec_mob.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h
spec_mob.o: h/externs.h h/spells.h h/interpreter.h h/utils.h h/comm.h
spec_mob.o: h/handler.h h/db.h h/limits.h h/rooms.h
signals.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
signals.o: h/externs.h h/spells.h h/interpreter.h h/ticks.h
board.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
board.o: h/comm.h h/db.h h/externs.h h/spells.h h/interpreter.h
magic.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/rooms.h
magic.o: h/utils.h h/comm.h h/spells.h h/handler.h h/limits.h h/db.h
magic.o: h/externs.h h/interpreter.h h/race.h h/ticks.h
magic2.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/externs.h
magic2.o: h/spells.h h/interpreter.h h/utils.h h/comm.h h/handler.h
magic2.o: h/limits.h h/db.h h/race.h
skills.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
skills.o: h/externs.h h/spells.h h/interpreter.h
opinion.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h
opinion.o: h/externs.h h/spells.h h/interpreter.h h/utils.h h/race.h h/db.h
trap.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
trap.o: h/spells.h h/trap.h h/comm.h h/externs.h h/interpreter.h
magicutils.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h
magicutils.o: h/utils.h h/comm.h h/spells.h h/handler.h h/limits.h
magicutils.o: h/externs.h h/interpreter.h
multiclass.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h
multiclass.o: h/externs.h h/spells.h h/interpreter.h h/utils.h h/comm.h
multiclass.o: h/handler.h h/limits.h h/race.h
hash.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
sound.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/externs.h
sound.o: h/spells.h h/interpreter.h h/utils.h h/comm.h h/handler.h h/db.h
sound.o: h/trap.h
spec_rooms.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h
spec_rooms.o: h/externs.h h/spells.h h/interpreter.h h/utils.h h/comm.h
spec_rooms.o: h/handler.h h/db.h h/limits.h h/rooms.h
player.save.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h
player.save.o: h/externs.h h/spells.h h/interpreter.h h/utils.h h/db.h
online.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
online.o: h/interpreter.h h/handler.h h/db.h h/comm.h h/externs.h h/spells.h
online.o: h/trap.h h/limits.h
act.new.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
act.new.o: h/comm.h h/interpreter.h h/handler.h h/db.h h/spells.h h/limits.h
act.new.o: h/race.h h/ticks.h h/externs.h
oedit.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/ticks.h
oedit.o: h/utils.h h/oedit.h
spec_obj.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h
spec_obj.o: h/externs.h h/spells.h h/interpreter.h h/utils.h h/comm.h
spec_obj.o: h/handler.h h/db.h h/limits.h h/rooms.h
nlog.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/externs.h
nlog.o: h/spells.h h/interpreter.h h/utils.h
medit.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/ticks.h
medit.o: h/utils.h h/medit.h
ticks.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/externs.h
ticks.o: h/spells.h h/interpreter.h h/ticks.h
act.builder.o: h/structs.h h/compat.h h/utils.h h/comm.h h/interpreter.h
act.builder.o: h/handler.h h/db.h h/spells.h h/limits.h h/fcns.h h/hash.h
act.builder.o: h/misc_proto.h h/ticks.h h/externs.h h/write.h
debug.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/externs.h
debug.o: h/spells.h h/interpreter.h h/utils.h
act.look.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
act.look.o: h/comm.h h/interpreter.h h/handler.h h/db.h h/spells.h h/limits.h
act.look.o: h/race.h h/trap.h h/externs.h
stat.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
stat.o: h/comm.h h/interpreter.h h/handler.h h/db.h h/spells.h h/limits.h
stat.o: h/externs.h
skills1.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
skills1.o: h/race.h h/spells.h h/comm.h h/handler.h h/externs.h
skills1.o: h/interpreter.h h/ticks.h
magic3.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/rooms.h
magic3.o: h/utils.h h/comm.h h/spells.h h/handler.h h/limits.h h/db.h
magic3.o: h/race.h h/ticks.h
maze.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
termcap.o: h/fcns.h h/structs.h h/compat.h h/hash.h h/misc_proto.h h/utils.h
tran.o: h/structs.h h/compat.h h/utils.h h/write.h
tran_util.o: h/structs.h h/compat.h h/utils.h
