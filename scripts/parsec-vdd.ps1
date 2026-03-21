param (
    [Parameter(Mandatory = $true)]
    [int]$DisplayCount
)

Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
using System.Threading;

public class ParsecVdd {
    const uint GENERIC_READ = 0x80000000;
    const uint GENERIC_WRITE = 0x40000000;
    const uint FILE_SHARE_READ = 0x00000001;
    const uint FILE_SHARE_WRITE = 0x00000002;
    const uint OPEN_EXISTING = 3;
    const uint FILE_FLAG_NO_BUFFERING = 0x20000000;
    const uint FILE_FLAG_OVERLAPPED = 0x40000000;
    const uint FILE_FLAG_WRITE_THROUGH = 0x80000000;
    const uint DIGCF_PRESENT = 0x00000002;
    const uint DIGCF_DEVICEINTERFACE = 0x00000010;
    const uint IOCTL_ADD = 0x0022e004;
    const uint IOCTL_UPDATE = 0x0022a00c;
    static readonly Guid VDD_ADAPTER_GUID = new Guid("00b41627-04c4-429e-a26e-0265cf50c8fa");

    [StructLayout(LayoutKind.Sequential)]
    struct SP_DEVICE_INTERFACE_DATA {
        public int cbSize;
        public Guid interfaceClassGuid;
        public uint flags;
        public IntPtr reserved;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    struct SP_DEVICE_INTERFACE_DETAIL_DATA_A {
        public int cbSize;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string DevicePath;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct OVERLAPPED {
        public IntPtr Internal;
        public IntPtr InternalHigh;
        public IntPtr Pointer;
        public IntPtr hEvent;
    }

    [DllImport("setupapi.dll", SetLastError = true)]
    static extern IntPtr SetupDiGetClassDevsA(
        ref Guid ClassGuid, IntPtr Enumerator, IntPtr hwndParent, uint Flags);

    [DllImport("setupapi.dll", SetLastError = true)]
    static extern bool SetupDiEnumDeviceInterfaces(
        IntPtr DeviceInfoSet, IntPtr DeviceInfoData,
        ref Guid InterfaceClassGuid, uint MemberIndex,
        ref SP_DEVICE_INTERFACE_DATA DeviceInterfaceData);

    [DllImport("setupapi.dll", SetLastError = true, CharSet = CharSet.Ansi)]
    static extern bool SetupDiGetDeviceInterfaceDetailA(
        IntPtr DeviceInfoSet, ref SP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
        ref SP_DEVICE_INTERFACE_DETAIL_DATA_A DeviceInterfaceDetailData,
        uint DeviceInterfaceDetailDataSize, out uint RequiredSize, IntPtr DeviceInfoData);

    [DllImport("setupapi.dll")]
    static extern bool SetupDiDestroyDeviceInfoList(IntPtr DeviceInfoSet);

    [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Ansi)]
    static extern IntPtr CreateFileA(
        string lpFileName, uint dwDesiredAccess, uint dwShareMode,
        IntPtr lpSecurityAttributes, uint dwCreationDisposition,
        uint dwFlagsAndAttributes, IntPtr hTemplateFile);

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern IntPtr CreateEvent(
        IntPtr lpEventAttributes, bool bManualReset,
        bool bInitialState, string lpName);

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern bool DeviceIoControl(
        IntPtr hDevice, uint dwIoControlCode,
        byte[] lpInBuffer, int nInBufferSize,
        out int lpOutBuffer, int nOutBufferSize,
        IntPtr lpBytesReturned, ref OVERLAPPED lpOverlapped);

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern bool GetOverlappedResultEx(
        IntPtr hFile, ref OVERLAPPED lpOverlapped,
        out uint lpNumberOfBytesTransferred,
        int dwMilliseconds, bool bAlertable);

    [DllImport("kernel32.dll")]
    static extern bool CloseHandle(IntPtr hObject);

