#include "stdafx.h"

enum CONTROLREG_INDEX
{
	REG_CR0,
	REG_CR2,
	REG_CR3,
	REG_CR4,
	REG_CR8,//?
};

enum SEGMENT_INDEX
{
	SEG_CS,
	SEG_DS,
	SEG_ES,
	SEG_FS,
	SEG_GS,
	SEG_SS,
};

VM_ADDR VCPU_GetControlRegister(VCPUStructure *VCPU, CONTROLREG_INDEX Index)
{
	ULONG_PTR ptr = (ULONG_PTR)VCPU;

	switch (Index)
	{
	case REG_CR0:		return *(VM_ADDR *)(ptr + 0x870);
	case REG_CR2:		return *(VM_ADDR *)(ptr + 0x860);
	case REG_CR3:		return *(VM_ADDR *)(ptr + 0x868);
	case REG_CR4:		return *(VM_ADDR *)(ptr + 0x874);
	default:			return *(VM_ADDR *)(ptr + 0x878);
	}
}

DWORD VCPU_GetSegmentRegister(VCPUStructure *VCPU, SEGMENT_INDEX Index)
{
	ULONG_PTR ptr = (ULONG_PTR)VCPU;

	switch (Index)
	{
	case SEG_CS:	return *(DWORD *)(ptr + 0xB4);
	case SEG_DS:	return *(DWORD *)(ptr + 0xBC);
	case SEG_ES:	return *(DWORD *)(ptr + 0xB0);
	case SEG_FS:	return *(DWORD *)(ptr + 0xC0);
	case SEG_GS:	return *(DWORD *)(ptr + 0xC4);
	case SEG_SS:	return *(DWORD *)(ptr + 0xB8);
	}
	
	return 0;
}

/*
bool Vm_ValidatePhysicalAddr(VM_ADDR Address, VM_ADDR Size)
{
	// @ 0x8F5D90
	// @ 0x1405D64F0 x64
	return true;
}

PVOID Vm_MapPhysicalAddr(VM_ADDR Address, VM_ADDR Size, int Flags, vm_map_structure *Unk)
{
	// @ 0x8F72A0
	// @ 0x1405D7E50 x64
	return nullptr;
}

void Vm_UnmapPhysicalAddr(vm_map_structure *Unk)
{
	// @ 0x8F7620
	// @ 0x1405D8200 x64
}
*/

#define PAGE_SIZE 0x1000
#define ROUND_TO_PAGES(Size)  (((ULONG)(Size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

BOOL Dll_Init(HMODULE hModule, DWORD ul_reason_for_call)
{
	if (AllocConsole())
	{
		freopen("CONOUT$", "w", stdout);
		freopen("CONIN$", "r", stdin);
	}

	ULONG_PTR moduleBase = (ULONG_PTR)GetModuleHandle(nullptr);

	*(PBYTE *)&Vm_ValidatePhysicalAddr	= (PBYTE)(moduleBase + 0x5D64F0);
	*(PBYTE *)&Vm_MapPhysicalAddr		= (PBYTE)(moduleBase + 0x5D7E50);
	*(PBYTE *)&Vm_UnmapPhysicalAddr		= (PBYTE)(moduleBase + 0x5D8200);

	*(PBYTE *)&GetVCPUStruct = (PBYTE)(moduleBase + 0x634300);//(PBYTE)(moduleBase + 0x5B0160);

//	someAddr = moduleBase + 0xA62B30;

	printf("Module base: 0x%p\n", moduleBase);
	printf("Vm_ValidatePhysicalAddr: 0x%p\n", Vm_ValidatePhysicalAddr);
	printf("Vm_MapPhysicalAddr: 0x%p\n", Vm_MapPhysicalAddr);
	printf("Vm_UnmapPhysicalAddr: 0x%p\n", Vm_UnmapPhysicalAddr);
	printf("GetVCPUStruct: 0x%p\n", GetVCPUStruct);
	//printf("someAddr: 0x%p\n", someAddr);

	for (int i = 0; i < 6; i++)
	{
		VCPUStructure *str = GetVCPUStruct(i, "__VC", 3048);
		printf("STRUCTURE AT: 0x%llx\n", str);
	}

	//if (!VmEstablishIPC())
	//	printf("FAILED TO ESTABLISH IPC!\n");

	return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if(ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		
		return Dll_Init(hModule, ul_reason_for_call);
	}

	return TRUE;
}