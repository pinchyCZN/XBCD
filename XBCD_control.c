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

ULONG _fltused = 0;

/*****************************************************************************/

/*! This pragma keeps code in physical memory */
#pragma LOCKEDCODE
NTSTATUS XBCDDispatchIntDevice(IN PDEVICE_OBJECT pFdo, IN PIRP pIrp)
{
//	unsigned int iTemp;
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION pDevExt = GET_MINIDRIVER_DEVICE_EXTENSION(pFdo);

		switch(stack->Parameters.DeviceIoControl.IoControlCode)
		{
		case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
			{
				PHID_DEVICE_ATTRIBUTES pHidAttributes;
				pHidAttributes = (PHID_DEVICE_ATTRIBUTES) pIrp->UserBuffer;
				
				KdPrint(("XBCDDispatchIntDevice - sending device attributes"));
				if (stack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(HID_DEVICE_ATTRIBUTES))
				{
					Status = STATUS_BUFFER_TOO_SMALL;
					KdPrint(("Buffer for Device Attributes is too small"));
					break;
				}
				
				RtlZeroMemory(pHidAttributes, sizeof(HID_DEVICE_ATTRIBUTES));
					
				pHidAttributes->Size = sizeof(HID_DEVICE_ATTRIBUTES);

				/*!
				These have to match with what's installed into the
				registry from the .INF file
				Should this really be hardwired? What about getting it
				from a config file or the registry (since it's in there
				somewhere anyhow...)
				*/

				/*
				Vendor and Product ID's are fixed to make it easier to
				write the .INF file.
				*/
				pHidAttributes->VendorID = 0x0738;//pDevExt->dd.idVendor;
				pHidAttributes->ProductID = 0x4516;//pDevExt->dd.idProduct;

				pHidAttributes->VersionNumber=0x0001;//pDevExt->dd.bcdDevice;
				
				pIrp->IoStatus.Information = sizeof(HID_DEVICE_ATTRIBUTES);
				
				KdPrint(("XBCDDispatchIntDevice - Sent Device Attributes"));

				break;
			}

		case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
			{
				PHID_DESCRIPTOR pHidDescriptor;
				USHORT RepDescSize;
				pHidDescriptor = (PHID_DESCRIPTOR) pIrp->UserBuffer;
				
				KdPrint(("XBCDDispatchIntDevice - sending device descriptor"));
				if (stack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(HID_DESCRIPTOR))
				{
					Status = STATUS_BUFFER_TOO_SMALL;
					KdPrint(("Buffer for Device Descriptor is too small"));
					break;
				}
				
				RtlZeroMemory(pHidDescriptor, sizeof(HID_DESCRIPTOR));
				
				pHidDescriptor->bLength = sizeof(HID_DESCRIPTOR);
				pHidDescriptor->bDescriptorType = HID_HID_DESCRIPTOR_TYPE;
				pHidDescriptor->bcdHID = HID_REVISION;
				pHidDescriptor->bCountry = 0;
				pHidDescriptor->bNumDescriptors = 1;
				pHidDescriptor->DescriptorList[0].bReportType = HID_REPORT_DESCRIPTOR_TYPE;
				RepDescSize = GetRepDesc(pDevExt, NULL);
				pHidDescriptor->DescriptorList[0].wReportLength = RepDescSize;
				
				pIrp->IoStatus.Information = sizeof(HID_DESCRIPTOR);
				KdPrint(("XBCDDispatchIntDevice - Sent Device Descriptor"));

				break;
			}

		case IOCTL_HID_GET_REPORT_DESCRIPTOR:
			{
				int iSize;
				PUCHAR pBuffer = (PUCHAR)pIrp->UserBuffer;

				KdPrint(("XBCDDispatchIntDevice - sending report descriptor"));

				iSize = GetRepDesc(pDevExt, NULL);

				if(stack->Parameters.DeviceIoControl.OutputBufferLength < iSize)
				{
					Status = STATUS_BUFFER_TOO_SMALL;
					KdPrint(("Buffer for Report Descriptor is too small"));
					break;
				}
				
				iSize = GetRepDesc(pDevExt, pBuffer);
				
				pIrp->IoStatus.Information = iSize;
				
				KdPrint(("XBCDDispatchIntDevice - Sent Report Descriptor"));

				break;
			}

		/*!
		Apparently this branch will be executed whenever some application
		tries to get data from the gamepad driver.
		*/
		case IOCTL_HID_READ_REPORT:
			{	
				LARGE_INTEGER timeout;

				KdPrint(("XBCDDispatchIntDevice - IOCTL_HID_READ_REPORT entry"));

				if(stack->Parameters.DeviceIoControl.OutputBufferLength < OUT_BUFFER_LEN)
				{
					KdPrint(("IOCTL_HID_READ_REPORT - Buffer is too small"));
					Status = STATUS_BUFFER_TOO_SMALL;
					break;
				}

				Status=XBCDReadData(pFdo, pIrp);

				//Set a timer to keep reading data for another 5 seconds
				//Fixes problems with button configuration in games
				timeout.QuadPart = -50000000;
				KeSetTimer(&pDevExt->timer, timeout, &pDevExt->timeDPC);
				pDevExt->timerEnabled = TRUE;
				
				Status = DeviceRead(pDevExt);

				KdPrint(("XBCDDispatchIntDevice - IOCTL_HID_READ_REPORT exit"));
				return Status;
			}

		/*!
		This branch is for writing data to the driver (or the device), e.g
		when changing button configuration via the config tool or when the
		rumble actuators are accessed...
		There have to be some hardwired values here.
		Quote: "The Xbox gamepad lacks the HID report descriptor that
		describes the input/output report formats"
		*/
		case IOCTL_HID_WRITE_REPORT:
			{
				PHID_XFER_PACKET pHxp;

				KdPrint(("XBCDDispatchIntDevice: IOCTL_HID_WRITE_REPORT"));

				if(stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(HID_XFER_PACKET))
				{
					KdPrint(("IOCTL_HID_WRITE_REPORT - Buffer is too small"));
					Status = STATUS_BUFFER_TOO_SMALL;
					break;
				}

				pHxp = (PHID_XFER_PACKET)pIrp->UserBuffer;

				if(pHxp->reportBufferLen != 3)
				{
					KdPrint(("IOCTL_HID_WRITE_REPORT - pHxp->reportBuffer has wrong size"));
					Status = STATUS_BUFFER_TOO_SMALL;
					break;
				}

				/* Hardwired values, 06=length of report */
				pDevExt->hwOutData[0]=0x00;
				pDevExt->hwOutData[1]=0x06;
				pDevExt->hwOutData[2]=0x00;
				pDevExt->hwOutData[4]=0x00;

				/*
				Rumble actuators.
				Output values are 0-255. For simplicity, the scale factors
				'LaFactor' and 'RaFactor' also range from 0 to 255.
				Thus, the final value calculates like this:
				Out=((ScaleFactor+1)*In)/256
				*/
				/* Left */
				pDevExt->hwOutData[3]=((1+pDevExt->LaFactor)*
					(unsigned int)(pHxp->reportBuffer[1]))>>8;

				/* Right */
				pDevExt->hwOutData[5]=((1+pDevExt->RaFactor)*
					(unsigned int)(pHxp->reportBuffer[2]))>>8;

				if(pDevExt->bHasMotors)
					Status = DeviceWrite(pDevExt/*, pIrp*/);
				else
					Status = STATUS_SUCCESS;

				break;
			}

		/*
		This request is received from the setup utility when the driver told to
		read a new configuration from the registry.  The buffer received must
		contain the signature for XBCD("XBSU") so the request can go through.
		*/
		case IOCTL_HID_SET_FEATURE:
			{
				PHID_XFER_PACKET pHxp;

				KdPrint(("XBCDDispatchIntDevice: IOCTL_HID_SET_FEATURE"));

				pHxp = (PHID_XFER_PACKET)pIrp->UserBuffer;

				switch(pHxp->reportId)
				{
					case FEATURE_CODE_SET_CONFIG:
						{
							PFEATURE_SET_CONFIG pFsc;

							if(pHxp->reportBufferLen != sizeof(FEATURE_SET_CONFIG))
							{
								KdPrint(("IOCTL_HID_SET_FEATURE - pHxp->reportBuffer is the wrong size"));
								Status = STATUS_INVALID_PARAMETER;
								break;
							}

							pFsc = (PFEATURE_SET_CONFIG)pHxp->reportBuffer;

							if(pFsc->Signature != XBCD_SIGNATURE)
							{
								KdPrint(("IOCTL_HID_SET_FEATURE - Unknown signature"));
								Status = STATUS_NOT_SUPPORTED;
								break;
							}

							XBCDReadConfig(pFdo);
							break;
						}
					default:
						{
							KdPrint(("IOCTL_HID_SET_FEATURE - Wrong report ID"));
							Status = STATUS_NOT_SUPPORTED;
							break;
						}
				}

				break;
			}

		case IOCTL_HID_GET_FEATURE:
			{
				PHID_XFER_PACKET pHxp;

				KdPrint(("XBCDDispatchIntDevice: IOCTL_HID_GET_FEATURE"));

				if(stack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(HID_XFER_PACKET))
				{
					KdPrint(("IOCTL_HID_GET_FEATURE - Buffer is too small"));
					Status = STATUS_BUFFER_TOO_SMALL;
					break;
				}

				pHxp = (PHID_XFER_PACKET)pIrp->UserBuffer;

				switch(pHxp->reportId)
				{
					case FEATURE_CODE_GET_VERSION:
						{
							PFEATURE_GET_VERSION pFgv;

							if(pHxp->reportBufferLen != sizeof(FEATURE_GET_VERSION))
							{
								KdPrint(("IOCTL_HID_GET_FEATURE - pHxp->reportBuffer is the wrong size"));
								Status = STATUS_INVALID_PARAMETER;
								break;
							}

							pFgv = (PFEATURE_GET_VERSION)pHxp->reportBuffer;

							if(pFgv->Signature != XBCD_SIGNATURE)
							{
								KdPrint(("IOCTL_HID_GET_FEATURE - Unknown signature"));
								Status = STATUS_NOT_SUPPORTED;
								break;
							}

							pFgv->Major = VER_MAJOR;
							pFgv->Minor = VER_MINOR;
							pFgv->Release = VER_RELEASE;

							pIrp->IoStatus.Information = sizeof(FEATURE_GET_VERSION);
						}
					default:
						{
							KdPrint(("IOCTL_HID_GET_FEATURE - Wrong report ID"));
							Status = STATUS_NOT_SUPPORTED;
						}
				}

				break;
			}

		default:
			{
				KdPrint(("XBCDDispatchIntDevice: Irp not implemented"));
				Status=STATUS_NOT_SUPPORTED;
				break;
			}
		}

		pIrp->IoStatus.Status = Status;

	if(Status != STATUS_PENDING)
	{
		KdPrint(("XBCDDispatchIntDevice - irp status not pending"));
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		//Status = STATUS_SUCCESS;
	}
	else
	{
		KdPrint(("XBCDDispatchIntDevice - mark the irp as pending"));
		IoMarkIrpPending(pIrp);
	}

	KdPrint(("XBCDDispatchIntDevice - returning"));
    return Status;
}

