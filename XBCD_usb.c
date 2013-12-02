/*	
    Copyright 2005 Helder Acevedo

    This file is part of XBCD.

    XBCD is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    XBCD is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with XBCD; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "XBCD_driver.h"
#include "extras.h"

#pragma PAGEDCODE

//Send URB to the lower device object
NTSTATUS SendAwaitUrb(PDEVICE_OBJECT pFdo, PURB pUrb)
{
	PDEVICE_EXTENSION pDevExt = GET_MINIDRIVER_DEVICE_EXTENSION(pFdo);
	KEVENT event;
	PIRP pIrp;
	IO_STATUS_BLOCK iostatus;
	PIO_STACK_LOCATION stack;
	NTSTATUS status;

	//PAGED_CODE();
	//ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
	KdPrint(("SendAwaitUrb - enter\n"));

	KeInitializeEvent(&event, NotificationEvent, FALSE);

	pIrp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_USB_SUBMIT_URB,
		pDevExt->pLowerPdo, NULL, 0, NULL, 0, TRUE, &event, &iostatus);

	if (!pIrp)
		{
			KdPrint(("SendAwaitUrb - Unable to allocate IRP for sending URB\n"));
			return STATUS_INSUFFICIENT_RESOURCES;
		}

	stack = IoGetNextIrpStackLocation(pIrp);
	stack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	stack->Parameters.Others.Argument1 = pUrb;
	status = IoCallDriver(pDevExt->pLowerPdo, pIrp);
	if (status == STATUS_PENDING)
		{
			KdPrint(("SendAwaitUrb - status_pending\n"));
			KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
			status = iostatus.Status;
		}
	if(NT_SUCCESS(status))
	{
		KdPrint(("SendAwaitUrb - status success\n"));
	}
	else
	{
		KdPrint(("SendAwaitUrb - error %d\n", status));
	}
	KdPrint(("SendAwaitUrb - returning\n"));
	return status;
}

#pragma LOCKEDCODE

NTSTATUS DeviceRead(PDEVICE_EXTENSION pDevExt)
{
	NTSTATUS status;
	PIRP Irp;
	PURB urb;
	PIO_STACK_LOCATION stack;
	KIRQL oldirql;

//	KdPrint(("DeviceRead - enter spinlock\n"));
	KeAcquireSpinLock(&pDevExt->ReadLock, &oldirql);

	// If a read is already pending.  Don't start another one.
	if (pDevExt->bReadPending)
	{
		//KdPrint(("DeviceWrite - Read already pending\n"));
		KeReleaseSpinLock(&pDevExt->ReadLock, oldirql);
		return STATUS_DEVICE_BUSY;
	}
	else
	{
		pDevExt->bReadPending = TRUE;
	}

	KeReleaseSpinLock(&pDevExt->ReadLock, oldirql);

	Irp = pDevExt->ReadInfo.pIrp;
	urb = pDevExt->ReadInfo.pUrb;
	if(!(Irp && urb))
	{
		KdPrint(("DeviceRead - Could not get Irp and urb from Device Extension\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// Acquire the remove lock so we can't remove the lower device while the IRP
	// is still active.
	status = AcquireRemoveLock(&pDevExt->RemoveLock, Irp);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("DeviceRead - Acquiring remove lock failed\n"));
		pDevExt->bReadPending = FALSE;
		return status;
	}

//	KdPrint(("DeviceRead - Building urb\n"));
	UsbBuildInterruptOrBulkTransferRequest(urb, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
		pDevExt->hInPipe, &pDevExt->hwInData, NULL, 20, USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK, NULL);

	// Install "ReadCompletion" as the completion routine for the IRP.
	IoSetCompletionRoutine(Irp, (PIO_COMPLETION_ROUTINE) ReadCompletion, pDevExt, TRUE, TRUE, TRUE);

	// Initialize the IRP for an internal control request
	stack = IoGetNextIrpStackLocation(Irp);
	stack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	stack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
	stack->Parameters.Others.Argument1 = urb;

	// Set the IRP Cancel flag to false.  It could have been canceled previously
	// and we need to reuse it.  IoReuseIrp is not available in Windows 9x.
	Irp->Cancel = FALSE;

//	KdPrint(("DeviceRead - Returning\n"));
	status = IoCallDriver(pDevExt->pLowerPdo, Irp);
	if(!NT_SUCCESS(status))
	{
		KdPrint(("DeviceRead - IoCallDriver status, %X\n", status));
	}
	return status;
}

#pragma LOCKEDCODE

NTSTATUS ReadCompletion(PDEVICE_OBJECT junk, PIRP pIrp, PVOID Context)
{
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)Context;
	KIRQL oldirql;
	NTSTATUS Status;

//	KdPrint(("ReadCompletion - enter\n"));
	KeAcquireSpinLock(&pDevExt->ReadLock, &oldirql);

	if (NT_SUCCESS(pIrp->IoStatus.Status))
	{
//		KdPrint(("ReadCompletion - Success reading report\n"));
	}
	else
	{
//		KdPrint(("ReadCompletion - Failed to read report\n"));

		pDevExt->timerEnabled = FALSE;
		KeCancelTimer(&pDevExt->timer);
	}

	pDevExt->bReadPending = FALSE;	// allow another read to be started
	KeReleaseSpinLock(&pDevExt->ReadLock, oldirql);

//	KdPrint(("ReadCompletion - Releasing Removelock\n"));
	ReleaseRemoveLock(&pDevExt->RemoveLock, pDevExt->ReadInfo.pIrp);

	if(pDevExt->timerEnabled)
		Status = DeviceRead(pDevExt);
	
//	KdPrint(("ReadCompletion - Returning\n"));
	return STATUS_MORE_PROCESSING_REQUIRED;
}

#pragma LOCKEDCODE

NTSTATUS DeviceWrite(PDEVICE_EXTENSION pDevExt/*, PIRP pIrp*/)
{
	NTSTATUS status;
	PIRP Irp;
	PURB Urb;
	PIO_STACK_LOCATION stack;
	KIRQL oldirql;

	KeAcquireSpinLock(&pDevExt->WriteLock, &oldirql);

	// If a write is already pending.  Don't start another one.
	if (pDevExt->bWritePending)
	{
		KdPrint(("DeviceWrite - Write already pending\n"));
		KeReleaseSpinLock(&pDevExt->WriteLock, oldirql);
		return STATUS_DEVICE_BUSY;
	}
	else
	{
		pDevExt->bWritePending = TRUE;
	}

	KeReleaseSpinLock(&pDevExt->WriteLock, oldirql);

	Irp = pDevExt->WriteInfo.pIrp;
	Urb = pDevExt->WriteInfo.pUrb;
	if(!(Irp && Urb))
	{
		KdPrint(("DeviceRead - Could not get Irp and urb from Device Extension\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// Acquire the remove lock so we can't remove the lower device while the IRP
	// is still active.
	status = AcquireRemoveLock(&pDevExt->RemoveLock, Irp);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("DeviceWrite - Acquiring remove lock failed\n"));
		pDevExt->bWritePending = FALSE;
		return status;
	}

	KdPrint(("DeviceWrite - Building urb\n"));
	UsbBuildInterruptOrBulkTransferRequest(Urb, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
		pDevExt->hOutPipe, &pDevExt->hwOutData, NULL, 6, USBD_TRANSFER_DIRECTION_OUT | USBD_SHORT_TRANSFER_OK, NULL);

	IoSetCompletionRoutine(Irp, (PIO_COMPLETION_ROUTINE) WriteCompletion, pDevExt, TRUE, TRUE, TRUE);

	stack = IoGetNextIrpStackLocation(Irp);
	stack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	stack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
	stack->Parameters.Others.Argument1 = Urb;

	// Set the IRP Cancel flag to false.  It could have been canceled previously
	// and we need to reuse it.  IoReuseIrp is not available in Windows 9x.
	Irp->Cancel = FALSE;

	status = IoCallDriver(pDevExt->pLowerPdo, Irp);

	KdPrint(("DeviceWrite - returning\n"));

	if(status == STATUS_PENDING)
		status = STATUS_SUCCESS;

	if(!NT_SUCCESS(status))
	{
		KdPrint(("DeviceWrite - IoCallDriver status, %X\n", status));
	}

	return status;
}

#pragma LOCKEDCODE

NTSTATUS WriteCompletion(PDEVICE_OBJECT junk, PIRP pIrp, PVOID Context)
{
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)Context;
	KIRQL oldirql;
	NTSTATUS Status;

	KeAcquireSpinLock(&pDevExt->WriteLock, &oldirql);

	if (NT_SUCCESS(pIrp->IoStatus.Status))
	{
		KdPrint(("WriteCompletion - Success writing report\n"));
	}
	else
	{
		KdPrint(("WriteCompletion - Failed to write report\n"));
	}

	pDevExt->bWritePending = FALSE;	// allow another write to be started

	KeReleaseSpinLock(&pDevExt->WriteLock, oldirql);

	KdPrint(("WriteCompletion - Releasing Removelock\n"));
	ReleaseRemoveLock(&pDevExt->RemoveLock, pDevExt->WriteInfo.pIrp);
	
	KdPrint(("WriteCompletion - Returning\n"));
	return STATUS_MORE_PROCESSING_REQUIRED;
}

