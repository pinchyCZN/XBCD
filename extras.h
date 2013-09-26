// http://www.codemachine.com/resources.html

#define IoSkipCurrentIrpStackLocation(_Irp) \
{ \
  (_Irp)->CurrentLocation++; \
  (_Irp)->Tail.Overlay.CurrentStackLocation++; \
}


typedef struct _KFLOATING_SAVE {
  ULONG  ControlWord;
  ULONG  StatusWord;
  ULONG  ErrorOffset;
  ULONG  ErrorSelector;
  ULONG  DataOffset;
  ULONG  DataSelector;
  ULONG  Cr0NpxState;
  ULONG  Spare1;
} KFLOATING_SAVE, *PKFLOATING_SAVE;

#ifndef NTOSAPI
#define NTOSAPI __declspec( dllexport )
#endif
#define DDKAPI _stdcall
#define DDKFASTAPI _fastcall
#define DDKCDECLAPI _cdecl


NTOSAPI
NTSTATUS
DDKAPI
KeSaveFloatingPointState(
  OUT PKFLOATING_SAVE  FloatSave);



/*
** IRP function codes
*/

#define IRP_MJ_CREATE                     0x00
#define IRP_MJ_CREATE_NAMED_PIPE          0x01
#define IRP_MJ_CLOSE                      0x02
#define IRP_MJ_READ                       0x03
#define IRP_MJ_WRITE                      0x04
#define IRP_MJ_QUERY_INFORMATION          0x05
#define IRP_MJ_SET_INFORMATION            0x06
#define IRP_MJ_QUERY_EA                   0x07
#define IRP_MJ_SET_EA                     0x08
#define IRP_MJ_FLUSH_BUFFERS              0x09
#define IRP_MJ_QUERY_VOLUME_INFORMATION   0x0a
#define IRP_MJ_SET_VOLUME_INFORMATION     0x0b
#define IRP_MJ_DIRECTORY_CONTROL          0x0c
#define IRP_MJ_FILE_SYSTEM_CONTROL        0x0d
#define IRP_MJ_DEVICE_CONTROL             0x0e
#define IRP_MJ_INTERNAL_DEVICE_CONTROL    0x0f
#define IRP_MJ_SHUTDOWN                   0x10
#define IRP_MJ_LOCK_CONTROL               0x11
#define IRP_MJ_CLEANUP                    0x12
#define IRP_MJ_CREATE_MAILSLOT            0x13
#define IRP_MJ_QUERY_SECURITY             0x14
#define IRP_MJ_SET_SECURITY               0x15
#define IRP_MJ_POWER                      0x16
#define IRP_MJ_SYSTEM_CONTROL             0x17
#define IRP_MJ_DEVICE_CHANGE              0x18
#define IRP_MJ_QUERY_QUOTA                0x19
#define IRP_MJ_SET_QUOTA                  0x1a
#define IRP_MJ_PNP                        0x1b
#define IRP_MJ_PNP_POWER                  0x1b
#define IRP_MJ_MAXIMUM_FUNCTION           0x1b

#define IRP_MN_QUERY_DIRECTORY            0x01
#define IRP_MN_NOTIFY_CHANGE_DIRECTORY    0x02

#define IRP_MN_USER_FS_REQUEST            0x00
#define IRP_MN_MOUNT_VOLUME               0x01
#define IRP_MN_VERIFY_VOLUME              0x02
#define IRP_MN_LOAD_FILE_SYSTEM           0x03
#define IRP_MN_TRACK_LINK                 0x04
#define IRP_MN_KERNEL_CALL                0x04

#define IRP_MN_LOCK                       0x01
#define IRP_MN_UNLOCK_SINGLE              0x02
#define IRP_MN_UNLOCK_ALL                 0x03
#define IRP_MN_UNLOCK_ALL_BY_KEY          0x04

#define IRP_MN_NORMAL                     0x00
#define IRP_MN_DPC                        0x01
#define IRP_MN_MDL                        0x02
#define IRP_MN_COMPLETE                   0x04
#define IRP_MN_COMPRESSED                 0x08

#define IRP_MN_MDL_DPC                    (IRP_MN_MDL | IRP_MN_DPC)
#define IRP_MN_COMPLETE_MDL               (IRP_MN_COMPLETE | IRP_MN_MDL)
#define IRP_MN_COMPLETE_MDL_DPC           (IRP_MN_COMPLETE_MDL | IRP_MN_DPC)

#define IRP_MN_SCSI_CLASS                 0x01

#define IRP_MN_START_DEVICE               0x00
#define IRP_MN_QUERY_REMOVE_DEVICE        0x01
#define IRP_MN_REMOVE_DEVICE              0x02
#define IRP_MN_CANCEL_REMOVE_DEVICE       0x03
#define IRP_MN_STOP_DEVICE                0x04
#define IRP_MN_QUERY_STOP_DEVICE          0x05
#define IRP_MN_CANCEL_STOP_DEVICE         0x06

