# Qt HTTP 请求示例 & DeepSeek API 对话工具
这是一个基于 Qt 的 HTTP 请求示例项目，实现了 GET 和 POST 请求的调用，并集成 DeepSeek API 实现对话功能。项目使用 Qt 5.15.0 开发，支持通过 HTTPS 与 API 交互。
## 功能特性
- GET 请求：基础 HTTP GET 请求示例。
- POST 请求：通过 POST 请求调用 DeepSeek API 进行对话。
- SSL 支持：基于 OpenSSL 的 HTTPS 加密通信。

## 完成了Get请求与Post请求的调用
其中Post请求实现了调用DeepSeek的API进行对话

## 注意
- `.pro`文件中需要添加`network`模块
```
QT += network
```
- 使用前需具备OpenSSL环境
    - 本人是QT5.15.0 需要的OpenSSL版本为：OpenSSL 1.1.1d  10 Sep 2019
- 自行准备对应的API-key才可调用 


## 界面
![](https://img2024.cnblogs.com/blog/2734270/202503/2734270-20250313171638305-66524931.png)

- 增加模拟天气调用
![](https://img2024.cnblogs.com/blog/2734270/202503/2734270-20250319150359388-1416870985.png)
![](https://img2024.cnblogs.com/blog/2734270/202503/2734270-20250319150422236-1616285338.png)

## 更新
- 增加流式传输功能
- 增加function call功能，由于DeepSeek不支持function call，更换模型为qwen-max
    - 关键点：当收到返回消息有tool_calls时，**历史消息增加两条**，一条角色为assistant，参数需要tool_calls列表，一条角色为tool，参数要有调用函数的请求结果
### 一个完整的请求体示例
```
{
    "messages": [
        {
            "content": "调用函数时，仅根据用户最新消息来判断，历史消息不触发函数调用",
            "role": "system"
        },
        {
            "content": "北京天气",
            "role": "user"
        },
        {
            "content": "",
            "role": "assistant",
            "tool_calls": [
                {
                    "function": {
                        "arguments": "{\"city\": \"北京\"}",
                        "name": "get_weather"
                    },
                    "id": "call_08d86f66db154ff79b6e9c",
                    "index": 0,
                    "type": "function"
                }
            ]
        },
        {
            "content": "\n{\n    \"temperature\": \"9℃\",\n    \"weather\": \"阴天\"\n}\n",
            "name": "get_weather",
            "role": "tool",
            "tool_call_id": "call_08d86f66db154ff79b6e9c"
        }
    ],
    "model": "qwen-max",
    "tools": [
        {
            "function": {
                "description": "获取城市的天气信息",
                "name": "get_weather",
                "parameters": {
                    "properties": {
                        "city": {
                            "description": "城市名",
                            "enum": [
                                "杭州",
                                "北京"
                            ],
                            "type": "string"
                        }
                    },
                    "required": [
                        "city"
                    ],
                    "type": "object"
                }
            },
            "type": "function"
        }
    ]
}
```
