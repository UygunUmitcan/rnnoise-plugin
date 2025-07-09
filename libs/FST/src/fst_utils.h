#ifndef FST_FST_UTILS_H_
#define FST_FST_UTILS_H_

#include "fst.h"
#include <stdarg.h>

#ifdef _MSC_VER
# define PERTHREAD __declspec(thread)
#else
# define PERTHREAD __thread
#endif /* _MSC_VER */


#ifdef _WIN32
# include <windows.h>
#else
# include <dlfcn.h>
#endif

#include <stdio.h>
#include <string>

#include <unistd.h>
static void fstpause(float duration=1.0) {
  usleep(duration * 1000000);
}

#define FOURCC(fourcc) ((fourcc[0]<<24) | (fourcc[1]<<16) | (fourcc[2]<< 8) | (fourcc[3]<< 0))

static FILE*vprint_out = 0;

void print_init() {
  static int x = 0;
  x++;
  const char*logfile = getenv("FST_LOGFILE");

  if(logfile) {
    fprintf(stderr, "print_init '%s'\n", logfile);
    vprint_out=fopen(logfile, "a");
    fprintf(vprint_out, "print_init '%s'@%d\n", logfile, x);
  } else
    vprint_out=stdout;
}
void print_exit() {
  if(vprint_out != stdout) {
    fprintf(stderr, "print_exit\n");
    fprintf(vprint_out, "print_exit\n");
    fclose(vprint_out);
  }
}

int vprint(const char *format, va_list args_) {
   va_list args;
   int result = 0;
   FILE*out=0;
   if(!vprint_out)
     print_init();
   out = vprint_out;
   if(!out)
     out = stdout;
   va_copy(args, args_);

   result+=vfprintf(vprint_out, format, args);
   va_end(args);
   return result;
}

int print(const char *format=0, ...) {
   va_list args;
   int result = 0;

   va_start(args, format);
   result+=vprint(format, args);
   va_end(args);
   return result;
}

static const char*fourcc2str(unsigned int fourcc, char buf[4]) {
  buf[0] = (fourcc >> 24) & 0xFF;
  buf[1] = (fourcc >> 16) & 0xFF;
  buf[2] = (fourcc >>  8) & 0xFF;
  buf[3] = (fourcc >>  0) & 0xFF;
  return buf;
}
static const char*fourcc2str(unsigned int fourcc) {
  static PERTHREAD char buf[5];
  buf[4] = 0;
  return fourcc2str(fourcc, buf);
}

static const char*int2str(t_fstPtrInt ivalue, char*output, size_t length) {
  if(ivalue>0xFFFFFFFF) {
    snprintf(output, length, "0x%X", ivalue);
  } else {
    snprintf(output, length, "%llu=0x%X", ivalue, ivalue);
  }
  return output;
}

static const char*int2str(t_fstPtrInt ivalue) {
  static PERTHREAD char buffer[1024];
  return int2str(ivalue, buffer, 1024);
}
static const char*int2str2(t_fstPtrInt ivalue) {
  static PERTHREAD char buffer[1024];
  return int2str(ivalue, buffer, 1024);
}

int post(const char *format=0, ...) {
   va_list args;
   int result = 0;

   if(format) {
     result+=print("FST::");
     va_start(args, format);
     result+=vprint(format, args);
     va_end(args);
     print("\n");
   }
   fflush(stdout);
   return result;
}

static void post_hex(void*ptr, size_t length) {
  print("FST::DATA@%p [%d]", ptr, length);
  unsigned char* data = (unsigned char*)ptr;
  if(data) {
    for(size_t i=0; i<length; i++) {
      if(!(i%16))print("\nFST %04x\t", i);
      if(!(i% 8))print(" ");
      print(" %02X", *data++);
    }
  }
  print("\n");
}

static void dump_data(const char*basename, const void*data, size_t length) {
  const char*ptr = (const char*)data;
  std::string filename = std::string(basename);
  filename+=".bin";
  FILE*f = fopen(filename.c_str(), "w");
  for(size_t i=0; i<length; i++) {
    fprintf(f, "%c", *ptr++);
  }
  fclose(f);
}

template <class inttype>
static void print_binary(inttype data, const char*suffix="") {
  size_t bits = sizeof(data)*8;
  while(bits--)
    print("%d", (data>>bits)&0x1);
  print("%s", suffix);
}

#include <iostream>
template <class type>
static void print_numbers(type*data, size_t length) {
  while(length--) {
    std::cout << " " << *data++;
    if(!(length % 16))  std::cout << std::endl;
  }
  std::cout << std::endl;
}

