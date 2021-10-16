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
#include "le.h"
#include "collections.h"
#include "recovery.h"


/* 
  LE 64 XPLINK Compile
  
  xlc -DHAVE_PRINTF=1 -DSTACK_INFO_TEST=1 -D_XOPEN_SOURCE=600 -D_OPEN_THREADS=1 -DSUBPOOL=132 "-Wc,LP64,longname,langlvl(extc99),gonum,goff,ASM,asmlib('CEE.SCEEMAC','SYS1.MACLIB','SYS1.MODGEN')" "-Wl,ac=1" -I ${COMMON}/h -I ${ZSS}/h -o smftest1 smftest1.c ${COMMON}/c/zosfile.c ${COMMON}/c/zos.c ${COMMON}/c/collections.c ${COMMON}/c/timeutls.c ${COMMON}/c/xlate.c ${COMMON}/c/scheduling.c ${COMMON}/c/recovery.c ${COMMON}/c/le.c ${COMMON}/c/logging.c ${COMMON}/c/utils.c ${COMMON}/c/alloc.c

  */

typedef void IFAFunction(void *, int *, int *);

#pragma linkage(IFAFunction,OS64_NOSTACK)
#pragma linkage(IFAMCON,OS)
#pragma linkage(IFAMDSC,OS)
#pragma linkage(IFAMGET,OS)
#pragma linkage(IFAMQRY,OS)

/*
  MVS Commands 
    D SMF,M    -- shows in-memory resources - which we have none of right now
    
    D SMF,O    -- shows options
                  which I believe come from LVL0.PARMLIB(SMFPRMSV) 
    
    D PARMLIB 
          1      S    VPMVSD  VENDOR.PARMLIB   
          2      S    VTMVSG  SVTSC.PARMLIB    
          3      S    VTLVL0  LVL0.PARMLIB       LVL0.PARMLIB(SMFPRMSV) - has our configuration - IEASYSxx has SMF=xx in it
          4      S    VIMVSB  SYS1.PARMLIB     

       note:  - The parmlib data sets and volume serial numbers that are defined in LOADxx.

    D IPLINFO  -- says how the system booted and notes the IEASYSxx
       
      - are there programmatic interfaces to what display does?

    D LOGGER,L  shows the log data streams  (where is the JCL that set these up)
    D LOGGER,IXGCNF - shows config

    D XCF,COUPLE,TYPE=LOGR  - shows more of the base configuration of LOGR in the plex
     
      My Dallas system puts many resources under COUPLE.** with COUPLE.PLOGR.CDS being the CDS for defining my LOGR stuff

  In-Memory depends upon logstream

  Logsteam depends upon sysplex LOGR config

     LOGR is plex-wide,                                           
       there are also LOGRY and LOGRZ single-system configs

     https://www-01.ibm.com/servers/resourcelink/svc00100.nsf/pages/zOSV2R4sa231399/$file/ieaf100_v2r4.pdf
     IXGLOGR is the Address Space on each system that manages it.

     SYS1.SAMPLIB(IXGLOGRP) has an example of the utility to manage LOGR
        calls utility: IXCMIAPU

        assumes utility IXCL1DSU has already been run.  Which sets up the LOGR CDS

     Log stream data sets are VSAM LDS
     
     There is a Redbook (http://www.redbooks.ibm.com/redbooks/pdfs/sg246898.pdf)

     In a DASD-only log stream, interim storage for log data is
     contained in a data space in the z/OS system. The data spaces are
     associated with the System Logger address space,
     IXGLOGR. DASD-only log streams can only be used by exploiters on
     one system at a time.

     Utility 
       SYSIN

       DEFINE LOGSTREAM NAME(STREAM3)   
         DASDONLY(YES)                  
         MAXBUFSIZE(40000)              

  Share presentation:

  Naming
    <HLQ|EHLQ>.stream-name.<plexname>

  1) LOGR IXCMIAPU  or macro IXGINVNT
        SAF authorization to MVSADMIN.LOGR Facility class needed, see redbook
        also need RESOURCE(log_stream_name) CLASS(LOGSTRM) permission
         
  2) alternate SMFPRMxxx

  3) IXGCONN requires SAF read to RESOURCE(log_stream_name) in CLASS(LOGSTRM)
  
  Routines Here

  Re-read SMF 99 and 98
     Workload Interaction Correlator
  
  CVT->196(CVT) CVTSMCA  (SMF CONTROL AREA)
  376(SMCA) -> some table
  LLGT 15,180(,15)
  L    15,20(,15)     Load address of IFAMQRY
  
  What do the resources look like?

    
  */