/*! This pragma allows code to go into virtual memory */
#pragma PAGEDCODE
NTSTATUS XBCDDispatchDevice(IN PDEVICE_OBJECT pFdo, IN PIRP pIrp)
{
	PDEVICE_EXTENSION pDevExt = GET_MINIDRIVER_DEVICE_EXTENSION(pFdo);
	NTSTATUS status;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);

	IoSkipCurrentIrpStackLocation(pIrp);
	status = IoCallDriver(pDevExt->pLowerPdo, pIrp);
	return status;
}

#pragma PAGEDCODE
NTSTATUS XBCDDispatchSystem(IN PDEVICE_OBJECT pFdo, IN PIRP pIrp)
{
	PDEVICE_EXTENSION pDevExt = GET_MINIDRIVER_DEVICE_EXTENSION(pFdo);
	NTSTATUS status;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);

	IoSkipCurrentIrpStackLocation(pIrp);
	status = IoCallDriver(pDevExt->pLowerPdo, pIrp);
	return status;
}

/*****************************************************************************/

#pragma LOCKEDCODE
float sqrt2(float x)
{
	float result;

	_asm
		{
		fld x
		fsqrt
		fstp result
		}

	return result;
}

/*!
Float to integer conversion. Has to be done 'manually', since the conversion
via the usual (float) cast operator relies on some library function
(no such libraries are included here).
*/
int ftoi(float f)
{
	int rv;
	_asm {
		fld f
		fistp rv
		//fwait  // not sure if this is necessary on newer processors...
		}
	return rv ;
}

