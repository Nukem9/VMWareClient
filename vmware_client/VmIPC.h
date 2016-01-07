#pragma once

#define VM_CLIENT_READMEM_CMD		0
#define VM_CLIENT_WRITEMEM_CMD		1
#define VM_CLIENT_SHUTDOWN_CMD		2

#define VM_CLIENT_IPC_MEM_SIZE		8192
#define VM_CLIENT_IPC_MEM_PATH		"Global\\VmCliSharedMem"
#define VM_CLIENT_IPC_PATH			"\\\\.\\Pipe\\VmCliIpc"

struct IPCPacket
{
	ULONG		MessageType;
	ULONG		Status;
	ULONG64		Parameters[4];
};

bool VmEstablishIPC();
void VmTerminateIPC();

bool ReadFromPipe(IPCPacket *Packet);
bool WriteToPipe(IPCPacket *Packet);

DWORD WINAPI PipeThreadMonitor(LPVOID Arg);