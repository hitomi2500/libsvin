ifeq ($(strip $(YAUL_INSTALL_ROOT)),)
  $(error Undefined YAUL_INSTALL_ROOT (install root directory))
endif

include $(YAUL_INSTALL_ROOT)/share/pre.common.mk

SH_PROGRAM:= svindemo
SH_SRCS:= \
	mcufont/mf_encoding.c \
    mcufont/mf_font.c \
    mcufont/mf_justify.c \
    mcufont/mf_kerning.c \
    mcufont/mf_rlefont.c \
    mcufont/mf_bwfont.c \
    mcufont/mf_scaledfont.c \
    mcufont/mf_wordwrap.c \
	svindemo.c \
	svin_cd_access.c \
	svin_background.c \
	svin_filelist.c \
	svin_menu.c \
	svin_tapestry.c \
	svin_text.c \
	svin_textbox.c \
	svin_sprite.c \
	svin_script.c \
	svin_debug.c \
	svin_alloc.c \
	svin.c
	
SH_LIBRARIES:= tga
SH_CFLAGS+= -O2 -I. -Imcufont -save-temps

IP_VERSION:= V1.000
IP_RELEASE_DATE:= 20160101
IP_AREAS:= JTUBKAEL
IP_PERIPHERALS:= JAMKST
IP_TITLE:= libsvin demo
IP_MASTER_STACK_ADDR:= 0x060C4000
IP_SLAVE_STACK_ADDR:= 0x06001000
IP_1ST_READ_ADDR:= 0x06004000

M68K_PROGRAM:=
M68K_OBJECTS:=

include $(YAUL_INSTALL_ROOT)/share/post.common.mk