/*!
Apply deadzone. Uses floating point math. Assumes 0.0<=values<=1.0 !

Parameters:
PLONG lpAxis1   : Pointer to first axis value
PLONG lpAxis2   : Pointer to second axis value
LONG DeadZone : Deadzone
*/
void applyDeadzone2D(PLONG lpAxis1, PLONG lpAxis2, LONG DeadZone, BOOLEAN bFullAxes)
{
	//Floating Point enabled
	NTSTATUS Status;

	/*!
	Apparently before doing own floating point operations a current 'state'
	has to be saved and restored at the end of the function.
	(Looks a bit like pushing/pulling registers in asm)
	*/
	KFLOATING_SAVE saveData;

	/*!
	Quote from the DDK documentation:
	'The KeSaveFloatingPointState routine saves the nonvolatile floating-point
	context so the caller can carry out floating-point operations.'
	*/
	Status=KeSaveFloatingPointState(&saveData);

	if(NT_SUCCESS(Status))
	{	
		float VectorLength;

		float fAxis1 = (float)*lpAxis1/(float)MAX_VALUE;
		float fAxis2 = (float)*lpAxis2/(float)MAX_VALUE;
		float fDZ = (float)DeadZone/(float)MAX_VALUE;

		/*!
		This is the simple version. Everything inside the deadzone will be
		zeroed out. Anything outside will be used 'as is'. This ought to do,
		since the deadzone is intended to cancel movement 'noise' and small
		offsets around the center. Other gamepad drivers SEEM to do it like
		this, too.
		Still, if movement with this method 'feels' awkward, adding the other
		solution (below) might be better.
		*/

		/* Calculate 2D movement vector length */ 
		VectorLength=sqrt2((fAxis1 * fAxis1)+(fAxis2 * fAxis2));

		/* Cancel out, if smaller than deadzone. */
		if(VectorLength<fDZ)
		{
			fAxis1=fAxis2=0.0f;
		}
		/*!
		This the technique used in the original xbcd code (adapted to the
		numeric range here!). The idea is to zero out any movement smaller than
		the deadzone radius (OK, normal; done above).
		Then, any movement beyond the deadzone will be re-scaled to the full
		range, i.e. subtract deadzone from movement and rescale it to 0.0 - 1.0
		*/
		else
		{
			/*
			Make VectorLength a scale factor for the axes! To avoid numeric
			problems and divisions by zero (!) DeadZone is limited to 0.95f
			here (a little dirty, but it should be OK - who uses such a
			large deadzone anyway).
			*/
			if(fDZ>0.95f) fDZ=0.95f;
			VectorLength-=fDZ;
			VectorLength/=1.0f-fDZ;

			if(bFullAxes)
			{
				fAxis1*=1.4142136f;
				fAxis2*=1.4142136f;
			}

			fAxis1*=VectorLength;
			fAxis2*=VectorLength;

			/* Ensure the -1.0f - 1.0f range! */
			if(fAxis1>1.0f) fAxis1=1.0f;
			if(fAxis1<-1.0f) fAxis1=-1.0f;
			if(fAxis2>1.0f) fAxis2=1.0f;
			if(fAxis2<-1.0f) fAxis2=-1.0f;
		}

		*lpAxis1 = ftoi(fAxis1 * MAX_VALUE);
		*lpAxis2 = ftoi(fAxis2 * MAX_VALUE);

		KeRestoreFloatingPointState(&saveData);
	}
	else
	{
		//Do nothing.  Leave data as it is.
	}
}

