CC      = g++
#PROF    = -p

#Uncomment to compile in Cygwin
#CYGWIN = -DCYGWIN

#Uncomment the line below if you are getting undefined references to dlsym, dlopen, and dlclose.
#Comment it out if you get errors about ldl not being found.
NEED_DL = -ldl

#Some systems need this for dynamic linking to work.
EXPORT_SYMBOLS = -export-dynamic

# Uncomment the two lines below if compiling on a Solaris box
#SOLARIS_FLAG = -Dsun -DSYSV
#SOLARIS_LINK = -lnsl -lsocket

#Memory debugging flags. Cygwin lacks libasan support required for this to work.
ifndef CYGWIN
M_FLAGS = -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer
endif

W_FLAGS = -std=c++1z -Wall -Wshadow -Wformat-security -Wpointer-arith -Wcast-align -Wredundant-decls

# Optional libraries - auto-detect
HIREDIS_EXISTS := $(shell pkg-config --exists hiredis && echo yes || echo no)
CURL_EXISTS := $(shell pkg-config --exists libcurl && echo yes || echo no)

ifeq ($(HIREDIS_EXISTS),yes)
    REDIS_FLAGS = -DHAVE_HIREDIS
    REDIS_LIBS = -lhiredis
else
    $(warning Warning: hiredis not found - Redis support disabled. Install with: apt-get install libhiredis-dev)
    REDIS_FLAGS =
    REDIS_LIBS =
endif

ifeq ($(CURL_EXISTS),yes)
    CURL_FLAGS = -DHAVE_CURL
    CURL_LIBS = -lcurl
else
    $(warning Warning: libcurl not found - Ollama AI support disabled. Install with: apt-get install libcurl4-openssl-dev)
    CURL_FLAGS =
    CURL_LIBS =
endif

C_FLAGS = -g2 $(W_FLAGS) $(M_FLAGS) $(SOLARIS_FLAG) $(PROF) $(EXPORT_SYMBOLS) $(REDIS_FLAGS) $(CURL_FLAGS)
L_FLAGS = $(M_FLAGS) $(PROF) $(SOLARIS_LINK) -lz $(NEED_DL) $(REDIS_LIBS) $(CURL_LIBS)

#D_FLAGS : For the DNS Slave process. No need in linking all the extra libs for this.
D_FLAGS = -g2 -O $(M_FLAGS) $(PROF) $(SOLARIS_LINK)

C_FILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c ban.c boards.c \
          build.c calendar.c chess.c clans.c color.c comm.c comments.c const.c db.c deity.c \
          dns.c fight.c handler.c hashstr.c hint.c hotboot.c house.c imm_host.c interp.c \
          liquids.c magic.c makeobjs.c mapout.c mapper.c mccp.c \
          misc.c mpxset.c mssp.c mud_comm.c mud_prog.c news.c planes.c player.c polymorph.c \
          renumber.c reset.c save.c services.c sha256.c shops.c skills.c special.c tables.c \
          track.c update.c variables.c weather.c \
          redis_bridge.c world_history_tracker.c world_context.c ollama_integration.c \
          mob_home.c universal_mob_ai.c ai_context_analyzer.c economy_stub.c \
          beeler.c beeler_architect.c beeler_assign.c beeler_commands.c beeler_god_mode.c \
          mob_identity_stubs.c \
          leader_ai.c organic_creation.c periodicos.c player_world_impact.c \
          book_writing_system.c \
          # mob_creation_system.c  # TODO: Fix sed-mangled code \
          # persistent_memory.c cultural_evolution.c family_lineage.c global_trade.c resource_distribution.c  # TODO: Fix const char* issues

O_FILES := $(patsubst %.c,o/%.o,$(C_FILES))

H_FILES = $(wildcard *.h)

all:
	$(info    Using C-Flags: $(C_FLAGS))
	$(info    Using L-Flags: $(L_FLAGS))
	$(MAKE) -s smaug
	$(MAKE) -s dns

# pull in dependency info for *existing* .o files
-include dependencies.d

ifdef CYGWIN
smaug: $(O_FILES)
	rm -f smaug.exe
	dlltool --export-all --output-def smaug.def $(O_FILES)
	dlltool --dllname smaug.exe --output-exp smaug.exp --def smaug.def
	$(CC) -o smaug.exe $(O_FILES) smaug.exp $(L_FLAGS)
	@echo "Generating dependency file ...";
	@$(CC) -MM $(C_FLAGS) $(C_FILES) > dependencies.d
	@perl -pi -e 's.^([a-z]).o/$$1.g' dependencies.d
	@echo "Done compiling mud.";
	chmod g+w smaug.exe
	chmod a+x smaug.exe
	chmod g+w $(O_FILES)

clean:
	@rm -f o/*.o smaug.exe dependencies.d resolver.exe resolver.o *~

else
smaug: $(O_FILES)
	rm -f smaug
	$(CC) -export-dynamic -o smaug $(O_FILES) $(L_FLAGS)
	@echo "Generating dependency file ...";
	@$(CC) -MM $(C_FLAGS) $(C_FILES) > dependencies.d
	@perl -pi -e 's.^([a-z]).o/$$1.g' dependencies.d
	@echo "Done compiling mud.";
	chmod g+w smaug
	chmod a+x smaug
	chmod g+w $(O_FILES)

clean:
	@rm -f o/*.o smaug dependencies.d resolver resolver.o *~
endif

dns: resolver.o
	rm -f resolver
	$(CC) $(D_FLAGS) -o resolver resolver.o
	@echo "Done compiling DNS resolver.";
	chmod g+w resolver
	chmod a+x resolver
	chmod g+w resolver.o

indent:
	indent -ts3 -nut -nsaf -nsai -nsaw -npcs -npsl -ncs -nbc -bls -prs -bap -cbi0 -cli3 -bli0 -l125 -lp -i3 -cdb -c1 -cd1 -sc -pmt $(C_FILES)
	indent -ts3 -nut -nsaf -nsai -nsaw -npcs -npsl -ncs -nbc -bls -prs -bap -cbi0 -cli3 -bli0 -l125 -lp -i3 -cdb -c1 -cd1 -sc -pmt $(H_FILES)

indentclean:
	rm *.c~ *.h~

o/%.o: %.c
	echo "  Compiling $@...";
	$(CC) -c $(C_FLAGS) $< -o $@
