#include "fst.h"
#include "fst_utils.h"
#include <stdio.h>

#ifdef _WIN32
# include <windows.h>
#else
# include <dlfcn.h>
#endif

#include <string>
#include <string.h>

char effectname[1024];

float db2slider(float f) {
  return f;
}
float slider2db(float f) {
  return f;
}

static size_t curOpCode = -1;

t_fstPtrInt dispatch (AEffect* effect, int opcode, int index, t_fstPtrInt ivalue, void*ptr, float fvalue) {
  if(effect) {
    effect->resvd2 = opcode;
    return effect->dispatcher(effect, opcode, index, ivalue, ptr, fvalue);
  }
  return 0xDEAD;
}
t_fstPtrInt dispatch_v (AEffect* effect, int opcode, int index, t_fstPtrInt ivalue, void*ptr, float fvalue) {
  return dispatch_effect(effectname, effect->dispatcher, effect, opcode, index, ivalue, ptr, fvalue);
}

t_fstPtrInt dispatch_v1 (AEffect* effect, int opcode, int index, t_fstPtrInt ivalue, void*ptr, float fvalue) {
  if(effect) {
    char opcodestr[256];
    t_fstPtrInt result = effect->dispatcher(effect, opcode, index, ivalue, ptr, fvalue);
    if(result)
      post("AEffect.dispatch(%s, %s, %d, %lu, %p, %f) => %d"
             , effectname
             , effCode2string(opcode, opcodestr, 255), index, ivalue, ptr, fvalue
             , result
        );
    return result;
  }
  return 0xDEAD;
}

t_fstPtrInt dispatch_v0 (AEffect* effect, int opcode, int index, t_fstPtrInt ivalue, void*ptr, float fvalue) {
  if(effect) {
    char opcodestr[256];
    t_fstPtrInt result = effect->dispatcher(effect, opcode, index, ivalue, ptr, fvalue);
    post("AEffect.dispatch(%s, %s, %d, %lu, %p, %f) => %d"
           , effectname
           , effCode2string(opcode, opcodestr, 255), index, ivalue, ptr, fvalue
           , result
      );
    return result;
  }
  return 0xDEAD;
}


t_fstPtrInt dispatcher (AEffect* effect, int opcode, int index, t_fstPtrInt value, void*ptr, float opt) {
  char sbuf[256] = {0};
  post("FstHost::dispatcher[%d]", effect?effect->resvd2:-1);
  post("\t(%s, %d, %d, %p, %f)\n",
      hostCode2string(opcode, sbuf, 255),
      index, (int)value,
      ptr, opt);
  if(ptr) {
    char *str=(char*)ptr;
    if(*str) {
      post("\t'%.*s'", 512, str);
    } else
      post("\t<nil>");
  }
  switch(opcode) {
  case (int)(0xDEADBEEF):
    return (t_fstPtrInt)db2slider;
  case audioMasterCurrentId:
    return 0xDEAD;
  case audioMasterVersion:
    post("MasterVersion");
    return 2400;
  case audioMasterAutomate:
    post("automate parameter[%d] to %f", index, opt);
    break;
  case audioMasterGetProductString:
    for(size_t i=0; i<kVstMaxProductStrLen; i++) {
      ((char*)ptr)[i] = 64+i%60;
    }
    strncpy((char*)ptr, "FstProduct?", kVstMaxProductStrLen);
    return 1;
    break;
  case audioMasterGetVendorString:
    strncpy((char*)ptr, "FstVendor?", kVstMaxVendorStrLen);
    return 1;
  case 42:
    return 0;
  default:
    post("\tdyspatcher(%p, %d, %d, %d, %p, %f);", effect, opcode, index, value, ptr, opt);
    //post("(%p, %x, %x, %d, %p, %f);", effect, opcode, index, value, ptr, opt);
    do {
      char *str=(char*)ptr;
      if(str && *str) {
        post("\t'%.*s'", 512, str);
      }
    } while(0);
    break;
  }
  return 0;
}

void test_effCanDo(AEffect*effect, const char*feature) {
  dispatch_v(effect, effCanDo, 0, 0, (void*)feature, 0.000000);
}

