#ifdef METTLE 
#include <metal/metal.h>
#include <metal/stddef.h>
#include <metal/stdio.h>
#include <metal/stdlib.h>
#include <metal/stdint.h>
#include <metal/string.h>
#include <metal/stdarg.h>  
#else
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>  
#include <pthread.h>
#include <time.h>
#define _LARGE_TIME_API 
#include <sys/ps.h>
#endif

#include "zowetypes.h"
#include "alloc.h"
#include "utils.h"
#include "zos.h"
#include "zvt.h"
#include "le.h"
#include "collections.h"
#include "recovery.h"

#include "shrmem64.h"
#include "crossmemory.h"
#include "cpool64.h"
#include "zis/server.h"
#include "zis/plugin.h"

#include "zossutil.h"
#include "probes.h"
#include "authservices.h"
#include "po.h"
#include "srbcontrol.h"
#include "streameddefs.h"

/* 
  LE 64 XPLINK Compile
  
  xlc -DHAVE_PRINTF=1 -DINZPECT_SIMPLE_DEBUG=1 -DDYNEXIT_TEST=1 -D_XOPEN_SOURCE=600 -D_OPEN_THREADS=1 -DSUBPOOL=132 "-Wc,LP64,longname,langlvl(extc99),gonum,goff,ASM,asmlib('CEE.SCEEMAC','SYS1.MACLIB','SYS1.MODGEN')" "-Wl,ac=1" -I ${COMMON}/h -I ${INZPECT} -I ${ZSS}/h -o dynexit dynexit.c ${INZPECT}/srbcontrol.c ${INZPECT}/inzpectdata.c ${INZPECT}/probes.c ${INZPECT}/intertypes.c ${INZPECT}/authservices.c ${INZPECT}/authservicesimpl.c ${INZPECT}/po.c ${ZOSSLIB}/zossutil.c ${ZOSSLIB}/zosserror.c ${COMMON}/c/zvt.c ${COMMON}/c/crossmemory.c ${COMMON}/c/cmutils.c ${COMMON}/c/cpool64.c ${COMMON}/c/shrmem64.c ${COMMON}/c/pause-element.c ${COMMON}/c/zosfile.c ${COMMON}/c/zos.c ${COMMON}/c/collections.c ${COMMON}/c/timeutls.c ${COMMON}/c/xlate.c ${COMMON}/c/scheduling.c ${COMMON}/c/recovery.c ${COMMON}/c/le.c ${COMMON}/c/logging.c ${COMMON}/c/utils.c ${COMMON}/c/alloc.c ${ZSS}/c/zis/client.c  

  

  Authorization:

    Must be APF, or supervisor

  CSVDYNEX

  https://www.ibm.com/docs/en/zos/2.1.0?topic=services-examples-csvdynex-macro 

  */

#pragma pack(packed)



#define DYNEX_DEFINE  0
#define DYNEX_ADD     1
#define DYNEX_MODIFY  2
#define DYNEX_DELETE  3
#define DYNEX_UNDEFINE 4
#define DYNEX_ATTRIB   5
#define DYNEX_LIST     6
#define DYNEX_CALL    7
#define DYNEX_RECOVER 8
#define DYNEX_PROCESSDP 9
#define DYNEX_ACTIVATE  10
#define DYNEX_QUERY     11
#define DYNEX_REPLACE   12

#define DYNEX_POS_SYSTEM 0
#define DYNEX_POS_LAST 1
#define DYNEX_POS_FIRST 2
#define DYNEX_POS_KEEPFIRST 3
#define DYNEX_POS_KEEPLAST  4
#define DYNEX_POS_CONDFIRST 5
#define DYNEX_POS_CONDLASST 6

#define DYNEX_FLAGS_CALLSTOPRC 0x80
#define DYNEX_FLAGS_RCFROM     0x40
#define DYNEX_FLAGS_KEEPRC     0x20
#define DYNEX_FLAGS_FASTPATH   0x10
#define DYNEX_FLAGS_REENTRANT  0x08
#define DYNEX_FLAGS_ERROR    0x04
#define DYNEX_FLAGS_ACTIVE   0x02
#define DYNEX_FLAGS_INACTIVE 0x01

#define DYNEX_FLAGS2_ONEMODULE 0x80
#define DYNEX_FLAGS2_FORCE 0x40
#define DYNEX_FLAGS2_PERSIST_ADDRESSSPACE 0x20
#define DYNEX_FLAGS2_PERSIST_IPL          0x10
#define DYNEX_FLAGS2_ANYKEY 0x08
#define DYNEX_FLAGS2_ABENDCONSEC 0x04
#define DYNEX_FLAGS2_NOLINKSTACKOK 0x02
#define DYNEX_FLAGS2_STOKEN 0x01

#define DYNEX_FLAGS3_ 0x80

#define DYNEX_FLAGS5_ 0x80

#define DYNEX_VER_0 0 
#define DYNEX_VER_1 1  /* etc, .... */

#define DYNEX_V0_LEN 0x70

/* length = 0x70 */
typedef struct CSVDYNEXParms_tag{
  char           version;
  char           requestCode; /* see DYNEX_xxxx above */
  char           flags;       /* 
                                         RCFROM,KEEPRC,CALLSTOPRC 
                                         FASTPATH=YES   ->      0x10
                                         REENTRANT=REQ  ->      0x08
                                         MESSAGE=ERROR ->       0x04
                                         STATE=ACTIVE   ->      0x02 
                                         STATE=INACTIVE ->      0x01 */
  char           amode;       /* 1 for 24 bit mode, 2 for 'DEFINED'  */
  int            key;  
  char           flags2;      /*         ONEMODULE=YES ->        0x80
                                         FORCE=YES               0x40
                                         PERSIST=ADDRESSSPACE -> 0x20 
                                         PERSIST=IPL ->          0x10 
                                         ANYKEY=YES  ->          0x08 
					 ABENDCONSEC=YES ->      0x04
					 */
  char           flags3;      /* xflags3 : PERSIST,LOCALmWILDARDSTAR */
  char           pos;         /* POS one of (SYSTEM=0,LAST=1,FIRST=2,KEEPFIRST=3,KEEPLAST=4,CONDFIRST=5,CONDLAST=6) */
  char           unknown11;   /* EXAAVER */
  char           reserved12[3];
  char           flags5;
  char           exitname[16];
  /* OFFSET 0x20 */
  char           modname[8];
  int            cmdinfo;
  int            abendnum;  /* also called addabendnum in macro, for macro-y reasons */
  /* OFFSET 0x30 */
  int            rcTo; /* wtf */
  int            rcFrom;
  int            keepRC;
  char           keepRCComp;
  char           rcCompare;
  char           flags4;
  char           exRetVer;
  /* OFFSET 0x40 */
  int            callStopRC;
  union{
    struct{
      Addr31         workareaAddr;
      Addr31         retareaAddr;
      int            retareaALET;
      /* OFFSET 0x50 */
      int            retLen;
      Addr31         rubAddr;
      int            rubAlet;
      char           nextToken[8];
      /* OFFSET 0x64 */
      Addr31         sdwaAddr;
      Addr31         precallWAAddr;
    };
    struct{
      Addr31         answerArea;
      int            answerAreaALET;
      int            answerLength;
    };
    struct{               /* seen in ADD */
      char           unknown44[0x10];
      /* offset 0x54 */
      Addr31         modaddr;
      uint64_t       param;   /* for exit */
    };
  };
} CSVDYNEXParms;

typedef struct DYNEXAnswerArea_tag{
  int    entriesReturned;
  int    entriesOmitted;  /* due to lack of space */
  int    answerSize;      /* in bytes */
  struct DYNEXExitEntry_tag *__ptr32 firstExitEntry;
} DYNEXAnswerArea;

