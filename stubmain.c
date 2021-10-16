#include <metal/metal.h>
#include <metal/stdbool.h>
#include <metal/stddef.h>
#include <metal/stdint.h>
#include <metal/stdlib.h>
#include <metal/string.h>

#include "zowetypes.h"
#include "alloc.h"
#include "utils.h"

/*
   xlc -qmetal -q64 -DHAVE_PRINTF=1 -DSUBPOOL=132 -DMETTLE=1 "-Wc,longname,langlvl(extc99),goff,ASM,asmlib('CEE.SCEEMAC','SYS1.MACLIB','SYS1.MODGEN')" -I ${COMMON}/h -I ${ZSS}/h -I ${INZPECT}/h -S stubmain.c ${INZPECT}/printf.c

  as -mgoff -mobject -mflag=nocont --TERM --RENT -aegimrsx=stubmain.asm stubmain.s
  as -mgoff -mobject -mflag=nocont --TERM --RENT -aegimrsx=printf.asm printf.s 
  as -mgoff -mobject -mflag=nocont --TERM --RENT -aegimrsx=stubtest.asm stubtest.s

  ld -b ac=1 -b rent -b case=mixed -b map -b xref -b reus -e main -o stubmain stubmain.o stubtest.o printf.o

 */

extern int stubLongname(void);

int main(int argc, char **argv){
  printf("Hello Stubtest\n");
  printf("survey says 0x%x\n",stubLongname());
  return 0;
}