void test_setSpeakerArrangement(AEffect*effect) {
  VstSpeakerArrangement*iarr=(VstSpeakerArrangement*)calloc(1, sizeof(VstSpeakerArrangement));
  VstSpeakerArrangement*oarr=(VstSpeakerArrangement*)calloc(1, sizeof(VstSpeakerArrangement));
  dispatch_v(effect, effSetSpeakerArrangement, 0, (t_fstPtrInt)iarr, oarr, 0.);
}
void test_getSpeakerArrangement(AEffect*effect) {
  VstSpeakerArrangement*iarr=0, *oarr=0;
  dispatch_v(effect, effGetSpeakerArrangement, 0, (t_fstPtrInt)(&iarr), &oarr, 0.);
  post("gotSpeakerArrangements: %p %p: %d", iarr, oarr, (char*)oarr-(char*)iarr);
}
void test_SpeakerArrangement1(AEffect*effect) {
//  test_setSpeakerArrangement(effect);
  test_getSpeakerArrangement(effect);
}

void test_opcode3334(AEffect*effect) {
  size_t opcode = 0;
  VstPinProperties vpp;
  t_fstPtrInt res = 0;
  for(int chan=0; chan<effect->numInputs; chan++) {
    memset(&vpp, 0, sizeof(vpp));
    opcode = effGetInputProperties;
    post("trying: %d [channel:%d]", opcode, chan);
    res = dispatch (effect, opcode, chan, 0, &vpp, 0);
    post("returned %lu", res);
    if(res)
      post_pinproperties(&vpp);
  }
  for(int chan=0; chan<effect->numOutputs; chan++) {
    opcode = effGetOutputProperties;
    memset(&vpp, 0, sizeof(vpp));
    post("trying: %d [channel:%d]", opcode, chan);
    res = dispatch (effect, opcode, chan, 0, &vpp, 0);
    post("returned %lu", res);
    if(res) {
      post_pinproperties(&vpp);
      //post_hex(&vpp, sizeof(vpp));
    }
  }
}

static float test_setParameterS(AEffect*effect, size_t opcode, int index, char*str) {
  dispatch_v(effect, opcode, index, 0, str, 0.f);
  return effect->getParameter(effect, index);
}

static void test_setParameter(AEffect*effect) {
  int index = 0;
  float value = 0.666;
  post("testing get/set Parameters for %p..................", effect);
  effect->setParameter(effect, index, value);
  post("setParameter(%d, %f)", index, value);
  value = effect->getParameter(effect, index);
  post("getParameter(%d) -> %f", index, value);
  for(size_t opcode = 9; opcode < 65536; opcode++) {
    if(effEditOpen==opcode)continue;
    if(effProcessEvents==opcode)continue;
    if(42==opcode)continue;
    if(69==opcode)continue;
    char buf[512];
    snprintf(buf, 511, "%s", "0.2");
    buf[511] = 0;
    post("\ttesting#%d", opcode);
    value = test_setParameterS(effect, opcode, 0, buf);
    post("\t#%d: '%s' -> %f", opcode, buf, value);
  }
}

static void test_opcode42(AEffect*effect) {
  VstSpeakerArrangement setarr[10], getarr[10];
  for(size_t i=0; i<10; i++) memset(setarr+i, 0, sizeof(VstSpeakerArrangement));
  setarr[0].type = setarr[1].type = 0x1;
  setarr[0].numChannels = 2;
  setarr[1].numChannels = 2;
  dispatch_v(effect, 42, 0, (t_fstPtrInt)(setarr+0), (setarr+1), 0.f);
  post_hex(setarr+0, 8);
  post_hex(setarr+1, 8);

  post("-----------------------------");
  for(size_t opcode=69; opcode<70; opcode++) {
    VstSpeakerArrangement *arrptr[2] = {0,0};
    if(42 == opcode)continue;
    if(effGetEffectName==opcode)continue; if(effGetVendorString==opcode)continue; if(effGetProductString==opcode)continue;
    dispatch_v(effect, opcode, 0, (t_fstPtrInt)(arrptr+0), arrptr+1, 0.f);
    post_hex(arrptr[0], 8);
    post_hex(arrptr[1], 8);
  }
}

bool skipOpcodePtr(size_t opcode) {
  switch(opcode) {
  default: break;
  case effProcessEvents:
  case effEditOpen:
  case effSetSpeakerArrangement:
  case effGetSpeakerArrangement:
    return true;
  }
  return false;
}