/*
EXAAE    DSECT           EXAAE Record data format                      
.L0007   ANOP                                                          
EXAAENEXTADDR DS A       Address of next EXAAE. EXAAHNumREC must be    
                         used to determine how far along this chain to 
                         go.                                           
         ORG   EXAAENEXTADDR                                           
EXAAENEXT@ DS  A         Same as EXAAENEXTADDR                         
EXAAEFIRSTENTADDR DS A   Address of first EXAAM / EXAAM1 / EXAAM2 /    
                         EXAAM3 for this EXAAE                         
         ORG   EXAAEFIRSTENTADDR                                       
EXAAEFIRSTENT@ DS A      Same as EXAAEFIRSTENTADDR                     
EXAAENAME DS   CL16      Name of exit                                  
EXAAENUMENT DS H         Number of EXAAM/EXAAM1/EXAAM2/EXAAM3 entries  
                         associated with this exit                     
         ORG   EXAAENUMENT                                             
EXAAE#ENT DS   H         Same as EXAAENumENT                           
EXAAEAMODE DS  X         Amode: 0 = Amode 31, 1 = Amode 24, 2 = Amode  
                         Defined. Equates are provided below. They     
                         begin with EXAAEAMODE_                        
EXAAEKEY DS    X         Defined Key                                   
EXAAEFLAGS DS  B                                                       
*  Bit definitions:                                                    
EXAAEFASTPATHOK EQU X'80' Fast path acceptable for this                
EXAAEDEFINED EQU X'40'   Whether exit has been explicitly defined or   
                         simply has had modules added to it            
EXAAEREENTRANTREQUIRED EQU X'20' Reentrant was required for this exit  
EXAAEONEMODULEONLY EQU X'10' This exit is defined to allow only one    
                         module to be associated with it at a time.    
EXAAEABENDCONSEC EQU X'08' Whether or not the exit requested           
                         consecutive abends                            
EXAAEANYKEY EQU X'04'    Fast path exit supports any key               
EXAAEEXITTYPEINSTALLATION EQU X'02' This is an installation exit. It   
                         is possible that neither this nor the program 
                         exit bit is on.                               
EXAAEEXITTYPEPROGRAM EQU X'01' This is a program exit. It is possible  
                         that neither this nor the installation exit   
                         bit is on.                                    
EXAAEFLAGS1 DS B                                                       
*  Bit definitions:                                                    
EXAAELOADAPFYES EQU X'80' LOADAPF=YES was requested for this exit      
EXAAEPERSISTJOBSTEPTASK EQU X'40' PERSIST=JOBSTEPTASK was requested    
                         for this exit                                 
EXAAEPERSISTADDRESSSPACE EQU X'20' PERSIST=ADDRESSSPACE was requested  
                         for this exit                                 
EXAAEPERSISTIPL EQU X'10' PERSIST=IPL was requested for this exit      
EXAAEDISABLEDCALLOK EQU X'08' DISABLEDCALL=OK was requested for this   
                         exit                                          
EXAAEKEEPFIRST EQU X'04' Some exit routine is KeepFirst                
EXAAEKEEPLAST EQU X'02'  Some exit routine is KeepLast                 
         DS    CL2       Reserved                                      
EXAAEABENDNUM DS F       Number of abends allowed                      
EXAAEPRECALLROUTINEADDR DS A 050201                                    
EXAAEAMODE_31 EQU 0      Value for ExaaeAmode indicating AMODE 31      
EXAAEAMODE_24 EQU 1      Value for ExaaeAmode indicating AMODE 24      
 */

typedef struct DYNEXExitEntry_tag{
  struct DYNEXExitEntry_tag *__ptr32 nextExitEntry;
  struct DYNEXModuleEntry_tag *__ptr32 firstModuleEntry;
  char           name[16];  /* name of the exit */
  unsigned short numberOfModuleEntries;
  char           amode;
  char           key;
  char           flags;
  char           flags1;
  char           reserved[2];
  int            abendnum;  /* number of abends allowed */
  Addr31         precallRoutineAddress;
} DYNEXExitEntry;

typedef struct DYNEXModuleEntry_tag{
  Addr31 nextEntry;
  char   moduleName[8];
  char   flags;
#define DYNEX_ENTRYFLAGS_ACTIVE            0x80
#define DYNEX_ENTRYFLAGS_JOBNAME_PROVIDED  0x40
#define DYNEX_ENTRYFLAGS_STOKEN_PROVIDED   0x20
#define DYNEX_ENTRYFLAGS_ABEND_CONSECUTIVE 0x10 
  char   requestedPos;
  char   reserved[2];
  union{
    char   stoken[8];   /* if requested */
    char   jobname[8];  /* if requested */
  };
  Addr31 entryPoint;
  Addr31 loadPoint;
} DYNEXModuleEntry;

typedef struct IEFJMR_tag{
  char  jmrjob[8];  /* job name */
  int   jmrentry;   /* time in hundredths */
  int   jmredate;   /* 0CYYDDDF */
  int   jmrcpuid;   /* CPU - SID and MDF from SMCA */
  char  jmruseid[8];  /* user-defined common exit parameter area */
  char  jmrstep;
  char  jmrindc;     /* indicator switches */
  char  jmrflg;
  char  jmrclass;
  int   jmrucom;     /* user communication */
  /* and so on....., jeez */
} IEFJMR;

#pragma pack(reset)

static void showExits(DYNEXAnswerArea *area){
  printf("JOE3\n");
  fflush(stdout);
  DYNEXExitEntry *exitEntry = area->firstExitEntry;
  printf("showExits top 0x%p\n",exitEntry);

  while (exitEntry){
    printf("exit entry at 0x%p ---------------------\n",exitEntry);
    dumpbuffer((char*)exitEntry,sizeof(DYNEXExitEntry));
    /* here - show inner loop of modules per exit */
    DYNEXModuleEntry *moduleEntry = exitEntry->firstModuleEntry;
    for (int i=0; i<exitEntry->numberOfModuleEntries; i++){
      printf("    Module at 0x%p %8.8s flags=0x%02x\n",moduleEntry,moduleEntry->moduleName,moduleEntry->flags);
      /* dumpbuffer((char*)moduleEntry,0x10); */
      moduleEntry = moduleEntry->nextEntry;
    }
    exitEntry = exitEntry->nextExitEntry;
  }
}

/*
  The PC Routine

  L R1,<parmlist>
  L 14,X'10'
  L 14,X'304'(,14)
  L 14,X'170'(,14)
  PC 0(14)
  ST 15,<returnCode>
  ST 0,<reasonCode>

  
 */

int defineExit(){
  __asm(ASM_PREFIX
        " CSVDYNEX REQUEST=DEFINE,EXITNAME=(11)"
	",FASTPATH=YES,KEY=2"
	",PERSIST=IPL"
	",AMODE=24"
	",RETCODE=(8),RSNCODE=(9),MF=(E,(10)) \n"
	"EXNAME   DC CL16'EX1' \n"
	"DYNEXL   DC XL2'0000' "
	:::);

}

int addExit(){
  uint64_t myparam = 13;
  int returnCode = 0;
  int reasonCode = 0;
  
  __asm(ASM_PREFIX
        " CSVDYNEX REQUEST=ADD,POS=FIRST,PARAM=%2"
	",STATE=INACTIVE"
	",MODADDR=(8)"
	",MODNAME=(9)"
	",EXITNAME=(10)"
	",ADDABENDNUM=(5)"
	",RETCODE=%0,RSNCODE=%1,MF=(E,(11)) \n"
	:"=m"(returnCode),"=m"(reasonCode)
	:"m"(myparam)
	:);

}

int deleteExitTest(){
  uint64_t myparam = 13;
  int returnCode = 0;
  int reasonCode = 0;
  
  __asm(ASM_PREFIX
        " CSVDYNEX REQUEST=DELETE"
	",MODNAME=(9)"
	",EXITNAME=(10)"
	",RETCODE=%0,RSNCODE=%1,MF=(E,(11)) \n"
	:"=m"(returnCode),"=m"(reasonCode)
	:"m"(myparam)
	:);

}

/* here, addEXIT macro example 
   MODADDR,MODULE,ABENDNUM,POS,PARAM

   STUPID IEFBR14 JOB for testing

   Gawk mode to dump the scratch pad every few seconds 

   Exit itself
      describe UJI params
      get ALET parm of scratch pad
      write to scratch pad something!
   */

int installExitAtAddress(char *exitName, /* max 16 bytes, must be in 31-bit  */
			 char *moduleName, /* must be 8 bytes */
			 Addr31 entryPoint,
			 uint64_t exitParam,
			 int abendnum,
			 bool active){
  char *parmStorage = safeMalloc(DYNEX_V0_LEN,"DYNEX Parms");
  memset(parmStorage,0,DYNEX_V0_LEN);
  CSVDYNEXParms *parms = (CSVDYNEXParms*)parmStorage;
  memset(parms->exitname,0x40,16);
  memcpy(parms->exitname,exitName,strlen(exitName));
  memset(parms->modname,0x40,8);
  memcpy(parms->modname,moduleName,strlen(moduleName));
  int parmAddress = (int)(Addr31)parms;
  parms->flags = (active ? DYNEX_FLAGS_ACTIVE : DYNEX_FLAGS_INACTIVE);
  parms->requestCode = DYNEX_ADD;
  parms->abendnum = abendnum;
  parms->modaddr = entryPoint;
  parms->param = exitParam;
  printf("parmAddress (for ADD) at 0x%x\n",parmAddress);
  dumpbuffer(parmStorage,DYNEX_V0_LEN);
  fflush(stdout);
  int returnCode = 0;
  int reasonCode = 0;
  
  __asm(ASM_PREFIX
	" L 1,%2 \n"
#ifdef _LP64
        " SAM31 \n"
#endif
	" L 14,16 \n"
	" L 14,X'304'(,14) \n"
	" L 14,X'170'(,14) \n"
	/* " DC XL2'0000' \n" */
	" PC 0(14) \n"
#ifdef _LP64
        " SAM64 \n"
#endif
	" ST 15,%0 \n"
	" ST 0,%1 "
	: "=m"(returnCode),"=m"(reasonCode)
	: "m"(parmAddress)
	:);
  printf("DYNEX ADD return=0x%x reason=0x%x\n",returnCode,reasonCode);
  return returnCode;
}

