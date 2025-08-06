# USMART IP冲突检测功能说明

## 概述
USMART现在支持在设置IP地址时自动检测网络上是否已存在相同的IP地址，避免IP冲突。

## 功能特性

### 1. ARP检测机制
- 使用ARP (Address Resolution Protocol) 协议检测IP冲突
- 发送ARP请求到目标IP地址
- 如果收到回复，说明该IP已被其他设备使用

### 2. 多重验证
- **IP格式验证**：检查IP地址格式是否正确
- **范围验证**：验证IP是否在有效范围内
- **网段验证**：确保IP在当前子网内
- **冲突检测**：通过ARP协议检测是否已被使用

### 3. 重试机制
- 默认进行3次ARP检测尝试
- 每次尝试间隔50ms
- 单次检测超时时间200ms
- 提高检测可靠性

## 使用方法

### 命令格式
```
setip <IP地址>
```

### 使用示例
```
setip 192.168.1.100    # 设置IP为192.168.1.100
```

### 输出示例

#### 成功情况：
```
Validating IP: 192.168.1.100 (0xC0A80164)
Starting IP conflict detection...
Enhanced IP conflict detection for 192.168.1.100
ARP attempt 1/3
Sending ARP request to 192.168.1.100...
Waiting for ARP response (timeout: 200ms)...
ARP timeout - no response received.
No conflict detected after 3 attempts
IP conflict detection: IP available
IP conflict check passed. Setting new IP...
IP successfully set: 192.168.1.100 (0xC0A80164)
Set IP Address Success!
```

#### 冲突检测到：
```
Validating IP: 192.168.1.50 (0xC0A80132)
Starting IP conflict detection...
Enhanced IP conflict detection for 192.168.1.50
ARP attempt 1/3
Sending ARP request to 192.168.1.50...
Waiting for ARP response (timeout: 200ms)...
ARP response received! IP is in use.
IP conflict confirmed after 1 attempts
IP conflict detection: IP already in use
Error: IP address already in use on network!
Cannot set IP 192.168.1.50 - conflict detected.
Set IP Address Error!
```

## 错误类型

### 1. 格式错误
- `Invalid IP: Part XXX out of range (0-255)` - IP段超出范围
- `Invalid character in IP address: 'X'` - 包含非法字符
- `Invalid IP format: Expected 4 parts, got X` - IP格式不正确

### 2. 范围错误
- `Invalid IP: First octet XXX not allowed` - 第一段不允许（0, ≥224）
- `Invalid IP: Host part cannot be 0 or 255` - 主机部分不能为0或255

### 3. 网络错误
- `Target IP not in current subnet!` - 不在当前子网
- `IP address already in use on network!` - IP已被使用

## 配置选项

### 文件：usmart_network.h
```c
#define ARP_TIMEOUT_MS          200     // ARP响应超时时间(毫秒)
#define ARP_RETRY_COUNT         3       // 重试次数
#define ARP_REQUEST_INTERVAL    50      // 请求间隔(毫秒)
#define USMART_ENABLE_ARP_DETECTION  1  // 启用/禁用ARP检测
```

### 禁用ARP检测
如果需要禁用ARP检测功能，设置：
```c
#define USMART_ENABLE_ARP_DETECTION  0
```

## 技术实现

### 核心函数
1. **usmart_detect_ip_conflict()** - 主检测函数
2. **usmart_detect_ip_conflict_enhanced()** - 增强检测逻辑
3. **send_arp_request()** - 发送ARP请求
4. **check_arp_response()** - 检查ARP回复
5. **process_arp_packet()** - 处理接收到的ARP包

### ARP包结构
```c
typedef struct {
    u16 hw_type;        // 硬件类型(以太网=1)
    u16 proto_type;     // 协议类型(IP=0x0800)
    u8 hw_len;          // 硬件地址长度(MAC=6)
    u8 proto_len;       // 协议地址长度(IP=4)
    u16 operation;      // 操作(1=请求, 2=回复)
    u8 sender_mac[6];   // 发送者MAC地址
    u32 sender_ip;      // 发送者IP地址
    u8 target_mac[6];   // 目标MAC地址
    u32 target_ip;      // 目标IP地址
} arp_packet_t;
```

## 待完善功能

### 需要根据具体硬件平台实现：
1. **以太网发送函数** - 实际发送ARP包到网络
2. **定时器功能** - 精确的超时控制
3. **网络中断处理** - 接收和处理ARP回复包
4. **MAC地址获取** - 获取设备的MAC地址

### 示例集成代码：
```c
// 在你的网络中断处理函数中添加：
void ethernet_rx_handler(u8 *packet_data, u16 packet_length)
{
    // 检查是否为ARP包
    if(is_arp_packet(packet_data))
    {
        process_arp_packet(packet_data, packet_length);
    }
    // 处理其他包...
}
```

## 优势
1. **避免IP冲突** - 防止网络通信问题
2. **增强可靠性** - 多重验证确保IP设置正确
3. **用户友好** - 清晰的错误提示和状态信息
4. **可配置** - 可根据需要调整检测参数

这个功能大大提高了USMART系统的网络配置可靠性，特别适用于复杂的网络环境。