bool skipOpcode(size_t opcode) {

  switch(opcode) {
  case 1:
#if PLUGIN_JUCE || PLUGIN_DIGITS
    /* Digits plugin & JUCE plugins */
  case 3:
#endif
    //case 4:

    //case 5: /* program name [?] */

    //case 10: /* AM_AudioMan::reset() */
    //case 11: /* blocksize */
    //case 12: /* AM_VST_base::suspend () */
  case 13: /* AM_VST_Editor::getRect 1200 x 600 */
  case 14: /* AM_VST_Editor::open, exits with 1! */
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 29:
  case 33:
  case 34:
  case 35:
#if PLUGIN_JUCE
    /* JUCE plugins */
  case 42:
  case 44:
#endif
  case 45:
  case 47:  /* AM_VST_base::getVendorString (char* text) */
  case 48:
  case 49:
  case 51:
  case 58:
    //case 59: /* u-he plugin doesnt use key, returns false (VST) or jumps to default key handler (WindowProc) */
  case 63:
  case 69:
  case 71:
  case 72:
    //case 73:
    return true;
  default: break;
  }
  return false;
}

void test_opcodes(AEffect*effect, size_t endopcode=78, size_t startopcode=10) {
  post("testing dispatcher");
#if 0
  for(size_t opcode=0; opcode < 10; opcode++) {
  for(int i=0; i<effect->numPrograms; i++) {
   char buffer[512] = { 0 };
    t_fstPtrInt res = dispatch (effect, opcode, i, i, buffer, i);
    const char*str = (const char*)res;
    post("program#%d[%d=0x%X]: %.*s", i, res, res, 32, str);
    if(*buffer)
      post("\t'%.*s'", 512, buffer);
  }
  }
  return 0;
#endif
  //  endopcode = 0xDEADF00D+1;
  for(size_t opcode=startopcode; opcode<endopcode; opcode++) {
    post("testing %s [%d..%d]", effCode2string(opcode), startopcode, endopcode);
    curOpCode = opcode;
    if(!(opcode%65536)) {
      post("=== mark %d ===", opcode>>16);
    }
#if 1
    if(skipOpcodePtr(opcode))
       continue;
#endif
    if (skipOpcode(opcode)
#ifdef NOSKIP
        && (NOSKIP != opcode)
#endif
        ) {
      post("skipping: %d", opcode);
      continue;
    }
    //post("trying: %d", opcode);
    char buffer[200000] = { 0 };
    snprintf(buffer, 511, "name%d", opcode);

    int index = 0;
    t_fstPtrInt ivalue = 0;
    void*ptr=buffer;
    float fvalue = 0.f;

    post("host2plugin('%s', '%s', %s, %s, %p, %f)",
	 effectname, effCode2string(opcode),
	 int2str(index),
	 int2str2(ivalue),
	 ptr, fvalue);
    t_fstPtrInt res = dispatch (effect, opcode, index, ivalue, ptr, fvalue);
    post("host2plugin: %s (%s)", int2str(res), effCode2string(opcode));

#if 0
    if(res) {
      const char*str = (const char*)res;
      post("\t[%s]: %.*s", int2str(res), 32, str);
    }
#endif
    if(*buffer)
      post("\tbuffer '%.*s'", 512, buffer);
    switch(opcode) {
    default: break;
    case 4:
      post_hex(buffer, 16);
    }
    fstpause(0.2);
  }
  do {
    char buffer[200000] = { 0 };
    t_fstPtrInt res = dispatch (effect, 5, 0, 0, buffer, 0);
    post("gotProgName: %.*s", 20, buffer);
  } while(0);
  post("tested dispatcher with opcodes %u..%u", startopcode, endopcode);
}

bool skipOpcodeJUCE(size_t opcode) {
  switch(opcode) {
  default: break;
  case 2:
  case 3:
  case 4:
  case 5:
  case 13: case 14: case 15:
  case 42:
    return true;
  }
  return false;
}

void test_SpeakerArrangement0(AEffect*effect) {
  VstSpeakerArrangement*arr[2];
  dispatch_v(effect, effGetSpeakerArrangement, 0,
           (t_fstPtrInt)(&arr[0]), &arr[1], 0.);
  post("input : %p", arr[0]);
  post("output: %p", arr[1]);
}