int deleteExit(char *exitName, /* max 16 bytes, must be in 31-bit  */
	       char *moduleName){ /* must be 8 bytes */
  char *parmStorage = safeMalloc(DYNEX_V0_LEN,"DYNEX Parms");
  memset(parmStorage,0,DYNEX_V0_LEN);
  CSVDYNEXParms *parms = (CSVDYNEXParms*)parmStorage;
  memset(parms->exitname,0x40,16);
  memcpy(parms->exitname,exitName,strlen(exitName));
  memset(parms->modname,0x40,8);
  memcpy(parms->modname,moduleName,strlen(moduleName));
  int parmAddress = (int)(Addr31)parms;
  parms->requestCode = DYNEX_DELETE;
  printf("parmAddress (for DELETE) at 0x%x\n",parmAddress);
  dumpbuffer(parmStorage,DYNEX_V0_LEN);
  fflush(stdout);
  int returnCode = 0;
  int reasonCode = 0;
  
  __asm(ASM_PREFIX
	" L 1,%2 \n"
#ifdef _LP64
        " SAM31 \n"
#endif
	" L 14,16 \n"
	" L 14,X'304'(,14) \n"
	" L 14,X'170'(,14) \n"
	/* " DC XL2'0000' \n" */
	" PC 0(14) \n"
#ifdef _LP64
        " SAM64 \n"
#endif
	" ST 15,%0 \n"
	" ST 0,%1 "
	: "=m"(returnCode),"=m"(reasonCode)
	: "m"(parmAddress)
	:);

  printf("DYNEX DELETE return=0x%x reason=0x%x\n",returnCode,reasonCode);
  return returnCode;
}

int listExits1(){
  int answerLength = 10000;
  char *answer = safeMalloc31(answerLength,"CSVDYNEX LIST Answer");
  char *parmStorage = safeMalloc(DYNEX_V0_LEN,"DYNEX Parms");
  memset(parmStorage,0,DYNEX_V0_LEN);
  CSVDYNEXParms *parms = (CSVDYNEXParms*)parmStorage;
  int parmAddress = (int)(Addr31)parms;
  parms->requestCode = DYNEX_LIST;
  parms->answerArea = (Addr31)(int)answer;
  parms->answerLength = answerLength;
  printf("parmAddress at 0x%x\n",parmAddress);
  dumpbuffer(parmStorage,DYNEX_V0_LEN);
  fflush(stdout);
  int returnCode = 0;
  int reasonCode = 0;
  __asm(ASM_PREFIX
	" L 1,%2 \n"
#ifdef _LP64
        " SAM31 \n"
#endif
	" L 14,16 \n"
	" L 14,X'304'(,14) \n"
	" L 14,X'170'(,14) \n"
	/* " DC XL2'0000' \n" */
	" PC 0(14) \n"
#ifdef _LP64
        " SAM64 \n"
#endif
	" ST 15,%0 \n"
	" ST 0,%1 "
	: "=m"(returnCode),"=m"(reasonCode)
	: "m"(parmAddress)
	:);
  printf("CSVDYNEX (Mk2) LIST ret=0x%x reason=0x%x\n",returnCode,reasonCode);
  if (returnCode == 0){
    dumpbuffer(answer,0x400);
    printf("JOE2\n");
    showExits((DYNEXAnswerArea*)answer);
  }
  return returnCode;
}

/* benign exit to call
   
   WTO - maybe

   INZPECT 

   0) how does ZIS put something in LINKLIST (is there a dynamic service? - CSVDYLPA - how to use)

        crossmemory and/or ZIS use lpa.c in COMMON/c
   1) Shim like interrupt or srb to call a metal C routine
    1.1) simple test to WTO an extra line for each
   2) ENQ to lock free queue - or what - how does exit get its buffers?
   3) how does an exit get parameters
         at installation time?
	 thru a ZIS anchor?

   Displaying LPA and linklst

   COMMAND INPUT ===> /D PROG,LPA,MOD=IEFACTRT               
    RESPONSE=S0W1                                             
    CSV550I 13.59.29 LPA DISPLAY 521                         
    FLAGS  MODULE    ENTRY PT  LOAD PT   LENGTH    DIAG      
       P   IEFACTRT  87919728  07919728  000008D8  032FA6B8  

   Exits need 
     module in LPA or LINKLS
     SMFPARM

   --- From the installation Exits Book: ----------------------

   To define IEFU86 to the dynamic exits facility, you must specify
   the exit in both PROGxx and SMFPRMxx. The system does not call the
   exit if it is defined in PROGxx only. If you do not plan to use the
   dynamic exits facility for this exit, you need only define IEFU86
   in SMFPRMxx.

      - So where are PROGxx and SMFPRMxx on my system?
            
   The exit should reside in LPA, the LNKLST concatenation, or the
   nucleus. Do not use the DSNAME keyword when defining the exit in
   PROGxx, as the system will not be able to load the exit when
   restarting SMF.

   If you do not associate any exit routines with exit IEFU86 in
   PROGxx, the system defaults to using the exit routine name that
   matches the exit name (IEFU86).

     - IEFU86 isn't a bad name, but I would prefer to be distinct

   If you associate exit routines with this exit in PROGxx, the system
   does not use the default exit routine. If you need the default exit
   routine, you should explicitly add it to PROGxx.

     - 

   -----

   CSVDYNEX add,
     can use MODADDR? (had better be in ECSA, LPA, etc) 
     JOBNAME and STOKEN can limit applicability to just some job
     PARAM can be used, and actually passes the parms in access register 0 and 1, which is just whacky
     MODNAME is just a name if MODADDR is used

   

   ---- 

   The ZVT is he anchor

   ZVT in ecvtctbl

   ZVT_OFFSET is 0x23C

   ZIS server
     installPlugin(s)
       installServices()
    
 */

static int zisTest(char *serverName){
  ZVT *zvt = zvtGet();
  printf("ZOWEVT (ZVT) at 0x%p with size 0x%x\n",zvt,zvt->size);
  dumpbuffer((char*)zvt,zvt->size);
  ZVTEntry *firstZVTE = zvt->zvteSlots.zis;
  ZVTEntry *currZVTE = firstZVTE;
  int entryIdx = 0;
  for (entryIdx = 0; entryIdx < 100; entryIdx++) {

    if (currZVTE == NULL) {
      break;
    }

    printf("_________________________________________________\n");
    printf("ZVTE = 0x%p\n",currZVTE);
    dumpbuffer((char*)currZVTE,sizeof(ZVTEntry));
    void *productAnchor = currZVTE->productAnchor;
    printf("product anchor at 0x%p\n",productAnchor);
    dumpbuffer((char*)productAnchor,0x60);
    if (!memcmp((char*)productAnchor,CMS_GLOBAL_AREA_EYECATCHER,8)){
      CrossMemoryServerGlobalArea *globalArea = (CrossMemoryServerGlobalArea*)productAnchor;
      if (!memcmp(globalArea->serverName.nameSpacePadded,(CrossMemoryServerName*)serverName,16)){
	printf("Server found\n");
	dumpbuffer((char*)globalArea,0x100); /* sizeof(CrossMemoryServerGlobalArea)); */
	void *potentialZISAnchor = globalArea->userServerAnchor;
	printf("userServerAnchor 0x%p\n",globalArea->userServerAnchor);
	dumpbuffer((char*)potentialZISAnchor,0x80);
	if (!memcmp((char*)potentialZISAnchor,"ZISSRVAN",8)){
	  printf("ZIS Server Anchor found\n");
	  ZISServerAnchor *zisAnchor = (ZISServerAnchor*)potentialZISAnchor;
	  ZISPluginAnchor *pluginAnchor = zisAnchor->firstPlugin;
	  while (pluginAnchor){
	    printf("plugin: (at 0x%p)\n",pluginAnchor);
	    dumpbuffer((char*)pluginAnchor,sizeof(ZISPluginAnchor));
	    printf("pluginData:\n");
	    dumpbuffer((char*)pluginAnchor->pluginData.data,0x40);
	    pluginAnchor = pluginAnchor->next;

	  }
	}
      }
    }
    /* 
       if product is CMS globalArea the

         ZISServerAnchor *serverAnchor = globalArea->userServerAnchor;

	 -then-
  
         PAD_LONG(1, struct ZISPluginAnchor_tag *firstPlugin);


     */

    currZVTE = (ZVTEntry *)currZVTE->next;
  }

}