/*****************************************************************************/

/*! */
NTSTATUS XBCDReadData(PDEVICE_OBJECT pFdo, PIRP pIrp)
{
	NTSTATUS Status;
	int index, MapIndex;

	// Quote: '__int16 is synonymous with type short'
	// (Used as a general purpose 16 Bit variable in this function)
	__int16 SignedInt16;

	LONG StickX, StickY;

	// Output data (from this driver to the rest of the system)
	UCHAR OutData[OUT_BUFFER_LEN];

	// A float array for 'precalculated' output axes. These are used to apply
	// deadzones.

	static LONG PreOutAxes[NR_OUT_AXES];

	// The POV is defined in a (more or less) funny way:
	// 7 0 1
	//  \|/
	// 6-8-2
	//  /|\
	// 5 4 3
	// To map the 4 semiaxes to that, let's use something like a truth table (or
	// lookup table) with the input index being constructed from the 4 input
	// semiaxes as bits. This results in a truth table with 16 values, of which
	// only 9 are valid.
	// This peculiar approach seemed to be the most flexible and easy to implement
	// and debug. Thus, it would be easy to replace an 'invalid' position with an
	// intelligible alternative. For example, if Up/Left/Right are pressed the
	// result could be Up only (like Left and Right cancel out each other)
	// instead of an invalid position...
	static UCHAR OutPovValue[16]=
	{
		// Centered - any other occurence of '8' marks an invalid position!
		8,

		// Up
		0,

		// Right
		2,

		// Up/right
		1,

		// Down
		4, 8,

		// Down/right
		3, 8,

		// Left
		6,

		// Up/left
		7, 8, 8,

		// Down/left
		5, 8, 8, 8
	};

	// Arrays defining output button byte and bit locations in the output buffer
	// (in accordance with the report descriptor!)
	static unsigned int ButtonIndex[NR_OUT_BUTTONS]=
	{
		1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3
	};

	static unsigned int ButtonBits[NR_OUT_BUTTONS]=
	{
		0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
		0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
		0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
	};

	// Array defining locations of the axes in the output buffer
	// (in accordance with the report descriptor!)
	static unsigned int AxisIndex[NR_OUT_AXES]=
	{
		4,	//X
		6,	//Y
		8,	//Z
		10,	//RX
		12,	//RY
		14,	//RZ
		16	//Slider
	};

	static unsigned int POVBits[4]=
	{
		0x01,0x02,0x04,0x08
	};

	/* Get pointer to device extension data */
    PDEVICE_EXTENSION pDevExt=GET_MINIDRIVER_DEVICE_EXTENSION(pFdo);

	// Quote from the DDK documentation:
	// 'The RtlZeroMemory routine fills a block of memory with zeros, given a
	// pointer to the block and the length, in bytes, to be filled.'

	RtlZeroMemory(&OutData, sizeof(OutData));

	// Fill all semiaxes from input data. Since the semiaxes are mapped to
	// Windows gamecontrol items later (anyway), the sequence is chosen
	// arbitrarily, directly from the order inside the gamepad data packet.
	// NOTE: MAYBE THE OTHER HARDWIRED STUFF (I.E. THE BYTE AND BIT NUMBERS FROM
	//       THE *INPUT* BUFFER) COULD BE MOVED TO #DEFINES AS WELL !!!

	// Digital buttons first
	// Button 'D-pad up' [0]
	pDevExt->SemiAxes[0].Value=(pDevExt->hwInData[2])&0x01 ? MAX_VALUE : 0;

	// Button 'D-pad down' [1]
	pDevExt->SemiAxes[1].Value=(pDevExt->hwInData[2])&0x02 ? MAX_VALUE : 0;

	// Button 'D-pad left' [2]
	pDevExt->SemiAxes[2].Value=(pDevExt->hwInData[2])&0x04 ? MAX_VALUE : 0;

	// Button 'D-pad right' [3]
	pDevExt->SemiAxes[3].Value=(pDevExt->hwInData[2])&0x08 ? MAX_VALUE : 0;

	// Button 'Start' [4]
	pDevExt->SemiAxes[4].Value=(pDevExt->hwInData[2])&0x10 ? MAX_VALUE : 0;

	// Button 'Back' [5]
	pDevExt->SemiAxes[5].Value=(pDevExt->hwInData[2])&0x20 ? MAX_VALUE : 0;

	// Button 'Left stick press' [6]
	pDevExt->SemiAxes[6].Value=(pDevExt->hwInData[2])&0x40 ? MAX_VALUE : 0;

	// Button 'Right stick press' [7]
	pDevExt->SemiAxes[7].Value=(pDevExt->hwInData[2])&0x80 ? MAX_VALUE : 0;

	// Now the analogue buttons
	// Button 'A' [8]
	pDevExt->SemiAxes[8].Value = MAX_VALUE * pDevExt->hwInData[4]/255;

	// Button 'B' [9]
	pDevExt->SemiAxes[9].Value = MAX_VALUE * pDevExt->hwInData[5]/255;

	// Button 'X' [10]
	pDevExt->SemiAxes[10].Value = MAX_VALUE * pDevExt->hwInData[6]/255;

	// Button 'Y' [11]
	pDevExt->SemiAxes[11].Value = MAX_VALUE * pDevExt->hwInData[7]/255;

	// Button 'Black' [12]
	pDevExt->SemiAxes[12].Value = MAX_VALUE * pDevExt->hwInData[8]/255;

	// Button 'White' [13]
	pDevExt->SemiAxes[13].Value = MAX_VALUE * pDevExt->hwInData[9]/255;

	// Button 'Trigger left' [14]
	pDevExt->SemiAxes[14].Value = MAX_VALUE * pDevExt->hwInData[10]/255;

	// Button 'Trigger right' [15]
	pDevExt->SemiAxes[15].Value = MAX_VALUE * pDevExt->hwInData[11]/255;

	//And finally the analogue sticks (4 semiaxes each). Each negative semiaxis
	//is inverted to positive!
	//
	// Left stick horizontal (=> 2 semiaxes, negative first) [16], [17]
	SignedInt16=(pDevExt->hwInData[13])&0xff;
	SignedInt16<<=8;
	SignedInt16|=(pDevExt->hwInData[12])&0xff;
	StickX = SignedInt16 < 0? (MAX_VALUE * SignedInt16/32768) : (MAX_VALUE * SignedInt16/32767);

	// Left stick vertical (=> 2 semiaxes, negative first) [18], [19]
	SignedInt16=(pDevExt->hwInData[15])&0xff;
	SignedInt16<<=8;
	SignedInt16|=(pDevExt->hwInData[14])&0xff;
	StickY = SignedInt16 < 0? (MAX_VALUE * SignedInt16/32768) : (MAX_VALUE * SignedInt16/32767);

	applyDeadzone2D(&StickX, &StickY, pDevExt->LStickDZ, pDevExt->bFullRange[0]);

	pDevExt->SemiAxes[16].Value=StickX<0 ? -StickX : 0;
	pDevExt->SemiAxes[17].Value=StickX>=0 ? StickX : 0;
	pDevExt->SemiAxes[18].Value=StickY<0 ? -StickY : 0;
	pDevExt->SemiAxes[19].Value=StickY>=0 ? StickY : 0;

	// Right stick horizontal (=> 2 semiaxes, negative first) [20], [21]
	SignedInt16=(pDevExt->hwInData[17])&0xff;
	SignedInt16<<=8;
	SignedInt16|=(pDevExt->hwInData[16])&0xff;
	StickX = SignedInt16 < 0? (MAX_VALUE * SignedInt16/32768) : (MAX_VALUE * SignedInt16/32767);

	// Right stick vertical (=> 2 semiaxes, negative first) [22], [23]
	SignedInt16=(pDevExt->hwInData[19])&0xff;
	SignedInt16<<=8;
	SignedInt16|=(pDevExt->hwInData[18])&0xff;
	StickY = SignedInt16 < 0? (MAX_VALUE * SignedInt16/32768) : (MAX_VALUE * SignedInt16/32767);

	applyDeadzone2D(&StickX, &StickY, pDevExt->RStickDZ, pDevExt->bFullRange[1]);

	pDevExt->SemiAxes[20].Value=StickX<0 ? -StickX : 0;
	pDevExt->SemiAxes[21].Value=StickX>=0 ? StickX : 0;
	pDevExt->SemiAxes[22].Value=StickY<0 ? -StickY : 0;
	pDevExt->SemiAxes[23].Value=StickY>=0 ? StickY : 0;

	//Copy the original data from the Xbox controller to the end of the buffer
	RtlCopyMemory(&OutData[20], pDevExt->hwInData, sizeof(pDevExt->hwInData));

	// Now make sure, no SemiAxes[].Value is bigger than 1.0f ! (This actually
	// can happen through numerical inaccuracies)
	for(index=0; index!=NR_SEMIAXES; index++)
		if(pDevExt->SemiAxes[index].Value>MAX_VALUE) pDevExt->SemiAxes[index].Value=MAX_VALUE;

	// If both analog control sticks are PRESSED, cycle layouts.
	if((pDevExt->SemiAxes[6].Value > 0) && (pDevExt->SemiAxes[7].Value > 0))
	{
		// Only switch layout, if it has NOT been done the last time
		if(!pDevExt->LayoutSwitch)
		{
			// DISALLOW layout switching next time
			pDevExt->LayoutSwitch=TRUE;

			pDevExt->LayoutNr++;
			if(pDevExt->LayoutNr>=pDevExt->NrOfLayouts) pDevExt->LayoutNr=0;
		}
	}
	else
	{
		// ALLOW layout switching next time
		pDevExt->LayoutSwitch=FALSE;
	}

	OutData[19] = pDevExt->LayoutNr + 1;

	/*!
	Generate output data (i.e. fill the driver's data structure described by
	the report descriptor)
	Remember, OutData is zeroed above, so it doesn't need to be done here
	(good for the buttons' OR operations).
	This part, along with the mapping matrix has to contain
	this driver's 'magic'...
	*/

	SignedInt16=0;

	for(index=0; index!=NR_OUT_AXES; index++)
	{
		PreOutAxes[index] = 0;
	}

	for(index=0; index!=24; index++)
	{
		if(pDevExt->MapMatrix[pDevExt->LayoutNr][index] == 0)
		{
			//Not assigned to anything
		}

		if((pDevExt->MapMatrix[pDevExt->LayoutNr][index] >= 1) && (pDevExt->MapMatrix[pDevExt->LayoutNr][index] <= 24))
		{
			//It's a button

			if((index >= 8) && (index <= 13))
			{
				if(pDevExt->SemiAxes[index].Value < pDevExt->BThreshold)
					pDevExt->SemiAxes[index].Value=0;
			}
			else
			{
				if((index==14) || (index==15))
				{
					if(pDevExt->SemiAxes[index].Value < pDevExt->TThreshold)
						pDevExt->SemiAxes[index].Value=0;
				}
				else
				{
					if((index >= 16) && (index <= 23))
					{
						if(pDevExt->SemiAxes[index].Value < pDevExt->AThreshold)
							pDevExt->SemiAxes[index].Value=0;
					}
				}
			}

			if(pDevExt->SemiAxes[index].Value > 0)
				OutData[ButtonIndex[pDevExt->MapMatrix[pDevExt->LayoutNr][index]-1]] |= ButtonBits[pDevExt->MapMatrix[pDevExt->LayoutNr][index]-1];
		}

		if((pDevExt->MapMatrix[pDevExt->LayoutNr][index] >= 25) && (pDevExt->MapMatrix[pDevExt->LayoutNr][index] <= 38))
		{
			//It's part of an axis

			int AxIndex;
			int AxIndex2 = pDevExt->MapMatrix[pDevExt->LayoutNr][index]-25;

			AxIndex = AxIndex2/2;

			if((AxIndex*2) == AxIndex2)
			{
				//Negative side of axis

				if(PreOutAxes[AxIndex] < 0)
				{
					if(-pDevExt->SemiAxes[index].Value < PreOutAxes[AxIndex])
						PreOutAxes[AxIndex] = -pDevExt->SemiAxes[index].Value;
				}
				else
					PreOutAxes[AxIndex] -= pDevExt->SemiAxes[index].Value;
			}
			else
			{
				//Positive side of axis

				if(PreOutAxes[AxIndex] >= 0)
				{
					if(pDevExt->SemiAxes[index].Value > PreOutAxes[AxIndex])
						PreOutAxes[AxIndex] = pDevExt->SemiAxes[index].Value;
				}
				else
					PreOutAxes[AxIndex] += pDevExt->SemiAxes[index].Value;
			}
		}

		if((pDevExt->MapMatrix[pDevExt->LayoutNr][index] >= 39) && (pDevExt->MapMatrix[pDevExt->LayoutNr][index] <= 42))
		{
			//It's a POV direction

			int POVIndex = pDevExt->MapMatrix[pDevExt->LayoutNr][index]-39;

			if((index >= 8) && (index <= 13))
			{
				if(pDevExt->SemiAxes[index].Value < pDevExt->BThreshold)
					pDevExt->SemiAxes[index].Value=0;
			}
			else
			{
				if((index==14) || (index==15))
				{
					if(pDevExt->SemiAxes[index].Value < pDevExt->TThreshold)
						pDevExt->SemiAxes[index].Value=0;
				}
				else
				{
					if((index >= 16) && (index <= 23))
					{
						if(pDevExt->SemiAxes[index].Value < pDevExt->AThreshold)
							pDevExt->SemiAxes[index].Value=0;
					}
				}
			}

			// Build truth table index from semiaxes
			if(pDevExt->SemiAxes[index].Value > 0)
				SignedInt16 |= POVBits[POVIndex];
		}
	}

	// Output POV (using the truth table defined above...)
	OutData[OUT_POV1_INDEX] = OutPovValue[SignedInt16];

	// Write to output axes (converted to integers with proper range!)
	for(index=0; index!=NR_OUT_AXES; index++)
	{
		//Rescale for less sensitive axes
		PreOutAxes[index] = PreOutAxes[index] * pDevExt->AxesScale[index]/MAX_VALUE;
		SignedInt16=(__int16)(OUT_AXIS_SCALE * PreOutAxes[index]/MAX_VALUE);
		OutData[AxisIndex[index]]=(UCHAR)(SignedInt16&0xff);
		OutData[AxisIndex[index]+1]=(UCHAR)((SignedInt16>>8)&0xff);
	}

	OutData[0] = 1; //Report ID
	/*! Debug output */
	KdPrint(("XBCDReadData - Win: %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x",
		OutData[1], OutData[2], OutData[3], OutData[4], OutData[5], OutData[6], OutData[7],
		OutData[8], OutData[9], OutData[10], OutData[11], OutData[12], OutData[13],
		OutData[14], OutData[15], OutData[16], OutData[17], OutData[18]));
	KdPrint(("XBCDReadData - Xbox: %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x",
		OutData[20], OutData[21], OutData[22], OutData[23], OutData[24], OutData[25], OutData[26],
		OutData[27], OutData[28], OutData[29], OutData[30], OutData[31], OutData[32], OutData[33],
		OutData[34], OutData[35], OutData[36], OutData[37], OutData[38], OutData[39]));

	RtlCopyMemory(pIrp->UserBuffer, OutData, sizeof(OutData));
	pIrp->IoStatus.Information=sizeof(OutData);
	pIrp->IoStatus.Status=STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	
	return STATUS_SUCCESS;
}

VOID timerDPCProc(IN PKDPC Dpc, IN PDEVICE_EXTENSION pDevExt, IN PVOID SystemArgument1, IN PVOID SystemArgument2)
{
	pDevExt->timerEnabled = FALSE;
}

#pragma LOCKEDCODE
