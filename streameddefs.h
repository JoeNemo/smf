#ifndef __ZOSSTEAM_ZISSHARED__
#define __ZOSSTEAM_ZISSHARED__ 1

typedef struct ExitIdentification_tag{
  char eyecatcher[8];      /* EXITIDNT */
  void *metalRoutine;      /* the receiving routine, so the exit itself can find Metal Routine */
  char zisServerName[16];
  char pluginName[16];     /* for extra safety */
  char exitName[16];       /* These are IBM-defined for CSVDYNEX */
} ExitIdentification;


#endif