#define SYSIEFUJI "SYS.IEFUJI      "

typedef struct SMFExitConstants_tag{
  char    eyecatcher[8];        /* CNSTAREA */
  char    productAnchorName[8]; /* "RSCMSRVG" */
  char    serverName[16];
  /* Offset 0x20 */
  char    zisServerEyecatcher[8];  /* ZISSRVAN */
  char    zisPluginName[16];
} SMFExitConstants;

static void *getTestSMFExit(int *routineLengthPtr){
  Addr31 routineAddress = NULL;
  int routineLength;
  __asm(ASM_PREFIX
      "         LARL  1,L$UXIT00                                               \n"
      "         ST    1,%0                                                     \n"
      "         LRL   1,L$RTNLEN                                               \n"
      "         ST    1,%1                                                     \n"
      "         J     L$UXITEX                                                 \n"
	/* "         DC    A(S$RTNLEN)                                              \n" */
      "L$UXIT00 DS    0H                                                       \n"
      "L$UXITRT DS    0H                                                       \n"
      "         STM   14,12,12(13)                                             \n"
      "         EAR   2,1                     AR1 has ECSA scratch page        \n"
      "         EAR   3,0                     AR0 has the constant pool        \n"
      "         ST    3,12(,2) \n"
      "         STCKF 0(2)                    Update the scratch               \n"
      "         L     14,16                   GET CVTPTR                       \n"
      "         L     14,X'8C'(,14)           GET ECVT                         \n"
      "         L     14,X'CC'(,14)           GET CSRCTABL                     \n"
      "         L     14,X'23C'(,14)          GET ZVT                          \n"
      "         MVHI  X'10'(2),X'0010' \n"
      "         ST    14,X'18'(,2)     \n"
      "         LTR   14,14                   TEST NON ZERO                    \n"
      "         BZ    L$NOZVT                                                  \n"
      "         L     14,X'9C'(,14)           FIRST ZVTE                       \n"
      "ZVTELOOP LTR   14,14                   NULL CHECK ZVTE                  \n"
      "         BZ    L$NOZVTE                                                 \n"
      "         MVHI  X'20'(2),X'0020'                                         \n"
      "         ST    14,X'28'(,2)            Log the ZVTE                     \n"
      "         L     4,X'4C'(,14)            PRODUCT ANCHOR                   \n"
      "         MVHI  X'30'(2),X'0030'                                         \n"
      "         ST    4,X'38'(,2)             Log the ProductAnchor            \n"
      "         MVC   X'40'(8,2),0(4)   \n"
      "         MVC   X'48'(8,2),8(3)   \n"
      "         CLC   0(8,4),8(3)             Is it RSCMSRVG                   \n"
      "         BNE   ZVTECNTU                No CMS Global Server             \n"
      "         CLC   X'50'(16,4),X'10'(3)    Is it the right server name      \n"
      "         BE    ZVTEFND                 Found it !                       \n"
      "ZVTECNTU LG    14,X'40'(,14)           ZVTE = ZVTE->NEXT                \n"
      "         B     ZVTELOOP                Should limit by N, too           \n"

      "ZVTEFND  L     5,X'4C'(,4)             user server anchor               \n"
      "         MVHI  8(2),X'0777'            lucky sevens                     \n"
      "         MVHI  X'50'(2),X'0050'                                         \n"
      "         ST    5,X'58'(,2)             Log the ZISAnchor                \n"      
      "         CLC   0(8,5),X'20'(3)         Is it ZISRVAN                    \n"
      "         BNE   L$NOZISA                No ZIS Anchor                    \n"
      "         LG    6,X'20'(,5)             First ZIS Plugin                 \n"
      "         MVHI  X'60'(2),X'0060'                                         \n"
      "         ST    6,X'68'(,2)             ZIS Plugin                       \n"
      "PLGNLOOP LTR   6,6                                                      \n"
      "         BZ    L$NOPLGN                                                 \n"
      "         CLC   X'18'(8,6),X'28'(3)     Test if desired PLUGIN           \n"
      "         BE    PLGNFND                                                  \n"
      "         LG    6,X'30'(,6)             plgn = plgn->next                \n"
      "         B     PLGNLOOP                should count limit, too          \n"
      "PLGNFND  MVHI  8(2),X'0888'            crazy eights                     \n"
      /* here, put real data in the Inzpect plugin,
               write a real init/term thing
	       */
      "         B     L$RETURN                Non error end                    \n"
      "L$NOZVT  MVHI  8(2),X'0008'            RTN CODE 0008                    \n"
      "         B     L$RETURN                                                 \n"         
      "L$NOZVTE MVHI  8(2),X'000C'            RTN CODE 000C                    \n"
      "         B     L$RETURN                                                 \n"         
      "L$NOCMSG MVHI  8(2),X'0010'            RTN CODE 0010                    \n"
      "         B     L$RETURN                                                 \n"
      "L$NOZISA MVHI  8(2),X'0014'            RTN CODE 0014                    \n"
      "         B     L$RETURN                                                 \n"         
      "L$NOPLGN MVHI  8(2),X'0018'            RTN CODE 0014                    \n"
      "         B     L$RETURN                                                 \n"         
      "L$RETURN L     14,12(,13)              Restore everything but R15       \n"
      "         LA    15,0                    Let's not change the outside world \n"
      "         LM    0,12,20(13)             Restore                          \n"
      "         BR    14                                                       \n"
      /* non executable code */
      "         LTORG                                                          \n"
      "L$RTNLEN DC    A(*-L$UXIT00)                                            \n"
      "L$UXITEX DS    0H                                                       \n"
      : "=m"(routineAddress),"=m"(routineLength)
      :
      : "r1");
  *routineLengthPtr = routineLength;
  return routineAddress;
}

