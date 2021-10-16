#include <metal/metal.h>
#include <metal/stddef.h>
#include <metal/stdio.h>
#include <metal/string.h>
#include <metal/stdbool.h>

#include "zowetypes.h"
#include "alloc.h"
#include "utils.h"
#include "collections.h"
#include "cmutils.h"
#include "crossmemory.h"
#include "shrmem64.h"
#include "zis/plugin.h"
#include "zis/server.h"
#include "zis/service.h"
#include "authservices.h"
#include "streameddefs.h"

/*
  make routine visible in zis plugin data

  meld SRB trampoline ideas with existing exit.
    - hide the R13 from exit caller in the low bytes instead of the STMG, because STM will cover all others
    - when restoring carry the 15 back from metal all the way back

  simple code can find exitDispatcher, but smarter code will find ZVT/ZVTE/CMSGA/ZISANCHOR/INZPLUGIN each time
  (that's if exit installation is on longer lifecycle than the inzpect server)

  memory is in Private for exit 31 
    eventually a separate PC-cp for handing out common-memory key 0 stacks for routines
    it's too crazy to cover all cases, and overhead for shared is bad
  
  HERE:
    IEFU84 and IEFU85
    queing will be simple enough in MetalC 
    Verify 83,84 for more record types with sprintf into scratch page

    buffer manager and q - with absolute limits

     

  Maybe the shared pools should be COMMON key 0 or 4 to avoid addressability games and the overhead per exit call.
 */

static int exitDispatcher(ExitIdentification *exitIdentification,
			  char *debugScratchPage,     /* if non-zero can help with debugging */
			  void *exitR0,               /* see doc for each exit */
			  void *exitR1){              /* see doc for each exit - most put everything here */
  memcpy(debugScratchPage+0x80,"INMETALNOW",10);
  memcpy(debugScratchPage+0xA0,(char*)exitR1,0x30);
}


  /* to initialize global data,  

     at this point in time plugin->services array will have the N services made in getPluginDescriptor()
     
     and at this point each service will have a service ancho
     */

typedef struct StreamedPluginData_tag{
  uint64_t initTime;
  Addr31   exitDispatcherAddress;
  uint64_t smfCellPoolID;
  /* 40 bytes free 
     stacks, PC numbers, etc 
     
     Transactional memory */

} StreamedPluginData;

static int initStreamedData(struct ZISContext_tag *context,
			    ZISPlugin *plugin,
			    ZISPluginAnchor *anchor) {
  StreamedPluginData *pluginData = (StreamedPluginData *)&anchor->pluginData;

  __asm(" STCK %0 " : "=m"(pluginData->initTime) :);
  
  uint64_t pswLow = 0;
  __asm(ASM_PREFIX
	" BASR 15,0 \n"
	" STG  15,%0"
	:"=m"(pswLow)
	::"r15");
  

  memcpy(anchor->reserved1,"JOEWUZHERE01",12);
  pluginData->exitDispatcherAddress = (void*)exitDispatcher;

  zowelog(NULL, LOG_COMP_STCBASE, ZOWE_LOG_INFO, "Initializing InZpect plugin (Mk3) anchor=0x%p pluginData=0x%p executing at 0x%p\n",
	 anchor,pluginData,pswLow);

  int returnCode = 0;
  int reasonCode = 0;

  uint64_t cpid = iarcp64Create(false,32768,&returnCode,&reasonCode);
  zowelog(NULL, LOG_COMP_STCBASE, ZOWE_LOG_INFO, "iarcp64 cpid=0x%llx ret=0x%x, reason=0x%x\n",cpid,returnCode,reasonCode);
  
  if (cpid && !returnCode){
    pluginData->smfCellPoolID;
  }
  

  return RC_ZIS_PLUGIN_OK;
}

static int termStreamedData(struct ZISContext_tag *context,
			    ZISPlugin *plugin,
			    ZISPluginAnchor *anchor) {

  StreamedPluginData *pluginData = (StreamedPluginData *)&anchor->pluginData;

  pluginData->initTime = -1;

  return RC_ZIS_PLUGIN_OK;
}

static int handleStreamedDataCommands(struct ZISContext_tag *context,
				      ZISPlugin *plugin,
				      ZISPluginAnchor *anchor,
				      const CMSModifyCommand *command,
				      CMSModifyCommandStatus *status) {
  
  if (command->commandVerb == NULL) {
    return RC_ZIS_PLUGIN_OK;
  }

  if (command->argCount != 1) {
    return RC_ZIS_PLUGIN_OK;
  }

  if (!strcmp(command->commandVerb, "D") ||
      !strcmp(command->commandVerb, "DIS") ||
      !strcmp(command->commandVerb, "DISPLAY")) {

    if (!strcmp(command->args[0], "STATUS")) {

      StreamedPluginData *pluginData = (StreamedPluginData *)&anchor->pluginData;

      /* We can use zowelog but I don't want to link with a lot of unnecessary
       * object files.  */
      cmsPrintf(&context->cmServer->name,
                "ZOSS Streamed Data plug-in v%d - anchor = 0x%p, init TOD = %16.16llX\n",
                plugin->pluginVersion, anchor, pluginData->initTime);

      *status = CMS_MODIFY_COMMAND_STATUS_CONSUMED;
    }

  }

  return RC_ZIS_PLUGIN_OK;
}

ZISPlugin *getPluginDescriptor() {
  ZISPluginName pluginName = {.text = "ZOSSSTREAMEDDATA"};
  ZISPluginNickname pluginNickname = {.text = "ZSTR"};

  ZISPlugin *plugin = zisCreatePlugin(pluginName, pluginNickname,
                                      initStreamedData,  /* function */
				      termStreamedData, 
				      handleStreamedDataCommands,
                                      1,  /* version */
				      0,  /* service count */
				      ZIS_PLUGIN_FLAG_LPA);
  if (plugin == NULL) {
    return NULL;
  }

  return plugin;
}

