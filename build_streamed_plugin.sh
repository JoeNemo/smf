#! /bin/bash
AUTHSDK=/u/zossteam/jdevlin/authsdk
ZOSSLIB=/u/zossteam/jdevlin/zosslib
OCODE=/u/zossteam/jdevlin/ocode
COMMON=/u/zossteam/jdevlin/repos/zss/deps/zowe-common-c
ZSS=/u/zossteam/jdevlin/repos/zss

CFLAGS=(-S -M -qmetal -q64 -DSUBPOOL=132 -DMETTLE=1 -DMSGPREFIX='"IDX"'
-DHAVE_METALIO=1
-qreserved_reg=r12
-Wc,"arch(8),agg,exp,list(),so(),off,xref,roconst,longname,lp64" 
-I $COMMON/h -I $ZSS/h -I $ZOSSLIB -I $AUTHSDK -I $OCODE -I .)

ASFLAGS=(-mgoff -mobject -mflag=nocont --TERM --RENT)

LDFLAGS=(-V -b ac=1 -b rent -b case=mixed -b map -b xref -b reus)

xlc "${CFLAGS[@]}" -DCMS_CLIENT \
$COMMON/c/alloc.c \
$COMMON/c/cmutils.c \
$COMMON/c/collections.c \
$COMMON/c/crossmemory.c \
$COMMON/c/logging.c \
$COMMON/c/metalio.c \
$COMMON/c/qsam.c \
$COMMON/c/timeutls.c \
$COMMON/c/utils.c \
$COMMON/c/xlate.c \
$COMMON/c/le.c \
$COMMON/c/logging.c \
$COMMON/c/recovery.c \
$COMMON/c/scheduling.c \
$COMMON/c/pause-element.c \
$COMMON/c/zosfile.c \
$COMMON/c/zos.c \
$COMMON/c/zvt.c \
$ZSS/c/zis/plugin.c \
$ZSS/c/zis/service.c \
$ZOSSLIB/zosserror.c \
$ZOSSLIB/zossutil.c \
streamedmain.c 

as "${ASFLAGS[@]}" -aegimrsx=alloc.asm alloc.s
as "${ASFLAGS[@]}" -aegimrsx=cmutils.asm cmutils.s
as "${ASFLAGS[@]}" -aegimrsx=collections.asm collections.s
as "${ASFLAGS[@]}" -aegimrsx=crossmemory.asm crossmemory.s
as "${ASFLAGS[@]}" -aegimrsx=logging.asm logging.s
as "${ASFLAGS[@]}" -aegimrsx=metalio.asm metalio.s
as "${ASFLAGS[@]}" -aegimrsx=qsam.asm qsam.s
as "${ASFLAGS[@]}" -aegimrsx=timeutls.asm timeutls.s
as "${ASFLAGS[@]}" -aegimrsx=utils.asm utils.s
as "${ASFLAGS[@]}" -aegimrsx=zos.asm zos.s
as "${ASFLAGS[@]}" -aegimrsx=zosfile.asm zosfile.s
as "${ASFLAGS[@]}" -aegimrsx=le.asm le.s
as "${ASFLAGS[@]}" -aegimrsx=recovery.asm recovery.s
as "${ASFLAGS[@]}" -aegimrsx=scheduling.asm scheduling.s
as "${ASFLAGS[@]}" -aegimrsx=pause-element.asm pause-element.s
as "${ASFLAGS[@]}" -aegimrsx=xlate.asm xlate.s
as "${ASFLAGS[@]}" -aegimrsx=zvt.asm zvt.s
as "${ASFLAGS[@]}" -aegimrsx=plugin.asm plugin.s
as "${ASFLAGS[@]}" -aegimrsx=service.asm service.s
as "${ASFLAGS[@]}" -aegimrsx=zossutil.asm zossutil.s
as "${ASFLAGS[@]}" -aegimrsx=streamedmain.asm streamedmain.s
as "${ASFLAGS[@]}" -aegimrsx=zisstubs.asm ${ZSS}/c/zis/zisstubs.s

export _LD_SYSLIB="//'SYS1.CSSLIB'://'CEE.SCEELKEX'://'CEE.SCEELKED'://'CEE.SCEERUN'://'CEE.SCEERUN2'://'CSF.SCSFMOD0'"

ld "${LDFLAGS[@]}" -e getPluginDescriptor \
-o "//'$USER.DEV.LOADLIB(INZPECT1)'" \
alloc.o \
cmutils.o \
collections.o \
crossmemory.o \
le.o \
logging.o \
metalio.o \
qsam.o \
pause-element.o \
recovery.o \
scheduling.o \
timeutls.o \
utils.o \
xlate.o \
zos.o \
zosfile.o \
zvt.o \
plugin.o \
service.o \
zossutil.o \
zisstubs.o \
streamedmain.o \
> INZPECT1.link