void test_setChunk(AEffect*effect) {
  t_fstPtrInt data=0;
  t_fstPtrInt size=0;
  int index = 0;
  /* get data */
  size = dispatch(effect, effGetChunk, index, 0, &data, 0.f);
  post("index#%d: got %d bytes @ 0x%X", index, size, data);

  index = 1;
  size = dispatch(effect, effGetChunk, index, 0, &data, 0.f);
  post("index#%d: got %d bytes @ 0x%X", index, size, data);

  index = 0;
  t_fstPtrInt result = dispatch(effect, effSetChunk, index, size, &data, 0.f);
  post("index#%d: setChunk[%d] returned %lu", index, (int)effSetChunk, result);
  size = dispatch(effect, effGetChunk, index, 0, &data, 0.f);
  post("index#%d: got %d bytes @ 0x%X", index, size, data);
}

void test_opcode23(AEffect*effect) {
  size_t opcode = 23;
  int index = 0;

  t_fstPtrInt*buffer[8] = {0};
  post("testing OP:%d", opcode);
  t_fstPtrInt result = dispatch(effect, opcode, index, 0, buffer, 0.f);
  post("\tresult |\t%lu 0x%lX", result, result);
  if(*buffer) {
    post("\tbuffer '%.*s'", 512, (char*)*buffer);
  }
  //post_hex(*buffer, result);
}

void test_opcode56(AEffect*effect) {
  size_t opcode = 56;
  const size_t bufsize = 1024;
  char*buffer[bufsize] = {0};
  for(size_t i=0; i<sizeof(bufsize); i++) {
    buffer[i] = 0;
  }

  post("testing OP:%d", opcode);
  t_fstPtrInt result = dispatch_v(effect, opcode, 0, 0, (void*)0x1, 0.f);
  post("\tresult |\t%lu 0x%lX", result, result);
  if(*buffer) {
    post("\tbuffer '%.*s'", bufsize, (char*)buffer);
  }
  post_hex(buffer, bufsize);
}

void test_opcode29(AEffect*effect) {
  for(int i=0; i<effect->numPrograms; i++) {
    size_t opcode = 29; //effGetProgramNameIndexed;
    char buffer[200] = { 0 };
    t_fstPtrInt result = dispatch(effect, opcode, i, 0, buffer, 0.f);
    post("opcode:%d index:%d -> %lu", opcode, i, result);
    if(*buffer) {
      post("\tbuffer '%.*s'", 512, buffer);
    }
  }
}

void test_opcodesJUCE(AEffect*effect) {
 for(size_t opcode=16; opcode<63; opcode++) {
    if(skipOpcodeJUCE(opcode))continue;
    char buffer[200] = { 0 };
    t_fstPtrInt result = dispatch(effect, opcode, 0, 0, buffer, 0.f);
    if(result || *buffer)
      printf("FST::tested %d", opcode);
    if(result)
      printf("\t|\t%lu 0x%lX", result, result);
    if(*buffer) {
      printf("\t|\tbuffer '%.*s'", 512, buffer);
      //post_hex(buffer, 16);
    }
    if(result || *buffer)
      post("");
  }
}
void test_opcode25(AEffect*effect) {
  const unsigned char midi[4] = {0x90, 0x40, 0x7f, 0};
  VstEvents*ves = create_vstevents(midi);
  //post_events(ves);
  dispatch_v(effect, effProcessEvents, 0, 0, ves, 0.);
}
static float** makeFSamples(size_t channels, size_t frames) {
  float**samples = new float*[channels];
  for(size_t i=0; i<channels; i++) {
    samples[i] = new float[frames];
    float*samps=samples[i];
    for(size_t j=0; j<frames; j++) {
      samps[j] = 0.f;
    }
  }
  return samples;
}


void test_unknown(AEffect*effect) {
  char opcodestr[256];
  fstpause(0.5);
  int index = 0;
  t_fstPtrInt ivalue = 0;
  void*ptr = 0;
  float fvalue = 0;
  for(t_fstPtrInt opcode=2; opcode<128; opcode++) {
    if(effKnown(opcode))
      continue;
    dispatch_v1(effect, opcode, index, ivalue, ptr, fvalue);
    fstpause(0.01);
  }

  fstpause(0.5);
}