static void *getSMFExit(int *routineLengthPtr){
  Addr31 routineAddress = NULL;
  int routineLength;
  __asm(ASM_PREFIX
      "         LARL  1,L$SXIT00                                               \n"
      "         ST    1,%0                                                     \n"
      "         LRL   1,L$SXTLEN                                               \n"
      "         ST    1,%1                                                     \n"
      "         J     L$SXITEX                                                 \n"
	/* HERE set up stack, but return short if it doesn't work
           clobber 13
	   SAM 64/31
	   squirrel old R13 in some register to be restored
           clean stack
           follow example of probes.c 

	   in probe, SMF Q RECEIVE BLOCK

	   HERE2 - work inwards towards a running ZIS writing things in scratchpad 
           in CMS global area there is 0 for eyecatcher
	                              4C for userServerAnchor (31-bit pointer )
                                      40 for serverName (16 bytes, padded 0x40, ZWESIS_JOE)
                                      ZIS server Anchor
				      0 for ZISSRVAN

           HERE3
             check whether logic works, and begin to think about SS-PC (ZIS Service) to enqueue info
	     and the big pool/queue
             also how the server inits the pool.   
	 */
	/* "         DC    A(S$RTNLEN)                                              \n" */
      "L$SXIT00 DS    0H                                                       \n"
      "L$SXITRT DS    0H                                                       \n"
      "         STM   14,12,12(13)                                             \n"

      "         EAR   2,1                     AR1 has ECSA scratch page        \n"
      "         EAR   3,0                     AR1 has install parameters       \n"
      "         LARL  7,S$CMSGA               Fist Constant in Constant Pool   \n"
      "         STCKF 0(2)                    Update the scratch               \n"
      "         LGFI  8,X'C1C2C3C4' \n"
      "         ST    8,X'E0'(,2)   \n"
      "         ST    1,X'E4'(,2)   \n"
      "         LGF   8,0(,1)                 Squirrel the arglist             \n"
      "         ST    8,X'E8'(,2)   \n"
      "         MVC   X'C0'(32,2),0(8)           \n"
	/* Find the ZVT/ZVTE/CMSGA/ZISANCHOR/INZPECTPLUGIN 
           14,4,5,6 are scratch registers for doing so 
        */
      "         L     14,16                   GET CVTPTR                       \n"
      "         L     14,X'8C'(,14)           GET ECVT                         \n"
      "         L     14,X'CC'(,14)           GET CSRCTABL                     \n"
      "         L     14,X'23C'(,14)          GET ZVT                          \n"
      "         MVHI  X'10'(2),X'0010' \n"
      "         ST    14,X'18'(,2)     \n"
      "         LTR   14,14                   TEST NON ZERO                    \n"
      "         BZ    S$NOZVT                                                  \n"
      "         L     14,X'9C'(,14)           FIRST ZVTE                       \n"
      "SZVTELOP LTR   14,14                   NULL CHECK ZVTE                  \n"
      "         BZ    S$NOZVTE                                                 \n"
      "         L     4,X'4C'(,14)            PRODUCT ANCHOR                   \n"
      "         ST    7,X'20'(,2)     \n"
      "         MVC   X'24'(8,2),0(7) \n"
      "         CLC   0(8,4),0(7)             Is it RSCMSRVG                   \n"
      "         BNE   SZVTECNT                No CMS Global Server             \n"
      "         CLC   X'50'(16,4),X'10'(3)    Is it the right server name      \n"
      "         BE    SZVTEFND                Found it !                       \n"
      "SZVTECNT LG    14,X'40'(,14)           ZVTE = ZVTE->NEXT                \n"
      "         B     SZVTELOP                Should limit by N, too           \n"
      "SZVTEFND L     5,X'4C'(,4)             user server anchor               \n"
      "         MVHI  8(2),X'0777'            lucky sevens                     \n"
      "         CLC   0(8,5),8(7)             Is it ZISRVAN                    \n"
      "         BNE   S$NOZISA                No ZIS Anchor                    \n"
      "         LG    6,X'20'(,5)             First ZIS Plugin                 \n"
      "         MVHI  X'60'(2),X'0060'                                         \n"
      "         ST    6,X'68'(,2)             ZIS Plugin                       \n"
      "SPLGNLOP LTR   6,6                                                      \n"
      "         BZ    S$NOPLGN                                                 \n"
      "         CLC   X'18'(16,6),X'20'(3)    Test if desired PluginName       \n"
      "         BE    SPLGNFND                                                 \n"
      "         LG    6,X'30'(,6)             plgn = plgn->next                \n"
      "         B     SPLGNLOP                should count limit, too          \n"
      "SPLGNFND MVHI  8(2),X'0888'            crazy eights                     \n"
	/*  X'7C' of R6 now has the metal dispatch routine */
      /* Expansion of STORAGE OBTAIN,LENGTH=(6),SP=229,LINKAGE=SYSTEM,LOC=31,KEY=0   */
      "         LGFI  0,X'00010000'           The length                       \n"
      "         LGFI  15,X'4000E572'           Bits,key=9,sub=229,bits          \n"
      "         XGR   1,1                     Clear R1                         \n"
      "         MVHI  8(2),X'0001'            Ckpt1  \n"
      "         LLGT  14,16(0,0)              CVT                              \n"
      "         L     14,772(14,0)            \n"
      "         L     14,160(14,0)            \n"
      "         PC    0(14)                   PC Call STORAGE OBTAIN/RELASE    \n"
      "         MVHI  X'C'(2),X'0002'         Ckpt2                            \n"
      "         ST    15,X'10'(,2)            Log R15 \n"
      "         LTR   15,15                   Did Storage Obtain work          \n"
      "         BNZ   S$RETURN                NO stackee, no workee            \n"
      "         ST    13,0(,1)                Remember the save area from OS   \n"
      "         LLGTR 13,1                    R13 is now the metal stack       \n"
      "         MVHI  X'14'(2),X'0003'        Ckpt3 \n"
      "         LLGF  15,X'7C'(,6)            load function pointer            \n"
      "         STG   15,X'18'(,2)            show metal entry point           \n"
      "         SAM64                         Metal ZIS code is 64-bit         \n"
      "         MVHI  X'20'(2),X'0020'   \n"
      "         STG   3,X'28'(,2)        \n"
      "         STG   2,X'30'(,2)        \n"
      "         STG   8,X'38'(,2)        \n"
      "         STG   3,X'90'(,13)            set first arg exit ident.        \n"
      "         STG   2,X'98'(,13)            set second arg - scratch page    \n"
      "         STG   0,X'A0'(,13)            r0 from OS                       \n"
      "         STG   8,X'A8'(,13)            (squirreled) r1 from OS          \n"
      "         LAE   1,X'90'(,13)            point at first arg               \n"
      "         LGR   11,13                   scratch 11, and start making nab \n"
      "         AHI   11,X'B0'                NAB is first byte above what we've used \n"
      "         STG   11,X'88'(,13)           set the NAB                     \n"
      "         BASR  14,15                   call the service routine        \n"
      "         LGR   1,13                    hold on to the metal stack      \n"
      "         L     13,0(,13)               R13 gets original save area     \n"
      "         SAM31                                                          \n"
      /* STORAGE RELEASE,LENGTH=65536,SP=229,ADDR=(1),LINKAGE=SYSTEM,KEY=0    */
      "         LGFI  0,X'00010000'           The length                       \n"
      "         LGFI  15,X'4000E503'          Bits,key=0,sub=229,bits          \n"
      "         LLGT  14,16(0,0)              CVT                              \n"
      "         L     14,772(14,0)                                             \n"
      "         L     14,160(14,0)                                             \n"
      "         PC    0(14)                   PC Call STORAGE OBTAIN/RELASE    \n"
      "         B     S$RETURN                Skip around fail labelS          \n"
      "S$NOZVT  MVHI  8(2),X'0008'            RTN CODE 0008                    \n"
      "         B     S$RETURN                                                 \n"         
      "S$NOZVTE MVHI  8(2),X'000C'            RTN CODE 000C                    \n"
      "         B     S$RETURN                                                 \n"         
      "S$NOCMSG MVHI  8(2),X'0010'            RTN CODE 0010                    \n"
      "         B     S$RETURN                                                 \n"
      "S$NOZISA MVHI  8(2),X'0014'            RTN CODE 0014                    \n"
      "         B     S$RETURN                                                 \n"         
      "S$NOPLGN MVHI  8(2),X'0018'            RTN CODE 0014                    \n"
      "         B     S$RETURN                                                 \n"         
      "S$SXFAIL MVHI  8(2),X'001C'            NOP for now                      \n"
      "S$RETURN L     14,12(,13)              Restore everything but R15       \n"
      "         LA    15,0                    Let's not change the outside world \n"
      "         LM    0,12,20(13)             Restore                          \n"
      "         BR    14                                                       \n"
      /* non executable code */
      "         LTORG                                                          \n"
      "S$CMSGA  DC    CL8'RSCMSRVG'                                            \n" 
      "S$ZISSRV DC    CL8'ZISSRVAN'                                            \n"
      "L$SXTLEN DC    A(*-L$SXIT00)                                            \n"
      "L$SXITEX DS    0H                                                       \n"
      : "=m"(routineAddress),"=m"(routineLength)
      :
      : "r1");
  *routineLengthPtr = routineLength;
  return routineAddress;
}

static int traceDYNExit = 1;

#define X_MEMORY_SERVER_KEY     0
#define X_MEMORY_SERVER_SUBPOOL 228


static void *getExitRoutineInCommon(InzpectAuthAPI *authAPI, int extraSpace, int *routineLengthPtr, bool isTest){
  int routineLength = 0;
  Addr31 routine = (isTest ? getTestSMFExit(&routineLength) : getSMFExit(&routineLength));
  *routineLengthPtr = routineLength;
  if (traceDYNExit >= 1){
    printf("iefUJIExit at 0x%p len=%d\n",routine,routineLength);
    fflush(STDOUT);
  }
  char *routineInECSA = NULL;
  allocateCommon31Memory(authAPI,(void**)&routineInECSA,routineLength+extraSpace,X_MEMORY_SERVER_KEY,X_MEMORY_SERVER_SUBPOOL);
  copyIntoProtectedStorage(authAPI,routineInECSA,(char*)routine,routineLength);
  if (traceDYNExit >= 1){
    printf("getTrampInCommon.end\n");
    fflush(STDOUT);
  }
  return routineInECSA;
}

#ifndef METTLE
#pragma linkage(BPX4SLP,OS)
#pragma linkage(BPX1SLP,OS)
#endif

static void bpxsleep(int seconds){
  int returnValue;
  int *returnValuePtr;
  
#ifdef _LP64
  returnValuePtr = &returnValue;
  BPX4SLP(seconds,returnValuePtr); 
#else
  returnValuePtr = (int*) (0x80000000 | ((int)&returnValue));
  BPX1SLP(seconds,returnValuePtr); 
#endif
}