/*
  For Ed

  What are the presentations people have used.
  Kibana, Prometheus, Grafana

  Do one thing "make the data analyzable"? 

  What differentiates from Splunk that seems to make claims of all types.

  Kibana is only visualization or is this open, too

  Hypercubes as input, stream of hypercubes

  Is the key value statement to make a commodity level storage top-tier analytical lake?  

  Mainframe managers know if something is
     - totally out of whack
     - approaching capacity
  Don't know 
     - why
     - clustering or grouping for causility
     - what is normal, relative to operational and demand cycles.
  

  Logging on MVS has 
     textual Operlog
     binary  SMF
     system-level or subsystem -
     standard-modern (LogStream) (Customizaion Plugins, Called "exits" in ZOS)
  
   Thomas said "every day is a computer-science problem" 
      How much of the data-edge format (factored symbols and locality) is the core of what the company does as opposed to
      specific format ingest engines, adaptation and integration with query and analytical capabilities, and of course presentation
      capabilities. 

   Amazon S3 Select works when data is serialized JSON, CSV, or Parquet, but how does that fit in with DataEdge format??

   Extend the file formats to support mainframe types (don't really know the storage abstraction, and flexibliity, but hope it's 
    better than Elastic

   Patents talk about "data-edging" in cloud object storage and say it works for text or binary things such as images
   
   S3
      Bucket
          - owned by the AWS account that created and not transferrable
            create(name,region)  
            100 buckets by default, up to 1000 no perf-diff by having more or fewer
              name 3-63 characters of (lowercase,digit,hyphen,dot) start/end with alphanumeric, cannot be num and dots only
              dots should be considered deprecated, because some services do not allow dots
          - have ACL's on them
        Objects
          - have user-assigned keys
          - max 10 or 50 tags per object 128 chars per tag, 256 per value

   AWS Free Tier  
    - https://aws.amazon.com/free/?all-free-tier.sort-by=item.additionalFields.SortRank&all-free-tier.sort-order=asc&awsf.Free%20Tier%20Types=*all&awsf.Free%20Tier%20Categories=*all

   idea
     SMF to Parquet on amazon (then very queryable)
       which presentation / analytic layers are query-centric, rather than storage centric
     Amazon Athena may be better than S3 Select
     Are amazon Athena and S3 Select compatible at the table storage level??
 */ 

#pragma pack(packed)

typedef struct ConParmBlock_tag{
  char    eyecatcher[4]; /* CNPB */
  short   length;        /* of this block */
  char    reserved06;    /* must be 0 */
  char    version;       /* must be 1 */
  int     reserved08;    /* MBZ */
  short   nameLength;
  char    resourceName[26]; 
  /* OFFSET 0x28 */
  char    reserved28[16];
  char    token[16];      /* used for subsequent calls to IFAMGET */
} ConParmBlock;

typedef struct QueryParmBlock_tag{
  char     eyecatcher[4]; /* QRPB */
  short    length;        /* of this block */
  char     reserved06;    /* must be 0 */
  char     version;       /* must be 1 */
  short    flags;         /* 1 (0x4000) request return of extended types */
  short    unused0A;
  int      resourceCount;  /* output parameter */
  int      bufferSize;       /* size of output buffer to contian info for resources
                                RC=08, reason=0808 indicates insufficient space */
  int      unused14;
  uint64_t outputBufferAddress; /* mapped by SMFResourceEntry */

} QueryParmBlock;

typedef struct SMFResoureEntry_tag{
  short    nameLength;
  char     name[26];
  char     types[32];
  char     reserved[8];
} SMFResoureEntry;

#define IFAMGET_FLAGS_MULTIPLE      0x80
#define IFAMGET_FLAGS_RET_ON_32SUB5 0x20
#define IGAMGET_FLAGS_NO_WAIT       0x10   /* return immediately on no data available */

typedef struct GetParmBlock_tag{
  char     eyecatcher[4]; /* GTPB */
  short    length;        /* of this block */
  char     reserved06;    /* must be 0 */
  char     version;       /* must be 1 */
  int      flags;
  int      reserved;
  /* offset 0x10 */
  char     token[16];     /* from the IFAMCON Call */
  int      outputBufferLength;   /* must be >= 32768 */
  int      returnedDataSize;
  uint64_t outputBufferAddress;
} GetParmBlock;

typedef struct DiscParmBlock_tag{
  char     eyecatcher[4]; /* GTPB */
  short    length;        /* of this block */
  char     reserved06;    /* must be 0 */
  char     version;       /* must be 1 */
  char     token[16];     /* from IFAMCON */
} DiscParmBlock;

#pragma pack(reset)

static int queryTest(){
  QueryParmBlock parms;
  int returnCode;
  int reasonCode;
  int bufferLength = 0x10000;
  char *outputBuffer = safeMalloc(bufferLength,"QueryBuffer");
  
  memset(&parms,0,sizeof(parms));
  memcpy(parms.eyecatcher,"QRPB",4);
  parms.length = sizeof(parms);
  parms.version = 1;
  parms.bufferSize = bufferLength;
  parms.outputBufferAddress = (uint64_t)outputBuffer;
  
  
  CVT *cvt = getCVT();
  Addr31 smca = cvt->cvtsmca;
  int smcx = ((int*)smca)[376/4];
  printf("smcx = 0x%x\n",smcx);
  dumpbuffer((char*)smcx,0x100);
  int ifaRoutineVector = (((int*)smcx)[180/4])&0x7FFFFFFF;
  printf("ifaRoutineVector = 0x%x\n",ifaRoutineVector);
  dumpbuffer((char*)ifaRoutineVector,0x100);
  IFAFunction *ifamqry = (IFAFunction*)((int*)ifaRoutineVector)[20/4];
  printf("ifamqry at 0x%p\n",ifamqry);
  fflush(stdout);
  dumpbuffer((char*)ifamqry,0x200);
  ifamqry(&parms,&returnCode,&reasonCode);
  printf("ifamquery ret=0x%x reason=0x%x\n",returnCode,reasonCode);
  if (returnCode == 0){
    dumpbuffer(outputBuffer,0x800);
  }
}

int main(int argc, char **argv){

  queryTest();
  return 0;

}
