#include <wdm.h>
#include <ntddk.h>
#include "hidclass.h"

#define HID_USAGE_JOYSTICK 0x04
#define HID_USAGE_GAMEPAD 0x05

typedef struct _HID_DEVICE_EXTENSION {

    PDEVICE_OBJECT  PhysicalDeviceObject;
	PDEVICE_OBJECT  NextDeviceObject;
	PVOID           MiniDeviceExtension;

} HID_DEVICE_EXTENSION, *PHID_DEVICE_EXTENSION;


#define GET_MINIDRIVER_DEVICE_EXTENSION(DO)  \
    ((PDEVICE_EXTENSION) (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->MiniDeviceExtension))

#define GET_LOWER_DEVICE_OBJECT(DO) (((PHID_DEVICE_EXTENSION) ((DO)->DeviceExtension)) \
  ->NextDeviceObject)

#define GET_PHYSICAL_DEVICE_OBJECT(DO) (((PHID_DEVICE_EXTENSION) ((DO)->DeviceExtension)) \
  ->PhysicalDeviceObject)


typedef struct _HID_MINIDRIVER_REGISTRATION {

    ULONG           Revision;
	PDRIVER_OBJECT  DriverObject;
	PUNICODE_STRING RegistryPath;
	ULONG           DeviceExtensionSize;
	BOOLEAN         DevicesArePolled;
    UCHAR           Reserved[3];

} HID_MINIDRIVER_REGISTRATION, *PHID_MINIDRIVER_REGISTRATION;


NTSTATUS HidRegisterMinidriver(IN PHID_MINIDRIVER_REGISTRATION  MinidriverRegistration);


#include <pshpack1.h>
typedef struct _HID_DESCRIPTOR
{
    UCHAR   bLength;
    UCHAR   bDescriptorType;
    USHORT  bcdHID;
    UCHAR   bCountry;
    UCHAR   bNumDescriptors;

    /*
     *  This is an array of one OR MORE descriptors.
     */
    struct _HID_DESCRIPTOR_DESC_LIST
	{
       UCHAR   bReportType;
       USHORT  wReportLength;
    } DescriptorList [1];

} HID_DESCRIPTOR, * PHID_DESCRIPTOR;
#include <poppack.h>


typedef struct _HID_DEVICE_ATTRIBUTES {

    ULONG           Size;
    USHORT          VendorID;
    USHORT          ProductID;
    USHORT          VersionNumber;
    USHORT          Reserved[11];

} HID_DEVICE_ATTRIBUTES, * PHID_DEVICE_ATTRIBUTES;


#define IOCTL_HID_GET_DEVICE_DESCRIPTOR     HID_CTL_CODE(0)
#define IOCTL_HID_GET_REPORT_DESCRIPTOR     HID_CTL_CODE(1)
#define IOCTL_HID_READ_REPORT               HID_CTL_CODE(2)
#define IOCTL_HID_WRITE_REPORT              HID_CTL_CODE(3)
//#define IOCTL_HID_GET_STRING                HID_CTL_CODE(4)
//#define IOCTL_HID_ACTIVATE_DEVICE           HID_CTL_CODE(7)
//#define IOCTL_HID_DEACTIVATE_DEVICE         HID_CTL_CODE(8)
#define IOCTL_HID_GET_DEVICE_ATTRIBUTES     HID_CTL_CODE(9)

#define HID_HID_DESCRIPTOR_TYPE             0x21
#define HID_REPORT_DESCRIPTOR_TYPE          0x22
#include "usbdi.h"
#include "extras.h"


typedef struct _DEVICE_EXTENSION{
    PDEVICE_OBJECT pFdo;
    PDEVICE_OBJECT pLowerPdo;
    PDEVICE_OBJECT pPdo;
	BOOLEAN DeviceStarted;

	/* Overall number of layouts */


	/*
	These 20 bytes are the data that comes from the controller.
	Description, see "Inside Xbox Controller"
	*/
	UCHAR hwInData[20];

	/*
	These 6 bytes are the data that goes to the contoller.
	Description, see "Inside Xbox Controller"
	*/
	UCHAR hwOutData[6];

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

NTSTATUS HidRegisterMinidriver(IN PHID_MINIDRIVER_REGISTRATION  MinidriverRegistration)
{
	return TRUE;
}
ULONG _cdecl DbgPrint(PCH Format,...)
{
	return 0;
}
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
    NTSTATUS status = STATUS_SUCCESS;
	HID_MINIDRIVER_REGISTRATION hidMinidriverRegistration;

    pDriverObject->MajorFunction[IRP_MJ_CREATE]		= 0;//XBCDCreate;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE]		= 1;//XBCDClose;
    pDriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]	= 2;//XBCDDispatchIntDevice;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]				= 3;//XBCDDispatchDevice;
	pDriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]				= 4;//XBCDDispatchSystem;
    pDriverObject->MajorFunction[IRP_MJ_POWER]		= 5;//XBCDDispatchPower;
    pDriverObject->MajorFunction[IRP_MJ_PNP]		= 6;//XBCDDispatchPnp;
	pDriverObject->DriverUnload						= 7;//XBCDUnload;
    pDriverObject->DriverExtension->AddDevice		= 8;//XBCDAddDevice;

	RtlZeroMemory(&hidMinidriverRegistration, sizeof(HID_MINIDRIVER_REGISTRATION));

	hidMinidriverRegistration.Revision				= HID_REVISION;
	hidMinidriverRegistration.DriverObject			= pDriverObject;
	hidMinidriverRegistration.RegistryPath			= pRegistryPath;

	/*
	The size of this driver's own 'device extension'. With this, enough memory
	will be allocated by the system automatically
	*/
	hidMinidriverRegistration.DeviceExtensionSize	= sizeof(DEVICE_EXTENSION);
	hidMinidriverRegistration.DevicesArePolled		= TRUE;

	status = HidRegisterMinidriver(&hidMinidriverRegistration);

	if (NT_SUCCESS(status))
	{
		KdPrint(("Minidriver Registration Worked"));
		/*RegistryPath.Buffer = (PWSTR) ExAllocatePool(PagedPool, pRegistryPath->Length + sizeof(WCHAR));
		RegistryPath.MaximumLength = pRegistryPath->Length + sizeof(WCHAR);
		RtlCopyUnicodeString(&RegistryPath, pRegistryPath);
		RegistryPath.Buffer[pRegistryPath->Length/sizeof(WCHAR)] = 0;*/
		KdPrint(("%ws", pRegistryPath->Buffer));
	}
	else
	{
		KdPrint(("Minidriver Registration Failed"));
	}
    return status;
}

int main(int argc,char **argv)
{
	DRIVER_OBJECT obj={0};
	UNICODE_STRING path[100]={0};
	char buf[1024];
	obj.DriverExtension=&buf;
	DriverEntry(&obj,&path);
}