#pragma PAGEDCODE
//Create an Interrupt Urb
NTSTATUS CreateInterruptUrb(PDEVICE_OBJECT pFdo)
{
	PDEVICE_EXTENSION pDevExt = GET_MINIDRIVER_DEVICE_EXTENSION(pFdo);
	
	pDevExt->ReadInfo.pIrp = IoAllocateIrp(pDevExt->pLowerPdo->StackSize, FALSE);
	if (!pDevExt->ReadInfo.pIrp)
	{
		KdPrint(("CreateInterruptUrb - Unable to create IRP for reading\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pDevExt->ReadInfo.pUrb = (PURB) ExAllocatePool(NonPagedPool, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));
	if (!pDevExt->ReadInfo.pUrb)
	{
		KdPrint(("CreateInterruptUrb - Unable to allocate URB for reading\n"));
		IoFreeIrp(pDevExt->ReadInfo.pIrp);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pDevExt->WriteInfo.pIrp = IoAllocateIrp(pDevExt->pLowerPdo->StackSize, FALSE);
	if (!pDevExt->WriteInfo.pIrp)
	{
		KdPrint(("CreateInterruptUrb - Unable to create IRP for writing\n"));
		ExFreePool(pDevExt->ReadInfo.pUrb);
		IoFreeIrp(pDevExt->ReadInfo.pIrp);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pDevExt->WriteInfo.pUrb = (PURB) ExAllocatePool(NonPagedPool, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));
	if (!pDevExt->WriteInfo.pUrb)
	{
		KdPrint(("CreateInterruptUrb - Unable to allocate URB for writing\n"));
		IoFreeIrp(pDevExt->WriteInfo.pIrp);
		ExFreePool(pDevExt->ReadInfo.pUrb);
		IoFreeIrp(pDevExt->ReadInfo.pIrp);
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	KdPrint(("CreateInterruptUrb - sucess\n"));

	return STATUS_SUCCESS;
}

#pragma PAGEDCODE
//Delete the Interrup Urb
VOID DeleteInterruptUrb(PDEVICE_OBJECT pFdo)
{
	PDEVICE_EXTENSION pDevExt = GET_MINIDRIVER_DEVICE_EXTENSION(pFdo);

	if(pDevExt->ReadInfo.pUrb)
	{
		ExFreePool(pDevExt->ReadInfo.pUrb);
		pDevExt->ReadInfo.pUrb = NULL;
	}
	
	if(pDevExt->ReadInfo.pIrp)
	{
		IoFreeIrp(pDevExt->ReadInfo.pIrp);
		pDevExt->ReadInfo.pIrp = NULL;
	}

	if(pDevExt->WriteInfo.pUrb)
	{
		ExFreePool(pDevExt->WriteInfo.pUrb);
		pDevExt->WriteInfo.pUrb = NULL;
	}
	
	if(pDevExt->WriteInfo.pIrp)
	{
		IoFreeIrp(pDevExt->WriteInfo.pIrp);
		pDevExt->WriteInfo.pIrp = NULL;
	}
	KdPrint(("DeleteInterruptUrb - sucess\n"));

}

#pragma LOCKEDCODE

VOID StopInterruptUrb(PDEVICE_EXTENSION pDevExt)
{
	pDevExt->timerEnabled = FALSE;
	KeCancelTimer(&pDevExt->timer);

	KdPrint(("StopInterruptUrb - Entered\n"));
	if (pDevExt->bReadPending)
	{
		KdPrint(("StopInterruptUrb - Canceling read Irp\n"));
		IoCancelIrp(pDevExt->ReadInfo.pIrp);
	}

	if (pDevExt->bWritePending)
	{
		KdPrint(("StopInterruptUrb - Canceling write Irp\n"));
		IoCancelIrp(pDevExt->WriteInfo.pIrp);
	}
	return;
}

/*++
 
Routine Description:

    This routine synchronously submits a URB_FUNCTION_RESET_PIPE
    request down the stack.

Arguments:

    DeviceObject - pointer to device object
    PipeInfo - pointer to PipeInformation structure
               to retrieve the pipe handle

Return Value:

    NT status value

--*/

/*NTSTATUS ResetPipe(IN PDEVICE_OBJECT DeviceObject, IN USBD_PIPE_HANDLE *PipeHandle)
{
    PURB              urb;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    //
    // initialize variables
    //

    urb = NULL;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;


    urb = ExAllocatePool(NonPagedPool, 
                         sizeof(struct _URB_PIPE_REQUEST));

    if(urb) {

        urb->UrbHeader.Length = (USHORT) sizeof(struct _URB_PIPE_REQUEST);
        urb->UrbHeader.Function = URB_FUNCTION_RESET_PIPE;
        urb->UrbPipeRequest.PipeHandle = PipeHandle;

        ntStatus = SendAwaitUrb(DeviceObject, urb);

        ExFreePool(urb);
    }
    else {

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if(NT_SUCCESS(ntStatus)) {
    
        KdPrint(("ResetPipe - success\n\n"));
        ntStatus = STATUS_SUCCESS;
    }
    else {

        KdPrint(("ResetPipe - failed\n\n"));
    }

    return ntStatus;
}*/