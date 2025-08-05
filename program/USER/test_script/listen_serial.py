import serial

def listen_serial(com_port):
    ser = serial.Serial(com_port, 115200, timeout=1)
    print(f"开始监听串口 {com_port}，按Ctrl+C退出...")
    try:
        while True:
            if ser.in_waiting:
                data = ser.read_all().decode()
                print(f"收到数据：{data}")
    except KeyboardInterrupt:
        ser.close()
        print("监听结束")

if __name__ == "__main__":
    listen_serial("COM3")  # 替换为实际COM口