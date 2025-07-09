#include "fst.h"
#include "fst_utils.h"
#include <stdio.h>

#include <string>
#include <string.h>

#include <map>

static std::map<AEffect*, AEffectDispatcherProc>s_host2plugin;
static std::map<AEffect*, AEffectDispatcherProc>s_plugin2host;
static std::map<AEffect*, std::string>s_pluginname;
static AEffectDispatcherProc s_plug2host;

static
t_fstPtrInt host2plugin (AEffect* effect, int opcode, int index, t_fstPtrInt ivalue, void*ptr, float fvalue) {
  //post("%s(%d, %d, %lld, %p, %f)",  __FUNCTION__, opcode, index, ivalue, ptr, fvalue);
  switch(opcode) {
  case effGetVendorString:
    post("getVendorString");
    snprintf((char*)ptr, 16, "ProxyVendor");
    return 1;
  case effGetEffectName:
    post("getEffectName");
    snprintf((char*)ptr, 16, "ProxyEffect");
    return 1;
  case effCanBeAutomated:
    post("effCanBeAutomated: %d", index);
    return (index<5);
  case effCanDo: {
    typedef struct _cando {
      const char*ID;
      unsigned int res;
    } t_cando;
    t_cando candos[] = {
      //{"hasCockosExtensions", 0xbeef0000},
      //{"hasCockosEmbeddedUI", 0xbeef0000},
      //{"hasCockosNoScrollUI", 1 /* ? */ },
      //{"hasCockosSampleAccurateAutomation", 1 /* ? */ },
      //{"hasCockosViewAsConfig", 1 /* ? */ },
      //{"cockosLoadingConfigAsParameters", 1 /* ? */ },

      //{"receiveVstEvents", 1},
      //{"receiveVstMidiEvent", 1},
      //{"sendVstEvents", 1},
      //{"sendVstMidiEvent", 1},
      //{"supportsGainReductionInfo", 1},
      //{"wantsChannelCountNotifications", 1},

      {0, 0} /* END */
    };
    t_cando*cando;
    post("canDo '%s'?", (char*)ptr);
    if(!ptr) return 0;
    for(cando=candos; cando->ID; cando++) {
      if(!strcmp((char*)ptr, cando->ID)) {
	post("canDo '%s': %d", cando->ID, cando->res);
        return cando->res;
      }
    }
  }
    break;
  case effSetSpeakerArrangement:
    post("SetSpeakerArrangment: %d, %lld, %p, %f", index, ivalue, ptr, fvalue);
    break;
  case effVendorSpecific:
    post("host2plugin('%s', effVendorSpecific, %s, %s, %p, %f)",
             s_pluginname[effect].c_str(),
             int2str(index),
             int2str2(ivalue),
             ptr, fvalue);
    post_hex(ptr, 256);
    break;
  case 56:
#if 0
    post("OPCODE56");
    post_hex(ptr, 256);
    return dispatch_effect("???", s_host2plugin[effect], effect, opcode, index, ivalue, 0, fvalue);
#endif
    break;
  case 62:
    return 0;
    post("OPCODE62?");
    post_hex(ptr, 256);
      // >=90: stack smashing
      // <=85: ok
    snprintf((char*)ptr, 85, "JMZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");
  case effGetMidiKeyName: {
#if 0
    char*cptr = (char*)ptr;
    int*iptr = (int*)ptr;
    post("effGetMidiKeyName: %3d %3d", iptr[0], iptr[1]);
    if (60 == iptr[1])
      snprintf((char*)ptr+8, 52, "middle C", iptr[1]);
    else
      snprintf((char*)ptr+8, 52, "note:#%d", iptr[1]);
    #endif
  }
    //return 1;
  default:
    break;
  }
  AEffectDispatcherProc h2p = s_host2plugin[effect];
  if(!h2p) {
    post("host2plugin:: NO CALLBACK!");
    return 0xDEAD;
  }
  const char*pluginname = 0;
  if(effect)
    pluginname = s_pluginname[effect].c_str();

  bool doPrint = true;
#ifdef FST_EFFKNOWN
  doPrint = !effKnown(opcode);
#endif
  switch(opcode) {
  default: break;
  case 56:
  case effIdle:
  case effGetChunk: case effSetChunk:
    doPrint = false;
    break;
  case effVendorSpecific:
    //doPrint = false;
    break;
  }
  t_fstPtrInt result = 0;
  if(doPrint) {
    dispatch_effect(pluginname, h2p, effect, opcode, index, ivalue, ptr, fvalue);
  } else {
    result = h2p(effect, opcode, index, ivalue, ptr, fvalue);
  }
  switch(opcode) {
  default: break;
  case 56:
    post_hex(ptr, 256);
    break;
  case 62:
    post("OPCODE62!");
    post_hex(ptr, 256);
  }
  return result;
}
static
t_fstPtrInt host2plugin_wrapper (AEffect* effect, int opcode, int index, t_fstPtrInt ivalue, void*ptr, float fvalue)
{
  t_fstPtrInt result;
  fflush(stdout);
  result = host2plugin(effect, opcode, index, ivalue, ptr, fvalue);
  fflush(stdout);

  return result;
}
static
t_fstPtrInt plugin2host (AEffect* effect, int opcode, int index, t_fstPtrInt ivalue, void*ptr, float fvalue) {
  //post("%s:%d", __FUNCTION__, opcode); fflush(stdout);
  AEffectDispatcherProc p2h = s_plugin2host[effect];
  if(!p2h)p2h = s_plug2host;
  if(effect && !s_host2plugin[effect]) {
    s_host2plugin[effect] = effect->dispatcher;
    effect->dispatcher = host2plugin_wrapper;
  }
  const char*pluginname = 0;
  if(effect)
    pluginname = s_pluginname[effect].c_str();

  bool doPrint = true;
#ifdef FST_HOSTKNOWN
  doPrint = !hostKnown(opcode);
#endif
  switch(opcode) {
  default: break;
  case audioMasterNeedIdle:
    post("audioMasterNeedIdle");
    //return 1;
    break;
  case audioMasterUpdateDisplay:
    post("audioMasterUpdateDisplay");
    //return 0;
    break;
  case audioMasterGetCurrentProcessLevel:
  case audioMasterGetTime:
    doPrint = false;
    break;
  case (int)0xDEADBEEF:
    post("0xDEADBEEF");
  }
  t_fstPtrInt result = -1;

  fflush(stdout);
  if(doPrint) {
    if(0xDEADBEEF ==opcode) {
      unsigned int uindex = (unsigned int) index;
      switch(uindex) {
      case 0xDEADF00D:
        post("\t0x%X/0x%X '%s' ->", opcode, index, ptr);
        break;
      default:
        post("\t0x%X/0x%X ->", opcode, index);
        break;
      }
    }

    result = dispatch_host(pluginname, p2h, effect, opcode, index, ivalue, ptr, fvalue);
  } else {
    result = p2h(effect, opcode, index, ivalue, ptr, fvalue);
  }
  fflush(stdout);
  return result;
}


extern "C"
AEffect*VSTPluginMain(AEffectDispatcherProc dispatch4host) {
  char pluginname[512] = {0};
  char*pluginfile = getenv("FST_PROXYPLUGIN");
  print_init();
  post("FstProxy: '%s'", pluginfile);
  if(!pluginfile)return 0;
  s_plug2host = dispatch4host;

  t_fstMain*plugMain = fstLoadPlugin(pluginfile);
  if(!plugMain)return 0;

  AEffect*plug = plugMain(plugin2host);
  if(!plug)
    return plug;

  post("plugin.dispatcher '%p' -> '%p'", plug->dispatcher, host2plugin_wrapper);
  if(plug->dispatcher != host2plugin_wrapper) {
    s_host2plugin[plug] = plug->dispatcher;
    plug->dispatcher = host2plugin_wrapper;
  }

  s_host2plugin[plug](plug, effGetEffectName, 0, 0, pluginname, 0);
  if(*pluginname)
    s_pluginname[plug] = pluginname;
  else
    s_pluginname[plug] = pluginfile;

  s_plugin2host[plug] = dispatch4host;
  post_aeffect(plug);
  fflush(stdout);
  return plug;
}
