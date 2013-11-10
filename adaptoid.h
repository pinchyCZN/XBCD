0x05, 0x01,         /*  Usage Page (Desktop),               */
0x09, 0x02,         /*  Usage (Mouse),                      */
0xA1, 0x01,         /*  Collection (Application),           */
0x09, 0x01,         /*      Usage (Pointer),                */
0xA1, 0x00,         /*      Collection (Physical),          */
0x85, 0x03,         /*          Report ID (3),              */
0x05, 0x09,         /*          Usage Page (Button),        */
0x19, 0x01,         /*          Usage Minimum (01h),        */
0x29, 0x03,         /*          Usage Maximum (03h),        */
0x15, 0x00,         /*          Logical Minimum (0),        */
0x25, 0x01,         /*          Logical Maximum (1),        */
0x75, 0x01,         /*          Report Size (1),            */
0x95, 0x03,         /*          Report Count (3),           */
0x81, 0x02,         /*          Input (Variable),           */
0x75, 0x05,         /*          Report Size (5),            */
0x95, 0x01,         /*          Report Count (1),           */
0x81, 0x01,         /*          Input (Constant),           */
0x05, 0x01,         /*          Usage Page (Desktop),       */
0x09, 0x30,         /*          Usage (X),                  */
0x09, 0x31,         /*          Usage (Y),                  */
0x09, 0x38,         /*          Usage (Wheel),              */
0x15, 0x81,         /*          Logical Minimum (-127),     */
0x25, 0x7F,         /*          Logical Maximum (127),      */
0x75, 0x08,         /*          Report Size (8),            */
0x95, 0x03,         /*          Report Count (3),           */
0x81, 0x06,         /*          Input (Variable, Relative), */
0xC0,               /*      End Collection,                 */
0xC0,               /*  End Collection,                     */
0x05, 0x01,         /*  Usage Page (Desktop),               */
0x09, 0x06,         /*  Usage (Keyboard),                   */
0xA1, 0x01,         /*  Collection (Application),           */
0x85, 0x02,         /*      Report ID (2),                  */
0x05, 0x07,         /*      Usage Page (Keyboard),          */
0x19, 0xE0,         /*      Usage Minimum (KB Leftcontrol), */
0x29, 0xE7,         /*      Usage Maximum (KB Right GUI),   */
0x15, 0x00,         /*      Logical Minimum (0),            */
0x25, 0x01,         /*      Logical Maximum (1),            */
0x75, 0x01,         /*      Report Size (1),                */
0x95, 0x08,         /*      Report Count (8),               */
0x81, 0x02,         /*      Input (Variable),               */
0x95, 0x01,         /*      Report Count (1),               */
0x75, 0x08,         /*      Report Size (8),                */
0x81, 0x01,         /*      Input (Constant),               */
0x95, 0x05,         /*      Report Count (5),               */
0x75, 0x01,         /*      Report Size (1),                */
0x05, 0x08,         /*      Usage Page (LED),               */
0x19, 0x01,         /*      Usage Minimum (01h),            */
0x29, 0x05,         /*      Usage Maximum (05h),            */
0x91, 0x02,         /*      Output (Variable),              */
0x95, 0x01,         /*      Report Count (1),               */
0x75, 0x03,         /*      Report Size (3),                */
0x91, 0x01,         /*      Output (Constant),              */
0x95, 0x0A,         /*      Report Count (10),              */
0x75, 0x08,         /*      Report Size (8),                */
0x05, 0x07,         /*      Usage Page (Keyboard),          */
0x19, 0x00,         /*      Usage Minimum (None),           */
0x2A, 0xA5, 0x00,   /*      Usage Maximum (A5h),            */
0x15, 0x00,         /*      Logical Minimum (0),            */
0x26, 0xA5, 0x00,   /*      Logical Maximum (165),          */
0x81, 0x00,         /*      Input,                          */
0xC0,               /*  End Collection,                     */
0x05, 0x01,         /*  Usage Page (Desktop),               */
0x09, 0x04,         /*  Usage (Joystik),                    */
0xA1, 0x01,         /*  Collection (Application),           */
0x09, 0x01,         /*      Usage (Pointer),                */
0xA1, 0x00,         /*      Collection (Physical),          */
0x85, 0x01,         /*          Report ID (1),              */
0x05, 0x01,         /*          Usage Page (Desktop),       */
0x09, 0x30,         /*          Usage (X),                  */
0x09, 0x31,         /*          Usage (Y),                  */
0x16, 0x50, 0xFB,   /*          Logical Minimum (-1200),    */
0x26, 0xB0, 0x04,   /*          Logical Maximum (1200),     */
0x36, 0x00, 0x00,   /*          Physical Minimum (0),       */
0x46, 0x60, 0x09,   /*          Physical Maximum (2400),    */
0x75, 0x0C,         /*          Report Size (12),           */
0x95, 0x02,         /*          Report Count (2),           */
0x81, 0x02,         /*          Input (Variable),           */
0xC0,               /*      End Collection,                 */
0x05, 0x09,         /*      Usage Page (Button),            */
0x19, 0x01,         /*      Usage Minimum (01h),            */
0x29, 0x0E,         /*      Usage Maximum (0Eh),            */
0x15, 0x00,         /*      Logical Minimum (0),            */
0x25, 0x01,         /*      Logical Maximum (1),            */
0x35, 0x00,         /*      Physical Minimum (0),           */
0x45, 0x01,         /*      Physical Maximum (1),           */
0x75, 0x01,         /*      Report Size (1),                */
0x95, 0x0E,         /*      Report Count (14),              */
0x81, 0x02,         /*      Input (Variable),               */
0x95, 0x02,         /*      Report Count (2),               */
0x75, 0x01,         /*      Report Size (1),                */
0x81, 0x01,         /*      Input (Constant),               */
0xC0                /*  End Collection                      */
