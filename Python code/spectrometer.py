# Version 1.1 18 March 2021
# Updated to check Vendo ID and Product ID to identify Arduino Nano Every rather than
# assuming it is the most recently connected device

import subprocess
import sys

class spectralSensorData:
    "Class to hold sensor data"
    def __init__(self, nm415, nm445, nm480, nm515, nm555, nm590, nm630, nm680, full, NIR):
        self.nm415 = nm415
        self.nm445 = nm445
        self.nm480 = nm480
        self.nm515 = nm515
        self.nm555 = nm555
        self.nm590 = nm590
        self.nm630 = nm630
        self.nm680 = nm680
        self.full = full
        self.NIR = NIR

def install(package):
    subprocess.check_call([sys.executable, "-m", "pip", "install", package])

try:
    import serial
except ModuleNotFoundError:
    print ('Installing pyserial...')
    install ('pyserial')
    import serial
finally:
    import serial.tools.list_ports
    import time
    from scipy.optimize import curve_fit
    ser = serial.Serial()

def setUpSerialPort():
    global ser
    setupstring = ''
    ports = serial.tools.list_ports.comports()
    for p in ports:
        if ((p.vid == 9025) and (p.pid == 88)):
            serialport = str(p.device)
            ser = serial.Serial(serialport, baudrate=115200, timeout=2)
    print ('Connected to '+serialport+'\n')
    for i in range(3):
        b = ser.readline()
        readstring = b.decode("utf-8")
        setupstring+=readstring
    print(setupstring)

def listSerialPorts():
    global ser
    ports = serial.tools.list_ports.comports()
    for p in ports:
        print(p.device)

    print (len(ports), 'ports found')

def connectToSerialPort(connectTo):
    global ser
    setupstring = ''
    ser = serial.Serial(connectTo, baudrate=115200, timeout=10)
    print ('Connected to '+connectTo+'\n')
    for i in range(3):
        b = ser.readline()
        readstring = b.decode("utf-8")
        setupstring+=readstring
    print(setupstring)

def readNextLine():
    b = ser.readline()
    readstring = b.decode("utf-8")
    print (readstring)

def setWhiteLightBrightness(brightness):
    global ser
    writestring = '<T'+str(brightness)+'>'
    bytestowrite = writestring.encode()
    ser.write(bytestowrite)
    b = ser.readline()
    readstring = b.decode("utf-8")
    print('White LED brightness set to '+readstring[29:])

def setUVLightBrightness(brightness):
    global ser
    writestring = '<F'+str(brightness)+'>'
    bytestowrite = writestring.encode()
    ser.write(bytestowrite)
    b = ser.readline()
    readstring = b.decode("utf-8")
    print('UV LED brightness set to '+readstring[29:])

def setIntegrationTime(inttime):
    global ser
    inttime = max(0, inttime)
    inttime = min(4000, inttime)
    writestring = '<E'+str(inttime)+'>'
    bytestowrite = writestring.encode()
    ser.write(bytestowrite)
    b = ser.readline()
    readstring = b.decode("utf-8")
    print('Integration time set to '+readstring[18:])

def setGain(gain):
    global ser
    if (gain<=0.5):
        gainCode = 0
    elif (gain<=1):
        gainCode = 1
    elif (gain<=2):
        gainCode = 2
    elif (gain<=4):
        gainCode = 3
    elif (gain<=8):
        gainCode = 4
    elif (gain<=16):
        gainCode = 5
    elif (gain<=32):
        gainCode = 6
    elif (gain<=64):
        gainCode = 7
    elif (gain<=128):
        gainCode = 8
    elif (gain<=256):
        gainCode = 9
    elif (gain<=512):
        gainCode = 10
    else:
        gainCode = 10

    writestring = '<G'+str(gainCode)+'>'
    bytestowrite = writestring.encode()
    ser.write(bytestowrite)
    b = ser.readline()
    readstring = b.decode("utf-8")
    print('Gain set to '+readstring[6:])

def readAll():
    global ser

    nm415 = 0
    nm445 = 0
    nm480 = 0
    nm515 = 0
    nm555 = 0
    nm590 = 0
    nm630 = 0
    nm680 = 0
    full = 0
    NIR = 0

    datastring = ''
    writestring = '<R>'
    bytestowrite = writestring.encode()
    ser.write(bytestowrite)

    b = ser.readline()
    readstring = b.decode("utf-8")
    datastring+=readstring
    nm415 = float(readstring[16:])

    b = ser.readline()
    readstring = b.decode("utf-8")
    datastring+=readstring
    nm445 = float(readstring[16:])

    b = ser.readline()
    readstring = b.decode("utf-8")
    datastring+=readstring
    nm480 = float(readstring[16:])

    b = ser.readline()
    readstring = b.decode("utf-8")
    datastring+=readstring
    nm515 = float(readstring[16:])

    b = ser.readline()
    readstring = b.decode("utf-8")
    datastring+=readstring
    nm555 = float(readstring[16:])

    b = ser.readline()
    readstring = b.decode("utf-8")
    datastring+=readstring
    nm590 = float(readstring[16:])

    b = ser.readline()
    readstring = b.decode("utf-8")
    datastring+=readstring
    nm630 = float(readstring[16:])

    b = ser.readline()
    readstring = b.decode("utf-8")
    datastring+=readstring
    nm680 = float(readstring[16:])

    b = ser.readline()
    readstring = b.decode("utf-8")
    datastring+=readstring
    full = float(readstring[16:])

    b = ser.readline()
    readstring = b.decode("utf-8")
    datastring+=readstring
    NIR = float(readstring[16:])

    #print(datastring)

    return spectralSensorData(nm415, nm445, nm480, nm515, nm555, nm590, nm630, nm680, full, NIR)

def read415():
    return readAll().nm415

def read445():
    return readAll().nm445

def read480():
    return readAll().nm480

def read515():
    return readAll().nm515

def read555():
    return readAll().nm555

def read590():
    return readAll().nm590

def read630():
    return readAll().nm630

def read680():
    return readAll().nm680

def readFull():
    return readAll().full

def readNIR():
    return readAll().NIR

def sensorDetails():
    global ser
    detailsstring = ''
    writestring = '<D>'
    bytestowrite = writestring.encode()
    ser.write(bytestowrite)
    for i in range(6):
        b = ser.readline()
        readstring = b.decode("utf-8")
        detailsstring+=readstring
    print(detailsstring)

def firmwareDetails():
    global ser
    detailsstring = ''
    writestring = '<I>'
    bytestowrite = writestring.encode()
    ser.write(bytestowrite)
    for i in range(3):
        b = ser.readline()
        readstring = b.decode("utf-8")
        detailsstring+=readstring
    print(detailsstring)

def close():
    global ser
    ser.close()
    print('Serial port closed')

setUpSerialPort()
