# IP地址Flash存储功能使用指南

## 功能概述
该功能允许通过串口设置的IP地址在STM32F4芯片重启后保持不变，IP地址被保存在内部Flash存储器中。

## 新增的串口命令

### 1. setip 命令（已增强）
**功能**: 设置IP地址并自动保存到Flash
**格式**: `setip 192.168.1.100`
**说明**: 
- 设置IP地址时会自动进行冲突检测
- 设置成功后自动保存到Flash存储器
- 重启后IP地址会自动加载

### 2. saveip 命令（新增）
**功能**: 手动保存当前IP地址到Flash
**格式**: `saveip`
**说明**: 
- 将当前运行时的IP地址保存到Flash
- 用于备份当前IP设置

### 3. loadip 命令（新增）
**功能**: 从Flash加载IP地址
**格式**: `loadip`
**说明**: 
- 从Flash读取已保存的IP地址
- 立即应用到当前系统

### 4. resetip 命令（新增）
**功能**: 重置IP配置到出厂默认值
**格式**: `resetip`
**说明**: 
- 清除Flash中保存的IP配置
- 恢复到默认IP: 192.168.0.1
- 需要重启系统才能生效

## 使用流程

### 首次设置IP地址
```
> setip 192.168.1.100
Validating IP: 192.168.1.100 (0xC0A80164)
Starting IP conflict detection...
Enhanced IP conflict detection for 192.168.1.100
Target IP is in private network range - allowing...
No conflict detected after 3 attempts
IP conflict check passed. Setting new IP...
IP automatically saved to Flash for persistence.
IP successfully set: 192.168.1.100 (0xC0A80164)
Set IP Address Success!
```

### 重启后自动加载
```
USMART initialized with network detection and Flash storage capability.
Configuration loaded from Flash.
Device IP: 192.168.1.100
IP loaded from Flash: 192.168.1.100
```

### 手动保存当前IP
```
> saveip
IP Address saved to Flash successfully!
```

### 加载已保存的IP
```
> loadip
IP Address loaded from Flash: 192.168.1.100
```

### 重置到默认配置
```
> resetip
IP configuration reset successfully!
Please restart system to apply defaults.
```

## 技术实现细节

### Flash存储区域
- **使用扇区**: Sector 11 (0x080E0000 - 0x080FFFFF)
- **存储大小**: 128KB
- **数据结构**: 包含IP地址、子网掩码、MAC地址等网络配置

### 数据完整性保护
- **魔数验证**: 使用0x12345678作为有效配置标识
- **CRC32校验**: 防止数据损坏
- **版本控制**: 支持配置数据结构升级

### 配置数据结构
```c
typedef struct {
    u32 magic_number;       // 魔数验证
    u32 config_version;     // 配置版本
    u32 device_ip;          // 设备IP地址
    u32 subnet_mask;        // 子网掩码
    u32 gateway_ip;         // 网关IP（保留）
    u8  device_mac[6];      // 设备MAC地址
    u32 crc32;             // CRC32校验和
    u8  reserved[100];     // 预留空间
} flash_config_t;
```

## 重要注意事项

### 1. Flash寿命
- STM32F4 Flash典型擦写次数: 10,000次
- 正常使用情况下可使用多年
- 建议避免频繁保存操作

### 2. 断电保护
- 配置数据在完全断电后仍然保留
- 包括电池耗尽的情况

### 3. 错误恢复
- 如果Flash数据损坏，系统会自动使用默认配置
- 可使用resetip命令手动恢复默认设置

### 4. 兼容性
- 向后兼容旧版本USMART
- 如果Flash中没有配置数据，使用默认IP: 192.168.0.1

## 故障排除

### 问题1: IP地址重启后丢失
**原因**: Flash保存失败
**解决**: 
1. 使用saveip命令手动保存
2. 检查Flash是否有写保护
3. 使用resetip重置后重新设置

### 问题2: 设置IP地址失败
**原因**: Flash扇区擦除失败
**解决**:
1. 检查Flash是否被其他程序占用
2. 确认电源电压稳定
3. 重启系统后重试

### 问题3: 加载的IP地址不正确
**原因**: Flash数据损坏
**解决**:
1. 使用resetip命令清除配置
2. 重新设置IP地址
3. 检查系统稳定性

## 系统集成

### 初始化顺序
1. USMART系统初始化
2. Flash配置系统初始化
3. 网络检测系统初始化
4. 自动加载保存的IP配置

### 与现有代码的集成
- 完全兼容现有USMART功能
- IP冲突检测功能保持不变
- 增强了IP设置的持久性

## 性能影响

### Flash操作时间
- 扇区擦除: ~500ms
- 数据写入: ~1ms
- 数据读取: <1ms

### 启动时间影响
- 增加Flash读取时间: <10ms
- 对系统启动速度影响极小

这个功能确保了IP地址设置的持久性，大大提高了系统的易用性和可靠性。