    public static IntPtr OpenHandle() {
        var guid = VDD_ADAPTER_GUID;
        var devInfo = SetupDiGetClassDevsA(
            ref guid, IntPtr.Zero, IntPtr.Zero, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        if (devInfo == new IntPtr(-1)) return new IntPtr(-1);

        var ifaceData = new SP_DEVICE_INTERFACE_DATA();
        ifaceData.cbSize = Marshal.SizeOf(ifaceData);

        for (uint i = 0; SetupDiEnumDeviceInterfaces(
                devInfo, IntPtr.Zero, ref guid, i, ref ifaceData); i++) {
            var detail = new SP_DEVICE_INTERFACE_DETAIL_DATA_A();
            detail.cbSize = IntPtr.Size == 8 ? 8 : 6;
            uint needed;
            SetupDiGetDeviceInterfaceDetailA(
                devInfo, ref ifaceData, ref detail,
                (uint)Marshal.SizeOf(detail), out needed, IntPtr.Zero);
            var handle = CreateFileA(
                detail.DevicePath,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                IntPtr.Zero, OPEN_EXISTING,
                FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED | FILE_FLAG_WRITE_THROUGH,
                IntPtr.Zero);
            if (handle != IntPtr.Zero && handle != new IntPtr(-1)) {
                SetupDiDestroyDeviceInfoList(devInfo);
                return handle;
            }
        }

        SetupDiDestroyDeviceInfoList(devInfo);
        return new IntPtr(-1);
    }

    static int IoControl(IntPtr vdd, uint code, byte[] input) {
        var inBuf = new byte[32];
        if (input != null) Array.Copy(input, inBuf, Math.Min(input.Length, inBuf.Length));
        var ov = new OVERLAPPED();
        ov.hEvent = CreateEvent(IntPtr.Zero, true, false, null);
        int outBuf = 0;
        DeviceIoControl(vdd, code, inBuf, inBuf.Length, out outBuf, 4, IntPtr.Zero, ref ov);
        uint transferred;
        GetOverlappedResultEx(vdd, ref ov, out transferred, 5000, false);
        if (ov.hEvent != IntPtr.Zero) CloseHandle(ov.hEvent);
        return outBuf;
    }

    public static void Update(IntPtr vdd) { IoControl(vdd, IOCTL_UPDATE, null); }

    public static int AddDisplay(IntPtr vdd) {
        int idx = IoControl(vdd, IOCTL_ADD, null);
        Update(vdd);
        return idx;
    }

    public static void Keepalive(IntPtr vdd) {
        while (true) { Update(vdd); Thread.Sleep(100); }
    }

    const string VDD_DISPLAY_NAME = "ParsecVDA";

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    struct DISPLAY_DEVICE {
        public int cb;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string DeviceName;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string DeviceString;
        public uint StateFlags;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string DeviceID;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string DeviceKey;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    struct DEVMODE {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string dmDeviceName;
        public ushort dmSpecVersion;
        public ushort dmDriverVersion;
        public ushort dmSize;
        public ushort dmDriverExtra;
        public uint dmFields;
        public int dmPositionX;
        public int dmPositionY;
        public uint dmDisplayOrientation;
        public uint dmDisplayFixedOutput;
        public short dmColor;
        public short dmDuplex;
        public short dmYResolution;
        public short dmTTOption;
        public short dmCollate;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string dmFormName;
        public ushort dmLogPixels;
        public uint dmBitsPerPel;
        public uint dmPelsWidth;
        public uint dmPelsHeight;
        public uint dmDisplayFlags;
        public uint dmDisplayFrequency;
        public uint dmICMMethod;
        public uint dmICMIntent;
        public uint dmMediaType;
        public uint dmDitherType;
        public uint dmReserved1;
        public uint dmReserved2;
        public uint dmPanningWidth;
        public uint dmPanningHeight;
    }

    [DllImport("user32.dll", CharSet = CharSet.Ansi)]
    static extern bool EnumDisplayDevicesA(string lpDevice, uint iDevNum, ref DISPLAY_DEVICE lpDisplayDevice, uint dwFlags);

    [DllImport("user32.dll", CharSet = CharSet.Ansi)]
    static extern int ChangeDisplaySettingsExA(string lpszDeviceName, ref DEVMODE lpDevMode, IntPtr hwnd, uint dwflags, IntPtr lParam);

    const uint CDS_UPDATEREGISTRY = 0x00000001;
    const uint CDS_NORESET = 0x10000000;
    const uint DM_PELSWIDTH = 0x00080000;
    const uint DM_PELSHEIGHT = 0x00100000;
    const uint DM_DISPLAYFREQUENCY = 0x00400000;

    public static string[] GetParsecDisplayNames() {
        var names = new System.Collections.Generic.List<string>();
        var adapter = new DISPLAY_DEVICE();
        adapter.cb = Marshal.SizeOf(adapter);
        for (uint i = 0; EnumDisplayDevicesA(null, i, ref adapter, 0); i++) {
            var monitor = new DISPLAY_DEVICE();
            monitor.cb = Marshal.SizeOf(monitor);
            if (EnumDisplayDevicesA(adapter.DeviceName, 0, ref monitor, 0)) {
                if (monitor.DeviceString.IndexOf(VDD_DISPLAY_NAME, StringComparison.OrdinalIgnoreCase) >= 0) {
                    names.Add(adapter.DeviceName);
                }
            }
        }
        return names.ToArray();
    }

    public static bool SetResolution(string deviceName, uint width, uint height, uint hz) {
        var dm = new DEVMODE();
        dm.dmSize = (ushort)Marshal.SizeOf(dm);
        dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
        dm.dmPelsWidth = width;
        dm.dmPelsHeight = height;
        dm.dmDisplayFrequency = hz;
        int result = ChangeDisplaySettingsExA(deviceName, ref dm, IntPtr.Zero, CDS_UPDATEREGISTRY | CDS_NORESET, IntPtr.Zero);
        return result == 0;
    }
}
"@

$vdd = [ParsecVdd]::OpenHandle()
if ($vdd -eq [IntPtr]::new(-1)) {
    Write-Error "Failed to open the Parsec VDD device handle."
    exit 1
}

for ($i = 1; $i -le $DisplayCount; $i++) {
    $idx = [ParsecVdd]::AddDisplay($vdd)
    Write-Information "Added virtual display at index $idx"
}

# wait for Windows to register the new displays
Start-Sleep -Seconds 2

# assign distinct resolutions so Windows can distinguish the otherwise-identical
# virtual displays, which is required for topology operations to work correctly
$resolutions = @(
    @(1920, 1080, 60),
    @(1280, 720, 60),
    @(1600, 900, 60)
)
$parsecDisplays = [ParsecVdd]::GetParsecDisplayNames()
Write-Information "Found $($parsecDisplays.Count) Parsec virtual display(s)"
for ($i = 0; $i -lt $parsecDisplays.Count; $i++) {
    $res = $resolutions[$i % $resolutions.Count]
    $ok = [ParsecVdd]::SetResolution($parsecDisplays[$i], $res[0], $res[1], $res[2])
    Write-Information "Set $($parsecDisplays[$i]) to $($res[0])x$($res[1])@$($res[2])Hz: $ok"
}

Write-Information "Keeping $DisplayCount virtual display(s) alive..."
[ParsecVdd]::Keepalive($vdd)
