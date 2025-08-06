# 项目编译配置指南

## 问题解决

### 原始错误
```
..\OBJ\Template.axf: Error: L6218E: Undefined symbol current_subnet_mask (referred from usmart_network.o).
```

### 解决方案
创建了独立的网络配置文件来提供缺失的符号定义。

## 需要添加到项目的文件

### 1. 核心文件
- `usmart_network.h` - 网络检测头文件
- `usmart_network.c` - ARP冲突检测实现
- `network_config.h` - 网络配置头文件  
- `network_config.c` - 网络配置实现

### 2. 修改的文件
- `usmart.c` - 已集成IP冲突检测功能

## Keil uVision项目配置

### 在Project窗口中添加文件：

1. **展开项目树**
2. **右键点击 "Source Group 1" 或适当的组**
3. **选择 "Add Files to Group..."**
4. **添加以下文件：**
   ```
   usmart_network.c
   network_config.c
   ```

### 包含路径设置：

1. **项目属性 -> C/C++ -> Include Paths**
2. **添加路径：**
   ```
   ..\USER
   ```

## 默认配置

### 网络配置默认值：
```c
// 子网掩码: 255.255.255.0 (/24)
u32 current_subnet_mask = 0xFFFFFF00;

// 默认MAC地址: 02:00:00:00:00:01
u8 device_mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
```

### 自定义配置：
```c
// 设置子网掩码为 255.255.0.0 (/16)
usmart_set_subnet_mask(0xFFFF0000);

// 设置自定义MAC地址
u8 my_mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
usmart_set_device_mac(my_mac);
```

## 使用示例

### 在main.c或初始化函数中：
```c
#include "network_config.h"

void system_init(void)
{
    // 其他初始化代码...
    
    // 可选：设置自定义网络配置
    usmart_set_subnet_mask(0xFFFFFF00);  // 255.255.255.0
    
    // 初始化USMART（会自动初始化网络检测）
    usmart_init(72);  // 72MHz系统时钟
}
```

### IP冲突检测将自动工作：
```
setip 192.168.1.100    # 会自动检测冲突
```

## 编译验证

确保以下文件都被包含在编译中：
- [x] `usmart.c`
- [x] `usmart_network.c` 
- [x] `network_config.c`
- [x] 相应的头文件在包含路径中

## 可选优化

### 如果您的项目已有网络模块：
1. 删除 `network_config.c`
2. 在您的网络模块中定义：
   ```c
   u32 current_subnet_mask;
   u8 device_mac[6];
   u32 current_device_ip;  // 如果还没有的话
   ```

### 实际硬件集成：
- 实现 `send_arp_request()` 中的真实以太网发送
- 实现 `check_arp_response()` 中的真实定时器
- 在网络中断中调用 `process_arp_packet()`

项目现在应该能够成功编译和链接！
