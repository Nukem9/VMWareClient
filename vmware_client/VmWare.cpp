#include "stdafx.h"

bool(__fastcall * Vm_ValidatePhysicalAddr)	(VM_ADDR Address, VM_ADDR Size);
PVOID(__fastcall * Vm_MapPhysicalAddr)		(VM_ADDR Address, VM_ADDR Size, int Flags, vm_map_structure *Unk);
void(__fastcall * Vm_UnmapPhysicalAddr)		(vm_map_structure *Unk);

VCPUStructure	*(__fastcall * GetVCPUStruct)(int CPUIndex, const char *Identifier, int unk2);

bool Vm_ReadMemory(ULONG_PTR PhysicalAddress, PVOID Buffer, ULONG_PTR Size)
{
	vm_map_structure map;
	memset(&map, 0, sizeof(vm_map_structure));

	// Map the virtual machine memory
	PVOID ret = Vm_MapPhysicalAddr(PhysicalAddress, Size, 2, &map);

	if (ret >= (PVOID)0xFFFFFFFFFFFFFFF0)
	{
		printf("0x%p resolved to 0x%p\n", PhysicalAddress, ret);
		printf("ADDRESS FAULT\n");
		return false;
	}

	memcpy(Buffer, ret, Size);

	// Unmap the memory
	Vm_UnmapPhysicalAddr(&map);

	return true;
}

bool Vm_WriteMemory(ULONG_PTR PhysicalAddress, PVOID Buffer, ULONG_PTR Size)
{
	vm_map_structure map;
	memset(&map, 0, sizeof(vm_map_structure));

	// Map the virtual machine memory
	PVOID ret = Vm_MapPhysicalAddr(PhysicalAddress, Size, 2, &map);

	if (ret >= (PVOID)0xFFFFFFFFFFFFFFF0)
	{
		printf("0x%p resolved to 0x%p\n", PhysicalAddress, ret);
		printf("ADDRESS FAULT\n");
		return false;
	}

	memcpy(ret, Buffer, Size);

	// Unmap the memory
	Vm_UnmapPhysicalAddr(&map);

	return true;
}