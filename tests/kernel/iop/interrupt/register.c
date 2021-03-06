#include <stdio.h>
#include <intrman.h>
#include <thsemap.h>

static int g_handlerResult = -1;
static int g_handlerSema = 0;

void intrHandler(int value) {
	g_handlerResult = value;
	iSignalSema(g_handlerSema);
}

void createHandlerSema() {
	iop_sema_t semaInfo = {};
	semaInfo.attr    = 0;
	semaInfo.option  = 0;
	semaInfo.initial = 0;
	semaInfo.max     = 1;
	g_handlerSema = CreateSema(&semaInfo);
}

void testMultipleRegister() {
	int oldStat = 0, oldIntr = 0;

	createHandlerSema();

	printf("multiple register:\n");
	CpuSuspendIntr(&oldStat);
	{
		//Make sure no handler is currently registered
		ReleaseIntrHandler(IOP_IRQ_VBLANK);
		DisableIntr(IOP_IRQ_VBLANK, &oldIntr);

		int firstRegisterResult = RegisterIntrHandler(IOP_IRQ_VBLANK, 0, (void *)&intrHandler, (void *)0xAAAA);
		printf("  register when no handler is present: %d\n", firstRegisterResult);
		
		int secondRegisterResult = RegisterIntrHandler(IOP_IRQ_VBLANK, 0, (void *)&intrHandler, (void *)0xBBBB);
		printf("  register when handler is already present: %d\n", secondRegisterResult);
		
		EnableIntr(IOP_IRQ_VBLANK);
	}
	CpuResumeIntr(oldStat);
	
	//Wait for handler to be called at least once
	WaitSema(g_handlerSema);
	
	CpuSuspendIntr(&oldStat);
	{
		printf("  handler result: %d\n", g_handlerResult);

		ReleaseIntrHandler(IOP_IRQ_VBLANK);
		DisableIntr(IOP_IRQ_VBLANK, &oldIntr);
	}
	CpuResumeIntr(oldStat);
	
	DeleteSema(g_handlerSema);
}

void testNullHandler() {
	int oldStat = 0;

	printf("null handler:\n");

	CpuSuspendIntr(&oldStat);
	{
		int nullHandlerResult1 = RegisterIntrHandler(IOP_IRQ_VBLANK, 0, NULL, NULL);
		printf("  registering null handler: %d\n", nullHandlerResult1);

		int nullHandlerResult2 = RegisterIntrHandler(IOP_IRQ_VBLANK, 0, NULL, NULL);
		printf("  registering null handler after null handler was registered: %d\n", nullHandlerResult2);

		//Remove previous handler, just in case
		ReleaseIntrHandler(IOP_IRQ_VBLANK);

		RegisterIntrHandler(IOP_IRQ_VBLANK, 0, (void *)&intrHandler, NULL);
		int nullHandlerResult3 = RegisterIntrHandler(IOP_IRQ_VBLANK, 0, NULL, NULL);
		printf("  registering null handler after non-null handler was registered: %d\n", nullHandlerResult3);

		//Remove previous handler, just in case (again)
		ReleaseIntrHandler(IOP_IRQ_VBLANK);
		
		RegisterIntrHandler(IOP_IRQ_VBLANK, 0, NULL, NULL);
		int nullHandlerResult4 = RegisterIntrHandler(IOP_IRQ_VBLANK, 0, (void *)&intrHandler, NULL);
		printf("  registering non-null handler after null handler was registered: %d\n", nullHandlerResult4);
		
		//Clean-up
		ReleaseIntrHandler(IOP_IRQ_VBLANK);
	}
	CpuResumeIntr(oldStat);
}

void testInvalidParams() {
	int oldStat = 0;

	printf("invalid params:\n");

	CpuSuspendIntr(&oldStat);
	{
		int invalidIntrLineResult = RegisterIntrHandler(~0, 0, (void *)&intrHandler, NULL);
		printf("  invalid interrupt line: %d\n", invalidIntrLineResult);
		
		int invalidModeResult1 = RegisterIntrHandler(IOP_IRQ_VBLANK, 0x80000000, (void *)&intrHandler, NULL);
		ReleaseIntrHandler(IOP_IRQ_VBLANK);
		printf("  invalid mode (INT_MIN): %d\n", invalidModeResult1);
		
		int invalidModeResult2 = RegisterIntrHandler(IOP_IRQ_VBLANK, 0x7FFFFFFF, (void *)&intrHandler, NULL);
		ReleaseIntrHandler(IOP_IRQ_VBLANK);
		printf("  invalid mode (INT_MAX): %d\n", invalidModeResult2);
	}
	CpuResumeIntr(oldStat);
}

int _start(int argc, char **argv) {
	printf("-- TEST BEGIN\n");
	
	testMultipleRegister();
	testNullHandler();
	testInvalidParams();

	printf("-- TEST END\n");
	
	return 0;
}