#define IRP_MN_QUERY_DEVICE_RELATIONS       0x07
#define IRP_MN_QUERY_INTERFACE              0x08
#define IRP_MN_QUERY_CAPABILITIES           0x09
#define IRP_MN_QUERY_RESOURCES              0x0A
#define IRP_MN_QUERY_RESOURCE_REQUIREMENTS  0x0B
#define IRP_MN_QUERY_DEVICE_TEXT            0x0C
#define IRP_MN_FILTER_RESOURCE_REQUIREMENTS 0x0D

#define IRP_MN_READ_CONFIG                  0x0F
#define IRP_MN_WRITE_CONFIG                 0x10
#define IRP_MN_EJECT                        0x11
#define IRP_MN_SET_LOCK                     0x12
#define IRP_MN_QUERY_ID                     0x13
#define IRP_MN_QUERY_PNP_DEVICE_STATE       0x14
#define IRP_MN_QUERY_BUS_INFORMATION        0x15
#define IRP_MN_DEVICE_USAGE_NOTIFICATION    0x16
#define IRP_MN_SURPRISE_REMOVAL             0x17
#define IRP_MN_QUERY_LEGACY_BUS_INFORMATION 0x18


#define IRP_MN_QUERY_ALL_DATA             0x00
#define IRP_MN_QUERY_SINGLE_INSTANCE      0x01
#define IRP_MN_CHANGE_SINGLE_INSTANCE     0x02
#define IRP_MN_CHANGE_SINGLE_ITEM         0x03
#define IRP_MN_ENABLE_EVENTS              0x04
#define IRP_MN_DISABLE_EVENTS             0x05
#define IRP_MN_ENABLE_COLLECTION          0x06
#define IRP_MN_DISABLE_COLLECTION         0x07
#define IRP_MN_REGINFO                    0x08
#define IRP_MN_EXECUTE_METHOD             0x09

#define IRP_MN_REGINFO_EX                 0x0b










/* DEVICE_OBJECT.Flags */

#define DO_VERIFY_VOLUME                  0x00000002      
#define DO_BUFFERED_IO                    0x00000004      
#define DO_EXCLUSIVE                      0x00000008      
#define DO_DIRECT_IO                      0x00000010      
#define DO_MAP_IO_BUFFER                  0x00000020      
#define DO_DEVICE_HAS_NAME                0x00000040      
#define DO_DEVICE_INITIALIZING            0x00000080      
#define DO_SYSTEM_BOOT_PARTITION          0x00000100      
#define DO_LONG_TERM_REQUESTS             0x00000200      
#define DO_NEVER_LAST_DEVICE              0x00000400      
#define DO_SHUTDOWN_REGISTERED            0x00000800      
#define DO_BUS_ENUMERATED_DEVICE          0x00001000      
#define DO_POWER_PAGABLE                  0x00002000      
#define DO_POWER_INRUSH                   0x00004000      
#define DO_LOW_PRIORITY_FILESYSTEM        0x00010000   

#ifdef _DEVICE_CAPABILITIES
#undef _DEVICE_CAPABILITIES
#endif


typedef struct _HID_XFER_PACKET {
  PUCHAR  reportBuffer;
  ULONG  reportBufferLen;
  UCHAR  reportId;
} HID_XFER_PACKET, *PHID_XFER_PACKET;


