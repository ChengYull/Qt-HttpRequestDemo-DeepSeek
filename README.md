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