void test_reaper(AEffect*effect) {
  t_fstPtrInt ret=0;
  char strbuf[1024];
  const int blockSize = 512;
  float**insamples = makeFSamples(effect->numInputs, blockSize);
  float**outsamples = makeFSamples(effect->numOutputs, blockSize);

  post("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv");
  dispatch_v(effect, effGetPlugCategory, 0, 0, 000, 0.000000);
  dispatch_v(effect, effSetSampleRate, 0, 0, 000, 44100.000000);
  dispatch_v(effect, effSetBlockSize, 0, blockSize, 000, 0.000000);

  strbuf[0]=0;
  dispatch_v(effect, effGetEffectName, 0, 0, strbuf, 0.000000);
  strbuf[0] = 0;
  dispatch_v(effect, effGetVendorString, 0, 0, strbuf, 0.000000);

  test_effCanDo(effect, "hasCockosNoScrollUI");
  test_effCanDo(effect, "wantsChannelCountNotifications");
  //test_opcode42(effect);
  test_effCanDo(effect, "hasCockosExtensions");

  dispatch_v(effect, effGetVstVersion, 0, 0, 000, 0.000000);
  dispatch_v(effect, effMainsChanged, 0, 1, 000, 0.000000);
  dispatch_v(effect, effStartProcess, 0, 0, 000, 0.000000);

  test_effCanDo(effect, "receiveVstEvents");
  test_effCanDo(effect, "receiveVstMidiEvents");

  dispatch_v(effect, effGetPlugCategory, 0, 0, 000, 0.000000);

  test_effCanDo(effect, "sendVstEvents");
  test_effCanDo(effect, "sendVstMidiEvents");

  dispatch_v(effect, effGetProgram, 0, 0, 000, 0.000000);

  ret = dispatch(effect, effGetChunk, 0, 0, strbuf, 0.000000);
  dispatch_v(effect, effSetProgram, 0, 1, 000, 0.000000);
  strbuf[0] = 0;
  dispatch_v(effect, effGetProgramName, 0, 0, strbuf, 0.000000);
  fflush(stderr);  fflush(stdout);
//  fstpause(2.);
  test_opcode56(effect);

  dispatch_v(effect, effGetProgram, 0, 0, 000, 0.000000);

  post("=============PROC==============================");
  effect->processReplacing(effect, insamples, outsamples, blockSize);
  test_opcode25(effect);
  effect->processReplacing(effect, insamples, outsamples, blockSize);
  post("==============================================");
  dispatch_v(effect, effMainsChanged, 0, 0, 000, 0.000000);
  dispatch_v(effect, effMainsChanged, 0, 1, 000, 0.000000);
  post("==============================================");
  dispatch_v(effect, effMainsChanged, 0, 0, 000, 0.000000);
  dispatch_v(effect, effMainsChanged, 0, 1, 000, 0.000000);
  dispatch_v(effect, effMainsChanged, 0, 0, 000, 0.000000);
  dispatch_v(effect, effMainsChanged, 0, 1, 000, 0.000000);
  dispatch_v(effect, effStopProcess, 0, 0, 000, 0.000000);
  dispatch_v(effect, effMainsChanged, 0, 0, 000, 0.000000);
  post("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
}

int test_plugin(const char*filename) {
  t_fstMain*vstmain = fstLoadPlugin(filename);
  if(!vstmain)return post("'%s' was not loaded", filename);
  AEffect*effect = vstmain(&dispatcher);
  post("instantiated effect %p", effect);
  snprintf(effectname, 1024, "%s", filename);
  if(!effect)return post("unable to instantiate plugin from '%s'", filename);
  post_aeffect(effect);
  //dump_data(filename, effect, 160);
  if(effect->magic != 0x56737450) return printf("magic failed: 0x%08X", effect->magic);
  dispatch_v(effect, effOpen, 0, 0, 000, 0.000000);
  //post_aeffect(effect);
  //test_reaper(effect); return 0;
  //test_opcode29(effect); return 0;
  test_opcode3334(effect);
  //test_opcodesJUCE(effect);
  test_opcodes(effect, 129, 9);

  //test_unknown(effect);
  test_SpeakerArrangement1(effect);
  dispatch_v(effect, effClose, 0, 0, 000, 0.000000);
  return 0;
}

int usage(const char*progname) {
  fprintf(stderr, "usage: %s <pluginfile> [<pluginfile> ...]\n", progname);
  fprintf(stderr, ""
          "\n"
          "\tattempts to load each <pluginfile> as a plugin\n"
          "\tand runs some basic tests\n"
          "\n"
    );
  return 1;
}

int main(int argc, const char*argv[]) {
  if(argc <= 1)
    return usage(argv[0]);
  for(int i=1; i<argc; i++) {
    test_plugin(argv[i]);
  }
  return 0;
}