#define HID_BUFFER_CTL_CODE(id) \
  CTL_CODE (FILE_DEVICE_KEYBOARD, (id), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HID_IN_CTL_CODE(id) \
  CTL_CODE (FILE_DEVICE_KEYBOARD, (id), METHOD_IN_DIRECT, FILE_ANY_ACCESS)
#define HID_OUT_CTL_CODE(id) \
  CTL_CODE (FILE_DEVICE_KEYBOARD, (id), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)


#define IOCTL_GET_PHYSICAL_DESCRIPTOR         HID_OUT_CTL_CODE(102)
#define IOCTL_HID_GET_FEATURE                 HID_OUT_CTL_CODE(100)
#define IOCTL_HID_GET_HARDWARE_ID             HID_OUT_CTL_CODE(103)
#define IOCTL_HID_GET_INDEXED_STRING          HID_OUT_CTL_CODE(120)
#define IOCTL_HID_GET_INPUT_REPORT            HID_OUT_CTL_CODE(104)
#define IOCTL_HID_GET_MANUFACTURER_STRING     HID_OUT_CTL_CODE(110)
#define IOCTL_GET_NUM_DEVICE_INPUT_BUFFERS    HID_BUFFER_CTL_CODE(104)
#define IOCTL_HID_GET_POLL_FREQUENCY_MSEC     HID_BUFFER_CTL_CODE(102)
#define IOCTL_HID_GET_PRODUCT_STRING          HID_OUT_CTL_CODE(111)
#define IOCTL_HID_GET_SERIALNUMBER_STRING     HID_OUT_CTL_CODE(112)
#define IOCTL_HID_SET_FEATURE                 HID_IN_CTL_CODE(100)
#define IOCTL_SET_NUM_DEVICE_INPUT_BUFFERS    HID_BUFFER_CTL_CODE(105)
#define IOCTL_HID_SET_OUTPUT_REPORT           HID_IN_CTL_CODE(101)
#define IOCTL_HID_SET_POLL_FREQUENCY_MSEC     HID_BUFFER_CTL_CODE(103)

#define IOCTL_HID_GET_MS_GENRE_DESCRIPTOR     HID_OUT_CTL_CODE(121)



typedef struct _USBD_INTERFACE_LIST_ENTRY {
  PUSB_INTERFACE_DESCRIPTOR   InterfaceDescriptor;
  PUSBD_INTERFACE_INFORMATION Interface;
} USBD_INTERFACE_LIST_ENTRY, *PUSBD_INTERFACE_LIST_ENTRY;


/* URB TransferFlags constants */
#define USBD_TRANSFER_DIRECTION(x)        ((x) & USBD_TRANSFER_DIRECTION_IN)
#define USBD_TRANSFER_DIRECTION_OUT       0   
#define USBD_TRANSFER_DIRECTION_BIT       0
#define USBD_SHORT_TRANSFER_OK_BIT        1
#define USBD_START_ISO_TRANSFER_ASAP_BIT  2
#define USBD_DEFAULT_PIPE_TRANSFER_BIT    3
#define USBD_DEFAULT_PIPE_TRANSFER        (1 << USBD_DEFAULT_PIPE_TRANSFER_BIT)


/*
 * VOID
 * IoCopyCurrentIrpStackLocationToNext(
 *   IN PIRP  Irp)
 */
#define IoCopyCurrentIrpStackLocationToNext(_Irp) \
{ \
  PIO_STACK_LOCATION _IrpSp; \
  PIO_STACK_LOCATION _NextIrpSp; \
  _IrpSp = IoGetCurrentIrpStackLocation(_Irp); \
  _NextIrpSp = IoGetNextIrpStackLocation(_Irp); \
  RtlCopyMemory(_NextIrpSp, _IrpSp, \
    FIELD_OFFSET(IO_STACK_LOCATION, CompletionRoutine)); \
  _NextIrpSp->Control = 0; \
}


#define UsbBuildGetDescriptorRequest(urb, \
                                     length, \
                                     descriptorType, \
                                     index, \
                                     languageId, \
                                     transferBuffer, \
                                     transferBufferMDL, \
                                     transferBufferLength, \
                                     link) { \
            (urb)->UrbHeader.Function =  URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE; \
            (urb)->UrbHeader.Length = (length); \
            (urb)->UrbControlDescriptorRequest.TransferBufferLength = (transferBufferLength); \
            (urb)->UrbControlDescriptorRequest.TransferBufferMDL = (transferBufferMDL); \
            (urb)->UrbControlDescriptorRequest.TransferBuffer = (transferBuffer); \
            (urb)->UrbControlDescriptorRequest.DescriptorType = (descriptorType); \
            (urb)->UrbControlDescriptorRequest.Index = (index); \
            (urb)->UrbControlDescriptorRequest.LanguageId = (languageId); \
            (urb)->UrbControlDescriptorRequest.UrbLink = (link); }

#define UsbBuildSelectConfigurationRequest(urb, \
                                         length, \
                                         configurationDescriptor) { \
            (urb)->UrbHeader.Function =  URB_FUNCTION_SELECT_CONFIGURATION; \
            (urb)->UrbHeader.Length = (length); \
            (urb)->UrbSelectConfiguration.ConfigurationDescriptor = (configurationDescriptor);    }


#define UsbBuildInterruptOrBulkTransferRequest(urb, \
                                               length, \
                                               pipeHandle, \
                                               transferBuffer, \
                                               transferBufferMDL, \
                                               transferBufferLength, \
                                               transferFlags, \
                                               link) { \
            (urb)->UrbHeader.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER; \
            (urb)->UrbHeader.Length = (length); \
            (urb)->UrbBulkOrInterruptTransfer.PipeHandle = (pipeHandle); \
            (urb)->UrbBulkOrInterruptTransfer.TransferBufferLength = (transferBufferLength); \
            (urb)->UrbBulkOrInterruptTransfer.TransferBufferMDL = (transferBufferMDL); \
            (urb)->UrbBulkOrInterruptTransfer.TransferBuffer = (transferBuffer); \
            (urb)->UrbBulkOrInterruptTransfer.TransferFlags = (transferFlags); \
            (urb)->UrbBulkOrInterruptTransfer.UrbLink = (link); }