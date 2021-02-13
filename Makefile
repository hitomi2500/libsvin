ifeq ($(strip $(YAUL_INSTALL_ROOT)),)
  $(error Undefined YAUL_INSTALL_ROOT (install root directory))
endif

include $(YAUL_INSTALL_ROOT)/share/pre.common.mk

SH_PROGRAM:= svindemo
SH_OBJECTS:= \
	svindemo.o \
	cd-block_multiread.o \
	svin.o
	
SH_LIBRARIES:= tga
SH_CFLAGS+= -O2 -I. -I../shared/menu -save-temps

IP_VERSION:= V1.000
IP_RELEASE_DATE:= 20160101
IP_AREAS:= JTUBKAEL
IP_PERIPHERALS:= JAMKST
IP_TITLE:= libsvin demo
IP_MASTER_STACK_ADDR:= 0x06004000
IP_SLAVE_STACK_ADDR:= 0x06001000
IP_1ST_READ_ADDR:= 0x06004000

M68K_PROGRAM:=
M68K_OBJECTS:=

include $(YAUL_INSTALL_ROOT)/share/post.common.mk

bg.rom:
	@printf -- "$(V_BEGIN_YELLOW)$@$(V_END)\n"
	$(ECHO)$(YAUL_INSTALL_ROOT)/bin/genromfs -v -a 2048 -V "BG" -d ./bg/ -f $@
	$(ECHO)$(YAUL_INSTALL_ROOT)/bin/fsck.genromfs $@
	$(ECHO)cp $@ cd/$@