/*

  IGGPRE00_EXIT
  IGGPOST0_EXIT

  Before VTOC and other major 

  

00000000  1B91D968 1B91D910 C9C7C7D7 D9C5F0F0  6DC5E7C9 E3404040 00010205 60200000 |.jR..jR.IGGPRE00_EXIT   ....-...|
00000020  00000001 00000000                                                        |........|
    Module at 0x1B91D910 IGGPRE00 flags=0x80
exit entry at 0x1B91D968 ---------------------
00000000  1B91D9A8 1B91D950 C9C7C7D7 D6E2E3F0  6DC5E7C9 E3404040 00010205 60200000 |.jRy.jR&IGGPOST0_EXIT   ....-...|


  The straight dope on (not drugs but) SMF EXITS

  

  If you associate multiple exit routines with IEFU83, you can specify
  how the return information is to be handled using the ATTRIB KEEPRC
  function of the SETPROG EXIT command, the EXIT statement of PROGxx,
  or CSVDYNEX services. If multiple exit routines match the ATTRIB
  KEEPRC criteria, the system returns information from the exit
  routine that finished first.

  If you do not specify the ATTRIB KEEPRC function, the system returns
  the information from the exit routine whose return value was the
  greatest. If multiple exit routines return with the same highest
  value, the return information from the exit routine that finished
  first will be returned.

  If you associate multiple exit routines with exit IEFU83, and any of
  those exit routines return with a value of 4, the current SMF record
  is suppressed.

  In the TCB, the TCBTCT field points to the TCT. In the TCT, the
  TCTJMR field points to the JMR. The JMR and the PSA are mapped by
  macros IEFJMR and IHAPSA, respectively. See z/OS MVS Data Areas in
  the z/OS Internet library for the mapping of the JMR, and z/OS MVS
  Data Areas, Vol 5 (SSAG-XTLST) for the mappings of the TCB and the
  TCT.

  Joe's take:
   
    == this implies that returning 0, and not specifying KEEPRC 
       makes this exit "invisible" to any other logic on the system.  

    == returning 4 is an aggressive thing to do

    == R1 points at an SMF record in SMF83
       The first 4 bytes of record are an RDW, indicating that record will not exceed 32K (I Hope) 

    == SMF83 is oldskool SMFWTM or SMFEWTM with branch=NO
    == SMF84 is SMFEWTM BRANCH=YES, possibly locked or SRB
    == SMF85 is SMFEWTM BRANCH=YES, plus XMEM, not that it should matter to us.  
    == SMF86 encompasses the three previous, but R1 poins to a parameter list (aka SMXP) that points to SMF record

    == What's in a JMR (see above)

    == what about fragmentation and reassembly?
       == we hope that the RDW takes care of everything 

    == The 8-byte parameter is a blessing and a curse.
       The exit can get everything it needs from it, but it may point at resources that are dead or dormant.

    == Are CSVDYNEX's all too late?

    == installation dimensions
       -- (handler in memory, or, some LPA module) 
       -- memory allocation questionable in some context
           can a segment of 64 bit be allocated in all spots

    == what should the param have, if anything 
       the actual routine address - pretty safe and slow


    == logstreams *ARE* better than exits, because of persistence and well-orderedness and that they 
       probably start earlier in the IPL process.  So catching up and completeness are more achievable

    
    Environment Summary
    
            | InterruptEnabled | S/P | KEY | AMode | Locked | DUMode    | XMEM   | ASpace
    --------+------------------+-----+-----+-------+--------+-----------+-----------
    IEFU83  |      true        |  S  |  0  |  31   |        |           |        | SMFWTM and SMFEWTM,BRANCH=NO
    IEFU84  |      true        |  S  |  0  |  31   | Can Be | Task/SRB  |        | SMFEWTM BRANCH=Y  issuer
    IEFU85  |      true        |  S  |  0  |  31   | Can Be | Task/SRB  | Yes    | SMFEWTM BRANCH=Y,MODE=XMEM 
    IEFU86  |      true        |  S  |  0  |  31   | Can Be | Task/SRB  | Either | all of the above

    Which SMF Records from which exit (Extracted from SMF book)

    IEFU83:
      Type 6, 25, 26, 40, 41.3, 43, 45, 47, 48, 49, 57, 59, 62, 64, 70-79, 84, 89, 90.7

    IEFU84:
      Type 0, 4,5, 7, 8, 20, 22, 23, 30, 32, 33, 34 (old TSO), 35 (logoff).1, 41.1,41,2, 42/x,  90.(except sub 7), 98
      Type 30 subtype 1,3,4,5 are TCB, 2,6 are TCB

    IEFU85:
      Type 33.2, 92 (File system activity), 106 (BCPii activity)
      
       
    
*/

char *getStack(int *returnCodePtr){
  char *data;
  int returnCode = 0;
  __asm(ASM_PREFIX
        " STORAGE OBTAIN,LENGTH=65536,SP=77,LINKAGE=SYSTEM,LOC=31,CALLRKY=YES\n" 
	" LGR %0,1\n"
	" ST  15,%1 " :
	"=r"(data),"=m"(returnCode) :
	);
  /* only 0 is a good return code */
  return data;
}

char *getStackB(int *returnCodePtr){
  char *data;
  int returnCode = 0;
  __asm(ASM_PREFIX
        " STORAGE OBTAIN,LENGTH=65536,SP=77,LINKAGE=BRANCH,LOC=31,KEY=0\n" 
	" LGR %0,1\n"
	" ST  15,%1 " :
	"=r"(data),"=m"(returnCode) :
	);
  /* only 0 is a good return code */
  return data;
}

int freeStack(char *stack){
  int returnCode;
  __asm(ASM_PREFIX
        " STORAGE RELEASE,LENGTH=65536,SP=77,ADDR=(%1),LINKAGE=SYSTEM,CALLRKY=YES\n"  
	" ST 15,%0 "
	: "=m"(returnCode):
	"r"(stack));
  return returnCode;
}

/* SMF Exits 



    */


static int smf83Test(char *serverName){
  InzpectAuthAPI *authAPI = makeAuthAPI(NULL);
  SMFExitConstants constantPool;
  memcpy(&constantPool.eyecatcher[0],"CNSTAREA",8);
  memcpy(&constantPool.productAnchorName,"RSCMSRVG",8);
  memset(&constantPool.serverName,0x40,16);
  memcpy(&constantPool.serverName,serverName,strlen(serverName));
  memcpy(&constantPool.zisServerEyecatcher,"ZISSRVAN",8);
  memcpy(&constantPool.zisPluginName,"INZPECT         ",16);
  printf("Constants (in local mem)\n");
  dumpbuffer((char*)&constantPool,sizeof(SMFExitConstants));

  CommonModuleManager *moduleManager = makeCommonModuleManager(authAPI);
  char *scratchpad = NULL;
  int allocateStatus = allocateCommon31Memory(authAPI,(void**)&scratchpad,0x1000,X_MEMORY_SERVER_KEY,X_MEMORY_SERVER_SUBPOOL);
  printf("allocateStatus=0x%x scratchpad=0x%p\n",allocateStatus,scratchpad);
  int routineLength = 0;
  void *exitRoutine = getExitRoutineInCommon(authAPI,0x100,&routineLength,true);
  copyIntoProtectedStorage(authAPI,((char*)exitRoutine)+routineLength,(char*)&constantPool,sizeof(SMFExitConstants));
  int exitAddress = ((int)((Addr31)exitRoutine))|0x80000000;
  copyIntoProtectedStorage(authAPI,scratchpad,"INITVALU",8);
  printf("exitAddress=0x%x\n",exitAddress);
  fflush(stdout);
  char *exitName = "SYS.IEFU83"; /* UJI */
  uint64_t param = (uint64_t)scratchpad;
  printf("param1 = 0x%016llx\n",param);
  uint64_t highWord = ((uint64_t)exitRoutine)+routineLength;
  param |= (highWord << 32);
  printf("exitAddress=0x%x param=0x%016llx\n",exitAddress,param);
  printf("routine and constant pool 0x%x\n",exitAddress);
  dumpbuffer((char*)exitRoutine,routineLength+sizeof(SMFExitConstants));
  fflush(stdout);

  int status = installExitAtAddress(exitName,
				    "JOEUJI01",
				    (Addr31)exitAddress,
				    (uint64_t)param,
				    3,
				    true);
  
  for (int i=0; i<10; i++){
    sleep(5);
    printf("scratchpad (%d):\n",i);
    dumpbuffer(scratchpad,0x80);
  }

  if (status == 0){
    /* listExits1(); */
    printf("-----------------------  AFTER ---------------------------\n");
    deleteExit(exitName,"JOEUJI01");
    /* listExits1(); */
  }

  freeCommonModules(moduleManager);


}

