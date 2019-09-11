# Copyright 2016 The Fuchsia Authors
# Copyright (c) 2008-2015 Travis Geiselbrecht
#
# Use of this source code is governed by a MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT
ifeq ($(OS), Linux)
OBJOUT := $(BUILD)$(dir $(subst $(LITEOSTOPDIR),,$(shell pwd)))$(MODULE_NAME)
LOCAL_PWD := $(shell pwd)
else
TEMPLITEOSTOPDIR:=$(shell cygpath -u $(LITEOSTOPDIR))
OBJOUT := $(BUILD)$(dir $(subst $(TEMPLITEOSTOPDIR),,$(shell pwd)))$(MODULE_NAME)
LOCAL_PWD := $(shell cygpath -m $(shell pwd))
endif

ifeq ($(LOCAL_SO), y)
LIBSO := $(OUT)/lib/lib$(MODULE_NAME).so
LIBA := $(OUT)/lib/lib$(MODULE_NAME).a
else
LIBSO :=
LIBA := $(OUT)/lib/lib$(MODULE_NAME).a
endif
LIB := $(LIBA) $(LIBSO)

# create a separate list of objects per source type

LOCAL_CSRCS := $(filter %.c,$(LOCAL_SRCS))
LOCAL_CPPSRCS := $(filter %.cpp,$(LOCAL_SRCS))
LOCAL_ASMSRCS := $(filter %.S,$(LOCAL_SRCS))
LOCAL_ASMSRCS2 := $(filter %.s,$(LOCAL_SRCS))
LOCAL_CCSRCS := $(filter %.cc,$(LOCAL_SRCS))

LOCAL_COBJS := $(patsubst %.c,$(OBJOUT)/%.o,$(LOCAL_CSRCS))
LOCAL_CPPOBJS := $(patsubst %.cpp,$(OBJOUT)/%.o,$(LOCAL_CPPSRCS))
LOCAL_ASMOBJS := $(patsubst %.S,$(OBJOUT)/%.o,$(LOCAL_ASMSRCS))
LOCAL_ASMOBJS2 := $(patsubst %.s,$(OBJOUT)/%.o,$(LOCAL_ASMSRCS2))
LOCAL_CCOBJS := $(patsubst %.cc,$(OBJOUT)/%.o,$(LOCAL_CCSRCS))

LOCAL_OBJS := $(LOCAL_COBJS) $(LOCAL_CPPOBJS) $(LOCAL_ASMOBJS) $(LOCAL_ASMOBJS2)$(LOCAL_CCOBJS)

LOCAL_CGCH := $(patsubst %.h,%.h.gch,$(LOCAL_CHS))
LOCAL_CPPGCH := $(patsubst %.h,%.h.gch,$(LOCAL_CPPHS))
#$(info LOCAL_SRCS = $(LOCAL_SRCS))
#$(info LOCAL_CSRCS = $(LOCAL_CSRCS))
#$(info LOCAL_CPPSRCS = $(LOCAL_CPPSRCS))
#$(info LOCAL_ASMSRCS = $(LOCAL_ASMSRCS))
#$(info LOCAL_ASMSRCS2 = $(LOCAL_ASMSRCS2))
#$(info LOCAL_CCSRCS = $(LOCAL_CCSRCS))

#$(info LOCAL_OBJS = $(LOCAL_OBJS))
#$(info LOCAL_COBJS = $(LOCAL_COBJS))
#$(info LOCAL_CPPOBJS = $(LOCAL_CPPOBJS))
#$(info LOCAL_ASMOBJS = $(LOCAL_ASMOBJS))
#$(info LOCAL_ASMOBJS2 = $(LOCAL_ASMOBJS2))
#$(info LOCAL_CCOBJS = $(LOCAL_CCOBJS))

all : $(LIB)

$(LOCAL_COBJS): $(OBJOUT)/%.o: %.c
	$(HIDE)$(OBJ_MKDIR)
	$(HIDE)$(CC) $(LITEOS_CFLAGS) $(LOCAL_FLAGS) $(LOCAL_CFLAGS) -c $(LOCAL_PWD)/$< -o $@

$(LOCAL_CPPOBJS): $(OBJOUT)/%.o: %.cpp
	$(HIDE)$(OBJ_MKDIR)
	$(HIDE)$(GPP) $(LITEOS_CXXFLAGS) $(LOCAL_FLAGS) $(LOCAL_CPPFLAGS) -c $(LOCAL_PWD)/$< -o $@

$(LOCAL_ASMOBJS): $(OBJOUT)/%.o: %.S
	$(HIDE)$(OBJ_MKDIR)
	$(HIDE)$(CC) $(LITEOS_CFLAGS) $(LOCAL_FLAGS) $(LOCAL_ASFLAGS) -c $(LOCAL_PWD)/$< -o $@

$(LOCAL_ASMOBJS2): $(OBJOUT)/%.o: %.s
	$(HIDE)$(OBJ_MKDIR)
	$(HIDE)$(AS) $(LITEOS_ASFLAGS) $(LOCAL_FLAGS) $(LOCAL_ASFLAGS) -c $(LOCAL_PWD)/$< -o $@

$(LOCAL_CCOBJS): $(OBJOUT)/%.o: %.cc
	$(HIDE)$(OBJ_MKDIR)
	$(HIDE)$(GPP) $(LITEOS_CXXFLAGS) $(LOCAL_FLAGS) $(LOCAL_CPPFLAGS) -c $(LOCAL_PWD)/$< -o $@

$(LOCAL_CGCH): %.h.gch : %.h
	$(HIDE)$(CC) $(LITEOS_CFLAGS) $(LOCAL_FLAGS) $(LOCAL_CFLAGS) $> $^

$(LOCAL_CPPGCH): %.h.gch : %.h
	$(HIDE)$(GPP) $(LITEOS_CXXFLAGS) $(LOCAL_FLAGS) $(LOCAL_CPPFLAGS) -x c++-header $> $^

LOCAL_GCH := $(LOCAL_CGCH) $(LOCAL_CPPGCH)

$(LOCAL_OBJS): $(LOCAL_GCH)
$(LIBA): $(LOCAL_OBJS)
	$(HIDE)$(OBJ_MKDIR)

ifeq ($(OS), Linux)
	$(HIDE)$(AR) $(ARFLAGS) $@ $(LOCAL_OBJS)
else
ifeq ($(LOCAL_MODULES),)
	$(HIDE)$(AR) $(ARFLAGS) $@ $(LOCAL_OBJS)
else
	$(HIDE)for i in $(LOCAL_MODULES); do \
		pushd $(OBJOUT)/$$i 1>/dev/null; \
		$(AR) $(ARFLAGS) $@ *.o;\
		popd 1>/dev/null;\
	done
endif
endif

ifeq ($(LOCAL_SO), y)
$(LIBSO): $(LOCAL_OBJS)
	$(HIDE)$(CC) $(LITEOS_CFLAGS) -fPIC -shared $^ -o $@
endif

clean:
	$(HIDE)$(RM) $(LIB) $(OBJOUT) $(LOCAL_GCH) *.bak *~

.PHONY: all clean

# clear some variables we set here
LOCAL_CSRCS :=
LOCAL_CPPSRCS :=
LOCAL_ASMSRCS :=
LOCAL_COBJS :=
LOCAL_CPPOBJS :=
LOCAL_ASMOBJS :=
LOCAL_ASMOBJS2 :=

# LOCAL_OBJS is passed back
#LOCAL_OBJS :=

