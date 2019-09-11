DYNLOADTESTDIR := $(LITEOSTOPDIR)/test/sample/kernel_extend/dynload
export DYNLOADTESTDIR

##### command to test dynload #####
-include $(DYNLOADTESTDIR)/test_dynload.mk