static void *getInzpectExitDispatcher(char *serverName){
  ZVT *zvt = zvtGet();
  printf("ZOWEVT (ZVT) at 0x%p with size 0x%x\n",zvt,zvt->size);
  dumpbuffer((char*)zvt,zvt->size);
  ZVTEntry *firstZVTE = zvt->zvteSlots.zis;
  ZVTEntry *currZVTE = firstZVTE;
  int entryIdx = 0;
  for (entryIdx = 0; entryIdx < 100; entryIdx++) {

    if (currZVTE == NULL) {
      break;
    }

    void *productAnchor = currZVTE->productAnchor;
    if (!memcmp((char*)productAnchor,CMS_GLOBAL_AREA_EYECATCHER,8)){
      CrossMemoryServerGlobalArea *globalArea = (CrossMemoryServerGlobalArea*)productAnchor;
      if (!memcmp(globalArea->serverName.nameSpacePadded,(CrossMemoryServerName*)serverName,16)){
	void *potentialZISAnchor = globalArea->userServerAnchor;
	if (!memcmp((char*)potentialZISAnchor,"ZISSRVAN",8)){
	  printf("ZIS Server Anchor found\n");
	  ZISServerAnchor *zisAnchor = (ZISServerAnchor*)potentialZISAnchor;
	  ZISPluginAnchor *pluginAnchor = zisAnchor->firstPlugin;
	  while (pluginAnchor){
	    printf("plugin: (at 0x%p)\n",pluginAnchor);
	    dumpbuffer((char*)pluginAnchor,sizeof(ZISPluginAnchor));
	    printf("pluginData:\n");
	    char *pluginData = (char*)pluginAnchor->pluginData.data;
	    dumpbuffer(pluginData,0x40);
	    if (!memcmp(pluginAnchor->name.text,"INZPECT         ",16)){
	      InzpectPluginData *inzpectData = (InzpectPluginData*)pluginData;
	      return inzpectData->exitDispatcherAddress;
	    }
	    pluginAnchor = pluginAnchor->next;

	  }
	}
      }
    }
    currZVTE = (ZVTEntry *)currZVTE->next;
  }
  printf("NO EXIT DISPATCHER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  return (void*)0;
}

#define SMF_EXIT_COUNT 2

/* what about SYSSTC.IEFU83 */

static char *smfExitNames[SMF_EXIT_COUNT] =  { "SYS.IEFU83", "SYS.IEFU84"};


static int smfGeneralTest(char *serverName, int repetitionCount){
  InzpectAuthAPI *authAPI = makeAuthAPI(NULL);
  ExitIdentification identification;
  memcpy(&identification.eyecatcher[0],"EXITIDNT",8);
  identification.metalRoutine = 0x00000000; /* no derived from ZVT/ZVTE/INZPLUGIN etc, not getInzpectExitDispatcher(serverName); */
  memset(&identification.zisServerName,0x40,16);
  memcpy(&identification.zisServerName,serverName,strlen(serverName));
  memcpy(&identification.pluginName,"INZPECT         ",16);

  printf("ExitIdentification (in local mem)\n");
  dumpbuffer((char*)&identification,sizeof(ExitIdentification));

  CommonModuleManager *moduleManager = makeCommonModuleManager(authAPI);
  char *scratchpad = NULL;
  int allocateStatus = allocateCommon31Memory(authAPI,(void**)&scratchpad,0x1000,X_MEMORY_SERVER_KEY,X_MEMORY_SERVER_SUBPOOL);
  printf("allocateStatus=0x%x scratchpad=0x%p\n",allocateStatus,scratchpad);
  int routineLength = 0;
  void *exitRoutine = getExitRoutineInCommon(authAPI,0x100,&routineLength,false);
  copyIntoProtectedStorage(authAPI,((char*)exitRoutine)+routineLength,(char*)&identification,sizeof(ExitIdentification));
  int exitAddress = ((int)((Addr31)exitRoutine))|0x80000000;
  copyIntoProtectedStorage(authAPI,scratchpad,"INITVALU",8);
  printf("exitAddress=0x%x\n",exitAddress);
  fflush(stdout);
  uint64_t param = (uint64_t)scratchpad;
  printf("param1 = 0x%016llx\n",param);
  uint64_t highWord = ((uint64_t)exitRoutine)+routineLength;
  param |= (highWord << 32);
  printf("exitAddress=0x%x param=0x%016llx\n",exitAddress,param);
  printf("routine and exit identification 0x%x\n",exitAddress);
  dumpbuffer((char*)exitRoutine,routineLength+sizeof(SMFExitConstants));
  fflush(stdout);

  int status = 0,i=0;
  int installedCount = 0;
  for (i=0; i<SMF_EXIT_COUNT; i++){
    char *exitName = smfExitNames[i];
    int status = installExitAtAddress(exitName,
				      "JOEUJI01",
				      (Addr31)exitAddress,
				      (uint64_t)param,
				      3,
				      true);
    if (status){
      break;
    } else{
      installedCount++;
    }
  }
  
  for (int i=0; i<repetitionCount; i++){
    sleep(5);
    printf("scratchpad (%d):\n",i);
    dumpbuffer(scratchpad,0x100);
  }

  if (status == 0){
    /* listExits1(); */
    printf("-----------------------  AFTER ---------------------------\n");
    for (i=0; i<installedCount; i++){
      char *exitName = smfExitNames[i];
      deleteExit(exitName,"JOEUJI01");
    }
    /* listExits1(); */
  }

  freeCommonModules(moduleManager);


}

static void hackFix(){
  InzpectAuthAPI *authAPI = makeAuthAPI(NULL);
  char *ptr = (char*)0x19188000;
  copyIntoProtectedStorage(authAPI,ptr,"RSCMSRVG",8);
}

/* ChunkPool 
   Manage fixed blocks of 64 bit storage
   for common and shared users

   Christian Jacobi's paper is great on Z TX mem

   http://class.ece.iastate.edu/tyagi/cpre581/papers/Micro12TransactionalMemory.pdf

   Also TBEGINC is the shznit because of 4 DOUBLE WORDS and 

      - 256 bytes of code
      - 32 instructoins
      - no back branches or subroutine calls
   */   

typedef struct ChunkPool64_tag{
  char   eyecatcher[8];  /* "CKPOOL64" */
  InzpectAuthAPI *api;
  void **allExtents;
  void  *freeList;
  int    extentSize;    /* must be multple of 1MB */
  int    initialExtents;
  int    extentsPerExpansion;
  int    chunkSize;              /* must be less than segments*1MB */
  int    key;
  int    type;
  int    maxExtents;
  int    extentCount;
  uint64_t memtoken;
} ChunkPool64;

#define CHUNK_POOL64_PRIVATE 1
#define CHUNK_POOL64_COMMON  2
#define CHUNK_POOL64_SHARED  3

#define CHUNK_POOL64_NOT_MEGABYTES 8
#define CHUNK_POOL64_CHUNK_LARGER_THAN_EXTENT 12

static void *cp64Allocate(ChunkPool64 *pool, int extents);
static int cp64Release(ChunkPool64 *pool, void *chunk);

ChunkPool64 *makeChunkPool64(int type, 
			     int chunkSize, 
			     int extentSize,
			     int initialExtents,
			     int extentsPerExpansion,
			     int maxExtents,
			     int key,
			     int *errorCode){
  if (extentSize % 0x10000){
    *errorCode = CHUNK_POOL64_NOT_MEGABYTES;
    return NULL;
  } else if (chunkSize > extentSize){
    *errorCode = CHUNK_POOL64_CHUNK_LARGER_THAN_EXTENT;
    return NULL;
  }
  ChunkPool64 *pool = (ChunkPool64*)safeMalloc(sizeof(ChunkPool64),"ChunkPool64");
  memset(pool,0,sizeof(ChunkPool64));
  memcpy(pool->eyecatcher,"CKPOOL64",8);
  pool->type = type;
  pool->chunkSize = chunkSize;
  pool->extentSize = extentSize;
  pool->initialExtents = initialExtents;
  pool->extentsPerExpansion = extentsPerExpansion;
  pool->maxExtents = maxExtents;
  pool->allExtents = (void**)safeMalloc(maxExtents*sizeof(void*),"CPOOL64 Extent Vector");
  memset(pool->allExtents,0,maxExtents*sizeof(void*));
  pool->extentCount = 0;
  pool->key = key;
  for (int i=0; i<initialExtents; i++){
    void *extent = cp64Allocate(pool,initialExtents);
    char *extentData = (char*)extent;
    if (extent){
      for (int pos = 0; (pos+chunkSize)<extentSize; pos+=chunkSize){
	void *chunkToRelease = (void*)(extentData+pos);
	printf("pool = 0x%p pos=0x%x chunkToRelease=0x%p\n",pool,pos,chunkToRelease);
	fflush(stdout);
	cp64Release(pool,chunkToRelease);
      }
    }
  }
  pool->memtoken = (uint64_t)pool;
  return pool;
}

/* unpriveleged */
static char* getmain64(long long sizeInMegabytes, int *returnCode, int *reasonCode){
  
  char* data = NULL;
  
  int macroRetCode = 0;
  int macroResCode = 0;

  __asm("IARVPLST IARV64 MF=L\n" : "XL:DS:1000"(IARVPLST));

  __asm(" IARV64 REQUEST=GETSTOR,COND=YES,SEGMENTS=(%3),ORIGIN=(%2),TTOKEN=NO_TTOKEN,"
        "RETCODE=%0,RSNCODE=%1,MF=(E,(%4))\n" :
        "=m"(macroRetCode), "=m"(macroResCode): 
        "r"(&data),"r"(&sizeInMegabytes),"r"(&IARVPLST));

   if (returnCode) *returnCode = macroRetCode;
   if (reasonCode) *reasonCode = macroResCode;
  
   return data;

}

static char* getmain64Sup(long long sizeInMegabytes, int *returnCode, int *reasonCode, int key){
  
  char* data = NULL;
  
  int macroRetCode = 0;
  int macroResCode = 0;

  __asm("IARVPLST IARV64 MF=L\n" : "XL:DS:1000"(IARVPLST));

  __asm(" IARV64 REQUEST=GETSTOR,COND=YES,SEGMENTS=(%3),ORIGIN=(%2),TTOKEN=NO_TTOKEN,KEY=(%5),"
        "RETCODE=%0,RSNCODE=%1,MF=(E,(%4))\n" :
        "=m"(macroRetCode), "=m"(macroResCode): 
        "r"(&data),"r"(&sizeInMegabytes),"r"(&IARVPLST),"r"(key));

   if (returnCode) *returnCode = macroRetCode;
   if (reasonCode) *reasonCode = macroResCode;
  
   return data;

}


/* HERE,
   IARV64 with parm block (or at least private)
     see alloc.c - worry about memtokens (how unique?)
     if not unique, we need a token vector
     allocate can be protected by ENQ/DEQ, because should be relatively uncommon op
        system-level, not plex

    USERTKN=user token is an 8-byte token that relates two or more memory objects to each other. Later,
    the program can request a list of memory objects that have that same token and can delete them as a
    group.
   */

static void *cp64Allocate(ChunkPool64 *pool, int extents){
  void *extent = NULL;
  int returnCode = 0;
  int reasonCode = 0;
  switch (pool->type){
  case CHUNK_POOL64_PRIVATE:
    if (pool->key == 8){
      extent = getmain64((pool->extentSize*extents)>>20,&returnCode,&reasonCode);
    } else{
      printf("PANIC no non-key8 yet\n");
    }
    break;
  default:
    printf("PANIC unhandled CPOOL64 allocation case\n");
    break;
  }
  return extent;
}

static void *cp64GetFreeChunk(ChunkPool64 *pool, int *returnCodePtr){
  int returnCode = 0;
  void **freeListHandle = &(pool->freeList);
  void *chunk;
  __asm(ASM_PREFIX
        "         TBEGINC 0,0    Let's DO this!      \n"
	"         LTG 2,0(%2)    Get the freeList    \n"
        "         STG 2,%0       popped element      \n"
        "         BZ G$NOCHNK                        \n"
        "         LG  3,0(,2)    Chunk's next        \n"
        "         STG 3,0(%2)    Pop freelist        \n"
        "         B  G$DONE                          \n"
        "G$NOCHNK ST 8,%1        Say we failed       \n"
        "G$DONE   TEND                               "
	:"=m"(chunk),"=m"(returnCode)
	:"r"(freeListHandle)
	:
	"r2","r3");
  *returnCodePtr = returnCode;
  return chunk;
}

static int cp64Release(ChunkPool64 *pool, void *chunk){
  int returnCode = 0;
  void **freeListHandle = &(pool->freeList);
  /*
    printf("release pool=0x%p freeListHandle = 0x%p\n",pool,freeListHandle);
    fflush(stdout);
    */
  __asm(ASM_PREFIX
        "         TBEGINC 0,0    Let's DO this!      \n"
	"         LG  2,0(%1)    Get the freeList    \n"
        "         STG 2,0(%0)    chain the chunk     \n"
        "         STG %0,0(%1)   new list head       \n"
        "         TEND                               "
	:
	:"r"(chunk),"r"(freeListHandle)
	:"r2");
  return returnCode;
}

static void cp64Test(){
  int errorCode = 0;
  ChunkPool64 *pool = makeChunkPool64(CHUNK_POOL64_PRIVATE,
				      0x10000,
				      0x100000,
				      1,
				      1,
				      10,
				      8,
				      &errorCode);
  printf("WTF.1\n");
  fflush(stdout);
  if (pool){
    int returnCode = 0;
    printf("pool at 0x%p pool->freeList=0x%p\n",pool,pool->freeList);
    fflush(stdout);
    void *chunk = cp64GetFreeChunk(pool,&returnCode);
    printf("here's the chunk 0x%p\n",chunk);
    fflush(stdout);
    cp64Release(pool,chunk);
  } else{
    printf("could not make pool code=%d\n",errorCode);
  }
}

static void getDUID(){
  int *mem = (int*)0;
  TCB *tcb = (TCB*)mem[0x21C/4];
  printf("TCB\n");
  dumpbuffer((char*)tcb,0x120);
  STCB *stcb = (STCB*)tcb->tcbstcb;
  printf("STCB at 0x%p\n",stcb);
  dumpbuffer((char*)stcb,sizeof(STCB));
  /* 
     Addr31         stcbducv;  (0x28)
     Addr31         stcbducr;  (0x2C)
     */

}

/* https://www.ibm.com/docs/en/zos/2.2.0?topic=cpcps-description

   Can CELL Pools do 64 bit well

   ID can be in INZPECT block
   REGS=USE seems key
   */

static void stubTest(){
  uint64_t zvt = 0;
  uint64_t zvte = 0;
  uint64_t stubVector = 0;
  __asm(ASM_PREFIX
	" LLGT 15,16(0,0)       CVT \n"
	" LLGT 15,X'8C'(,15)    ECVT \n"
        " LLGT 15,X'CC'(,15)    CSRCTABL \n"
	" LLGT 15,X'23C'(,15)   ZVT \n"
	" STG  15,%0  \n"
	" LLGT 15,X'9C'(,15)    FIRST ZVTE (the ZIS) \n"
	" STG  15,%1  \n"
	" LG   15,X'80'(,15)    ZIS STUB VECTOR \n"
	" STG  15,%2  \n"
	/* " LG   15,X'F0'(,15)    CPL64CRE \n" */
	:
	:
	"m"(zvt),"m"(zvte),"m"(stubVector)
	:);
  printf("ZVT at 0x%llx\n",zvt);
  dumpbuffer((char*)zvt,0x100);
  printf("ZVTE at 0x%llx\n",zvte);
  dumpbuffer((char*)zvte,0x100);
  printf("STUBS at 0x%llx\n",stubVector);
  fflush(stdout);
  dumpbuffer((char*)stubVector,0x200);
}



int main(int argc, char **argv){
  printf("exit start\n");
  char *command = argv[1];
  if (!strcmp(command,"add")){
    addExit();
  } else if (!strcmp(command,"zis")){
    zisTest(argv[2]);
  } else if (!strcmp(command,"list")){
    listExits1();
  } else if (!strcmp(command,"simple")){
    smf83Test(argv[2]);
  } else if (!strcmp(command,"general")){
    if (argc < 4){
      printf("please supply ZISName and repetition count\n");
      return 0;
    }
    smfGeneralTest(argv[2],atoi(argv[3]));
  } else if (!strcmp(command,"hack")){
    hackFix();
  } else if (!strcmp(command,"delete")){
    if (argc < 4){
      printf("please supply exit and module name\n");
      return 0;
    }
    /* dynexit delete "SYS.IEFU83" "JOEUJI01" */
    char *exitName = argv[2];
    char *moduleName = argv[3];
    deleteExit(exitName,moduleName);
  } else if (!strcmp(command,"dump")){
    uint64_t address = strtoull(argv[2],NULL,16);
    int len = atoi(argv[3]);
    dumpbuffer((char*)address,len);
  } else if (!strcmp(command,"pool")){
    printf("here we are\n");
    fflush(stdout);
    cp64Test();
  } else if (!strcmp(command,"du")){
    getDUID();
  } else if (!strcmp(command,"zvte")){
    ZVTEntry zvteStorage;
    ZVTEntry *zvte = &zvteStorage;
    printf("zvte = 0x%p zvte->zisStubVector=0x%x\n",
	   zvte,&(zvte->zisStubVector));
  } else if (!strcmp(command,"cpool64")){
    int returnCode = 0;
    int reasonCode = 0;
    void *addrF = (void*)iarcp64Create;
    printf("iarCP64Create at 0x%p\n",addrF);
    dumpbuffer((char*)addrF,0x10);
    void *code = ((void**)addrF)[1];
    printf("code at 0x%p\n",code);
    dumpbuffer(code,0x300);
    uint64_t cpid = iarcp64Create(false,32768,&returnCode,&reasonCode);
    printf("iarcp64 cpid=0x%llx ret=0x%x, reason=0x%x\n",cpid,returnCode,reasonCode);
    fflush(stdout);
    if (cpid && !returnCode){
      void *cell1 = iarcp64Get(cpid,&returnCode,&reasonCode);
      printf("cell=0x%p ret=0x%x, reason=0x%x\n",cell1,returnCode,reasonCode);
      memcpy((char*)cell1,"A load of crap",10);
      returnCode = iarcp64Delete(cpid,&reasonCode);
    }
  } else if (!strcmp(command,"stubs")){
    stubTest();
  } else{
    printf("unknown command\n");
  }
  return 0;
}




