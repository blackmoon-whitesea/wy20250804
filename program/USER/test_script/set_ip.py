import serial
import time

def set_device_ip(com_port, new_ip, subnet, gateway):
    # 初始化串口
    ser = serial.Serial(
        port=com_port,
        baudrate=115200,
        timeout=2
    )
    if not ser.is_open:
        return "串口打开失败"
    
    # 发送IP配置指令（与开发板协议匹配）
    cmd = f"AT+IPCFG={new_ip},{subnet},{gateway}\r\n"
    ser.write(cmd.encode())
    time.sleep(0.5)  # 等待响应
    
    # 读取响应
    response = ser.read_all().decode().strip()
    ser.close()
    return f"配置结果：{response}"

# 调用示例（需替换为实际COM口和IP）
if __name__ == "__main__":
    result = set_device_ip(
        com_port="COM3",
        new_ip="192.168.1.100",
        subnet="255.255.255.0",
        gateway="192.168.1.1"
    )
    print(result)