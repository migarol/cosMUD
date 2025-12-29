CC      = gcc
#PROF    = -p
NOCRYPT =
 
# Please read all the documents in the system dir and README.1st in main dir.
# By default, this MUD will start on port 4100 and IMC on port 4125.
 
# Alter the directory below and mud.h to set the debugger
#DMALLOC = -L/home/ucmm2/rm4/src/dmalloc/lib -ldmalloclp -ldmalloc -ldl
 
# Uncomment the two lines below if compiling on a Solaris box
#SOLARIS_FLAG = -Dsun -DSYSV
#SOLARIS_LINK = -lnsl -lsocket
 
# Uncomment the line below if you are getting undefined crypt errors
NEED_CRYPT = -lcrypt
 
DBUGFLG = -DREQUESTS

# Code support for Intermud-3. Comment out to disable.
I3 = -DI3 -DI3SMAUG
 
# Uncomment the line below if you want a performance increase though beware
# your core files may not be as much of a benefit if you do.
#OPT_FLAG = -finline-functions -funroll-loops -fdefer-pop -fstrength-reduce
 
C_FLAGS = $(OPT_FLAG) -g3 -O -Wall $(I3) $(PROF) $(NOCRYPT) $(DBUGFLG) -DSMAUG $(SOLARIS_FLAG)
L_FLAGS = $(OPT_FLAG) $(PROF) $(SOLARIS_LINK) $(NEED_CRYPT) $(DMALLOC) -lcurl
 
O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o arena.o \
          ai_context_analyzer.o axsmaug.o ban.o bank.o base64.o beeler.o \
          beeler_architect.o beeler_assign.o beeler_commands.o beeler_god_mode.o boards.o \
          book_writing_system.o build.o clans.o comm.o \
          comments.o const.o cultural_evolution.o db.o deity.o economy.o economy_commands.o \
          family_lineage.o fight.o global_trade.o grub.o handler.o hashstr.o \
          hiscores.o hskelter.o i3.o ibuild.o ident.o imm_host.o immscore.o \
          interp.o leader_ai.o locker.o magic.o makeobjs.o medit.o medit_ai.o misc.o \
          mob_creation_system.o mob_home.o mob_identity.o motd.o \
          mpxset.o mud_comm.o mud_prog.o oedit.o \
          ollama_integration.o # persistent_memory.o \
          planes.o player.o # player_world_impact.o \
          polymorph.o redit.o requests.o reset.o # resource_distribution.o save.o services.o \
          shops.o skills.o special.o stat_obj.o suicide.o tables.o \
          track.o universal_mob_ai.o update.o websvr.o minilzo.o \
          world_context.o world_history_tracker.o periodicos.o organic_creation.o \
          ice.o icec.o icec-mercbase.o imc.o imc-config.o imc-events.o \
          imc-interp.o imc-mail.o imc-mercbase.o imc-util.o imc-version.o \

C_FILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c arena.c \
          ai_context_analyzer.c axsmaug.c ban.c bank.c base64.c beeler.c \
          beeler_architect.c beeler_assign.c beeler_commands.c beeler_god_mode.c boards.c \
          book_writing_system.c build.c clans.c comm.c \
          comments.c const.c cultural_evolution.c db.c deity.c economy.c economy_commands.c \
          family_lineage.c fight.c global_trade.c grub.c handler.c hashstr.c \
          hiscores.c hskelter.c i3.c ibuild.c ident.c imm_host.c immscore.c \
          interp.c leader_ai.c locker.c magic.c makeobjs.c medit.c medit_ai.c misc.c \
          mob_creation_system.c mob_home.c mob_identity.c motd.c \
          mpxset.c mud_comm.c mud_prog.c oedit.c \
          ollama_integration.c organic_creation.c periodicos.c persistent_memory.c \
          planes.c player.c player_world_impact.c \
          polymorph.c redit.c requests.c reset.c resource_distribution.c save.c services.c \
          shops.c skills.c special.c stat_obj.c suicide.c tables.c \
          track.c universal_mob_ai.c update.c websvr.c minilzo.c world_context.c \
          world_history_tracker.c \
          ice.c icec.c icec-mercbase.c imc.c imc-config.c imc-events.c \
          imc-interp.c imc-mail.c imc-mercbase.c imc-util.c imc-version.c \

H_FILES = ai_context_analyzer.h bank.h beeler.h beeler_architect.h beeler_god_mode.h bet.h \
          book_writing_system.h cultural_evolution.h economy.h family_lineage.h global_trade.h \
          leader_ai.h medit_ai.h mob_ai.h mob_creation_system.h mob_home.h mob_identity.h \
          mud.h olc.h ollama_integration.h organic_creation.h periodicos.h persistent_memory.h \
          player_world_impact.h resource_distribution.h \
          universal_mob_ai.h world_context.h world_history_tracker.h \
          ice.h icec.h icec-mercbase.h imc.h \
          imc-config.h imc-mercbase.h imc-mercdefs.h lzoconf.h minilzo.h \
 
all:
	make rmexe
 
rmexe: $(O_FILES)
	rm -f rmexe
	$(CC) -o rmexe $(O_FILES) $(L_FLAGS) $(DMALLOC)
	chmod g+w rmexe
	chmod a+x rmexe
	chmod g+w $(O_FILES)
 
.c.o: mud.h
	$(CC) -c $(C_FLAGS) $<
 
clean:
	rm -f *.o rmexe *~