#define FST_UTILS__OPCODESTR(x)                   \
  case x:                                         \
  if(x>100000)                                    \
    snprintf(output, length, "%d[%s?]", x, #x);   \
  else                                            \
    snprintf(output, length, "%s[%d]", #x, x);    \
  output[length-1] = 0;                           \
  return output

static const char*effCode2string(size_t opcode, char*output, size_t length) {
  switch(opcode) {
    FST_UTILS__OPCODESTR(effCanBeAutomated);
    FST_UTILS__OPCODESTR(effCanDo);
    FST_UTILS__OPCODESTR(effClose);
    FST_UTILS__OPCODESTR(effConnectInput);
    FST_UTILS__OPCODESTR(effConnectOutput);
    FST_UTILS__OPCODESTR(effEditClose);
    FST_UTILS__OPCODESTR(effEditDraw);
    FST_UTILS__OPCODESTR(effEditGetRect);
    FST_UTILS__OPCODESTR(effEditIdle);
    FST_UTILS__OPCODESTR(effEditMouse);
    FST_UTILS__OPCODESTR(effEditOpen);
    FST_UTILS__OPCODESTR(effEditSleep);
    FST_UTILS__OPCODESTR(effEditTop);
    FST_UTILS__OPCODESTR(effGetChunk);
    FST_UTILS__OPCODESTR(effGetCurrentMidiProgram);
    FST_UTILS__OPCODESTR(effGetMidiKeyName);
    FST_UTILS__OPCODESTR(effGetEffectName);
    FST_UTILS__OPCODESTR(effGetInputProperties);
    FST_UTILS__OPCODESTR(effGetNumMidiInputChannels);
    FST_UTILS__OPCODESTR(effGetNumMidiOutputChannels);
    FST_UTILS__OPCODESTR(effGetOutputProperties);
    FST_UTILS__OPCODESTR(effGetParamDisplay);
    FST_UTILS__OPCODESTR(effGetParamLabel);
    FST_UTILS__OPCODESTR(effGetParamName);
    FST_UTILS__OPCODESTR(effGetPlugCategory);
    FST_UTILS__OPCODESTR(effGetProductString);
    FST_UTILS__OPCODESTR(effGetProgram);
    FST_UTILS__OPCODESTR(effGetProgramName);
    FST_UTILS__OPCODESTR(effGetProgramNameIndexed);
    FST_UTILS__OPCODESTR(effGetSpeakerArrangement);
    FST_UTILS__OPCODESTR(effGetTailSize);
    FST_UTILS__OPCODESTR(effGetVendorString);
    FST_UTILS__OPCODESTR(effGetVendorVersion);
    FST_UTILS__OPCODESTR(effGetVstVersion);
    FST_UTILS__OPCODESTR(effIdentify);
    FST_UTILS__OPCODESTR(effIdle);
    FST_UTILS__OPCODESTR(effKeysRequired);
    FST_UTILS__OPCODESTR(effMainsChanged);
    FST_UTILS__OPCODESTR(effOpen);
    FST_UTILS__OPCODESTR(effProcessEvents);
    FST_UTILS__OPCODESTR(effSetBlockSize);
    FST_UTILS__OPCODESTR(effSetBypass);
    FST_UTILS__OPCODESTR(effSetChunk);
    FST_UTILS__OPCODESTR(effSetProcessPrecision);
    FST_UTILS__OPCODESTR(effSetProgram);
    FST_UTILS__OPCODESTR(effSetProgramName);
    FST_UTILS__OPCODESTR(effSetSampleRate);
    FST_UTILS__OPCODESTR(effSetSpeakerArrangement);
    FST_UTILS__OPCODESTR(effSetTotalSampleToProcess);
    FST_UTILS__OPCODESTR(effShellGetNextPlugin);
    FST_UTILS__OPCODESTR(effStartProcess);
    FST_UTILS__OPCODESTR(effStopProcess);
    FST_UTILS__OPCODESTR(effString2Parameter);
    FST_UTILS__OPCODESTR(effVendorSpecific);
  default: break;
  }
  return(int2str(opcode, output, length));
}
static const char*effCode2string(t_fstPtrInt opcode) {
  static PERTHREAD char buffer[1024];
  return effCode2string(opcode, buffer, 1024);
}

static const char*hostCode2string(t_fstPtrInt opcode, char*output, size_t length) {
  switch(opcode) {
    FST_UTILS__OPCODESTR(audioMasterAutomate);
    FST_UTILS__OPCODESTR(audioMasterVersion);
    FST_UTILS__OPCODESTR(audioMasterGetVendorString);
    FST_UTILS__OPCODESTR(audioMasterGetProductString);
    FST_UTILS__OPCODESTR(audioMasterGetVendorVersion);
    FST_UTILS__OPCODESTR(audioMasterBeginEdit);
    FST_UTILS__OPCODESTR(audioMasterEndEdit);
    FST_UTILS__OPCODESTR(audioMasterCanDo);
    FST_UTILS__OPCODESTR(audioMasterCloseWindow);
    FST_UTILS__OPCODESTR(audioMasterCurrentId);
    FST_UTILS__OPCODESTR(audioMasterGetAutomationState);
    FST_UTILS__OPCODESTR(audioMasterGetBlockSize);
    FST_UTILS__OPCODESTR(audioMasterGetCurrentProcessLevel);
    FST_UTILS__OPCODESTR(audioMasterGetDirectory);
    FST_UTILS__OPCODESTR(audioMasterGetInputLatency);
    FST_UTILS__OPCODESTR(audioMasterGetLanguage);
    FST_UTILS__OPCODESTR(audioMasterGetNextPlug);
    FST_UTILS__OPCODESTR(audioMasterGetNumAutomatableParameters);
    FST_UTILS__OPCODESTR(audioMasterGetOutputLatency);
    FST_UTILS__OPCODESTR(audioMasterGetOutputSpeakerArrangement);
    FST_UTILS__OPCODESTR(audioMasterGetParameterQuantization);
    FST_UTILS__OPCODESTR(audioMasterGetPreviousPlug);
    FST_UTILS__OPCODESTR(audioMasterGetSampleRate);
    FST_UTILS__OPCODESTR(audioMasterGetTime);
    FST_UTILS__OPCODESTR(audioMasterIdle);
    FST_UTILS__OPCODESTR(audioMasterIOChanged);
    FST_UTILS__OPCODESTR(audioMasterNeedIdle);
    FST_UTILS__OPCODESTR(audioMasterOfflineGetCurrentMetaPass);
    FST_UTILS__OPCODESTR(audioMasterOfflineGetCurrentPass);
    FST_UTILS__OPCODESTR(audioMasterOfflineRead);
    FST_UTILS__OPCODESTR(audioMasterOfflineStart);
    FST_UTILS__OPCODESTR(audioMasterOfflineWrite);
    FST_UTILS__OPCODESTR(audioMasterOpenWindow);
    FST_UTILS__OPCODESTR(audioMasterPinConnected);
    FST_UTILS__OPCODESTR(audioMasterProcessEvents);
    FST_UTILS__OPCODESTR(audioMasterSetIcon);
    FST_UTILS__OPCODESTR(audioMasterSetOutputSampleRate);
    FST_UTILS__OPCODESTR(audioMasterSetTime);
    FST_UTILS__OPCODESTR(audioMasterSizeWindow);
    FST_UTILS__OPCODESTR(audioMasterTempoAt);
    FST_UTILS__OPCODESTR(audioMasterUpdateDisplay);
    FST_UTILS__OPCODESTR(audioMasterVendorSpecific);
    FST_UTILS__OPCODESTR(audioMasterWantMidi);
    FST_UTILS__OPCODESTR(audioMasterWillReplaceOrAccumulate);
  default: break;
  }
  snprintf(output, length, "%llu=0x%X", opcode, opcode);
  return output;
}
static const char*hostCode2string(t_fstPtrInt opcode) {
  static PERTHREAD char buffer[1024];
  return hostCode2string(opcode, buffer, 1024);
}

static int effKnown(t_fstPtrInt opcode) {
  if(opcode>=100000)
    return 0;
  switch(opcode) {
  default: break;
    /*
      grep "^[[:space:]]*FST_EFFECT_OPCODE" fst/fst.h | sed -e 's|,.*||' -e 's|).*||' -e 's|$|:|' -e 's|.*(|  case eff|' | sort -u
    */
  case effBeginSetProgram:
  case effCanBeAutomated:
  case effCanDo:
  case effClose:
  case effConnectInput:
  case effConnectOutput:
  case effEditClose:
  case effEditDraw:
  case effEditGetRect:
  case effEditIdle:
  case effEditMouse:
  case effEditOpen:
  case effEditSleep:
  case effEditTop:
  case effEndSetProgram:
  case effGetChunk:
  case effGetCurrentMidiProgram:
  case effGetEffectName:
  case effGetInputProperties:
  case effGetMidiKeyName:
  case effGetNumMidiInputChannels:
  case effGetNumMidiOutputChannels:
  case effGetOutputProperties:
  case effGetParamDisplay:
  case effGetParamLabel:
  case effGetParamName:
  case effGetPlugCategory:
  case effGetProductString:
  case effGetProgram:
  case effGetProgramName:
  case effGetProgramNameIndexed:
  case effGetSpeakerArrangement:
  case effGetTailSize:
  case effGetVendorString:
  case effGetVendorVersion:
  case effGetVstVersion:
  case effIdentify:
  case effIdle:
  case effKeysRequired:
  case effMainsChanged:
  case effOpen:
  case effProcessEvents:
  case effSetBlockSize:
  case effSetBypass:
  case effSetChunk:
  case effSetProcessPrecision:
  case effSetProgram:
  case effSetProgramName:
  case effSetSampleRate:
  case effSetSpeakerArrangement:
  case effSetTotalSampleToProcess:
  case effShellGetNextPlugin:
  case effStartProcess:
  case effStopProcess:
  case effString2Parameter:
  case effVendorSpecific:
    return 1;
  }
  return 0;
}

static int hostKnown(t_fstPtrInt opcode) {
  if(opcode>=100000)
    return 0;
  switch(opcode) {
  default: break;
    /*
      grep "^[[:space:]]*FST_HOST_OPCODE" fst/fst.h | sed -e 's|,.*||' -e 's|).*||' -e 's|$|:|' -e 's|.*(|  case audioMaster|' | sort -u
    */
  case audioMasterAutomate:
  case audioMasterBeginEdit:
  case audioMasterCanDo:
  case audioMasterCloseFileSelector:
  case audioMasterCloseWindow:
  case audioMasterCurrentId:
  case audioMasterEditFile:
  case audioMasterEndEdit:
  case audioMasterGetAutomationState:
  case audioMasterGetBlockSize:
  case audioMasterGetChunkFile:
  case audioMasterGetCurrentProcessLevel:
  case audioMasterGetDirectory:
  case audioMasterGetInputLatency:
  case audioMasterGetInputSpeakerArrangement:
  case audioMasterGetLanguage:
  case audioMasterGetNextPlug:
  case audioMasterGetNumAutomatableParameters:
  case audioMasterGetOutputLatency:
  case audioMasterGetOutputSpeakerArrangement:
  case audioMasterGetParameterQuantization:
  case audioMasterGetPreviousPlug:
  case audioMasterGetProductString:
  case audioMasterGetSampleRate:
  case audioMasterGetTime:
  case audioMasterGetVendorString:
  case audioMasterGetVendorVersion:
  case audioMasterIOChanged:
  case audioMasterIdle:
  case audioMasterNeedIdle:
  case audioMasterOfflineGetCurrentMetaPass:
  case audioMasterOfflineGetCurrentPass:
  case audioMasterOfflineRead:
  case audioMasterOfflineStart:
  case audioMasterOfflineWrite:
  case audioMasterOpenFileSelector:
  case audioMasterOpenWindow:
  case audioMasterPinConnected:
  case audioMasterProcessEvents:
  case audioMasterSetIcon:
  case audioMasterSetOutputSampleRate:
  case audioMasterSetTime:
  case audioMasterSizeWindow:
  case audioMasterTempoAt:
  case audioMasterUpdateDisplay:
  case audioMasterVendorSpecific:
  case audioMasterVersion:
  case audioMasterWantMidi:
  case audioMasterWillReplaceOrAccumulate:
    return 1;
  }
  return 0;
}

static void post_aeffect(AEffect*eff) {
  post("AEffect @ %p", eff);
  if(!eff)return;
  if(eff->magic != 0x56737450)
    post("\tmagic=0x%X", eff->magic);
  else
    post("\tmagic=VstP");
  post("\tdispatcherCB=%p", eff->dispatcher);
  post("\tprocessCB=%p", eff->process);
  post("\tgetParameterCB=%p", eff->getParameter);
  post("\tsetParameterCB=%p", eff->setParameter);

  post("\tnumPrograms=%d", eff->numPrograms);
  post("\tnumParams=%d", eff->numParams);
  post("\tnumInputs=%d", eff->numInputs);
  post("\tnumOutputs=%d", eff->numOutputs);

  int flags = eff->flags;
  print("FST::\tflags="); print_binary(flags); print("\n");

#define FST_UTILS__FLAG(x) if(effFlags##x) {                      \
    if(effFlags##x & flags)post("\t      %s", #x);                \
    flags &= ~effFlags##x;                                        \
  }                                                               \
  else post("\t      ???%s???", #x)

  FST_UTILS__FLAG(HasEditor);
  FST_UTILS__FLAG(IsSynth);
  FST_UTILS__FLAG(CanDoubleReplacing);
  FST_UTILS__FLAG(CanReplacing);
  FST_UTILS__FLAG(NoSoundInStop);
  FST_UTILS__FLAG(ProgramChunks);
  if(flags) {
    print("FST::\t      ");
    print_binary(flags);
    print("\n");
  }

  post("\tresvd1=0x%X", eff->resvd1);
  post("\tresvd2=0x%X", eff->resvd2);
  post("\tinitialDelay=%d", eff->initialDelay);

  post("\tuser=%p", eff->user);
  post("\tobject=%p", eff->object);
  post("\tuniqueID=%d", eff->uniqueID);
  post("\tversion=%d", eff->version);

  post("\tprocessReplacingCB=%p", eff->processReplacing);
  post("\tprocessDoubleReplacingCB=%p", eff->processDoubleReplacing);
  post("");
}
static void post_event(VstEvent*ev, int hexdump, const char*prefix="") {
  print("FST::%sVstEvent @ %p", prefix, ev);
  if(!ev) {
    print(" [%d]\n", sizeof(VstEvent));
    return;
  }
  print("\n");
  if(hexdump) {
    post_hex(ev, ev->byteSize + 4*(kVstMidiType == ev->type));
  }

  if(ev->type == kVstMidiType) {
    VstMidiEvent*mev = (VstMidiEvent*)ev;
    post("%s [%d]", prefix, sizeof(VstMidiEvent));

    post("\ttype=%d", mev->type);
    post("\tbyteSize=%d", mev->byteSize);
    post("\tdeltaFrames=%d", mev->deltaFrames);
    post("\tMIDI: %02x %02x %02x %02x"
           , mev->midiData[0]
           , mev->midiData[1]
           , mev->midiData[2]
           , mev->midiData[3]);
    post("\tnote: length=%d\toffset=%d\tvelocity=%d\tdetune=%d",
      mev->noteLength,
      mev->noteOffset,
      mev->noteOffVelocity,
      mev->detune);
  } else if (ev->type == kVstSysExType) {
    VstMidiSysexEvent*sev = (VstMidiSysexEvent*)ev;
    post(" [%d]", sizeof(VstMidiSysexEvent));

    post("\ttype=%d", sev->type);
    post("\tbyteSize=%d", sev->deltaFrames);
    post("\tdeltaFrames=%d", sev->deltaFrames);
    post("\tSysEx %d bytes @ %p", sev->dumpBytes, sev->sysexDump);
    unsigned char*data=(unsigned char*)sev->sysexDump;
    print("FST::\t%s", prefix);
    for(int i=0; i<sev->dumpBytes; i++)
      print(" %02x", *data++);
    print("\n");
    post("\tflags=%d\treserved=%lu\t%lu",
        sev->flags, sev->resvd1, sev->resvd2);
  }
}

static void post_events(VstEvents*evs, int hexdump=0, const char*prefix="") {
  post("%s%d VstEvents @ %p", prefix, evs?evs->numEvents:0, evs);
  if(!evs)return;
  for(int i=0; i<evs->numEvents; i++) {
    post_event(evs->events[i], hexdump, prefix);
  }
}

static void post_erect(ERect*rect, const char*prefix="") {
  print("FST::%sERect[%p]", prefix, rect);
  if(rect)
    print(" = %d|%d - %d|%d", rect->top, rect->left, rect->bottom, rect->right);
  print("\n");
}
static void post_pinproperties(VstPinProperties*vpp) {
  post("VstPinProperties @ %p", vpp);
  if(!vpp) {
    return;
  }
  post("\tlabel     : '%.*s'", 64, vpp->label);
  post("\tlshortabel: '%.*s'",  8, vpp->shortLabel);
  post("\tarrangtype: %X", vpp->arrangementType);
  print("FST::\tflags     : ");print_binary(vpp->flags);print("\n");

}

static char*speakerArrangement2string(int type, char*output, size_t length) {
  output[0]=0;
  switch(type) {
  default:
    snprintf(output, length, "%d", type);
    break;
    FST_UTILS__OPCODESTR(kSpeakerArrEmpty);
    FST_UTILS__OPCODESTR(kSpeakerArrMono);
    FST_UTILS__OPCODESTR(kSpeakerArrStereo);
    FST_UTILS__OPCODESTR(kSpeakerArrStereoSurround);
    FST_UTILS__OPCODESTR(kSpeakerArrStereoCenter);
    FST_UTILS__OPCODESTR(kSpeakerArrStereoSide);
    FST_UTILS__OPCODESTR(kSpeakerArrStereoCLfe);
    FST_UTILS__OPCODESTR(kSpeakerArr30Cine);
    FST_UTILS__OPCODESTR(kSpeakerArr30Music);
    FST_UTILS__OPCODESTR(kSpeakerArr31Cine);
    FST_UTILS__OPCODESTR(kSpeakerArr31Music);
    FST_UTILS__OPCODESTR(kSpeakerArr40Cine);
    FST_UTILS__OPCODESTR(kSpeakerArr40Music);
    FST_UTILS__OPCODESTR(kSpeakerArr41Cine);
    FST_UTILS__OPCODESTR(kSpeakerArr41Music);
    FST_UTILS__OPCODESTR(kSpeakerArr50);
    FST_UTILS__OPCODESTR(kSpeakerArr51);
    FST_UTILS__OPCODESTR(kSpeakerArr60Cine);
    FST_UTILS__OPCODESTR(kSpeakerArr60Music);
    FST_UTILS__OPCODESTR(kSpeakerArr61Cine);
    FST_UTILS__OPCODESTR(kSpeakerArr61Music);
    FST_UTILS__OPCODESTR(kSpeakerArr70Cine);
    FST_UTILS__OPCODESTR(kSpeakerArr70Music);
    FST_UTILS__OPCODESTR(kSpeakerArr71Cine);
    FST_UTILS__OPCODESTR(kSpeakerArr71Music);
    FST_UTILS__OPCODESTR(kSpeakerArr80Cine);
    FST_UTILS__OPCODESTR(kSpeakerArr80Music);
    FST_UTILS__OPCODESTR(kSpeakerArr81Cine);
    FST_UTILS__OPCODESTR(kSpeakerArr81Music);
    FST_UTILS__OPCODESTR(kSpeakerArr102);
    FST_UTILS__OPCODESTR(kSpeakerArrUserDefined);
  }
  output[length-1]=0;
  return output;
}
static char*speaker2string(VstSpeakerProperties*props, char*output, size_t length) {
  output[0]=0;
  if(props) {
    switch(props->type) {
    default:
      snprintf(output, length, "%d [0x%X]", props->type, props->type);
      break;
      FST_UTILS__OPCODESTR(kSpeakerM);
      FST_UTILS__OPCODESTR(kSpeakerL);
      FST_UTILS__OPCODESTR(kSpeakerR);
      FST_UTILS__OPCODESTR(kSpeakerC);
      FST_UTILS__OPCODESTR(kSpeakerLfe);
      FST_UTILS__OPCODESTR(kSpeakerLs);
      FST_UTILS__OPCODESTR(kSpeakerRs);
      FST_UTILS__OPCODESTR(kSpeakerLc);
      FST_UTILS__OPCODESTR(kSpeakerRc);
      FST_UTILS__OPCODESTR(kSpeakerS);
      FST_UTILS__OPCODESTR(kSpeakerSl);
      FST_UTILS__OPCODESTR(kSpeakerSr);
      FST_UTILS__OPCODESTR(kSpeakerTm);
      FST_UTILS__OPCODESTR(kSpeakerTfl);
      FST_UTILS__OPCODESTR(kSpeakerTfc);
      FST_UTILS__OPCODESTR(kSpeakerTfr);
      FST_UTILS__OPCODESTR(kSpeakerTrl);
      FST_UTILS__OPCODESTR(kSpeakerTrc);
      FST_UTILS__OPCODESTR(kSpeakerTrr);
      FST_UTILS__OPCODESTR(kSpeakerLfe2);
    }
  }
  output[length-1]=0;
  return output;
}
static void post_non0bytes(void*bytes, size_t length) {
  char*data=(char*)bytes;
  for(size_t i=0; i<length; i++) {
    if(data[i]) {
      print("\nFST:: padding ");
      post_hex(data, length);
      return;
    }
  }
}
static void post_speakerpadding(VstSpeakerProperties*props) {
  /* print padding bytes if non-0 */
  post_non0bytes(props, 80);
  post_non0bytes(props->_padding2, sizeof(props->_padding2));
}

static void post_speakerarrangement(const char*name, VstSpeakerArrangement*vpp) {
  char buf[512];
  post("SpeakerArrangement[%s] @ %p: '%s'", name, vpp, (vpp?speakerArrangement2string(vpp->type, buf, 512):0));
  if(!vpp) {
    return;
  }
  for(int i=0; i < vpp->numChannels; i++) {
    post("\t#%d: %s", i, speaker2string(&(vpp->speakers[i]), buf, 512));
    post_speakerpadding(&(vpp->speakers[i]));
  }
  //post_hex(vpp, 1024);
}


static void post_timeinfo(VstTimeInfo*vti) {
  post("VstTimeInfo @ %p", vti);
  if(!vti) {
    return;
  }
#define FST_UTILS__VTI_g(ti, x) post("\t%s: %g", #x, ti->x)
#define FST_UTILS__VTI_d(ti, x) post("\t%s: %d", #x, ti->x)
#define FST_UTILS__VTI_x(ti, x) post("\t%s: 0x%X", #x, ti->x)
  FST_UTILS__VTI_g(vti, samplePos);
  FST_UTILS__VTI_g(vti, sampleRate);
  FST_UTILS__VTI_g(vti, nanoSeconds);
  post("\t\t= %gsec", (vti->nanoSeconds * 0.000000001));
  FST_UTILS__VTI_g(vti, ppqPos);
  FST_UTILS__VTI_g(vti, tempo);
  FST_UTILS__VTI_g(vti, barStartPos);
  FST_UTILS__VTI_g(vti, cycleStartPos);
  FST_UTILS__VTI_g(vti, cycleEndPos);
  FST_UTILS__VTI_d(vti, timeSigNumerator);
  FST_UTILS__VTI_d(vti, timeSigDenominator);
  FST_UTILS__VTI_d(vti, samplesToNextClock);
  FST_UTILS__VTI_x(vti, flags);

  int flags = vti->flags;
#define FST_UTILS__VTI_FLAGS(flags, f) do {if(flags & f)post("\t\t%s", #f); flags &= ~f;} while (0)
  FST_UTILS__VTI_FLAGS(flags, kVstTransportChanged);
  FST_UTILS__VTI_FLAGS(flags, kVstTransportPlaying);
  FST_UTILS__VTI_FLAGS(flags, kVstTransportCycleActive);
  FST_UTILS__VTI_FLAGS(flags, kVstTransportRecording);
  FST_UTILS__VTI_FLAGS(flags, kVstAutomationReading);
  FST_UTILS__VTI_FLAGS(flags, kVstAutomationWriting);
  FST_UTILS__VTI_FLAGS(flags, kVstNanosValid);
  FST_UTILS__VTI_FLAGS(flags, kVstPpqPosValid);
  FST_UTILS__VTI_FLAGS(flags, kVstTempoValid);
  FST_UTILS__VTI_FLAGS(flags, kVstBarsValid);
  FST_UTILS__VTI_FLAGS(flags, kVstCyclePosValid);
  FST_UTILS__VTI_FLAGS(flags, kVstTimeSigValid);
  FST_UTILS__VTI_FLAGS(flags, kVstSmpteValid);
  FST_UTILS__VTI_FLAGS(flags, kVstClockValid);
  if(flags)post("\t\tremainder: 0x%04X", flags);

  FST_UTILS__VTI_d(vti, smpteFrameRate);
  FST_UTILS__VTI_d(vti, smpteOffset);

  FST_UTILS__VTI_d(vti, currentBar);
  FST_UTILS__VTI_x(vti, magic);
}

/* direction 1: incoming (pre dispatcher)
 * direction 2: outgoing (post dispatcher)
 * retval: return-value for post-dispatcher calls
 */
static void post_effPtr(AEffect* effect,
                         int opcode, int index, t_fstPtrInt ivalue, void*ptr, float fvalue,
                         int direction, t_fstPtrInt retval=0) {
  bool incoming = direction?(direction&1):true;
  bool outgoing = direction?(direction&2):true;
  if(incoming) {
    switch(opcode) {
    default: break;
    case effCanDo:
      post("\tcanDo: '%s'?", (char*)ptr);
      break;
    case effEditOpen:
      post("\twindowId: %p", ptr);
      break;
    case effSetChunk:
      print("FST::\tchunk: ");
      post_hex(ptr, ivalue);
      break;
    case effSetSpeakerArrangement:
      post_speakerarrangement("input>", (VstSpeakerArrangement*)ivalue);
      post_speakerarrangement("output>", (VstSpeakerArrangement*)ptr);
      break;
    case effProcessEvents:
      post("\tevents: ");
      post_events((VstEvents*)ptr,0,"\t");
      break;
    case effString2Parameter:
    case effSetProgramName:
      post("\t'%s'", (char*)ptr);
      break;
    }
  }
  if(outgoing) {
    switch(opcode) {
    default: break;
    case effGetChunk:
      print("FST::\tchunk: ");
      post_hex(ptr, retval);
      break;
    case effGetParamLabel:
    case effGetParamDisplay:
    case effGetParamName:
    case effGetProductString:
    case effGetProgramNameIndexed:
    case effGetProgramName:
    case effGetVendorString:
      post("\t'%s'", (char*)ptr);
      break;
    case effGetSpeakerArrangement:
      post_speakerarrangement("input<", ((VstSpeakerArrangement**)ivalue)[0]);
      post_speakerarrangement("output<", ((VstSpeakerArrangement**)ptr)[0]);
      break;
    case effGetInputProperties:
    case effGetOutputProperties:
      post_pinproperties((VstPinProperties*)ptr);
      break;
    case effEditGetRect:
      post_erect((ERect*)ptr, "\t");
    }
  }
}

/* direction 1: incoming (pre dispatcher)
 * direction 2: outgoing (post dispatcher)
 * retval: return-value for post-dispatcher calls
 */
static void post_hostPtr(AEffect* effect,
                          int opcode, int index, t_fstPtrInt ivalue, void*ptr, float fvalue,
                          int direction, t_fstPtrInt retval=0) {
  bool incoming = direction?(direction|1):true;
  bool outgoing = direction?(direction|2):true;
  if(incoming) {
    switch(opcode) {
    default: break;
    case audioMasterProcessEvents:
      post("\tevents: ");
      post_events((VstEvents*)ptr, 0, "\n");
      break;
    case audioMasterCanDo:
      post("\tcanDo: '%s'?", (char*)ptr);
      break;
    }
  }
  if(outgoing) {
    switch(opcode) {
    default: break;
    case audioMasterGetTime:
      post_timeinfo((VstTimeInfo*)retval);
      return;
    case audioMasterGetDirectory:
      post("\t'%s'", (char*)retval);
      break;
      break;
    case audioMasterGetVendorString:
    case audioMasterGetProductString:
      post("\t'%s'", (char*)ptr);
      break;
    }
  }
}



static VstEvents*create_vstevents(const unsigned char midi[4]) {
  VstEvents*ves = (VstEvents*)calloc(1, sizeof(VstEvents)+sizeof(VstEvent*));
  VstMidiEvent*ve=(VstMidiEvent*)calloc(1, sizeof(VstMidiEvent));
  ves->numEvents = 1;
  ves->events[0]=(VstEvent*)ve;
  ve->type = kVstMidiType;
  ve->byteSize = sizeof(VstMidiEvent);
  for(size_t i=0; i<4; i++)
    ve->midiData[i] = midi[i];

  return ves;
}

static
t_fstPtrInt dispatch_effect (const char*name, AEffectDispatcherProc dispatchcb,
                             AEffect* effect, int opcode, int index, t_fstPtrInt ivalue, void*ptr, float fvalue) {
  if(effect) {
    int verbose = 1;
    char effname[64];
    snprintf(effname, 64, "%p", effect);
    const char*effectname = name?name:effname;
    char opcodestr[256];
    if(!dispatchcb)
      dispatchcb = effect->dispatcher;
    switch(opcode) {
    default:
      break;
    case effIdle:
    case effEditIdle:
    case effEditGetRect:
      //verbose = 0;
      break;
    }
    if(verbose) {
      if (opcode == effVendorSpecific) {
        char opcode2str[256];
        post("host2plugin('%s', '%s', '%s', %s, %p, %f)",
             effectname, effCode2string(opcode, opcodestr, 255),
             effCode2string(index, opcode2str, 255),
             int2str(ivalue),
             ptr, fvalue);
      } else {
        post("host2plugin('%s', '%s', %s, %s, %p, %f)",
             effectname, effCode2string(opcode, opcodestr, 255),
             int2str(index),
             int2str2(ivalue),
             ptr, fvalue);
      }
      post_effPtr(effect, opcode, index, ivalue, ptr, fvalue, 1);
    }
    fflush(stdout);
    t_fstPtrInt result = dispatchcb(effect, opcode, index, ivalue, ptr, fvalue);
    if(verbose) {
      post("host2plugin: %lu (0x%lX)", result, result);
      post_effPtr(effect, opcode, index, ivalue, ptr, fvalue, 2, result);
    }
    fflush(stdout);
    return result;
  }
  return 0xDEAD;
}
static
t_fstPtrInt dispatch_host (const char*name, AEffectDispatcherProc dispatchcb,
                           AEffect*effect, int opcode, int index, t_fstPtrInt ivalue, void*ptr, float fvalue) {
  char effname[64];
  snprintf(effname, 64, "%p", effect);
  const char*effectname = name?name:effname;
  char opcodestr[256];
  post("plugin2host('%s', '%s', %s, %s, %p, %f)",
      effectname, hostCode2string(opcode, opcodestr, 255),
       int2str(index),
       int2str2(ivalue),
       ptr, fvalue);
  post_hostPtr(effect, opcode, index, ivalue, ptr, fvalue, 1);
  fflush(stdout);
  t_fstPtrInt result = dispatchcb(effect, opcode, index, ivalue, ptr, fvalue);
  post("plugin2host: %lu (0x%lX)", result, result);
  post_hostPtr(effect, opcode, index, ivalue, ptr, fvalue, 2, result);

  fflush(stdout);
  return result;
}

typedef AEffect* (t_fstMain)(AEffectDispatcherProc);
static
t_fstMain* fstLoadPlugin(const char* filename) {
  t_fstMain*vstfun = 0;
#ifdef _WIN32
  HINSTANCE handle = LoadLibrary(filename);
  post("loading %s as %p", filename, handle);
  if(!handle){post("\tfailed!"); return 0; }
  if(!vstfun)vstfun=(t_fstMain*)GetProcAddress(handle, "VSTPluginMain");
  if(!vstfun)vstfun=(t_fstMain*)GetProcAddress(handle, "main");
  if(!vstfun)FreeLibrary(handle);
#else
  void*handle = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
  post("loading %s as %p", filename, handle);
  if(!handle){post("\t%s", dlerror()); return 0; }
  if(!vstfun)vstfun=(t_fstMain*)dlsym(handle, "VSTPluginMain");
  if(!vstfun)vstfun=(t_fstMain*)dlsym(handle, "main");
  if(!vstfun)dlclose(handle);
#endif
  post("loaded '%s' @ %p: %p", filename, handle, vstfun);
  fflush(stdout);
  fstpause(1.);
  return vstfun;
}

#endif /* FST_FST_UTILS_H_ */
